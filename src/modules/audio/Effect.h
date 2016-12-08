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

#ifndef LOVE_AUDIO_EFFECTS_H
#define LOVE_AUDIO_EFFECTS_H

#include "common/Object.h"
#include "common/StringMap.h"
#include <map>
#include <vector>

namespace love
{
namespace audio
{

class Effect
{
public:
	enum Type
	{
		TYPE_REVERB,
		TYPE_CHORUS,
		TYPE_DISTORTION,
		TYPE_ECHO,
		TYPE_FLANGER,
		TYPE_FREQSHIFTER,
		TYPE_MORPHER,
		TYPE_PITCHSHIFTER,
		TYPE_MODULATOR,
		TYPE_AUTOWAH,
		TYPE_COMPRESSOR,
		TYPE_EQUALIZER,
		TYPE_MAX_ENUM
	};

	enum ParameterType
	{
		PAR_FLOAT,
		PAR_BOOL,
		PAR_WAVEFORM,
		PAR_DIRECTION,
		PAR_PHONEME,
		PAR_MAX_ENUM
	};

	enum Waveform
	{
		WAVE_SINE,
		WAVE_TRIANGLE,
		WAVE_SAWTOOTH,
		WAVE_SQUARE,
		WAVE_MAX_ENUM
	};

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

	Effect();
	virtual ~Effect();
	Type getType() const;

	static bool getConstant(const char *in, Type &out);
	static bool getConstant(Type in, const char *&out);
	static bool getConstant(const char *in, Waveform &out);
	static bool getConstant(Waveform in, const char *&out);
	static bool getConstant(const char *in, Direction &out);
	static bool getConstant(Direction in, const char *&out);
	static bool getConstant(const char *in, Phoneme &out);
	static bool getConstant(Phoneme in, const char *&out);
	static const std::vector<ParameterType> &getParameterTypes(Type in);

protected:
	Type type;

private:
	static StringMap<Type, TYPE_MAX_ENUM>::Entry typeEntries[];
	static StringMap<Type, TYPE_MAX_ENUM> types;
	static StringMap<Waveform, WAVE_MAX_ENUM>::Entry waveformEntries[];
	static StringMap<Waveform, WAVE_MAX_ENUM> waveforms;
	static StringMap<Direction, DIR_MAX_ENUM>::Entry directionEntries[];
	static StringMap<Direction, DIR_MAX_ENUM> directions;
	static StringMap<Phoneme, PHONEME_MAX_ENUM>::Entry phonemeEntries[];
	static StringMap<Phoneme, PHONEME_MAX_ENUM> phonemes;
	static std::map<Type, std::vector<Effect::ParameterType>> parameterTypes;
};

} //audio
} //love

#endif //LOVE_AUDIO_EFFECTS_H
