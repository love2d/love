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

	resample.c

*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "timidity.h"

namespace Timidity
{

#define RESAMPLATION {\
	int o = ofs >> FRACTION_BITS, m = ofs & FRACTION_MASK; \
	*dest++ = src[o] + (src[o + 1] - src[o]) * m / (1 << FRACTION_BITS);\
  }

#define FINALINTERP if (ofs == le) *dest++ = src[ofs >> FRACTION_BITS];
/* So it isn't interpolation. At least it's final. */

/*************** resampling with fixed increment *****************/

static sample_t *rs_plain(sample_t *resample_buffer, Voice *v, int *countptr)
{
	/* Play sample until end, then free the voice. */

	const sample_t
		*src = v->sample->data;
	sample_t
		*dest = resample_buffer;
	int
		ofs = v->sample_offset,
		incr = v->sample_increment,
		le = v->sample->data_length,
		count = *countptr;

	int i;

	if (incr < 0) incr = -incr; /* In case we're coming out of a bidir loop */

	/* Precalc how many times we should go through the loop.
	NOTE: Assumes that incr > 0 and that ofs <= le */
	i = (le - ofs) / incr + 1;

	if (i > count)
	{
		i = count;
		count = 0;
	}
	else
	{
		count -= i;
	}

	while (i--)
	{
		RESAMPLATION;
		ofs += incr;
	}

	if (ofs >= le)
	{
		FINALINTERP;
		v->status = 0;
		*countptr -= count + 1;
	}

	v->sample_offset = ofs; /* Update offset */
	return resample_buffer;
}

static sample_t *rs_loop(sample_t *resample_buffer, Voice *vp, int count)
{
	/* Play sample until end-of-loop, skip back and continue. */

	int
		ofs = vp->sample_offset,
		incr = vp->sample_increment,
		le = vp->sample->loop_end,
		ll = le - vp->sample->loop_start;
	sample_t
		*dest = resample_buffer;
	const sample_t
		*src = vp->sample->data;

	int i;

	while (count)
	{
		if (ofs >= le)
			/* NOTE: Assumes that ll > incr and that incr > 0. */
			ofs -= ll;
		/* Precalc how many times we should go through the loop */
		i = (le - ofs) / incr + 1;
		if (i > count)
		{
			i = count;
			count = 0;
		}
		else
		{
			count -= i;
		}
		while (i--)
		{
			RESAMPLATION;
			ofs += incr;
		}
	}

	vp->sample_offset=ofs; /* Update offset */
	return resample_buffer;
}

static sample_t *rs_bidir(sample_t *resample_buffer, Voice *vp, int count)
{
	int
		ofs = vp->sample_offset,
		incr = vp->sample_increment,
		le = vp->sample->loop_end,
		ls = vp->sample->loop_start;
	sample_t
		*dest = resample_buffer;
	const sample_t
		*src = vp->sample->data;

	int
		le2 = le << 1,
		ls2 = ls << 1,
		i;
	/* Play normally until inside the loop region */

	if (ofs <= ls)
	{
		/* NOTE: Assumes that incr > 0, which is NOT always the case
		when doing bidirectional looping.  I have yet to see a case
		where both ofs <= ls AND incr < 0, however. */
		i = (ls - ofs) / incr + 1;
		if (i > count)
		{
			i = count;
			count = 0;
		}
		else
		{
			count -= i;
		}
		while (i--)
		{
			RESAMPLATION;
			ofs += incr;
		}
	}

	/* Then do the bidirectional looping */

	while(count)
	{
		/* Precalc how many times we should go through the loop */
		i = ((incr > 0 ? le : ls) - ofs) / incr + 1;
		if (i > count)
		{
			i = count;
			count = 0;
		}
		else
		{
			count -= i;
		}
		while (i--)
		{
			RESAMPLATION;
			ofs += incr;
		}
		if (ofs >= le)
		{
			/* fold the overshoot back in */
			ofs = le2 - ofs;
			incr *= -1;
		}
		else if (ofs <= ls)
		{
			ofs = ls2 - ofs;
			incr *= -1;
		}
	}

	vp->sample_increment = incr;
	vp->sample_offset = ofs; /* Update offset */
	return resample_buffer;
}

/*********************** vibrato versions ***************************/

/* We only need to compute one half of the vibrato sine cycle */
static int vib_phase_to_inc_ptr(int phase)
{
	if (phase < VIBRATO_SAMPLE_INCREMENTS / 2)
		return VIBRATO_SAMPLE_INCREMENTS / 2 - 1 - phase;
	else if (phase >= VIBRATO_SAMPLE_INCREMENTS * 3 / 2)
		return VIBRATO_SAMPLE_INCREMENTS * 5 / 2 - 1 - phase;
	else
		return phase - VIBRATO_SAMPLE_INCREMENTS / 2;
}

static int update_vibrato(float output_rate, Voice *vp, int sign)
{
	int depth;
	int phase;
	double a, pb;

	if (vp->vibrato_phase++ >= 2 * VIBRATO_SAMPLE_INCREMENTS - 1)
		vp->vibrato_phase = 0;
	phase = vib_phase_to_inc_ptr(vp->vibrato_phase);

	if (vp->vibrato_sample_increment[phase])
	{
		if (sign)
			return -vp->vibrato_sample_increment[phase];
		else
			return vp->vibrato_sample_increment[phase];
	}

	/* Need to compute this sample increment. */
	depth = vp->sample->vibrato_depth << 7;

	if (vp->vibrato_sweep != 0)
	{
		/* Need to update sweep */
		vp->vibrato_sweep_position += vp->vibrato_sweep;
		if (vp->vibrato_sweep_position >= (1<<SWEEP_SHIFT))
			vp->vibrato_sweep=0;
		else
		{
			/* Adjust depth */
			depth *= vp->vibrato_sweep_position;
			depth >>= SWEEP_SHIFT;
		}
	}

	a = FSCALE(((double)(vp->sample->sample_rate) * vp->frequency) /
		((double)(vp->sample->root_freq) * output_rate),
		FRACTION_BITS);

	pb = (sine(vp->vibrato_phase * (1.0/(2*VIBRATO_SAMPLE_INCREMENTS)))
		* (double)(depth) * VIBRATO_AMPLITUDE_TUNING);

	a *= pow(2.0, pb / (8192 * 12.f));

	/* If the sweep's over, we can store the newly computed sample_increment */
	if (!vp->vibrato_sweep)
		vp->vibrato_sample_increment[phase] = (int) a;

	if (sign)
		a = -a; /* need to preserve the loop direction */

	return (int) a;
}

static sample_t *rs_vib_plain(sample_t *resample_buffer, float rate, Voice *vp, int *countptr)
{
	/* Play sample until end, then free the voice. */

	sample_t
		*dest = resample_buffer;
	const sample_t
		*src = vp->sample->data;
	int
		le = vp->sample->data_length,
		ofs = vp->sample_offset,
		incr = vp->sample_increment,
		count = *countptr;
	int
		cc = vp->vibrato_control_counter;

	/* This has never been tested */

	if (incr < 0) incr = -incr; /* In case we're coming out of a bidir loop */

	while (count--)
	{
		if (!cc--)
		{
			cc = vp->vibrato_control_ratio;
			incr = update_vibrato(rate, vp, 0);
		}
		RESAMPLATION;
		ofs += incr;
		if (ofs >= le)
		{
			FINALINTERP;
			vp->status = 0;
			*countptr -= count+1;
			break;
		}
	}

	vp->vibrato_control_counter = cc;
	vp->sample_increment = incr;
	vp->sample_offset = ofs; /* Update offset */
	return resample_buffer;
}

static sample_t *rs_vib_loop(sample_t *resample_buffer, float rate, Voice *vp, int count)
{
	/* Play sample until end-of-loop, skip back and continue. */

	int
		ofs = vp->sample_offset,
		incr = vp->sample_increment,
		le = vp->sample->loop_end,
		ll = le - vp->sample->loop_start;
	sample_t
		*dest = resample_buffer;
	const sample_t
		*src = vp->sample->data;
	int
		cc = vp->vibrato_control_counter;

	int i;
	int
		vibflag=0;

	while (count)
	{
		/* Hopefully the loop is longer than an increment */
		if (ofs >= le)
			ofs -= ll;
		/* Precalc how many times to go through the loop, taking
		the vibrato control ratio into account this time. */
		i = (le - ofs) / incr + 1;
		if (i > count) i = count;
		if (i > cc)
		{
			i = cc;
			vibflag = 1;
		}
		else
		{
			cc -= i;
		}
		count -= i;
		while (i--)
		{
			RESAMPLATION;
			ofs += incr;
		}
		if (vibflag)
		{
			cc = vp->vibrato_control_ratio;
			incr = update_vibrato(rate, vp, 0);
			vibflag = 0;
		}
	}

	vp->vibrato_control_counter = cc;
	vp->sample_increment = incr;
	vp->sample_offset = ofs; /* Update offset */
	return resample_buffer;
}

static sample_t *rs_vib_bidir(sample_t *resample_buffer, float rate, Voice *vp, int count)
{
	int
		ofs = vp->sample_offset,
		incr = vp->sample_increment,
		le = vp->sample->loop_end,
		ls = vp->sample->loop_start;
	sample_t
		*dest = resample_buffer;
	const sample_t
		*src = vp->sample->data;
	int
		cc = vp->vibrato_control_counter;

	int
		le2 = le << 1,
		ls2 = ls << 1,
		i;
	int
		vibflag = 0;

	/* Play normally until inside the loop region */
	while (count && (ofs <= ls))
	{
		i = (ls - ofs) / incr + 1;
		if (i > count)
		{
			i = count;
		}
		if (i > cc)
		{
			i = cc;
			vibflag = 1;
		}
		else
		{
			cc -= i;
		}
		count -= i;
		while (i--)
		{
			RESAMPLATION;
			ofs += incr;
		}
		if (vibflag)
		{
			cc = vp->vibrato_control_ratio;
			incr = update_vibrato(rate, vp, 0);
			vibflag = 0;
		}
	}

	/* Then do the bidirectional looping */

	while (count)
	{
		/* Precalc how many times we should go through the loop */
		i = ((incr > 0 ? le : ls) - ofs) / incr + 1;
		if(i > count)
		{
			i = count;
		}
		if(i > cc)
		{
			i = cc;
			vibflag = 1;
		}
		else
		{
			cc -= i;
		}
		count -= i;
		while (i--)
		{
			RESAMPLATION;
			ofs += incr;
		}
		if (vibflag)
		{
			cc = vp->vibrato_control_ratio;
			incr = update_vibrato(rate, vp, (incr < 0));
			vibflag = 0;
		}
		if (ofs >= le)
		{
			/* fold the overshoot back in */
			ofs = le2 - ofs;
			incr *= -1;
		}
		else if (ofs <= ls)
		{
			ofs = ls2 - ofs;
			incr *= -1;
		}
	}

	vp->vibrato_control_counter = cc;
	vp->sample_increment = incr;
	vp->sample_offset = ofs; /* Update offset */
	return resample_buffer;
}

sample_t *resample_voice(Renderer *song, Voice *vp, int *countptr)
{
	int ofs;
	WORD modes;

	if (vp->sample->sample_rate == 0)
	{
		/* Pre-resampled data -- just update the offset and check if
		we're out of data. */
		ofs = vp->sample_offset >> FRACTION_BITS; /* Kind of silly to use FRACTION_BITS here... */
		if (*countptr >= (vp->sample->data_length >> FRACTION_BITS) - ofs)
		{
			/* Note finished. Free the voice. */
			vp->status = 0;

			/* Let the caller know how much data we had left */
			*countptr = (vp->sample->data_length >> FRACTION_BITS) - ofs;
		}
		else
		{
			vp->sample_offset += *countptr << FRACTION_BITS;
		}
		return vp->sample->data + ofs;
	}

	/* Need to resample. Use the proper function. */
	modes = vp->sample->modes;

	if (vp->status & VOICE_LPE)
	{
		if (vp->sample->loop_end - vp->sample->loop_start < 2)
		{ // Loop is too short; turn it off.
			vp->status &= ~VOICE_LPE;
		}
	}

	if (vp->vibrato_control_ratio)
	{
		if (vp->status & VOICE_LPE)
		{
			if (modes & PATCH_BIDIR)
				return rs_vib_bidir(song->resample_buffer, song->rate, vp, *countptr);
			else
				return rs_vib_loop(song->resample_buffer, song->rate, vp, *countptr);
		}
		else
		{
			return rs_vib_plain(song->resample_buffer, song->rate, vp, countptr);
		}
	}
	else
	{
		if (vp->status & VOICE_LPE)
		{
			if (modes & PATCH_BIDIR)
				return rs_bidir(song->resample_buffer, vp, *countptr);
			else
				return rs_loop(song->resample_buffer, vp, *countptr);
		}
		else
		{
			return rs_plain(song->resample_buffer, vp, countptr);
		}
	}
}

void pre_resample(Renderer *song, Sample *sp)
{
	double a, xdiff;
	int incr, ofs, newlen, count;
	sample_t *newdata, *dest, *src = sp->data;
	sample_t v1, v2, v3, v4, *vptr;
	static const char note_name[12][3] =
	{
		"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
	};

	if (sp->scale_factor != 0)
		return;

	cmsg(CMSG_INFO, VERB_NOISY, " * pre-resampling for note %d (%s%d)\n",
		sp->scale_note,
		note_name[sp->scale_note % 12], (sp->scale_note & 0x7F) / 12);

	a = (sp->sample_rate * note_to_freq(sp->scale_note)) / (sp->root_freq * song->rate);
	if (a <= 0)
		return;
	newlen = (int)(sp->data_length / a);
	if (newlen < 0 || (newlen >> FRACTION_BITS) > MAX_SAMPLE_SIZE)
		return;

	count = newlen >> FRACTION_BITS;
	dest = newdata = (sample_t *)safe_malloc(count * sizeof(float));

	ofs = incr = (sp->data_length - (1 << FRACTION_BITS)) / count;

	if (--count)
		*dest++ = src[0];

	/* Since we're pre-processing and this doesn't have to be done in
	real-time, we go ahead and do the full sliding cubic interpolation. */
	while (--count)
	{
		vptr = src + (ofs >> FRACTION_BITS);
		v1 = (vptr == src) ? *vptr : *(vptr - 1);
		v2 = *vptr;
		v3 = *(vptr + 1);
		v4 = *(vptr + 2);
		xdiff = FSCALENEG(ofs & FRACTION_MASK, FRACTION_BITS);
		*dest++ = sample_t(v2 + (xdiff / 6.0) * (-2 * v1 - 3 * v2 + 6 * v3 - v4 +
			xdiff * (3 * (v1 - 2 * v2 + v3) + xdiff * (-v1 + 3 * (v2 - v3) + v4))));
		ofs += incr;
	}

	if (ofs & FRACTION_MASK)
	{
		RESAMPLATION
	}
	else
	{
		*dest++ = src[ofs >> FRACTION_BITS];
	}

	sp->data_length = newlen;
	sp->loop_start = int(sp->loop_start / a);
	sp->loop_end = int(sp->loop_end / a);
	free(sp->data);
	sp->data = newdata;
	sp->sample_rate = 0;
}

}
