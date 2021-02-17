/*

	TiMidity -- Experimental MIDI to WAVE converter
	Copyright (C) 1995 Tuukka Toivonen <toivonen@clinet.fi>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	mix.c

*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "timidity.h"

namespace Timidity
{

static int convert_envelope_rate(Renderer *song, uint8_t rate)
{
	int r;

	r  = 3 - ((rate>>6) & 0x3);
	r *= 3;
	r  = (int)(rate & 0x3f) << r; /* 6.9 fixed point */

	/* 15.15 fixed point. */
	return int(((r * 44100) / song->rate) * song->control_ratio) << 9;
}

void Envelope::Init(Renderer *song, Voice *v)
{
	Type = v->sample->type;
	env.bUpdating = true;
	if (Type == INST_GUS)
	{
		gf1.Init(song, v);
		gf1.ApplyToAmp(v);
	}
	else
	{
		sf2.Init(song, v);
		sf2.ApplyToAmp(v);
	}
}

void GF1Envelope::Init(Renderer *song, Voice *v)
{
	/* Ramp up from 0 */
	stage = 0;
	volume = 0;

	for (int i = 0; i < 6; ++i)
	{
		offset[i] = v->sample->envelope.gf1.offset[i] << (7 + 15);
		rate[i] = convert_envelope_rate(song, v->sample->envelope.gf1.rate[i]);
	}
	Recompute(v);
}

void GF1Envelope::Release(Voice *v)
{
	if (!(v->sample->modes & PATCH_NO_SRELEASE) || (v->sample->modes & PATCH_FAST_REL))
	{
		/* ramp out to minimum volume with rate from final release stage */
		stage = GF1_RELEASEC+1;
		target = 0;
		increment = -rate[GF1_RELEASEC];
	}
	else if (v->sample->modes & PATCH_SUSTAIN)
	{
		if (stage < GF1_RELEASE)
		{
			stage = GF1_RELEASE;
		}
		Recompute(v);
	}
	bUpdating = true;
}

/* Returns 1 if envelope runs out */
bool GF1Envelope::Recompute(Voice *v)
{
	int newstage;

	newstage = stage;

	if (newstage > GF1_RELEASEC)
	{
		/* Envelope ran out. */
		increment = 0;
		bUpdating = false;
		v->status &= ~(VOICE_SUSTAINING | VOICE_LPE);
		v->status |= VOICE_RELEASING;
		/* play sampled release */
		return 0;
	}

	if (newstage == GF1_RELEASE && !(v->status & VOICE_RELEASING) && (v->sample->modes & PATCH_SUSTAIN))
	{
		v->status |= VOICE_SUSTAINING;
		/* Freeze envelope until note turns off. Trumpets want this. */
		increment = 0;
		bUpdating = false;
	}
	else
	{
		stage = newstage + 1;

		if (volume == offset[newstage])
		{
			return Recompute(v);
		}
		target = offset[newstage];
		increment = rate[newstage];
		if (target < volume)
			increment = -increment;
	}

	return 0;
}

bool GF1Envelope::Update(Voice *v)
{
	volume += increment;
	if (((increment < 0) && (volume <= target)) || ((increment > 0) && (volume >= target)))
	{
		volume = target;
		if (Recompute(v))
		{
			return 1;
		}
	}
	return 0;
}

void GF1Envelope::ApplyToAmp(Voice *v)
{
	double env_vol = v->attenuation;
	double final_amp;

	final_amp = FINAL_MIX_SCALE;
	if (v->tremolo_phase_increment != 0)
	{ // [RH] FIXME: This is wrong. Tremolo should offset the
	  // envelope volume, not scale it.
		env_vol *= v->tremolo_volume;
	}
	env_vol *= volume / float(1 << 30);
	env_vol = calc_gf1_amp(env_vol);
	env_vol *= final_amp;
	v->left_mix = float(env_vol * v->left_offset);
	v->right_mix = float(env_vol * v->right_offset);
}

void SF2Envelope::Init(Renderer *song, Voice *v)
{
	stage = 0;
	volume = 0;
	DelayTime = v->sample->envelope.sf2.delay_vol;
	AttackTime = v->sample->envelope.sf2.attack_vol;
	HoldTime = v->sample->envelope.sf2.hold_vol;
	DecayTime = v->sample->envelope.sf2.decay_vol;
	SustainLevel = v->sample->envelope.sf2.sustain_vol;
	ReleaseTime = v->sample->envelope.sf2.release_vol;
	SampleRate = song->rate;
	HoldStart = 0;
	RateMul = song->control_ratio / song->rate;
	RateMul_cB = RateMul * 960;
	bUpdating = true;
}

void SF2Envelope::Release(Voice *v)
{
	if (stage == SF2_ATTACK)
	{
		// The attack stage does not use an attenuation in cB like all the rest.
		volume = float(log10(volume) * -200);
	}
	stage = SF2_RELEASE;
	bUpdating = true;
}

static double timecent_to_sec(float timecent)
{
	if (timecent == -32768)
		return 0;
	return pow(2.0, timecent / 1200.0);
}

static double calc_rate(double ratemul, double sec)
{
	if (sec < 0.006)
		sec = 0.006;
	return ratemul / sec;
}

static void shutoff_voice(Voice *v)
{
	v->status &= ~(VOICE_SUSTAINING | VOICE_LPE);
	v->status |= VOICE_RELEASING | VOICE_STOPPING;
}

static bool check_release(double RateMul, double sec)
{
	double rate = calc_rate(960 * RateMul, sec);

	// Is release rate very fast? If so, don't do the release, but do
	// the voice off ramp instead.
	return (rate < 960/20);
}

/* Returns 1 if envelope runs out */
bool SF2Envelope::Update(Voice *v)
{
	double sec;
	double newvolume = 0;

	// NOTE! The volume scale is different for different stages of the
	// envelope generator:
	// Attack stage goes from 0.0 -> 1.0, multiplied directly to the output.
	// The following stages go from 0 -> -1000 cB (but recorded positively)
	switch (stage)
	{
	case SF2_DELAY:
		if (v->sample_count >= timecent_to_sec(DelayTime) * SampleRate)
		{
			stage = SF2_ATTACK;
			return Update(v);
		}
		return 0;

	case SF2_ATTACK:
		sec = timecent_to_sec(AttackTime);
		if (sec <= 0)
		{ // instantaneous attack
			newvolume = 1;
		}
		else
		{
			newvolume = volume + calc_rate(RateMul, sec);
		}
		if (newvolume >= 1)
		{
			volume = 0;
			HoldStart = v->sample_count;
			if (HoldTime <= -32768)
			{ // hold time is 0, so skip right to decay
				stage = SF2_DECAY;
			}
			else
			{
				stage = SF2_HOLD;
			}
			return Update(v);
		}
		break;

	case SF2_HOLD:
		if (v->sample_count - HoldStart >= timecent_to_sec(HoldTime) * SampleRate)
		{
			stage = SF2_DECAY;
			return Update(v);
		}
		return 0;

	case SF2_DECAY:
		sec = timecent_to_sec(DecayTime);
		if (sec <= 0)
		{ // instantaneous decay
			newvolume = SustainLevel;
		}
		else
		{
			newvolume = volume + calc_rate(RateMul_cB, sec);
		}
		if (newvolume >= SustainLevel)
		{
			newvolume = SustainLevel;
			stage = SF2_SUSTAIN;
			bUpdating = false;
			if (!(v->status & VOICE_RELEASING))
			{
				v->status |= VOICE_SUSTAINING;
			}
		}
		break;

	case SF2_SUSTAIN:
		// Stay here until released.
		return 0;

	case SF2_RELEASE:
		sec = timecent_to_sec(ReleaseTime);
		if (sec <= 0)
		{ // instantaneous release
			newvolume = 1000;
		}
		else
		{
			newvolume = volume + calc_rate(RateMul_cB, sec);
		}
		if (newvolume >= 960)
		{
			stage = SF2_FINISHED;
			shutoff_voice(v);
			bUpdating = false;
			return 1;
		}
		break;

	case SF2_FINISHED:
		return 1;
	}
	volume = (float)newvolume;
	return 0;
}

/* EMU 8k/10k don't follow spec in regards to volume attenuation.
 * This factor is used in the equation pow (10.0, cb / FLUID_ATTEN_POWER_FACTOR).
 * By the standard this should be -200.0. */
#define FLUID_ATTEN_POWER_FACTOR  (-531.509)
#define atten2amp(x) pow(10.0, (x) / FLUID_ATTEN_POWER_FACTOR)

static double cb_to_amp(double x)	// centibels to amp
{
	return pow(10, x / -200.f);
}

void SF2Envelope::ApplyToAmp(Voice *v)
{
	double amp;

	if (stage == SF2_DELAY)
	{
		v->left_mix = 0;
		v->right_mix = 0;
		return;
	}

	amp = v->sample->type == INST_SF2 ? atten2amp(v->attenuation) : cb_to_amp(v->attenuation);

	switch (stage)
	{
	case SF2_ATTACK:
		amp *= volume;
		break;

	case SF2_HOLD:
		break;

	default:
		amp *= cb_to_amp(volume);
		break;
	}
	amp *= FINAL_MIX_SCALE * 0.5;
	v->left_mix = float(amp * v->left_offset);
	v->right_mix = float(amp * v->right_offset);
}

void apply_envelope_to_amp(Voice *v)
{
	v->eg1.ApplyToAmp(v);
}

static void update_tremolo(Voice *v)
{
	int depth = v->sample->tremolo_depth << 7;

	if (v->tremolo_sweep != 0)
	{
		/* Update sweep position */

		v->tremolo_sweep_position += v->tremolo_sweep;
		if (v->tremolo_sweep_position >= (1 << SWEEP_SHIFT))
		{
			/* Swept to max amplitude */
			v->tremolo_sweep = 0;
		}
		else
		{
			/* Need to adjust depth */
			depth *= v->tremolo_sweep_position;
			depth >>= SWEEP_SHIFT;
		}
	}

	v->tremolo_phase += v->tremolo_phase_increment;

	v->tremolo_volume = (float)
		(1.0 - FSCALENEG((sine(v->tremolo_phase >> RATE_SHIFT) + 1.0)
		* depth * TREMOLO_AMPLITUDE_TUNING,
		17));

	/* I'm not sure about the +1.0 there -- it makes tremoloed voices'
	volumes on average the lower the higher the tremolo amplitude. */
}

/* Returns 1 if the note died */
static int update_signal(Voice *v)
{
	if (v->eg1.env.bUpdating && v->eg1.Update(v))
	{
		return 1;
	}
	if (v->tremolo_phase_increment != 0)
	{
		update_tremolo(v);
	}
	apply_envelope_to_amp(v);
	return 0;
}

static void mix_mystery_signal(int32_t control_ratio, const sample_t *sp, float *lp, Voice *v, int count)
{
	final_volume_t
		left = v->left_mix,
		right = v->right_mix;
	int cc;
	sample_t s;

	if (!(cc = v->control_counter))
	{
		cc = control_ratio;
		if (update_signal(v))
			return;	/* Envelope ran out */

		left = v->left_mix;
		right = v->right_mix;
	}

	while (count)
	{
		if (cc < count)
		{
			count -= cc;
			while (cc--)
			{
				s = *sp++;
				lp[0] += left * s;
				lp[1] += right * s;
				lp += 2;
			}
			cc = control_ratio;
			if (update_signal(v))
				return;	/* Envelope ran out */
			left = v->left_mix;
			right = v->right_mix;
		}
		else
		{
			v->control_counter = cc - count;
			while (count--)
			{
				s = *sp++;
				lp[0] += left * s;
				lp[1] += right * s;
				lp += 2;
			}
			return;
		}
	}
}

static void mix_single_signal(int32_t control_ratio, const sample_t *sp, float *lp, Voice *v, float *ampat, int count)
{
	final_volume_t amp;
	int cc;

	if (0 == (cc = v->control_counter))
	{
		cc = control_ratio;
		if (update_signal(v))
			return;		/* Envelope ran out */
	}
	amp = *ampat;

	while (count)
	{
		if (cc < count)
		{
			count -= cc;
			while (cc--)
			{
				lp[0] += *sp++ * amp;
				lp += 2;
			}
			cc = control_ratio;
			if (update_signal(v))
				return;	/* Envelope ran out */
			amp = *ampat;
		}
		else
		{
			v->control_counter = cc - count;
			while (count--)
			{
				lp[0] += *sp++ * amp;
				lp += 2;
			}
			return;
		}
	}
}

static void mix_single_left_signal(int32_t control_ratio, const sample_t *sp, float *lp, Voice *v, int count)
{
	mix_single_signal(control_ratio, sp, lp, v, &v->left_mix, count);
}

static void mix_single_right_signal(int32_t control_ratio, const sample_t *sp, float *lp, Voice *v, int count)
{
	mix_single_signal(control_ratio, sp, lp + 1, v, &v->right_mix, count);
}

static void mix_mono_signal(int32_t control_ratio, const sample_t *sp, float *lp, Voice *v, int count)
{
	final_volume_t
		left = v->left_mix;
	int cc;

	if (!(cc = v->control_counter))
	{
		cc = control_ratio;
		if (update_signal(v))
			return;	/* Envelope ran out */
		left = v->left_mix;
	}

	while (count)
	{
		if (cc < count)
		{
			count -= cc;
			while (cc--)
			{
				*lp++ += *sp++ * left;
			}
			cc = control_ratio;
			if (update_signal(v))
				return;	/* Envelope ran out */
			left = v->left_mix;
		}
		else
		{
			v->control_counter = cc - count;
			while (count--)
			{
				*lp++ += *sp++ * left;
			}
			return;
		}
	}
}

static void mix_mystery(int32_t control_ratio, const sample_t *sp, float *lp, Voice *v, int count)
{
	final_volume_t
		left = v->left_mix,
		right = v->right_mix;
	sample_t s;

	while (count--)
	{
		s = *sp++;
		lp[0] += s * left;
		lp[1] += s * right;
		lp += 2;
	}
}

static void mix_single(const sample_t *sp, float *lp, final_volume_t amp, int count)
{
	while (count--)
	{
		lp[0] += *sp++ * amp;
		lp += 2;
	}
}

static void mix_single_left(const sample_t *sp, float *lp, Voice *v, int count)
{
	mix_single(sp, lp, v->left_mix, count);
}
static void mix_single_right(const sample_t *sp, float *lp, Voice *v, int count)
{
	mix_single(sp, lp + 1, v->right_mix, count);
}

static void mix_mono(const sample_t *sp, float *lp, Voice *v, int count)
{
	final_volume_t
		left = v->left_mix;

	while (count--)
	{
		*lp++ += *sp++ * left;
	}
}

/* Ramp a note out in c samples */
static void ramp_out(const sample_t *sp, float *lp, Voice *v, int c)
{
	final_volume_t left, right, li, ri;

	sample_t s = 0; /* silly warning about uninitialized s */

	/* Fix by James Caldwell */
	if ( c == 0 ) c = 1;

	/* printf("Ramping out: left=%d, c=%d, li=%d\n", left, c, li); */

	if (v->right_mix == 0)			// All the way to the left
	{
		left = v->left_mix;
		li = -(left/c);
		if (li == 0) li = -1;

		while (c--)
		{
			left += li;
			if (left < 0)
				return;
			lp[0] += *sp++ * left;
			lp += 2;
		}
	}
	else if (v->left_mix == 0)		// All the way to the right
	{
		right = v->right_mix;
		ri = -(right/c);
		if (ri == 0) ri = -1;

		while (c--)
		{
			right += ri;
			if (right < 0)
				return;
			s = *sp++;
			lp[1] += *sp++ * right;
			lp += 2;
		}
	}
	else							// Somewhere in the middle
	{
		left = v->left_mix;
		li = -(left/c);
		if (li == 0) li = -1;
		right = v->right_mix;
		ri = -(right/c);
		if (ri == 0) ri = -1;

		right = v->right_mix;
		ri = -(right/c);
		while (c--)
		{
			left += li;
			right += ri;
			if (left < 0)
			{
				if (right < 0)
				{
					return;
				}
				left = 0;
			}
			else if (right < 0)
			{
				right = 0;
			}
			s = *sp++;
			lp[0] += s * left;
			lp[1] += s * right;
			lp += 2;
		}
	}
}


/**************** interface function ******************/

void mix_voice(Renderer *song, float *buf, Voice *v, int c)
{
	int count = c;
	sample_t *sp;
	if (c < 0)
	{
		return;
	}
	if (v->status & VOICE_STOPPING)
	{
		if (count >= MAX_DIE_TIME)
			count = MAX_DIE_TIME;
		sp = resample_voice(song, v, &count);
		ramp_out(sp, buf, v, count);
		v->status = 0;
	}
	else
	{
		sp = resample_voice(song, v, &count);
		if (count < 0)
		{
			return;
		}
		if (v->right_mix == 0)			// All the way to the left
		{
			if (v->eg1.env.bUpdating || v->tremolo_phase_increment != 0)
			{
				mix_single_left_signal(song->control_ratio, sp, buf, v, count);
			}
			else
			{
				mix_single_left(sp, buf, v, count);
			}
		}
		else if (v->left_mix == 0)		// All the way to the right
		{
			if (v->eg1.env.bUpdating || v->tremolo_phase_increment != 0)
			{
				mix_single_right_signal(song->control_ratio, sp, buf, v, count);
			}
			else
			{
				mix_single_right(sp, buf, v, count);
			}
		}
		else							// Somewhere in the middle
		{
			if (v->eg1.env.bUpdating || v->tremolo_phase_increment)
			{
				mix_mystery_signal(song->control_ratio, sp, buf, v, count);
			}
			else
			{
				mix_mystery(song->control_ratio, sp, buf, v, count);
			}
		}
		v->sample_count += count;
	}
}

}
