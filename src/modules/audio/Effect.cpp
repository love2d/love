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

namespace love
{
namespace audio
{

Effect::Effect()
{
}

Effect::~Effect()
{
}

Effect::Type Effect::getType() const
{
	return type;
}

bool Effect::getConstant(const char *in, Type &out)
{
	return types.find(in, out);
}

bool Effect::getConstant(Type in, const char *&out)
{
	return types.find(in, out);
}

bool Effect::getConstant(const char *in, Phoneme &out)
{
	return phonemes.find(in, out);
}

bool Effect::getConstant(Phoneme in, const char *&out)
{
	return phonemes.find(in, out);
}

bool Effect::getConstant(const char *in, Waveform &out)
{
	return waveforms.find(in, out);
}

bool Effect::getConstant(Waveform in, const char *&out)
{
	return waveforms.find(in, out);
}

bool Effect::getConstant(const char *in, Direction &out)
{
	return directions.find(in, out);
}

bool Effect::getConstant(Direction in, const char *&out)
{
	return directions.find(in, out);
}

const std::vector<Effect::ParameterType> &Effect::getParameterTypes(Effect::Type in)
{
	return parameterTypes[in];
}

StringMap<Effect::Type, Effect::TYPE_MAX_ENUM>::Entry Effect::typeEntries[] =
{
	{"reverb", Effect::TYPE_REVERB},
	{"chorus", Effect::TYPE_CHORUS},
	{"distortion", Effect::TYPE_DISTORTION},
	{"echo", Effect::TYPE_ECHO},
	{"flanger", Effect::TYPE_FLANGER},
	{"frequencyshifter", Effect::TYPE_FREQSHIFTER},
	{"vocalmorpher", Effect::TYPE_MORPHER},
	{"pitchshifter", Effect::TYPE_PITCHSHIFTER},
	{"ringmodulator", Effect::TYPE_MODULATOR},
	{"autowah", Effect::TYPE_AUTOWAH},
	{"compressor", Effect::TYPE_COMPRESSOR},
	{"equalizer", Effect::TYPE_EQUALIZER},
};

StringMap<Effect::Type, Effect::TYPE_MAX_ENUM> Effect::types(Effect::typeEntries, sizeof(Effect::typeEntries));

StringMap<Effect::Waveform, Effect::WAVE_MAX_ENUM>::Entry Effect::waveformEntries[] =
{
	{"sine", Effect::WAVE_SINE},
	{"triangle", Effect::WAVE_TRIANGLE},
	{"sawtooth", Effect::WAVE_SAWTOOTH},
	{"square", Effect::WAVE_SQUARE},
};

StringMap<Effect::Waveform, Effect::WAVE_MAX_ENUM> Effect::waveforms(Effect::waveformEntries, sizeof(Effect::waveformEntries));

StringMap<Effect::Direction, Effect::DIR_MAX_ENUM>::Entry Effect::directionEntries[] =
{
	{"up", Effect::DIR_UP},
	{"down", Effect::DIR_DOWN},
	{"none", Effect::DIR_NONE},
};

StringMap<Effect::Direction, Effect::DIR_MAX_ENUM> Effect::directions(Effect::directionEntries, sizeof(Effect::directionEntries));

StringMap<Effect::Phoneme, Effect::PHONEME_MAX_ENUM>::Entry Effect::phonemeEntries[] =
{
	{"a", Effect::PHONEME_A},
	{"e", Effect::PHONEME_E},
	{"i", Effect::PHONEME_I},
	{"o", Effect::PHONEME_O},
	{"u", Effect::PHONEME_U},
	{"aa", Effect::PHONEME_AA},
	{"ae", Effect::PHONEME_AE},
	{"ah", Effect::PHONEME_AH},
	{"ao", Effect::PHONEME_AO},
	{"eh", Effect::PHONEME_EH},
	{"er", Effect::PHONEME_ER},
	{"ih", Effect::PHONEME_IH},
	{"iy", Effect::PHONEME_IY},
	{"uh", Effect::PHONEME_UH},
	{"uw", Effect::PHONEME_UW},
	{"b", Effect::PHONEME_B},
	{"d", Effect::PHONEME_D},
	{"f", Effect::PHONEME_F},
	{"g", Effect::PHONEME_G},
	{"j", Effect::PHONEME_J},
	{"k", Effect::PHONEME_K},
	{"l", Effect::PHONEME_L},
	{"m", Effect::PHONEME_M},
	{"n", Effect::PHONEME_N},
	{"p", Effect::PHONEME_P},
	{"r", Effect::PHONEME_R},
	{"s", Effect::PHONEME_S},
	{"t", Effect::PHONEME_T},
	{"v", Effect::PHONEME_V},
	{"z", Effect::PHONEME_Z},
};

StringMap<Effect::Phoneme, Effect::PHONEME_MAX_ENUM> Effect::phonemes(Effect::phonemeEntries, sizeof(Effect::phonemeEntries));

std::map<Effect::Type, std::vector<Effect::ParameterType>> Effect::parameterTypes =
{
	//gain, high gain, density, diffusion, decay, high decay ratio, refl gain, refl delay, late gain, late delay, rolloff, air high gain, high limiter
	{Effect::TYPE_REVERB, {Effect::PAR_FLOAT, Effect::PAR_FLOAT, Effect::PAR_FLOAT, Effect::PAR_FLOAT, Effect::PAR_FLOAT, Effect::PAR_FLOAT,
		Effect::PAR_FLOAT, Effect::PAR_FLOAT, Effect::PAR_FLOAT, Effect::PAR_FLOAT, Effect::PAR_FLOAT, Effect::PAR_FLOAT, Effect::PAR_BOOL}},
	//waveform, phase, rate, depth, feedback, delay
	{Effect::TYPE_CHORUS, { Effect::PAR_WAVEFORM, Effect::PAR_FLOAT, Effect::PAR_FLOAT, Effect::PAR_FLOAT, Effect::PAR_FLOAT, Effect::PAR_FLOAT}},
	//gain, edge, lowpass cutoff, eq center, eq width
	{Effect::TYPE_DISTORTION, {Effect::PAR_FLOAT, Effect::PAR_FLOAT, Effect::PAR_FLOAT, Effect::PAR_FLOAT, Effect::PAR_FLOAT}},
	//delay, LR delay, damping, feedback, spread
	{Effect::TYPE_ECHO, {Effect::PAR_FLOAT, Effect::PAR_FLOAT, Effect::PAR_FLOAT, Effect::PAR_FLOAT, Effect::PAR_FLOAT}},
	//waveform, phase, rate, depth, feedback, delay
	{Effect::TYPE_FLANGER, {Effect::PAR_WAVEFORM, Effect::PAR_FLOAT, Effect::PAR_FLOAT, Effect::PAR_FLOAT, Effect::PAR_FLOAT, Effect::PAR_FLOAT}},
	//frequency, left direction, right direction
	{Effect::TYPE_FREQSHIFTER, {Effect::PAR_FLOAT, Effect::PAR_DIRECTION, Effect::PAR_DIRECTION}},
	//waveform, rate, phoneme A, phoneme B, phoneme A coarse tune, phoneme B coarse tune
	{Effect::TYPE_MORPHER, {Effect::PAR_WAVEFORM, Effect::PAR_FLOAT, Effect::PAR_PHONEME, Effect::PAR_PHONEME, Effect::PAR_FLOAT, Effect::PAR_FLOAT}},
	//pitch(semitones)
	{Effect::TYPE_PITCHSHIFTER, {Effect::PAR_FLOAT}},
	//waveform, frequency, highpass cutoff
	{Effect::TYPE_MODULATOR, {Effect::PAR_WAVEFORM, Effect::PAR_FLOAT, Effect::PAR_FLOAT}},
	//attack, release, resonance, peak gain
	{Effect::TYPE_AUTOWAH, {Effect::PAR_FLOAT, Effect::PAR_FLOAT, Effect::PAR_FLOAT, Effect::PAR_FLOAT}},
	// on-off switch
	{Effect::TYPE_COMPRESSOR, {Effect::PAR_BOOL}},
	//low gain, low cut, mid1 gain, mid1 freq, mid1 band, mid2 gain, mid2 freq, mid2 band, high gain, high cut
	{Effect::TYPE_EQUALIZER, {Effect::PAR_FLOAT, Effect::PAR_FLOAT, Effect::PAR_FLOAT, Effect::PAR_FLOAT, Effect::PAR_FLOAT, 
		Effect::PAR_FLOAT, Effect::PAR_FLOAT, Effect::PAR_FLOAT, Effect::PAR_FLOAT, Effect::PAR_FLOAT}},
};

} //audio
} //love
