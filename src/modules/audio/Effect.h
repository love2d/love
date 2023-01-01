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

#ifndef LOVE_AUDIO_EFFECTS_H
#define LOVE_AUDIO_EFFECTS_H

#include "common/Object.h"
#include "common/StringMap.h"
#include <map>
#include <vector>

template<typename T>
class LazierAndSlowerButEasilyArrayableStringMap
{
public:
	struct Entry
	{
		const char *key;
		T value;
	};
	LazierAndSlowerButEasilyArrayableStringMap()
	{
	}

	LazierAndSlowerButEasilyArrayableStringMap(const std::vector<Entry> &entries)
	{
		for (auto entry : entries)
		{
			forward[entry.key] = entry.value;
			reverse[entry.value] = entry.key;
		}
	}

	bool find(const char *key, T &t)
	{
		if (forward.find(key) == forward.end())
			return false;
		t = forward[key];
		return true;
	}

	bool find(T key, const char *&str)
	{
		if (reverse.find(key) == reverse.end())
			return false;
		str = reverse[key];
		return true;
	}

private:
	std::map<std::string, T> forward;
	std::map<T, const char*> reverse;
};

namespace love
{
namespace audio
{

class Effect
{
public:
	enum Type
	{
		TYPE_BASIC, //not a real type
		TYPE_REVERB,
		TYPE_CHORUS,
		TYPE_DISTORTION,
		TYPE_ECHO,
		TYPE_FLANGER,
		//TYPE_FREQSHIFTER,
		//TYPE_MORPHER,
		//TYPE_PITCHSHIFTER,
		TYPE_MODULATOR,
		//TYPE_AUTOWAH,
		TYPE_COMPRESSOR,
		TYPE_EQUALIZER,
		TYPE_MAX_ENUM
	};

	enum Parameter
	{
		EFFECT_TYPE,
		EFFECT_VOLUME,

		REVERB_GAIN,
		REVERB_HFGAIN,
		REVERB_DENSITY,
		REVERB_DIFFUSION,
		REVERB_DECAY,
		REVERB_HFDECAY,
		REVERB_EARLYGAIN,
		REVERB_EARLYDELAY,
		REVERB_LATEGAIN,
		REVERB_LATEDELAY,
		REVERB_ROLLOFF,
		REVERB_AIRHFGAIN,
		REVERB_HFLIMITER,

		CHORUS_WAVEFORM,
		CHORUS_PHASE,
		CHORUS_RATE,
		CHORUS_DEPTH,
		CHORUS_FEEDBACK,
		CHORUS_DELAY,

		DISTORTION_GAIN,
		DISTORTION_EDGE,
		DISTORTION_LOWCUT,
		DISTORTION_EQCENTER,
		DISTORTION_EQBAND,

		ECHO_DELAY,
		ECHO_LRDELAY,
		ECHO_DAMPING,
		ECHO_FEEDBACK,
		ECHO_SPREAD,

		FLANGER_WAVEFORM,
		FLANGER_PHASE,
		FLANGER_RATE,
		FLANGER_DEPTH,
		FLANGER_FEEDBACK,
		FLANGER_DELAY,
/*
		FREQSHIFTER_FREQ,
		FREQSHIFTER_LEFTDIR,
		FREQSHIFTER_RIGHTDIR,

		MORPHER_WAVEFORM,
		MORPHER_RATE,
		MORPHER_PHONEMEA,
		MORPHER_PHONEMEB,
		MORPHER_TUNEA,
		MORPHER_TUNEB,

		PITCHSHIFTER_PITCH,
*/
		MODULATOR_WAVEFORM,
		MODULATOR_FREQ,
		MODULATOR_HIGHCUT,
/*
		AUTOWAH_ATTACK,
		AUTOWAH_RELEASE,
		AUTOWAH_RESONANCE,
		AUTOWAH_PEAKGAIN,
*/
		COMPRESSOR_ENABLE,

		EQUALIZER_LOWGAIN,
		EQUALIZER_LOWCUT,
		EQUALIZER_MID1GAIN,
		EQUALIZER_MID1FREQ,
		EQUALIZER_MID1BAND,
		EQUALIZER_MID2GAIN,
		EQUALIZER_MID2FREQ,
		EQUALIZER_MID2BAND,
		EQUALIZER_HIGHGAIN,
		EQUALIZER_HIGHCUT,

		EFFECT_MAX_ENUM
	};

	enum ParameterType
	{
		PARAM_TYPE,
		PARAM_FLOAT,
		PARAM_BOOL,
		PARAM_WAVEFORM,
		//PARAM_DIRECTION,
		//PARAM_PHONEME,
		PARAM_MAX_ENUM
	};

	enum Waveform
	{
		WAVE_SINE,
		WAVE_TRIANGLE,
		WAVE_SAWTOOTH,
		WAVE_SQUARE,
		WAVE_MAX_ENUM
	};
	/*
	enum Direction
	{
		DIR_NONE,
		DIR_UP,
		DIR_DOWN,
		DIR_MAX_ENUM
	};

	enum Phoneme
	{
		PHONEME_A,
		PHONEME_E,
		PHONEME_I,
		PHONEME_O,
		PHONEME_U,
		PHONEME_AA,
		PHONEME_AE,
		PHONEME_AH,
		PHONEME_AO,
		PHONEME_EH,
		PHONEME_ER,
		PHONEME_IH,
		PHONEME_IY,
		PHONEME_UH,
		PHONEME_UW,
		PHONEME_B,
		PHONEME_D,
		PHONEME_F,
		PHONEME_G,
		PHONEME_J,
		PHONEME_K,
		PHONEME_L,
		PHONEME_M,
		PHONEME_N,
		PHONEME_P,
		PHONEME_R,
		PHONEME_S,
		PHONEME_T,
		PHONEME_V,
		PHONEME_Z,
		PHONEME_MAX_ENUM
	};
	*/

	Effect();
	virtual ~Effect();
	Type getType() const;

	static bool getConstant(const char *in, Type &out);
	static bool getConstant(Type in, const char *&out);
	static std::vector<std::string> getConstants(Type);
	static bool getConstant(const char *in, Waveform &out);
	static bool getConstant(Waveform in, const char *&out);
	//static bool getConstant(const char *in, Direction &out);
	//static bool getConstant(Direction in, const char *&out);
	//static bool getConstant(const char *in, Phoneme &out);
	//static bool getConstant(Phoneme in, const char *&out);
	static bool getConstant(const char *in, Parameter &out, Type t);
	static bool getConstant(Parameter in, const char *&out, Type t);
	static ParameterType getParameterType(Parameter in);

protected:
	Type type;

private:
	static StringMap<Type, TYPE_MAX_ENUM>::Entry typeEntries[];
	static StringMap<Type, TYPE_MAX_ENUM> types;
	static StringMap<Waveform, WAVE_MAX_ENUM>::Entry waveformEntries[];
	static StringMap<Waveform, WAVE_MAX_ENUM> waveforms;
	//static StringMap<Direction, DIR_MAX_ENUM>::Entry directionEntries[];
	//static StringMap<Direction, DIR_MAX_ENUM> directions;
	//static StringMap<Phoneme, PHONEME_MAX_ENUM>::Entry phonemeEntries[];
	//static StringMap<Phoneme, PHONEME_MAX_ENUM> phonemes;
#define StringMap LazierAndSlowerButEasilyArrayableStringMap
	static std::vector<StringMap<Effect::Parameter>::Entry> basicParameters;
	static std::vector<StringMap<Effect::Parameter>::Entry> reverbParameters;
	static std::vector<StringMap<Effect::Parameter>::Entry> chorusParameters;
	static std::vector<StringMap<Effect::Parameter>::Entry> distortionParameters;
	static std::vector<StringMap<Effect::Parameter>::Entry> echoParameters;
	static std::vector<StringMap<Effect::Parameter>::Entry> flangerParameters;
	//static std::vector<StringMap<Effect::Parameter>::Entry> freqshifterParameters;
	//static std::vector<StringMap<Effect::Parameter>::Entry> morpherParameters;
	//static std::vector<StringMap<Effect::Parameter>::Entry> pitchshifterParameters;
	static std::vector<StringMap<Effect::Parameter>::Entry> modulatorParameters;
	//static std::vector<StringMap<Effect::Parameter>::Entry> autowahParameters;
	static std::vector<StringMap<Effect::Parameter>::Entry> compressorParameters;
	static std::vector<StringMap<Effect::Parameter>::Entry> equalizerParameters;
	static std::map<Type, StringMap<Parameter>> parameterNames;
#undef StringMap
	static std::map<Parameter, ParameterType> parameterTypes;

};

} //audio
} //love

#endif //LOVE_AUDIO_EFFECTS_H
