/**
 * Copyright (c) 2006-2016 LOVE Development Team
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
	setParams(s.getType(), s.getParams());
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

bool Effect::setParams(Type type, const std::vector<float> &params)
{
	this->type = type;
	this->params = params;

	if (!generateEffect())
		return false;

	#ifdef ALC_EXT_EFX
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
	case TYPE_FREQSHIFTER:
		alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_FREQUENCY_SHIFTER);
		break;
	case TYPE_MORPHER:
		alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_VOCAL_MORPHER);
		break;
	case TYPE_PITCHSHIFTER:
		alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_PITCH_SHIFTER);
		break;
	case TYPE_MODULATOR:
		alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_RING_MODULATOR);
		break;
	case TYPE_AUTOWAH:
		alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_AUTOWAH);
		break;
	case TYPE_COMPRESSOR:
		alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_COMPRESSOR);
		break;
	case TYPE_EQUALIZER:
		alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_EQUALIZER);
		break;
	case TYPE_MAX_ENUM:
		break;
	}

	//failed to make effect specific type - not supported etc.
	if (alGetError() != AL_NO_ERROR)
	{
		deleteEffect();
		return false;
	}

	#define PARAMSTR(i,e,v) effect, AL_ ## e ## _ ## v, clampf(params[(i)], AL_ ## e ## _MIN_ ## v, AL_ ## e ## _MAX_ ## v, AL_ ## e ## _DEFAULT_ ## v)
	switch (type)
	{
	case TYPE_REVERB:
	{
		alEffectf(PARAMSTR(1,REVERB,GAIN));
		alEffectf(PARAMSTR(2,REVERB,GAINHF));
		alEffectf(PARAMSTR(3,REVERB,DENSITY));
		alEffectf(PARAMSTR(4,REVERB,DIFFUSION));
		alEffectf(PARAMSTR(5,REVERB,DECAY_TIME));
		alEffectf(PARAMSTR(6,REVERB,DECAY_HFRATIO));
		alEffectf(PARAMSTR(7,REVERB,REFLECTIONS_GAIN));
		alEffectf(PARAMSTR(8,REVERB,REFLECTIONS_DELAY));
		alEffectf(PARAMSTR(9,REVERB,LATE_REVERB_GAIN));
		alEffectf(PARAMSTR(10,REVERB,LATE_REVERB_DELAY));;
		alEffectf(PARAMSTR(11,REVERB,ROOM_ROLLOFF_FACTOR));
		alEffectf(PARAMSTR(12,REVERB,AIR_ABSORPTION_GAINHF));
		alEffecti(effect, AL_REVERB_DECAY_HFLIMIT, params[13] < 0.5 ? AL_FALSE : AL_TRUE);
		break;
	}
	case TYPE_CHORUS:
	{
		Effect::Waveform wave = static_cast<Effect::Waveform>(params[1]);
		if (wave == Effect::WAVE_SINE)
			alEffecti(effect, AL_CHORUS_WAVEFORM, AL_CHORUS_WAVEFORM_SINUSOID);
		else if (wave == Effect::WAVE_TRIANGLE)
			alEffecti(effect, AL_CHORUS_WAVEFORM, AL_CHORUS_WAVEFORM_TRIANGLE);
		else
			alEffecti(effect, AL_CHORUS_WAVEFORM, AL_CHORUS_DEFAULT_WAVEFORM);

		alEffecti(PARAMSTR(2,CHORUS,PHASE));
		alEffectf(PARAMSTR(3,CHORUS,RATE));
		alEffectf(PARAMSTR(4,CHORUS,DEPTH));
		alEffectf(PARAMSTR(5,CHORUS,FEEDBACK));
		alEffectf(PARAMSTR(6,CHORUS,DELAY));
		break;
	}
	case TYPE_DISTORTION:
		alEffectf(PARAMSTR(1,DISTORTION,GAIN));
		alEffectf(PARAMSTR(2,DISTORTION,EDGE));
		alEffectf(PARAMSTR(3,DISTORTION,LOWPASS_CUTOFF));
		alEffectf(PARAMSTR(4,DISTORTION,EQCENTER));
		alEffectf(PARAMSTR(5,DISTORTION,EQBANDWIDTH));
		break;

	case TYPE_ECHO:
		alEffectf(PARAMSTR(1,ECHO,DELAY));
		alEffectf(PARAMSTR(2,ECHO,LRDELAY));
		alEffectf(PARAMSTR(3,ECHO,DAMPING));
		alEffectf(PARAMSTR(4,ECHO,FEEDBACK));
		alEffectf(PARAMSTR(5,ECHO,SPREAD));
		break;

	case TYPE_FLANGER:
	{
		Effect::Waveform wave = static_cast<Effect::Waveform>(params[1]);
		if (wave == Effect::WAVE_SINE)
			alEffecti(effect, AL_FLANGER_WAVEFORM, AL_FLANGER_WAVEFORM_SINUSOID);
		else if (wave == Effect::WAVE_TRIANGLE)
			alEffecti(effect, AL_FLANGER_WAVEFORM, AL_FLANGER_WAVEFORM_TRIANGLE);
		else
			alEffecti(effect, AL_FLANGER_WAVEFORM, AL_FLANGER_DEFAULT_WAVEFORM);

		alEffecti(PARAMSTR(2,FLANGER,PHASE));
		alEffectf(PARAMSTR(3,FLANGER,RATE));
		alEffectf(PARAMSTR(4,FLANGER,DEPTH));
		alEffectf(PARAMSTR(5,FLANGER,FEEDBACK));
		alEffectf(PARAMSTR(6,FLANGER,DELAY));
		break;
	}
	case TYPE_FREQSHIFTER:
	{
		alEffectf(PARAMSTR(1,FREQUENCY_SHIFTER,FREQUENCY)); 

		Effect::Direction dir = static_cast<Effect::Direction>(params[2]);
		if (dir == Effect::DIR_NONE)
			alEffecti(effect, AL_FREQUENCY_SHIFTER_LEFT_DIRECTION, AL_FREQUENCY_SHIFTER_DIRECTION_OFF);
		else if(dir == Effect::DIR_UP)
			alEffecti(effect, AL_FREQUENCY_SHIFTER_LEFT_DIRECTION, AL_FREQUENCY_SHIFTER_DIRECTION_UP);
		else if(dir == Effect::DIR_DOWN)
			alEffecti(effect, AL_FREQUENCY_SHIFTER_LEFT_DIRECTION, AL_FREQUENCY_SHIFTER_DIRECTION_DOWN);
		else 
			alEffecti(effect, AL_FREQUENCY_SHIFTER_LEFT_DIRECTION, AL_FREQUENCY_SHIFTER_DEFAULT_LEFT_DIRECTION);

		dir = static_cast<Effect::Direction>(params[3]);
		if (dir == Effect::DIR_NONE)
			alEffecti(effect, AL_FREQUENCY_SHIFTER_RIGHT_DIRECTION, AL_FREQUENCY_SHIFTER_DIRECTION_OFF);
		else if(dir == Effect::DIR_UP)
			alEffecti(effect, AL_FREQUENCY_SHIFTER_RIGHT_DIRECTION, AL_FREQUENCY_SHIFTER_DIRECTION_UP);
		else if(dir == Effect::DIR_DOWN)
			alEffecti(effect, AL_FREQUENCY_SHIFTER_RIGHT_DIRECTION, AL_FREQUENCY_SHIFTER_DIRECTION_DOWN);
		else 
			alEffecti(effect, AL_FREQUENCY_SHIFTER_RIGHT_DIRECTION, AL_FREQUENCY_SHIFTER_DEFAULT_RIGHT_DIRECTION);
		break;
	}
	case TYPE_MORPHER:
	{
		Effect::Waveform wave = static_cast<Effect::Waveform>(params[1]);
		if (wave == Effect::WAVE_SINE)
			alEffecti(effect, AL_VOCAL_MORPHER_WAVEFORM, AL_VOCAL_MORPHER_WAVEFORM_SINUSOID);
		else if (wave == Effect::WAVE_TRIANGLE)
			alEffecti(effect, AL_VOCAL_MORPHER_WAVEFORM, AL_VOCAL_MORPHER_WAVEFORM_TRIANGLE);
		else if (wave == Effect::WAVE_SAWTOOTH)
			alEffecti(effect, AL_VOCAL_MORPHER_WAVEFORM, AL_VOCAL_MORPHER_WAVEFORM_SAWTOOTH);
		else
			alEffecti(effect, AL_VOCAL_MORPHER_WAVEFORM, AL_VOCAL_MORPHER_DEFAULT_WAVEFORM);

		alEffectf(PARAMSTR(2,VOCAL_MORPHER,RATE));

		if (isnanf(params[3]))
			alEffecti(effect, AL_VOCAL_MORPHER_PHONEMEA, AL_VOCAL_MORPHER_DEFAULT_PHONEMEA);
		else
			alEffecti(effect, AL_VOCAL_MORPHER_PHONEMEA, phonemeMap[static_cast<Effect::Phoneme>(params[2])]);

		if (isnanf(params[4]))
			alEffecti(effect, AL_VOCAL_MORPHER_PHONEMEB, AL_VOCAL_MORPHER_DEFAULT_PHONEMEB);
		else
			alEffecti(effect, AL_VOCAL_MORPHER_PHONEMEB, phonemeMap[static_cast<Effect::Phoneme>(params[3])]);

		alEffecti(PARAMSTR(5,VOCAL_MORPHER,PHONEMEA_COARSE_TUNING));
		alEffecti(PARAMSTR(6,VOCAL_MORPHER,PHONEMEB_COARSE_TUNING));
		break;
	}
	case TYPE_PITCHSHIFTER:
	{
		int coarse = AL_PITCH_SHIFTER_DEFAULT_COARSE_TUNE;
		int fine = AL_PITCH_SHIFTER_DEFAULT_FINE_TUNE;
		if (!isnanf(params[1]))
		{
			coarse = (int)floor(params[1]);
			fine = (int)(fmod(params[1], 1.0)*100.0);
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
		}
		alEffecti(effect, AL_PITCH_SHIFTER_COARSE_TUNE, coarse);
		alEffecti(effect, AL_PITCH_SHIFTER_FINE_TUNE, fine);
		break;
	}

	case TYPE_MODULATOR:
	{
		Effect::Waveform wave = static_cast<Effect::Waveform>(params[1]);
		if (wave == Effect::WAVE_SINE)
			alEffecti(effect, AL_RING_MODULATOR_WAVEFORM, AL_RING_MODULATOR_SINUSOID);
		else if (wave == Effect::WAVE_SAWTOOTH)
			alEffecti(effect, AL_RING_MODULATOR_WAVEFORM, AL_RING_MODULATOR_SAWTOOTH);
		else if (wave == Effect::WAVE_SQUARE)
			alEffecti(effect, AL_RING_MODULATOR_WAVEFORM, AL_RING_MODULATOR_SQUARE);
		else
			alEffecti(effect, AL_RING_MODULATOR_WAVEFORM, AL_RING_MODULATOR_DEFAULT_WAVEFORM);

		alEffectf(PARAMSTR(2,RING_MODULATOR,FREQUENCY));
		alEffectf(PARAMSTR(3,RING_MODULATOR,HIGHPASS_CUTOFF));
		break;
	}
	case TYPE_AUTOWAH:
		alEffectf(PARAMSTR(1,AUTOWAH,ATTACK_TIME));
		alEffectf(PARAMSTR(2,AUTOWAH,RELEASE_TIME));
		alEffectf(PARAMSTR(3,AUTOWAH,RESONANCE));
		alEffectf(PARAMSTR(4,AUTOWAH,PEAK_GAIN));
		break;

	case TYPE_COMPRESSOR:
		alEffecti(effect, AL_COMPRESSOR_ONOFF, params[1] < 0.5 ? 0 : 1);
		break;

	case TYPE_EQUALIZER:
		alEffectf(PARAMSTR(1,EQUALIZER,LOW_GAIN));
		alEffectf(PARAMSTR(2,EQUALIZER,LOW_CUTOFF));
		alEffectf(PARAMSTR(3,EQUALIZER,MID1_GAIN));
		alEffectf(PARAMSTR(4,EQUALIZER,MID1_CENTER));
		alEffectf(PARAMSTR(5,EQUALIZER,MID1_WIDTH));
		alEffectf(PARAMSTR(6,EQUALIZER,MID2_GAIN));
		alEffectf(PARAMSTR(7,EQUALIZER,MID2_CENTER));
		alEffectf(PARAMSTR(8,EQUALIZER,MID2_WIDTH));
		alEffectf(PARAMSTR(9,EQUALIZER,HIGH_GAIN));
		alEffectf(PARAMSTR(10,EQUALIZER,HIGH_CUTOFF));
		break;

	case TYPE_MAX_ENUM:
		break;
	}
	#undef PARAMSTR
	//alGetError();

	return true;
	#else
	return false;
	#endif //ALC_EXT_EFX
}

const std::vector<float> &Effect::getParams() const
{
	return params;
}

//clamp values silently to avoid randomly throwing errors due to implementation differences
float Effect::clampf(float val, float min, float max, float def)
{
	if (isnanf(val)) return def;
	else if (val < min) val = min;
	else if (val > max) val = max;
	return val;
}

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

} //openal
} //audio
} //love
