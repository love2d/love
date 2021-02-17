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

	playmidi.c -- random stuff in need of rearrangement

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "timidity.h"

namespace Timidity
{

void Renderer::reset_voices()
{
	memset(voice, 0, sizeof(voice[0]) * voices);
}

/* Process the Reset All Controllers event */
void Renderer::reset_controllers(int c)
{
	channel[c].volume = 100;
	channel[c].expression = 127;
	channel[c].sustain = 0;
	channel[c].pitchbend = 0x2000;
	channel[c].pitchfactor = 0; /* to be computed */
	channel[c].mono = 0;
	channel[c].rpn = RPN_RESET;
	channel[c].nrpn = RPN_RESET;
}

void Renderer::reset_midi()
{
	for (int i = 0; i < MAXCHAN; i++)
	{
		reset_controllers(i);
		/* The rest of these are unaffected by the Reset All Controllers event */
		channel[i].program = default_program;
		channel[i].panning = NO_PANNING;
		channel[i].pitchsens = 200;
		channel[i].bank = 0; /* tone bank or drum set */
	}
	reset_voices();
}

void Renderer::recompute_freq(int v)
{
	Channel *ch = &channel[voice[v].channel];
	int
		sign = (voice[v].sample_increment < 0), /* for bidirectional loops */
		pb = ch->pitchbend;
	double a;

	if (voice[v].sample->sample_rate == 0)
	{
		return;
	}

	if (voice[v].vibrato_control_ratio != 0)
	{
		/* This instrument has vibrato. Invalidate any precomputed
		   sample_increments. */
		memset(voice[v].vibrato_sample_increment, 0, sizeof(voice[v].vibrato_sample_increment));
	}

	if (pb == 0x2000 || pb < 0 || pb > 0x3FFF)
	{
		voice[v].frequency = voice[v].orig_frequency;
	}
	else
	{
		pb -= 0x2000;
		if (ch->pitchfactor == 0)
		{
			/* Damn. Somebody bent the pitch. */
			ch->pitchfactor = float(pow(2.f, ((abs(pb) * ch->pitchsens) / (8191.f * 1200.f))));
		}
		if (pb < 0)
		{
			voice[v].frequency = voice[v].orig_frequency / ch->pitchfactor;
		}
		else
		{
			voice[v].frequency = voice[v].orig_frequency * ch->pitchfactor;
		}
	}

	a = FSCALE(((double)(voice[v].sample->sample_rate) * voice[v].frequency) /
		((double)(voice[v].sample->root_freq) * rate),
		FRACTION_BITS);

	if (sign)
		a = -a; /* need to preserve the loop direction */

	voice[v].sample_increment = (int)(a);
}

static const uint8_t vol_table[] = {
000 /* 000 */,	129 /* 001 */,	145 /* 002 */,	155 /* 003 */,
161 /* 004 */,	166 /* 005 */,	171 /* 006 */,	174 /* 007 */,
177 /* 008 */,	180 /* 009 */,	182 /* 010 */,	185 /* 011 */,
187 /* 012 */,	188 /* 013 */,	190 /* 014 */,	192 /* 015 */,
193 /* 016 */,	195 /* 017 */,	196 /* 018 */,	197 /* 019 */,
198 /* 020 */,	199 /* 021 */,	201 /* 022 */,	202 /* 023 */,
203 /* 024 */,	203 /* 025 */,	204 /* 026 */,	205 /* 027 */,
206 /* 028 */,	207 /* 029 */,	208 /* 030 */,	208 /* 031 */,
209 /* 032 */,	210 /* 033 */,	211 /* 034 */,	211 /* 035 */,
212 /* 036 */,	213 /* 037 */,	213 /* 038 */,	214 /* 039 */,
214 /* 040 */,	215 /* 041 */,	215 /* 042 */,	216 /* 043 */,
217 /* 044 */,	217 /* 045 */,	218 /* 046 */,	218 /* 047 */,
219 /* 048 */,	219 /* 049 */,	219 /* 050 */,	220 /* 051 */,
220 /* 052 */,	221 /* 053 */,	221 /* 054 */,	222 /* 055 */,
222 /* 056 */,	222 /* 057 */,	223 /* 058 */,	223 /* 059 */,
224 /* 060 */,	224 /* 061 */,	224 /* 062 */,	225 /* 063 */,
225 /* 064 */,	226 /* 065 */,	227 /* 066 */,	228 /* 067 */,
229 /* 068 */,	230 /* 069 */,	231 /* 070 */,	231 /* 071 */,
232 /* 072 */,	233 /* 073 */,	234 /* 074 */,	234 /* 075 */,
235 /* 076 */,	236 /* 077 */,	236 /* 078 */,	237 /* 079 */,
238 /* 080 */,	238 /* 081 */,	239 /* 082 */,	239 /* 083 */,
240 /* 084 */,	241 /* 085 */,	241 /* 086 */,	242 /* 087 */,
242 /* 088 */,	243 /* 089 */,	243 /* 090 */,	244 /* 091 */,
244 /* 092 */,	244 /* 093 */,	245 /* 094 */,	245 /* 095 */,
246 /* 096 */,	246 /* 097 */,	247 /* 098 */,	247 /* 099 */,
247 /* 100 */,	248 /* 101 */,	248 /* 102 */,	249 /* 103 */,
249 /* 104 */,	249 /* 105 */,	250 /* 106 */,	250 /* 107 */,
250 /* 108 */,	251 /* 109 */,	251 /* 110 */,	251 /* 111 */,
252 /* 112 */,	252 /* 113 */,	252 /* 114 */,	253 /* 115 */,
253 /* 116 */,	253 /* 117 */,	254 /* 118 */,	254 /* 119 */,
254 /* 120 */,	254 /* 121 */,	255 /* 122 */,	255 /* 123 */,
255 /* 124 */,	255 /* 125 */,	255 /* 126 */,	255 /* 127 */,
};

void Renderer::recompute_amp(Voice *v)
{
	Channel *chan = &channel[v->channel];
	int chanvol = chan->volume;
	int chanexpr = chan->expression;

	if (v->sample->type == INST_GUS)
	{
		v->attenuation = (vol_table[(chanvol * chanexpr) / 127] * vol_table[v->velocity]) * ((127 + 64) / 12419775.f);
	}
	else
	{
		// Implicit modulators from SF2 spec
		double velatten, cc7atten, cc11atten;

		velatten = log10(127.0 / v->velocity);
		cc7atten = log10(127.0 / chanvol);
		cc11atten = log10(127.0 / chanexpr);
		v->attenuation = float(400 * (velatten + cc7atten + cc11atten)) + v->sample->initial_attenuation;
	}
}

// Pan must be in the range [0,1]
void Renderer::compute_pan(double pan, int type, float &left_offset, float &right_offset)
{
	if (pan <= 0)
	{
		left_offset = 1;
		right_offset = 0;
	}
	else if (pan >= 127/128.0)
	{
		left_offset = 0;
		right_offset = 1;
	}
	else
	{
		if (type == INST_GUS)
		{
			/* Original amp equation looks like this:
			 *    calc_gf1_amp(atten + offset)
			 * which expands to:
			 *    2^(16*(atten + offset) - 16)
			 * Keeping in mind that 2^(x + y) == 2^x * 2^y, we can
			 * rewrite this to avoid doing two pows in GF1Envelope::ApplyToAmp():
			 *    2^(16*atten + 16*offset - 16)
			 *    2^(16*atten - 16 + 16 * offset + 16 - 16)
			 *    2^(16*atten - 16) * 2^(16*offset + 16 - 16)
			 *    2^(16*atten - 16) * 2^(16*(offset + 1) - 16)
			 *    calc_gf1_amp(atten) * calc_gf1_amp(offset + 1)
			 */
			right_offset = (float)calc_gf1_amp((log(pan) * (1 / (log_of_2 * 32))) + 1);
			left_offset = (float)calc_gf1_amp((log(1 - pan) * (1 / (log_of_2 * 32))) + 1);
		}
		else
		{
			/* Equal Power Panning for SF2/DLS.
			 */
			left_offset = (float)sqrt(1 - pan);
			right_offset = (float)sqrt(pan);
		}
	}
}

void Renderer::kill_key_group(int i)
{
	int j = voices;

	if (voice[i].sample->key_group == 0)
	{
		return;
	}
	while (j--)
	{
		if ((voice[j].status & VOICE_RUNNING) && !(voice[j].status & (VOICE_RELEASING | VOICE_STOPPING))) continue;
		if (i == j) continue;
		if (voice[i].channel != voice[j].channel) continue;
		if (voice[j].sample->key_group != voice[i].sample->key_group) continue;
		kill_note(j);
	}
}

float Renderer::calculate_scaled_frequency(Sample *sp, int note)
{
	double scalednote = (note - sp->scale_note) * sp->scale_factor / 1024.0 + sp->scale_note + sp->tune * 0.01;
	return (float)note_to_freq(scalednote);
}

bool Renderer::start_region(int chan, int note, int vel, Sample *sp, float f)
{
	int voicenum;
	Voice *v;

	voicenum = allocate_voice();
	if (voicenum < 0)
	{
		return false;
	}
	v = &voice[voicenum];
	v->sample = sp;

	if (sp->type == INST_GUS)
	{
		v->orig_frequency = f;
	}
	else
	{
		if (sp->scale_factor != 1024)
		{
			v->orig_frequency = calculate_scaled_frequency(sp, note);
		}
		else if (sp->tune != 0)
		{
			v->orig_frequency = note_to_freq(note + sp->tune * 0.01);
		}
		else
		{
			v->orig_frequency = note_to_freq(note);
		}
	}

	v->status = VOICE_RUNNING;
	v->channel = chan;
	v->note = note;
	v->velocity = vel;
	v->sample_offset = 0;
	v->sample_increment = 0; /* make sure it isn't negative */
	v->sample_count = 0;

	v->tremolo_phase = 0;
	v->tremolo_phase_increment = v->sample->tremolo_phase_increment;
	v->tremolo_sweep = v->sample->tremolo_sweep_increment;
	v->tremolo_sweep_position = 0;

	v->vibrato_sweep = v->sample->vibrato_sweep_increment;
	v->vibrato_sweep_position = 0;
	v->vibrato_control_ratio = v->sample->vibrato_control_ratio;
	v->vibrato_control_counter = v->vibrato_phase = 0;

	kill_key_group(voicenum);

	memset(v->vibrato_sample_increment, 0, sizeof(v->vibrato_sample_increment));

	if (sp->type == INST_SF2)
	{
		// Channel pan is added to instrument pan.
		double pan;
		if (channel[chan].panning == NO_PANNING)
		{
			pan = (sp->panning + 500) / 1000.0;
		}
		else
		{
			pan = channel[chan].panning / 128.0 + sp->panning / 1000.0;
		}
		compute_pan(pan, sp->type, v->left_offset, v->right_offset);
	}
	else if (channel[chan].panning != NO_PANNING)
	{
		compute_pan(channel[chan].panning / 128.0, sp->type, v->left_offset, v->right_offset);
	}
	else
	{
		v->left_offset = v->sample->left_offset;
		v->right_offset = v->sample->right_offset;
	}

	recompute_freq(voicenum);
	recompute_amp(v);
	v->control_counter = 0;

	v->eg1.Init(this, v);

	if (v->sample->modes & PATCH_LOOPEN)
	{
		v->status |= VOICE_LPE;
	}
	return true;
}

void Renderer::start_note(int chan, int note, int vel)
{
	Instrument *ip;
	Sample *sp;
	int bank = channel[chan].bank;
	int prog = channel[chan].program;
	int i;
	float f;

	note &= 0x7f;
	if (ISDRUMCHANNEL(chan))
	{
		if (NULL == drumset[bank] || NULL == (ip = drumset[bank]->instrument[note]))
		{
			if (!(ip = drumset[0]->instrument[note]))
				return; /* No instrument? Then we can't play. */
		}
		assert(ip != MAGIC_LOAD_INSTRUMENT);
		if (ip == MAGIC_LOAD_INSTRUMENT)
		{
			return;
		}
		if (ip->samples != 1 && ip->sample->type == INST_GUS)
		{
			cmsg(CMSG_WARNING, VERB_VERBOSE,
				"Strange: percussion instrument with %d samples!", ip->samples);
		}
	}
	else
	{
		if (channel[chan].program == SPECIAL_PROGRAM)
		{
			ip = default_instrument;
		}
		else if (NULL == tonebank[bank] || NULL == (ip = tonebank[bank]->instrument[prog]))
		{
			if (NULL == (ip = tonebank[0]->instrument[prog]))
				return; /* No instrument? Then we can't play. */
		}
		assert(ip != MAGIC_LOAD_INSTRUMENT);
		if (ip == MAGIC_LOAD_INSTRUMENT)
		{
			return;
		}
	}

	if (NULL == ip->sample || ip->samples == 0)
		return;	/* No samples? Then nothing to play. */

	// For GF1 patches, scaling is based solely on the first
	// waveform in this layer.
	if (ip->sample->type == INST_GUS && ip->sample->scale_factor != 1024)
	{
		f = calculate_scaled_frequency(ip->sample, note);
	}
	else
	{
		f = note_to_freq(note);
	}

	if (ip->sample->type == INST_GUS)
	{
		/* We're more lenient with matching ranges for GUS patches, since the
		 * official Gravis ones don't cover the full range of possible
		 * frequencies for every instrument.
		 */
		if (ip->samples == 1)
		{ // If there's only one sample, definitely play it.
			start_region(chan, note, vel, ip->sample, f);
		}
		for (i = ip->samples, sp = ip->sample; i != 0; --i, ++sp)
		{
			// GUS patches don't have velocity ranges, so no need to compare against them.
			if (sp->low_freq <= f && sp->high_freq >= f)
			{
				if (i > 1 && (sp + 1)->low_freq <= f && (sp + 1)->high_freq >= f)
				{ /* If there is a range of contiguous regions that match our
				   * desired frequency, the last one in that block is used.
				   */
					continue;
				}
				start_region(chan, note, vel, sp, f);
				break;
			}
		}
		if (i == 0)
		{ /* Found nothing. Try again, but look for the one with the closest root frequency.
		   * As per the suggestion in the original TiMidity function, this search uses
		   * note values rather than raw frequencies.
		   */
			double cdiff = 1e10;
			double want_note = freq_to_note(f);
			Sample *closest = sp = ip->sample;
			for (i = ip->samples; i != 0; --i, ++sp)
			{
				double diff = fabs(freq_to_note(sp->root_freq) - want_note);
				if (diff < cdiff)
				{
					cdiff = diff;
					closest = sp;
				}
			}
			start_region(chan, note, vel, closest, f);
		}
	}
	else
	{
		for (i = ip->samples, sp = ip->sample; i != 0; --i, ++sp)
		{
			if ((sp->low_vel <= vel && sp->high_vel >= vel &&
				 sp->low_freq <= f && sp->high_freq >= f))
			{
				if (!start_region(chan, note, vel, sp, f))
				{ // Ran out of voices
					break;
				}
			}
		}
	}
}

void Renderer::kill_note(int i)
{
	Voice *v = &voice[i];

	if (v->status & VOICE_RUNNING)
	{
		v->status &= ~VOICE_SUSTAINING;
		v->status |= VOICE_RELEASING | VOICE_STOPPING;
	}
}

int Renderer::allocate_voice()
{
	int i, lowest;
	float lv, v;

	for (i = 0; i < voices; ++i)
	{
		if (!(voice[i].status & VOICE_RUNNING))
		{
			return i; /* Can't get a lower volume than silence */
		}
	}

	/* Look for the decaying note with the lowest volume */
	lowest = -1;
	lv = 1e10;
	i = voices;
	while (i--)
	{
		if ((voice[i].status & VOICE_RELEASING) && !(voice[i].status & VOICE_STOPPING))
		{
			v = voice[i].attenuation;
			if (v < lv)
			{
				lv = v;
				lowest = i;
			}
		}
	}

	if (lowest >= 0)
	{
		/* This can still cause a click, but if we had a free voice to
		   spare for ramping down this note, we wouldn't need to kill it
		   in the first place... Still, this needs to be fixed. Perhaps
		   we could use a reserve of voices to play dying notes only. */

		cut_notes++;
		voice[lowest].status = 0;
	}
	else
	{
		lost_notes++;
	}
	return lowest;
}

void Renderer::note_on(int chan, int note, int vel)
{
	if (vel == 0)
	{
		note_off(chan, note, 0);
		return;
	}

	int i = voices;

	/* Only one instance of a note can be playing on a single channel. */
	while (i--)
	{
		if (voice[i].channel == chan && ((voice[i].note == note && !voice[i].sample->self_nonexclusive) || channel[chan].mono))
		{
			if (channel[chan].mono)
			{
				kill_note(i);
			}
			else
			{
				finish_note(i);
			}
		}
	}

	start_note(chan, note, vel);
}

void Renderer::finish_note(int i)
{
	Voice *v = &voice[i];

	if ((v->status & (VOICE_RUNNING | VOICE_RELEASING)) == VOICE_RUNNING)
	{
		v->status &= ~VOICE_SUSTAINING;
		v->status |= VOICE_RELEASING;

		if (!(v->sample->modes & PATCH_NO_SRELEASE))
		{
			v->status &= ~VOICE_LPE;	/* sampled release */
		}
		v->eg1.Release(v);
		v->eg2.Release(v);
	}
}

void Renderer::note_off(int chan, int note, int vel)
{
	int i;

	for (i = voices; i-- > 0; )
	{
		if ((voice[i].status & VOICE_RUNNING) && !(voice[i].status & (VOICE_RELEASING | VOICE_STOPPING))
			&& voice[i].channel == chan && voice[i].note == note)
		{
			if (channel[chan].sustain)
			{
				voice[i].status |= NOTE_SUSTAIN;
			}
			else
			{
				finish_note(i);
			}
		}
	}
}

/* Process the All Notes Off event */
void Renderer::all_notes_off(int chan)
{
	int i = voices;
	while (i--)
	{
		if ((voice[i].status & VOICE_RUNNING) && voice[i].channel == chan)
		{
			if (channel[chan].sustain)
			{
				voice[i].status |= NOTE_SUSTAIN;
			}
			else
			{
				finish_note(i);
			}
		}
	}
}

/* Process the All Sounds Off event */
void Renderer::all_sounds_off(int chan)
{
	int i = voices;
	while (i--)
	{
		if (voice[i].channel == chan &&
			(voice[i].status & VOICE_RUNNING) &&
			!(voice[i].status & VOICE_STOPPING))
		{
			kill_note(i);
		}
	}
}

void Renderer::adjust_pressure(int chan, int note, int amount)
{
	int i = voices;
	while (i--)
	{
		if ((voice[i].status & VOICE_RUNNING) &&
			voice[i].channel == chan &&
			voice[i].note == note)
		{
			voice[i].velocity = amount;
			recompute_amp(&voice[i]);
			apply_envelope_to_amp(&voice[i]);
			if (!(voice[i].sample->self_nonexclusive))
			{
				return;
			}
		}
	}
}

void Renderer::adjust_panning(int chan)
{
	Channel *chanp = &channel[chan];
	int i = voices;
	while (i--)
	{
		Voice *v = &voice[i];
		if ((v->channel == chan) && (v->status & VOICE_RUNNING))
		{
			double pan = chanp->panning / 128.0;
			if (v->sample->type == INST_SF2)
			{ // Add instrument pan to channel pan.
				pan += v->sample->panning / 500.0;
			}
			compute_pan(pan, v->sample->type, v->left_offset, v->right_offset);
			apply_envelope_to_amp(v);
		}
	}
}

void Renderer::drop_sustain(int chan)
{
	int i = voices;
	while (i--)
	{
		if (voice[i].channel == chan && (voice[i].status & NOTE_SUSTAIN))
		{
			finish_note(i);
		}
	}
}

void Renderer::adjust_pitchbend(int chan)
{
	int i = voices;
	while (i--)
	{
		if ((voice[i].status & VOICE_RUNNING) && voice[i].channel == chan)
		{
			recompute_freq(i);
		}
	}
}

void Renderer::adjust_volume(int chan)
{
	int i = voices;
	while (i--)
	{
		if (voice[i].channel == chan && (voice[i].status & VOICE_RUNNING))
		{
			recompute_amp(&voice[i]);
			apply_envelope_to_amp(&voice[i]);
		}
	}
}

void Renderer::HandleEvent(int status, int parm1, int parm2)
{
	int command = status & 0xF0;
	int chan	= status & 0x0F;

	switch (command)
	{
	case ME_NOTEON:
		note_on(chan, parm1, parm2);
		break;

	case ME_NOTEOFF:
		note_off(chan, parm1, parm2);
		break;

	case ME_KEYPRESSURE:
		adjust_pressure(chan, parm1, parm2);
		break;

	case ME_CONTROLCHANGE:
		HandleController(chan, parm1, parm2);
		break;

	case ME_PROGRAM:
		if (ISDRUMCHANNEL(chan))
		{
			/* Change drum set */
			channel[chan].bank = parm1;
		}
		else
		{
			channel[chan].program = parm1;
		}
		break;

	case ME_CHANNELPRESSURE:
		/* Unimplemented */
		break;

	case ME_PITCHWHEEL:
		channel[chan].pitchbend = parm1 | (parm2 << 7);
		channel[chan].pitchfactor = 0;
		/* Adjust for notes already playing */
		adjust_pitchbend(chan);
		break;
	}
}

void Renderer::HandleController(int chan, int ctrl, int val)
{
	switch (ctrl)
	{
    /* These should be the SCC-1 tone bank switch
       commands. I don't know why there are two, or
       why the latter only allows switching to bank 0.
       Also, some MIDI files use 0 as some sort of
       continuous controller. This will cause lots of
       warnings about undefined tone banks. */
	case CTRL_BANK_SELECT:
		channel[chan].bank = val;
		break;

	case CTRL_BANK_SELECT+32:
		if (val == 0)
		{
			channel[chan].bank = 0;
		}
		break;

	case CTRL_VOLUME:
		channel[chan].volume = val;
		adjust_volume(chan);
		break;

	case CTRL_EXPRESSION:
		channel[chan].expression = val;
		adjust_volume(chan);
		break;

	case CTRL_PAN:
		channel[chan].panning = val;
		adjust_panning(chan);
		break;

	case CTRL_SUSTAIN:
		channel[chan].sustain = val;
		if (val == 0)
		{
			drop_sustain(chan);
		}
		break;

	case CTRL_NRPN_LSB:
		channel[chan].nrpn = (channel[chan].nrpn & 0x3F80) | (val);
		channel[chan].nrpn_mode = true;
		break;

	case CTRL_NRPN_MSB:
		channel[chan].nrpn = (channel[chan].nrpn & 0x007F) | (val << 7);
		channel[chan].nrpn_mode = true;
		break;

	case CTRL_RPN_LSB:
		channel[chan].rpn = (channel[chan].rpn & 0x3F80) | (val);
		channel[chan].nrpn_mode = false;
		break;

	case CTRL_RPN_MSB:
		channel[chan].rpn = (channel[chan].rpn & 0x007F) | (val << 7);
		channel[chan].nrpn_mode = false;
		break;

	case CTRL_DATA_ENTRY:
		if (channel[chan].nrpn_mode)
		{
			DataEntryCoarseNRPN(chan, channel[chan].nrpn, val);
		}
		else
		{
			DataEntryCoarseRPN(chan, channel[chan].rpn, val);
		}
		break;

	case CTRL_DATA_ENTRY+32:
		if (channel[chan].nrpn_mode)
		{
			DataEntryFineNRPN(chan, channel[chan].nrpn, val);
		}
		else
		{
			DataEntryFineRPN(chan, channel[chan].rpn, val);
		}
		break;

	case CTRL_ALL_SOUNDS_OFF:
		all_sounds_off(chan);
		break;

	case CTRL_RESET_CONTROLLERS:
		reset_controllers(chan);
		break;

	case CTRL_ALL_NOTES_OFF:
		all_notes_off(chan);
		break;
	}
}

void Renderer::DataEntryCoarseRPN(int chan, int rpn, int val)
{
	switch (rpn)
	{
	case RPN_PITCH_SENS:
		channel[chan].pitchsens = (channel[chan].pitchsens % 100) + (val * 100);
		channel[chan].pitchfactor = 0;
		break;

	// TiMidity resets the pitch sensitivity when a song attempts to write to
	// RPN_RESET. My docs tell me this is just a dummy value that is guaranteed
	// to not cause future data entry to go anywhere until a new RPN is set.
	}
}

void Renderer::DataEntryFineRPN(int chan, int rpn, int val)
{
	switch (rpn)
	{
	case RPN_PITCH_SENS:
		channel[chan].pitchsens = (channel[chan].pitchsens / 100) * 100 + val;
		channel[chan].pitchfactor = 0;
		break;
	}
}

void Renderer::DataEntryCoarseNRPN(int chan, int nrpn, int val)
{
}

void Renderer::DataEntryFineNRPN(int chan, int nrpn, int val)
{
}

void Renderer::HandleLongMessage(const uint8_t *data, int len)
{
	// SysEx handling goes here.
}

void Renderer::Reset()
{
	lost_notes = cut_notes = 0;
	reset_midi();
}

}
