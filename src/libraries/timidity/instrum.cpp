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

	instrum.c

	Code to load and unload GUS-compatible instrument patches.

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "timidity.h"
#include "common.h"
#include "gf1patch.h"

namespace Timidity
{

extern Instrument *load_instrument_dls(Renderer *song, int drum, int bank, int instrument);

Instrument::Instrument()
: samples(0), sample(NULL)
{
}

Instrument::~Instrument()
{
	Sample *sp;
	int i;

	for (i = samples, sp = &(sample[0]); i != 0; i--, sp++)
	{
		if (sp->type == INST_GUS && sp->data != NULL)
		{
			free(sp->data);
		}
	}
	free(sample);
}

ToneBank::ToneBank()
{
	tone = new ToneBankElement[128];
	for (int i = 0; i < MAXPROG; ++i)
	{
		instrument[i] = 0;
	}
}

ToneBank::~ToneBank()
{
	delete[] tone;
	for (int i = 0; i < MAXPROG; i++)
	{
		if (instrument[i] != NULL && instrument[i] != MAGIC_LOAD_INSTRUMENT)
		{
			delete instrument[i];
			instrument[i] = NULL;
		}
	}
}

int convert_tremolo_sweep(Renderer *song, uint8_t sweep)
{
	if (sweep == 0)
		return 0;

	return
		int(((song->control_ratio * SWEEP_TUNING) << SWEEP_SHIFT) / (song->rate * sweep));
}

int convert_vibrato_sweep(Renderer *song, uint8_t sweep, int vib_control_ratio)
{
	if (sweep == 0)
		return 0;

	return
		(int) (FSCALE((double) (vib_control_ratio) * SWEEP_TUNING, SWEEP_SHIFT) / (song->rate * sweep));

	/* this was overflowing with seashore.pat

	((vib_control_ratio * SWEEP_TUNING) << SWEEP_SHIFT) / (song->rate * sweep);
	*/
}

int convert_tremolo_rate(Renderer *song, uint8_t rate)
{
	return
		int(((song->control_ratio * rate) << RATE_SHIFT) / (TREMOLO_RATE_TUNING * song->rate));
}

int convert_vibrato_rate(Renderer *song, uint8_t rate)
{
	/* Return a suitable vibrato_control_ratio value */
	return
		int((VIBRATO_RATE_TUNING * song->rate) / (rate * 2 * VIBRATO_SAMPLE_INCREMENTS));
}

static void reverse_data(sample_t *sp, int ls, int le)
{
	sample_t s, *ep = sp + le;
	sp += ls;
	le -= ls;
	le /= 2;
	while (le--)
	{
		    s = *sp;
		*sp++ = *ep;
		*ep-- = s;
	}
}

/*
	If panning or note_to_use != -1, it will be used for all samples,
	instead of the sample-specific values in the instrument file.

	For note_to_use, any value <0 or >127 will be forced to 0.

	For other parameters, 1 means yes, 0 means no, other values are
	undefined.

	TODO: do reverse loops right */
static Instrument *load_instrument(Renderer *song, const char *name, int percussion,
					int panning, int note_to_use,
					int strip_loop, int strip_envelope,
					int strip_tail)
{
	Instrument *ip;
	Sample *sp;
	FILE *fp;
	GF1PatchHeader header;
	GF1InstrumentData idata;
	GF1LayerData layer_data;
	GF1PatchData patch_data;
	int i, j;
	bool noluck = false;

	if (!name) return 0;

	/* Open patch file */
	if ((fp = open_file(name)) == NULL)
	{
		/* Try with various extensions */
            std::string tmp(name);
		tmp += ".pat";
		if ((fp = open_file(tmp.c_str())) == NULL)
		{
                    noluck = true;
		}
	}

	if (noluck)
	{
		cmsg(CMSG_ERROR, VERB_NORMAL, "Instrument `%s' can't be found.\n", name);
		return 0;
	}

	cmsg(CMSG_INFO, VERB_NOISY, "Loading instrument %s\n", name);

	/* Read some headers and do cursory sanity checks. */

	if (sizeof(header) != fread(&header, sizeof(header), 1, fp))
	{
failread:
		cmsg(CMSG_ERROR, VERB_NORMAL, "%s: Error reading instrument.\n", name);
		delete fp;
		return 0;
	}
	if (strncmp(header.Header, GF1_HEADER_TEXT, HEADER_SIZE - 4) != 0)
	{
		cmsg(CMSG_ERROR, VERB_NORMAL, "%s: Not an instrument.\n", name);
		delete fp;
		return 0;
	}
	if (strcmp(header.Header + 8, "110") < 0)
	{
		cmsg(CMSG_ERROR, VERB_NORMAL, "%s: Is an old and unsupported patch version.\n", name);
		delete fp;
		return 0;
	}
	if (sizeof(idata) != fread(&idata, sizeof(idata), 1, fp))
	{
		goto failread;
	}

	// header.WaveForms = LittleShort(header.WaveForms);
	// header.MasterVolume LittleShort(header.MasterVolume);
	// header.DataSize = LittleLong(header.DataSize);
	// idata.Instrument = LittleShort(idata.Instrument);

	if (header.Instruments != 1 && header.Instruments != 0) /* instruments. To some patch makers, 0 means 1 */
	{
		cmsg(CMSG_ERROR, VERB_NORMAL, "Can't handle patches with %d instruments.\n", header.Instruments);
		delete fp;
		return 0;
	}

	if (idata.Layers != 1 && idata.Layers != 0) /* layers. What's a layer? */
	{
		cmsg(CMSG_ERROR, VERB_NORMAL, "Can't handle instruments with %d layers.\n", idata.Layers);
		delete fp;
		return 0;
	}

	if (sizeof(layer_data) != fread(&layer_data, sizeof(layer_data), 1, fp))
	{
		goto failread;
	}

	if (layer_data.Samples == 0)
	{
		cmsg(CMSG_ERROR, VERB_NORMAL, "Instrument has 0 samples.\n");
		delete fp;
		return 0;
	}

	ip = new Instrument;
	ip->samples = layer_data.Samples;
	ip->sample = (Sample *)safe_malloc(sizeof(Sample) * layer_data.Samples);
	memset(ip->sample, 0, sizeof(Sample) * layer_data.Samples);
	for (i = 0; i < layer_data.Samples; ++i)
	{
            if (sizeof(patch_data) != fread(&patch_data, sizeof(patch_data), 1, fp))
		{
fail:
			cmsg(CMSG_ERROR, VERB_NORMAL, "Error reading sample %d.\n", i);
			delete ip;
			delete fp;
			return 0;
		}

		sp = &(ip->sample[i]);

		sp->data_length = patch_data.WaveSize; // LittleLong(patch_data.WaveSize);
		sp->loop_start = patch_data.StartLoop; // LittleLong(patch_data.StartLoop);
		sp->loop_end = patch_data.EndLoop; // LittleLong(patch_data.EndLoop);
		sp->sample_rate = patch_data.SampleRate; // LittleShort(patch_data.SampleRate);
		sp->low_freq = float(patch_data.LowFrequency); // LittleLong(patch_data.LowFrequency);
                    sp->high_freq = patch_data.HighFrequency; // float(LittleLong(patch_data.HighFrequency)) + 0.9999f;
		sp->root_freq = float(patch_data.RootFrequency); // LittleLong(patch_data.RootFrequency));
		sp->high_vel = 127;
		sp->velocity = -1;
		sp->type = INST_GUS;

		// Expand to SF2 range.
		if (panning == -1)
		{
			sp->panning = (patch_data.Balance & 0x0F) * 1000 / 15 - 500;
		}
		else
		{
			sp->panning = (panning & 0x7f) * 1000 / 127 - 500;
		}
		song->compute_pan((sp->panning + 500) / 1000.0, INST_GUS, sp->left_offset, sp->right_offset);

		/* tremolo */
		if (patch_data.TremoloRate == 0 || patch_data.TremoloDepth == 0)
		{
			sp->tremolo_sweep_increment = 0;
			sp->tremolo_phase_increment = 0;
			sp->tremolo_depth = 0;
			cmsg(CMSG_INFO, VERB_DEBUG, " * no tremolo\n");
		}
		else
		{
			sp->tremolo_sweep_increment = convert_tremolo_sweep(song, patch_data.TremoloSweep);
			sp->tremolo_phase_increment = convert_tremolo_rate(song, patch_data.TremoloRate);
			sp->tremolo_depth = patch_data.TremoloDepth;
			cmsg(CMSG_INFO, VERB_DEBUG, " * tremolo: sweep %d, phase %d, depth %d\n",
				sp->tremolo_sweep_increment, sp->tremolo_phase_increment, sp->tremolo_depth);
		}

		/* vibrato */
		if (patch_data.VibratoRate == 0 || patch_data.VibratoDepth == 0)
		{
			sp->vibrato_sweep_increment = 0;
			sp->vibrato_control_ratio = 0;
			sp->vibrato_depth = 0;
			cmsg(CMSG_INFO, VERB_DEBUG, " * no vibrato\n");
		}
		else
		{
			sp->vibrato_control_ratio = convert_vibrato_rate(song, patch_data.VibratoRate);
			sp->vibrato_sweep_increment = convert_vibrato_sweep(song, patch_data.VibratoSweep, sp->vibrato_control_ratio);
			sp->vibrato_depth = patch_data.VibratoDepth;
			cmsg(CMSG_INFO, VERB_DEBUG, " * vibrato: sweep %d, ctl %d, depth %d\n",
				sp->vibrato_sweep_increment, sp->vibrato_control_ratio, sp->vibrato_depth);
		}

		sp->modes = patch_data.Modes;

		/* Mark this as a fixed-pitch instrument if such a deed is desired. */
		if (note_to_use != -1)
		{
			sp->scale_note = note_to_use;
			sp->scale_factor = 0;
		}
		else
		{
			sp->scale_note = patch_data.ScaleFrequency; // LittleShort(patch_data.ScaleFrequency);
			sp->scale_factor = patch_data.ScaleFactor; // LittleShort(patch_data.ScaleFactor);
			if (sp->scale_factor <= 2)
			{
				sp->scale_factor *= 1024;
			}
			else if (sp->scale_factor > 2048)
			{
				sp->scale_factor = 1024;
			}
			if (sp->scale_factor != 1024)
			{
				cmsg(CMSG_INFO, VERB_DEBUG, " * Scale: note %d, factor %d\n",
					sp->scale_note, sp->scale_factor);
			}
		}

#if 0
		/* seashore.pat in the Midia patch set has no Sustain. I don't
		   understand why, and fixing it by adding the Sustain flag to
		   all looped patches probably breaks something else. We do it
		   anyway. */

		if (sp->modes & PATCH_LOOPEN)
		{
			sp->modes |= PATCH_SUSTAIN;
		}
#endif
		/* [RH] Alas, eawpats has percussion instruments with bad envelopes. :(
		 * (See cymchina.pat for one example of this sadness.)
		 * Do this logic for instruments without a description, only. Hopefully that
		 * catches all the patches that need it without including any extra.
		 */
		for (j = 0; j < DESC_SIZE; ++j)
		{
			if (header.Description[j] != 0)
				break;
		}
		/* Strip any loops and envelopes we're permitted to */
		/* [RH] (But PATCH_BACKWARD isn't a loop flag at all!) */
		if ((strip_loop == 1) &&
			(sp->modes & (PATCH_SUSTAIN | PATCH_LOOPEN | PATCH_BIDIR | PATCH_BACKWARD)))
		{
			cmsg(CMSG_INFO, VERB_DEBUG, " - Removing loop and/or sustain\n");
			if (j == DESC_SIZE)
			{
				sp->modes &= ~(PATCH_SUSTAIN | PATCH_LOOPEN | PATCH_BIDIR | PATCH_BACKWARD);
			}
		}

		if (strip_envelope == 1)
		{
			cmsg(CMSG_INFO, VERB_DEBUG, " - Removing envelope\n");
			/* [RH] The envelope isn't really removed, but this is the way the standard
			 * Gravis patches get that effect: All rates at maximum, and all offsets at
			 * a constant level.
			 */
			if (j == DESC_SIZE)
			{
				int k;
				for (k = 1; k < ENVELOPES; ++k)
				{ /* Find highest offset. */
					if (patch_data.EnvelopeOffset[k] > patch_data.EnvelopeOffset[0])
					{
						patch_data.EnvelopeOffset[0] = patch_data.EnvelopeOffset[k];
					}
				}
				for (k = 0; k < ENVELOPES; ++k)
				{
					patch_data.EnvelopeRate[k] = 63;
					patch_data.EnvelopeOffset[k] = patch_data.EnvelopeOffset[0];
				}
			}
		}

		for (j = 0; j < 6; j++)
		{
			sp->envelope.gf1.rate[j] = patch_data.EnvelopeRate[j];
			/* [RH] GF1NEW clamps the offsets to the range [5,251], so we do too. */
			sp->envelope.gf1.offset[j] = clamp<uint8_t>(patch_data.EnvelopeOffset[j], 5, 251);
		}

		/* Then read the sample data */
		if (((sp->modes & PATCH_16) && sp->data_length/2 > MAX_SAMPLE_SIZE) ||
			(!(sp->modes & PATCH_16) && sp->data_length > MAX_SAMPLE_SIZE))
		{
			goto fail;
		}
		sp->data = (sample_t *)safe_malloc(sp->data_length);

		if (sp->data_length != fread(sp->data, sp->data_length, 1, fp))
			goto fail;

		convert_sample_data(sp, sp->data);

		/* Reverse reverse loops and pass them off as normal loops */
		if (sp->modes & PATCH_BACKWARD)
		{
			int t;
			/* The GUS apparently plays reverse loops by reversing the
			   whole sample. We do the same because the GUS does not SUCK. */

			cmsg(CMSG_WARNING, VERB_NORMAL, "Reverse loop in %s\n", name);
			reverse_data((sample_t *)sp->data, 0, sp->data_length);
			sp->data[sp->data_length] = sp->data[sp->data_length - 1];

			t = sp->loop_start;
			sp->loop_start = sp->data_length - sp->loop_end;
			sp->loop_end = sp->data_length - t;

			sp->modes &= ~PATCH_BACKWARD;
			sp->modes |= PATCH_LOOPEN; /* just in case */
		}

		/* Then fractional samples */
		sp->data_length <<= FRACTION_BITS;
		sp->loop_start <<= FRACTION_BITS;
		sp->loop_end <<= FRACTION_BITS;

		/* Adjust for fractional loop points. */
		sp->loop_start |= (patch_data.Fractions & 0x0F) << (FRACTION_BITS-4);
		sp->loop_end   |= (patch_data.Fractions & 0xF0) << (FRACTION_BITS-4-4);

		/* If this instrument will always be played on the same note,
		   and it's not looped, we can resample it now. */
		if (sp->scale_factor == 0 && !(sp->modes & PATCH_LOOPEN))
		{
			pre_resample(song, sp);
		}

		if (strip_tail == 1)
		{
			/* Let's not really, just say we did. */
			cmsg(CMSG_INFO, VERB_DEBUG, " - Stripping tail\n");
			sp->data_length = sp->loop_end;
		}
	}
	delete fp;
	return ip;
}

void convert_sample_data(Sample *sp, const void *data)
{
	/* convert everything to 32-bit floating point data */
	sample_t *newdata = NULL;

	switch (sp->modes & (PATCH_16 | PATCH_UNSIGNED))
	{
	case 0:
	  {					/* 8-bit, signed */
		int8_t *cp = (int8_t *)data;
		newdata = (sample_t *)safe_malloc((sp->data_length + 1) * sizeof(sample_t));
		for (int i = 0; i < sp->data_length; ++i)
		{
			if (cp[i] < 0)
			{
				newdata[i] = float(cp[i]) / 128.f;
			}
			else
			{
				newdata[i] = float(cp[i]) / 127.f;
			}
		}
		break;
	  }

	case PATCH_UNSIGNED:
	  {					/* 8-bit, unsigned */
		uint8_t *cp = (uint8_t *)data;
		newdata = (sample_t *)safe_malloc((sp->data_length + 1) * sizeof(sample_t));
		for (int i = 0; i < sp->data_length; ++i)
		{
			int c = cp[i] - 128;
			if (c < 0)
			{
				newdata[i] = float(c) / 128.f;
			}
			else
			{
				newdata[i] = float(c) / 127.f;
			}
		}
		break;
	  }

	case PATCH_16:
	  {					/* 16-bit, signed */
		int16_t *cp = (int16_t *)data;
		/* Convert these to samples */
		sp->data_length >>= 1;
		sp->loop_start >>= 1;
		sp->loop_end >>= 1;
		newdata = (sample_t *)safe_malloc((sp->data_length + 1) * sizeof(sample_t));
		for (int i = 0; i < sp->data_length; ++i)
		{
                    int c = cp[i]; // LittleShort(cp[i]);
                        if (c < 0)
			{
				newdata[i] = float(c) / 32768.f;
			}
			else
			{
				newdata[i] = float(c) / 32767.f;
			}
		}
		break;
	  }

	case PATCH_16 | PATCH_UNSIGNED:
	  {					/* 16-bit, unsigned */
		uint16_t *cp = (uint16_t *)data;
		/* Convert these to samples */
		sp->data_length >>= 1;
		sp->loop_start >>= 1;
		sp->loop_end >>= 1;
		newdata = (sample_t *)safe_malloc((sp->data_length + 1) * sizeof(sample_t));
		for (int i = 0; i < sp->data_length; ++i)
		{
                    int c = cp[i] - 32768; // LittleShort(cp[i]) - 32768;
			if (c < 0)
			{
				newdata[i] = float(c) / 32768.f;
			}
			else
			{
				newdata[i] = float(c) / 32767.f;
			}
		}
		break;
	  }
	}
	/* Duplicate the final sample for linear interpolation. */
	newdata[sp->data_length] = newdata[sp->data_length - 1];
	if (sp->data != NULL)
	{
		free(sp->data);
	}
	sp->data = newdata;
}

static int fill_bank(Renderer *song, int dr, int b)
{
	int i, errors = 0;
	ToneBank *bank = ((dr) ? drumset[b] : tonebank[b]);
	if (bank == NULL)
	{
		cmsg(CMSG_ERROR, VERB_NORMAL,
			"Huh. Tried to load instruments in non-existent %s %d\n",
			(dr) ? "drumset" : "tone bank", b);
		return 0;
	}
	for (i = 0; i < MAXPROG; i++)
	{
		if (bank->instrument[i] == MAGIC_LOAD_INSTRUMENT)
		{
			bank->instrument[i] = NULL;
			bank->instrument[i] = load_instrument_dls(song, dr, b, i);
			if (bank->instrument[i] != NULL)
			{
				continue;
			}
			Instrument *ip;
			ip = load_instrument_font_order(song, 0, dr, b, i);
			if (ip == NULL)
			{
				if (bank->tone[i].fontbank >= 0)
				{
                                    ip = load_instrument_font(song, bank->tone[i].name.c_str(), dr, b, i);
				}
				else
				{
                                    ip = load_instrument(song, bank->tone[i].name.c_str(),
						(dr) ? 1 : 0,
						bank->tone[i].pan,
						(bank->tone[i].note != -1) ? bank->tone[i].note : ((dr) ? i : -1),
						(bank->tone[i].strip_loop != -1) ? bank->tone[i].strip_loop : ((dr) ? 1 : -1),
						(bank->tone[i].strip_envelope != -1) ? bank->tone[i].strip_envelope : ((dr) ? 1 : -1),
						bank->tone[i].strip_tail);
				}
				if (ip == NULL)
				{
					ip = load_instrument_font_order(song, 1, dr, b, i);
				}
			}
			bank->instrument[i] = ip;
			if (ip == NULL)
			{
				if (bank->tone[i].name.empty())
				{
					cmsg(CMSG_WARNING, (b != 0) ? VERB_VERBOSE : VERB_NORMAL,
						"No instrument mapped to %s %d, program %d%s\n",
						(dr) ? "drum set" : "tone bank", b, i,
						(b != 0) ? "" : " - this instrument will not be heard");
				}
				else
				{
					cmsg(CMSG_ERROR, VERB_NORMAL,
						"Couldn't load instrument %s (%s %d, program %d)\n",
                                             bank->tone[i].name.c_str(),
						(dr) ? "drum set" : "tone bank", b, i);
				}
				if (b != 0)
				{
					/* Mark the corresponding instrument in the default
					   bank / drumset for loading (if it isn't already) */
					if (((dr) ? drumset[0] : tonebank[0])->instrument[i] != NULL)
					{
						((dr) ? drumset[0] : tonebank[0])->instrument[i] = MAGIC_LOAD_INSTRUMENT;
					}
				}
				errors++;
			}
		}
	}
	return errors;
}

int Renderer::load_missing_instruments()
{
	int i = MAXBANK, errors = 0;
	while (i--)
	{
		if (tonebank[i] != NULL)
			errors += fill_bank(this, 0,i);
		if (drumset[i] != NULL)
			errors += fill_bank(this, 1,i);
	}
	return errors;
}

void free_instruments()
{
	int i = MAXBANK;
	while (i--)
	{
		if (tonebank[i] != NULL)
		{
			delete tonebank[i];
			tonebank[i] = NULL;
		}
		if (drumset[i] != NULL)
		{
			delete drumset[i];
			drumset[i] = NULL;
		}
	}
}

int Renderer::set_default_instrument(const char *name)
{
	Instrument *ip;
	if ((ip = load_instrument(this, name, 0, -1, -1, 0, 0, 0)) == NULL)
	{
		return -1;
	}
	if (default_instrument != NULL)
	{
		delete default_instrument;
	}
	default_instrument = ip;
	default_program = SPECIAL_PROGRAM;
	return 0;
}

}
