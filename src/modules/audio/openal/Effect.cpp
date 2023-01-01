/**
 * Copyright (c) 2006-2023 LOVE Development Team
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

#include "Effect.h"
#include "common/Exception.h"

#include <cmath>
#include <iostream>

namespace love
{
namespace audio
{
namespace openal
{

//base class
Effect::Effect()
{
	generateEffect();
}

Effect::Effect(const Effect &s)
	: Effect()
{
	setParams(s.getParams());
}

Effect::~Effect()
{
	deleteEffect();
}

Effect *Effect::clone()
{
	return new Effect(*this);
}

bool Effect::generateEffect()
{
#ifdef ALC_EXT_EFX
	if (!alGenEffects)
		return false;

	if (effect != AL_EFFECT_NULL)
		return true;

	alGenEffects(1, &effect);
	if (alGetError() != AL_NO_ERROR)
		throw love::Exception("Failed to create sound Effect.");

	return true;
#else
	return false;
#endif
}

void Effect::deleteEffect()
{
#ifdef ALC_EXT_EFX
	if (effect != AL_EFFECT_NULL)
		alDeleteEffects(1, &effect);
#endif
	effect = AL_EFFECT_NULL;
}

ALuint Effect::getEffect() const
{
	return effect;
}

bool Effect::setParams(const std::map<Parameter, float> &params)
{
	this->params = params;
	type = (Type)(int) this->params[EFFECT_TYPE];

	if (!generateEffect())
		return false;

#ifdef ALC_EXT_EFX
	//parameter table without EFFECT_TYPE entry is illegal
	switch (type)
	{
	case TYPE_REVERB:
		alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
		break;
	case TYPE_CHORUS:
		alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_CHORUS);
		break;
	case TYPE_DISTORTION:
		alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_DISTORTION);
		break;
	case TYPE_ECHO:
		alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_ECHO);
		break;
	case TYPE_FLANGER:
		alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_FLANGER);
		break;
/*
	case TYPE_FREQSHIFTER:
		alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_FREQUENCY_SHIFTER);
		break;
	case TYPE_MORPHER:
		alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_VOCAL_MORPHER);
		break;
	case TYPE_PITCHSHIFTER:
		alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_PITCH_SHIFTER);
		break;
*/
	case TYPE_MODULATOR:
		alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_RING_MODULATOR);
		break;
/*
	case TYPE_AUTOWAH:
		alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_AUTOWAH);
		break;
*/
	case TYPE_COMPRESSOR:
		alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_COMPRESSOR);
		break;
	case TYPE_EQUALIZER:
		alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_EQUALIZER);
		break;
	case TYPE_BASIC:
	case TYPE_MAX_ENUM:
		break;
	}

	//failed to make effect specific type - not supported etc.
	if (alGetError() != AL_NO_ERROR)
	{
		deleteEffect();
		return false;
	}

#define clampf(v,l,h) fmax(fmin((v),(h)),(l))
#define PARAMSTR(i,e,v) effect,AL_##e##_##v,clampf(getValue(i,AL_##e##_DEFAULT_##v),AL_##e##_MIN_##v,AL_##e##_MAX_##v)
	switch (type)
	{
	case TYPE_REVERB:
	{
		alEffectf(PARAMSTR(REVERB_GAIN,REVERB,GAIN));
		alEffectf(PARAMSTR(REVERB_HFGAIN,REVERB,GAINHF));
		alEffectf(PARAMSTR(REVERB_DENSITY,REVERB,DENSITY));
		alEffectf(PARAMSTR(REVERB_DIFFUSION,REVERB,DIFFUSION));
		alEffectf(PARAMSTR(REVERB_DECAY,REVERB,DECAY_TIME));
		alEffectf(PARAMSTR(REVERB_HFDECAY,REVERB,DECAY_HFRATIO));
		alEffectf(PARAMSTR(REVERB_EARLYGAIN,REVERB,REFLECTIONS_GAIN));
		alEffectf(PARAMSTR(REVERB_EARLYDELAY,REVERB,REFLECTIONS_DELAY));
		alEffectf(PARAMSTR(REVERB_LATEGAIN,REVERB,LATE_REVERB_GAIN));
		alEffectf(PARAMSTR(REVERB_LATEDELAY,REVERB,LATE_REVERB_DELAY));;
		alEffectf(PARAMSTR(REVERB_ROLLOFF,REVERB,ROOM_ROLLOFF_FACTOR));
		alEffectf(PARAMSTR(REVERB_AIRHFGAIN,REVERB,AIR_ABSORPTION_GAINHF));
		alEffecti(effect, AL_REVERB_DECAY_HFLIMIT, getValue(REVERB_HFLIMITER, 0));
		break;
	}
	case TYPE_CHORUS:
	{
		Waveform wave = static_cast<Waveform>(getValue(CHORUS_WAVEFORM, static_cast<int>(WAVE_MAX_ENUM)));
		if (wave == WAVE_SINE)
			alEffecti(effect, AL_CHORUS_WAVEFORM, AL_CHORUS_WAVEFORM_SINUSOID);
		else if (wave == WAVE_TRIANGLE)
			alEffecti(effect, AL_CHORUS_WAVEFORM, AL_CHORUS_WAVEFORM_TRIANGLE);
		else
			alEffecti(effect, AL_CHORUS_WAVEFORM, AL_CHORUS_DEFAULT_WAVEFORM);

		alEffecti(PARAMSTR(CHORUS_PHASE,CHORUS,PHASE));
		alEffectf(PARAMSTR(CHORUS_RATE,CHORUS,RATE));
		alEffectf(PARAMSTR(CHORUS_DEPTH,CHORUS,DEPTH));
		alEffectf(PARAMSTR(CHORUS_FEEDBACK,CHORUS,FEEDBACK));
		alEffectf(PARAMSTR(CHORUS_DELAY,CHORUS,DELAY));
		break;
	}
	case TYPE_DISTORTION:
		alEffectf(PARAMSTR(DISTORTION_GAIN,DISTORTION,GAIN));
		alEffectf(PARAMSTR(DISTORTION_EDGE,DISTORTION,EDGE));
		alEffectf(PARAMSTR(DISTORTION_LOWCUT,DISTORTION,LOWPASS_CUTOFF));
		alEffectf(PARAMSTR(DISTORTION_EQCENTER,DISTORTION,EQCENTER));
		alEffectf(PARAMSTR(DISTORTION_EQBAND,DISTORTION,EQBANDWIDTH));
		break;

	case TYPE_ECHO:
		alEffectf(PARAMSTR(ECHO_DELAY,ECHO,DELAY));
		alEffectf(PARAMSTR(ECHO_LRDELAY,ECHO,LRDELAY));
		alEffectf(PARAMSTR(ECHO_DAMPING,ECHO,DAMPING));
		alEffectf(PARAMSTR(ECHO_FEEDBACK,ECHO,FEEDBACK));
		alEffectf(PARAMSTR(ECHO_SPREAD,ECHO,SPREAD));
		break;

	case TYPE_FLANGER:
	{
		Waveform wave = static_cast<Waveform>(getValue(FLANGER_WAVEFORM, static_cast<int>(WAVE_MAX_ENUM)));
		if (wave == WAVE_SINE)
			alEffecti(effect, AL_FLANGER_WAVEFORM, AL_FLANGER_WAVEFORM_SINUSOID);
		else if (wave == WAVE_TRIANGLE)
			alEffecti(effect, AL_FLANGER_WAVEFORM, AL_FLANGER_WAVEFORM_TRIANGLE);
		else
			alEffecti(effect, AL_FLANGER_WAVEFORM, AL_FLANGER_DEFAULT_WAVEFORM);

		alEffecti(PARAMSTR(FLANGER_PHASE,FLANGER,PHASE));
		alEffectf(PARAMSTR(FLANGER_RATE,FLANGER,RATE));
		alEffectf(PARAMSTR(FLANGER_DEPTH,FLANGER,DEPTH));
		alEffectf(PARAMSTR(FLANGER_FEEDBACK,FLANGER,FEEDBACK));
		alEffectf(PARAMSTR(FLANGER_DELAY,FLANGER,DELAY));
		break;
	}
/*
	case TYPE_FREQSHIFTER:
	{
		alEffectf(PARAMSTR(FREQSHIFTER_FREQ,FREQUENCY_SHIFTER,FREQUENCY));

		Direction dir = static_cast<Direction>(getValue(FREQSHIFTER_LEFTDIR, static_cast<int>(DIR_MAX_ENUM)));
		if (dir == DIR_NONE)
			alEffecti(effect, AL_FREQUENCY_SHIFTER_LEFT_DIRECTION, AL_FREQUENCY_SHIFTER_DIRECTION_OFF);
		else if(dir == DIR_UP)
			alEffecti(effect, AL_FREQUENCY_SHIFTER_LEFT_DIRECTION, AL_FREQUENCY_SHIFTER_DIRECTION_UP);
		else if(dir == DIR_DOWN)
			alEffecti(effect, AL_FREQUENCY_SHIFTER_LEFT_DIRECTION, AL_FREQUENCY_SHIFTER_DIRECTION_DOWN);
		else
			alEffecti(effect, AL_FREQUENCY_SHIFTER_LEFT_DIRECTION, AL_FREQUENCY_SHIFTER_DEFAULT_LEFT_DIRECTION);

		dir = static_cast<Direction>(getValue(FREQSHIFTER_RIGHTDIR, static_cast<int>(DIR_MAX_ENUM)));
		if (dir == DIR_NONE)
			alEffecti(effect, AL_FREQUENCY_SHIFTER_RIGHT_DIRECTION, AL_FREQUENCY_SHIFTER_DIRECTION_OFF);
		else if(dir == DIR_UP)
			alEffecti(effect, AL_FREQUENCY_SHIFTER_RIGHT_DIRECTION, AL_FREQUENCY_SHIFTER_DIRECTION_UP);
		else if(dir == DIR_DOWN)
			alEffecti(effect, AL_FREQUENCY_SHIFTER_RIGHT_DIRECTION, AL_FREQUENCY_SHIFTER_DIRECTION_DOWN);
		else
			alEffecti(effect, AL_FREQUENCY_SHIFTER_RIGHT_DIRECTION, AL_FREQUENCY_SHIFTER_DEFAULT_RIGHT_DIRECTION);
		break;
	}
	case TYPE_MORPHER:
	{
		Waveform wave = static_cast<Waveform>(getValue(MORPHER_WAVEFORM, static_cast<int>(WAVE_MAX_ENUM)));
		if (wave == WAVE_SINE)
			alEffecti(effect, AL_VOCAL_MORPHER_WAVEFORM, AL_VOCAL_MORPHER_WAVEFORM_SINUSOID);
		else if (wave == WAVE_TRIANGLE)
			alEffecti(effect, AL_VOCAL_MORPHER_WAVEFORM, AL_VOCAL_MORPHER_WAVEFORM_TRIANGLE);
		else if (wave == WAVE_SAWTOOTH)
			alEffecti(effect, AL_VOCAL_MORPHER_WAVEFORM, AL_VOCAL_MORPHER_WAVEFORM_SAWTOOTH);
		else
			alEffecti(effect, AL_VOCAL_MORPHER_WAVEFORM, AL_VOCAL_MORPHER_DEFAULT_WAVEFORM);

		Phoneme phoneme = static_cast<Phoneme>(getValue(MORPHER_PHONEMEA, static_cast<int>(PHONEME_MAX_ENUM)));
		if (phoneme == PHONEME_MAX_ENUM)
			alEffecti(effect, AL_VOCAL_MORPHER_PHONEMEA, AL_VOCAL_MORPHER_DEFAULT_PHONEMEA);
		else
			alEffecti(effect, AL_VOCAL_MORPHER_PHONEMEA, phonemeMap[phoneme]);

		phoneme = static_cast<Phoneme>(getValue(MORPHER_PHONEMEB, static_cast<int>(PHONEME_MAX_ENUM)));
		if (phoneme == PHONEME_MAX_ENUM)
			alEffecti(effect, AL_VOCAL_MORPHER_PHONEMEB, AL_VOCAL_MORPHER_DEFAULT_PHONEMEB);
		else
			alEffecti(effect, AL_VOCAL_MORPHER_PHONEMEB, phonemeMap[phoneme]);

		alEffectf(PARAMSTR(MORPHER_RATE,VOCAL_MORPHER,RATE));
		alEffecti(PARAMSTR(MORPHER_TUNEA,VOCAL_MORPHER,PHONEMEA_COARSE_TUNING));
		alEffecti(PARAMSTR(MORPHER_TUNEB,VOCAL_MORPHER,PHONEMEB_COARSE_TUNING));
		break;
	}
	case TYPE_PITCHSHIFTER:
	{
		float tune = getValue(PITCHSHIFTER_PITCH, (float)AL_PITCH_SHIFTER_DEFAULT_COARSE_TUNE + (float)(AL_PITCH_SHIFTER_DEFAULT_FINE_TUNE - 50) / 100.0 );

		int coarse = (int)floor(tune);
		int fine = (int)(fmod(tune, 1.0)*100.0);
		if (fine > 50)
		{
			fine -= 100;
			coarse += 1;
		}
		else if (fine < -50)
		{
			fine += 100;
			coarse -= 1;
		}
		if (coarse > AL_PITCH_SHIFTER_MAX_COARSE_TUNE)
		{
			coarse = AL_PITCH_SHIFTER_MAX_COARSE_TUNE;
			fine = AL_PITCH_SHIFTER_MAX_FINE_TUNE;
		}
		else if (coarse < AL_PITCH_SHIFTER_MIN_COARSE_TUNE)
		{
			coarse = AL_PITCH_SHIFTER_MIN_COARSE_TUNE;
			fine = AL_PITCH_SHIFTER_MIN_FINE_TUNE;
		}
		alEffecti(effect, AL_PITCH_SHIFTER_COARSE_TUNE, coarse);
		alEffecti(effect, AL_PITCH_SHIFTER_FINE_TUNE, fine);
		break;
	}
*/
	case TYPE_MODULATOR:
	{
		Waveform wave = static_cast<Waveform>(getValue(MODULATOR_WAVEFORM,static_cast<int>(WAVE_MAX_ENUM)));
		if (wave == WAVE_SINE)
			alEffecti(effect, AL_RING_MODULATOR_WAVEFORM, AL_RING_MODULATOR_SINUSOID);
		else if (wave == WAVE_SAWTOOTH)
			alEffecti(effect, AL_RING_MODULATOR_WAVEFORM, AL_RING_MODULATOR_SAWTOOTH);
		else if (wave == WAVE_SQUARE)
			alEffecti(effect, AL_RING_MODULATOR_WAVEFORM, AL_RING_MODULATOR_SQUARE);
		else
			alEffecti(effect, AL_RING_MODULATOR_WAVEFORM, AL_RING_MODULATOR_DEFAULT_WAVEFORM);

		alEffectf(PARAMSTR(MODULATOR_FREQ,RING_MODULATOR,FREQUENCY));
		alEffectf(PARAMSTR(MODULATOR_HIGHCUT,RING_MODULATOR,HIGHPASS_CUTOFF));
		break;
	}
/*
	case TYPE_AUTOWAH:
		alEffectf(PARAMSTR(AUTOWAH_ATTACK,AUTOWAH,ATTACK_TIME));
		alEffectf(PARAMSTR(AUTOWAH_RELEASE,AUTOWAH,RELEASE_TIME));
		alEffectf(PARAMSTR(AUTOWAH_RESONANCE,AUTOWAH,RESONANCE));
		alEffectf(PARAMSTR(AUTOWAH_PEAKGAIN,AUTOWAH,PEAK_GAIN));
		break;
*/
	case TYPE_COMPRESSOR:
		alEffecti(effect, AL_COMPRESSOR_ONOFF, getValue(COMPRESSOR_ENABLE,static_cast<int>(AL_COMPRESSOR_DEFAULT_ONOFF)));
		break;

	case TYPE_EQUALIZER:
		alEffectf(PARAMSTR(EQUALIZER_LOWGAIN,EQUALIZER,LOW_GAIN));
		alEffectf(PARAMSTR(EQUALIZER_LOWCUT,EQUALIZER,LOW_CUTOFF));
		alEffectf(PARAMSTR(EQUALIZER_MID1GAIN,EQUALIZER,MID1_GAIN));
		alEffectf(PARAMSTR(EQUALIZER_MID1FREQ,EQUALIZER,MID1_CENTER));
		alEffectf(PARAMSTR(EQUALIZER_MID1BAND,EQUALIZER,MID1_WIDTH));
		alEffectf(PARAMSTR(EQUALIZER_MID2GAIN,EQUALIZER,MID2_GAIN));
		alEffectf(PARAMSTR(EQUALIZER_MID2FREQ,EQUALIZER,MID2_CENTER));
		alEffectf(PARAMSTR(EQUALIZER_MID2BAND,EQUALIZER,MID2_WIDTH));
		alEffectf(PARAMSTR(EQUALIZER_HIGHGAIN,EQUALIZER,HIGH_GAIN));
		alEffectf(PARAMSTR(EQUALIZER_HIGHCUT,EQUALIZER,HIGH_CUTOFF));
		break;
	case TYPE_BASIC:
	case TYPE_MAX_ENUM:
		break;
	}
#undef PARAMSTR
#undef clampf
	//alGetError();

	return true;
#else
	return false;
#endif //ALC_EXT_EFX
}

const std::map<Effect::Parameter, float> &Effect::getParams() const
{
	return params;
}

float Effect::getValue(Parameter in, float def) const
{
	return params.find(in) == params.end() ? def : params.at(in);
}

int Effect::getValue(Parameter in, int def) const
{
	return params.find(in) == params.end() ? def : static_cast<int>(params.at(in));
}

/*
std::map<Effect::Phoneme, ALint> Effect::phonemeMap =
{
	{Effect::PHONEME_A, AL_VOCAL_MORPHER_PHONEME_A},
	{Effect::PHONEME_E, AL_VOCAL_MORPHER_PHONEME_E},
	{Effect::PHONEME_I, AL_VOCAL_MORPHER_PHONEME_I},
	{Effect::PHONEME_O, AL_VOCAL_MORPHER_PHONEME_O},
	{Effect::PHONEME_U, AL_VOCAL_MORPHER_PHONEME_U},
	{Effect::PHONEME_AA, AL_VOCAL_MORPHER_PHONEME_AA},
	{Effect::PHONEME_AE, AL_VOCAL_MORPHER_PHONEME_AE},
	{Effect::PHONEME_AH, AL_VOCAL_MORPHER_PHONEME_AH},
	{Effect::PHONEME_AO, AL_VOCAL_MORPHER_PHONEME_AO},
	{Effect::PHONEME_EH, AL_VOCAL_MORPHER_PHONEME_EH},
	{Effect::PHONEME_ER, AL_VOCAL_MORPHER_PHONEME_ER},
	{Effect::PHONEME_IH, AL_VOCAL_MORPHER_PHONEME_IH},
	{Effect::PHONEME_IY, AL_VOCAL_MORPHER_PHONEME_IY},
	{Effect::PHONEME_UH, AL_VOCAL_MORPHER_PHONEME_UH},
	{Effect::PHONEME_UW, AL_VOCAL_MORPHER_PHONEME_UW},
	{Effect::PHONEME_B, AL_VOCAL_MORPHER_PHONEME_B},
	{Effect::PHONEME_D, AL_VOCAL_MORPHER_PHONEME_D},
	{Effect::PHONEME_F, AL_VOCAL_MORPHER_PHONEME_F},
	{Effect::PHONEME_G, AL_VOCAL_MORPHER_PHONEME_G},
	{Effect::PHONEME_J, AL_VOCAL_MORPHER_PHONEME_J},
	{Effect::PHONEME_K, AL_VOCAL_MORPHER_PHONEME_K},
	{Effect::PHONEME_L, AL_VOCAL_MORPHER_PHONEME_L},
	{Effect::PHONEME_M, AL_VOCAL_MORPHER_PHONEME_M},
	{Effect::PHONEME_N, AL_VOCAL_MORPHER_PHONEME_N},
	{Effect::PHONEME_P, AL_VOCAL_MORPHER_PHONEME_P},
	{Effect::PHONEME_R, AL_VOCAL_MORPHER_PHONEME_R},
	{Effect::PHONEME_S, AL_VOCAL_MORPHER_PHONEME_S},
	{Effect::PHONEME_T, AL_VOCAL_MORPHER_PHONEME_T},
	{Effect::PHONEME_V, AL_VOCAL_MORPHER_PHONEME_V},
	{Effect::PHONEME_Z, AL_VOCAL_MORPHER_PHONEME_Z}
};
*/

} //openal
} //audio
} //love
