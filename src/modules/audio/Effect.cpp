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

std::vector<std::string> Effect::getConstants(Type)
{
	return types.getNames();
}

/*
bool Effect::getConstant(const char *in, Phoneme &out)
{
	return phonemes.find(in, out);
}

bool Effect::getConstant(Phoneme in, const char *&out)
{
	return phonemes.find(in, out);
}
*/
bool Effect::getConstant(const char *in, Waveform &out)
{
	return waveforms.find(in, out);
}

bool Effect::getConstant(Waveform in, const char *&out)
{
	return waveforms.find(in, out);
}
/*
bool Effect::getConstant(const char *in, Direction &out)
{
	return directions.find(in, out);
}

bool Effect::getConstant(Direction in, const char *&out)
{
	return directions.find(in, out);
}
*/
bool Effect::getConstant(const char *in, Parameter &out, Type t)
{
	return parameterNames[t].find(in, out);
}

bool Effect::getConstant(Parameter in, const char *&out, Type t)
{
	return parameterNames[t].find(in, out);
}

Effect::ParameterType Effect::getParameterType(Effect::Parameter in)
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
	//{"frequencyshifter", Effect::TYPE_FREQSHIFTER},
	//{"vocalmorpher", Effect::TYPE_MORPHER},
	//{"pitchshifter", Effect::TYPE_PITCHSHIFTER},
	{"ringmodulator", Effect::TYPE_MODULATOR},
	//{"autowah", Effect::TYPE_AUTOWAH},
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

/*
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
*/

#define StringMap LazierAndSlowerButEasilyArrayableStringMap

std::vector<StringMap<Effect::Parameter>::Entry> Effect::basicParameters =
{
	{"type", Effect::EFFECT_TYPE},
	{"volume", Effect::EFFECT_VOLUME}
};

std::vector<StringMap<Effect::Parameter>::Entry> Effect::reverbParameters =
{
	{"gain", Effect::REVERB_GAIN},
	{"highgain", Effect::REVERB_HFGAIN},
	{"density", Effect::REVERB_DENSITY},
	{"diffusion", Effect::REVERB_DIFFUSION},
	{"decaytime", Effect::REVERB_DECAY},
	{"decayhighratio", Effect::REVERB_HFDECAY},
	{"earlygain", Effect::REVERB_EARLYGAIN},
	{"earlydelay", Effect::REVERB_EARLYDELAY},
	{"lategain", Effect::REVERB_LATEGAIN},
	{"latedelay", Effect::REVERB_LATEDELAY},
	{"roomrolloff", Effect::REVERB_ROLLOFF},
	{"airabsorption", Effect::REVERB_AIRHFGAIN},
	{"highlimit", Effect::REVERB_HFLIMITER}
};

std::vector<StringMap<Effect::Parameter>::Entry> Effect::chorusParameters =
{
	{"waveform", Effect::CHORUS_WAVEFORM},
	{"phase", Effect::CHORUS_PHASE},
	{"rate", Effect::CHORUS_RATE},
	{"depth", Effect::CHORUS_DEPTH},
	{"feedback", Effect::CHORUS_FEEDBACK},
	{"delay", Effect::CHORUS_DELAY}
};

std::vector<StringMap<Effect::Parameter>::Entry> Effect::distortionParameters =
{
	{"gain", Effect::DISTORTION_GAIN},
	{"edge", Effect::DISTORTION_EDGE},
	{"lowcut", Effect::DISTORTION_LOWCUT},
	{"center", Effect::DISTORTION_EQCENTER},
	{"bandwidth", Effect::DISTORTION_EQBAND}
};

std::vector<StringMap<Effect::Parameter>::Entry> Effect::echoParameters =
{
	{"delay", Effect::ECHO_DELAY},
	{"tapdelay", Effect::ECHO_LRDELAY},
	{"damping", Effect::ECHO_DAMPING},
	{"feedback", Effect::ECHO_FEEDBACK},
	{"spread", Effect::ECHO_SPREAD}
};

std::vector<StringMap<Effect::Parameter>::Entry> Effect::flangerParameters =
{
	{"waveform", Effect::FLANGER_WAVEFORM},
	{"phase", Effect::FLANGER_PHASE},
	{"rate", Effect::FLANGER_RATE},
	{"depth", Effect::FLANGER_DEPTH},
	{"feedback", Effect::FLANGER_FEEDBACK},
	{"delay", Effect::FLANGER_DELAY}
};
/*
std::vector<StringMap<Effect::Parameter>::Entry> Effect::freqshifterParameters =
{
	{"frequency", Effect::FREQSHIFTER_FREQ},
	{"leftdirection", Effect::FREQSHIFTER_LEFTDIR},
	{"rightdirection", Effect::FREQSHIFTER_RIGHTDIR}
};

std::vector<StringMap<Effect::Parameter>::Entry> Effect::morpherParameters =
{
	{"waveform", Effect::MORPHER_WAVEFORM},
	{"rate", Effect::MORPHER_RATE},
	{"phonemea", Effect::MORPHER_PHONEMEA},
	{"phonemeb", Effect::MORPHER_PHONEMEB},
	{"tunea", Effect::MORPHER_COARSEA},
	{"tuneb", Effect::MORPHER_COARSEB}
}
;
std::vector<StringMap<Effect::Parameter>::Entry> Effect::pitchshifterParameters =
{
	{"pitch", Effect::PITCHSHIFTER_PITCH}
};
*/
std::vector<StringMap<Effect::Parameter>::Entry> Effect::modulatorParameters =
{
	{"waveform", Effect::MODULATOR_WAVEFORM},
	{"frequency", Effect::MODULATOR_FREQ},
	{"highcut", Effect::MODULATOR_HIGHCUT}
};
/*
std::vector<StringMap<Effect::Parameter>::Entry> Effect::autowahParameters =
{
	{"attack", Effect::AUTOWAH_ATTACK},
	{"release", Effect::AUTOWAH_RELEASE},
	{"resonance", Effect::AUTOWAH_RESONANCE},
	{"peakgain", Effect::AUTOWAH_PEAKGAIN}
};
*/
std::vector<StringMap<Effect::Parameter>::Entry> Effect::compressorParameters =
{
	{"enable", Effect::COMPRESSOR_ENABLE}
};

std::vector<StringMap<Effect::Parameter>::Entry> Effect::equalizerParameters =
{
	{"lowgain", Effect::EQUALIZER_LOWGAIN},
	{"lowcut", Effect::EQUALIZER_LOWCUT},
	{"lowmidgain", Effect::EQUALIZER_MID1GAIN},
	{"lowmidfrequency", Effect::EQUALIZER_MID1FREQ},
	{"lowmidbandwidth", Effect::EQUALIZER_MID1BAND},
	{"highmidgain", Effect::EQUALIZER_MID2GAIN},
	{"highmidfrequency", Effect::EQUALIZER_MID2FREQ},
	{"highmidbandwidth", Effect::EQUALIZER_MID2BAND},
	{"highgain", Effect::EQUALIZER_HIGHGAIN},
	{"highcut", Effect::EQUALIZER_HIGHCUT}
};

std::map<Effect::Type, StringMap<Effect::Parameter>> Effect::parameterNames =
{
	{Effect::TYPE_BASIC, Effect::basicParameters},
	{Effect::TYPE_REVERB, Effect::reverbParameters},
	{Effect::TYPE_CHORUS, Effect::chorusParameters},
	{Effect::TYPE_DISTORTION, Effect::distortionParameters},
	{Effect::TYPE_ECHO, Effect::echoParameters},
	{Effect::TYPE_FLANGER, Effect::flangerParameters},
	//{Effect::TYPE_FREQSHIFTER, Effect::freqshifterParameters},
	//{Effect::TYPE_MORPHER, Effect::morpherbParameters},
	//{Effect::TYPE_PITCHSHIFTER, Effect::pitchshifterParameters},
	{Effect::TYPE_MODULATOR, Effect::modulatorParameters},
	//{Effect::TYPE_AUTOWAH, Effect::autowahParameters},
	{Effect::TYPE_COMPRESSOR, Effect::compressorParameters},
	{Effect::TYPE_EQUALIZER, Effect::equalizerParameters}
};
#undef StringMap

std::map<Effect::Parameter, Effect::ParameterType> Effect::parameterTypes =
{
	{Effect::EFFECT_TYPE, Effect::PARAM_TYPE},
	{Effect::EFFECT_VOLUME, Effect::PARAM_FLOAT},

	{Effect::REVERB_GAIN, Effect::PARAM_FLOAT},
	{Effect::REVERB_HFGAIN, Effect::PARAM_FLOAT},
	{Effect::REVERB_DENSITY, Effect::PARAM_FLOAT},
	{Effect::REVERB_DIFFUSION, Effect::PARAM_FLOAT},
	{Effect::REVERB_DECAY, Effect::PARAM_FLOAT},
	{Effect::REVERB_HFDECAY, Effect::PARAM_FLOAT},
	{Effect::REVERB_EARLYGAIN, Effect::PARAM_FLOAT},
	{Effect::REVERB_EARLYDELAY, Effect::PARAM_FLOAT},
	{Effect::REVERB_LATEGAIN, Effect::PARAM_FLOAT},
	{Effect::REVERB_LATEDELAY, Effect::PARAM_FLOAT},
	{Effect::REVERB_ROLLOFF, Effect::PARAM_FLOAT},
	{Effect::REVERB_AIRHFGAIN, Effect::PARAM_FLOAT},
	{Effect::REVERB_HFLIMITER, Effect::PARAM_BOOL},

	{Effect::CHORUS_WAVEFORM, Effect::PARAM_WAVEFORM},
	{Effect::CHORUS_PHASE, Effect::PARAM_FLOAT},
	{Effect::CHORUS_RATE, Effect::PARAM_FLOAT},
	{Effect::CHORUS_DEPTH, Effect::PARAM_FLOAT},
	{Effect::CHORUS_FEEDBACK, Effect::PARAM_FLOAT},
	{Effect::CHORUS_DELAY, Effect::PARAM_FLOAT},

	{Effect::DISTORTION_GAIN, Effect::PARAM_FLOAT},
	{Effect::DISTORTION_EDGE, Effect::PARAM_FLOAT},
	{Effect::DISTORTION_LOWCUT, Effect::PARAM_FLOAT},
	{Effect::DISTORTION_EQCENTER, Effect::PARAM_FLOAT},
	{Effect::DISTORTION_EQBAND, Effect::PARAM_FLOAT},

	{Effect::ECHO_DELAY, Effect::PARAM_FLOAT},
	{Effect::ECHO_LRDELAY, Effect::PARAM_FLOAT},
	{Effect::ECHO_DAMPING, Effect::PARAM_FLOAT},
	{Effect::ECHO_FEEDBACK, Effect::PARAM_FLOAT},
	{Effect::ECHO_SPREAD, Effect::PARAM_FLOAT},

	{Effect::FLANGER_WAVEFORM, Effect::PARAM_WAVEFORM},
	{Effect::FLANGER_PHASE, Effect::PARAM_FLOAT},
	{Effect::FLANGER_RATE, Effect::PARAM_FLOAT},
	{Effect::FLANGER_DEPTH, Effect::PARAM_FLOAT},
	{Effect::FLANGER_FEEDBACK, Effect::PARAM_FLOAT},
	{Effect::FLANGER_DELAY, Effect::PARAM_FLOAT},
/*
	{Effect::FREQSHIFTER_FREQ, Effect::PARAM_FLOAT},
	{Effect::FREQSHIFTER_LEFTDIR, Effect::PARAM_DIRECTION},
	{Effect::FREQSHIFTER_RIGHTDIR, Effect::PARAM_DIRECTION},

	{Effect::MORPHER_WAVEFORM, Effect::PARAM_WAVEFORM},
	{Effect::MORPHER_RATE, Effect::PARAM_FLOAT},
	{Effect::MORPHER_PHONEMEA, Effect::PARAM_PHONEME},
	{Effect::MORPHER_PHONEMEB, Effect::PARAM_PHONEME},
	{Effect::MORPHER_TUNEA, Effect::PARAM_FLOAT},
	{Effect::MORPHER_TUNEB, Effect::PARAM_FLOAT},

	{Effect::PITCHSHIFTER_PITCH, Effect::PARAM_FLOAT},
*/
	{Effect::MODULATOR_WAVEFORM, Effect::PARAM_WAVEFORM},
	{Effect::MODULATOR_FREQ, Effect::PARAM_FLOAT},
	{Effect::MODULATOR_HIGHCUT, Effect::PARAM_FLOAT},
/*
	{Effect::AUTOWAH_ATTACK, Effect::PARAM_FLOAT},
	{Effect::AUTOWAH_RELEASE, Effect::PARAM_FLOAT},
	{Effect::AUTOWAH_RESONANCE, Effect::PARAM_FLOAT},
	{Effect::AUTOWAH_PEAKGAIN, Effect::PARAM_FLOAT},
*/
	{Effect::COMPRESSOR_ENABLE, Effect::PARAM_BOOL},

	{Effect::EQUALIZER_LOWGAIN, Effect::PARAM_FLOAT},
	{Effect::EQUALIZER_LOWCUT, Effect::PARAM_FLOAT},
	{Effect::EQUALIZER_MID1GAIN, Effect::PARAM_FLOAT},
	{Effect::EQUALIZER_MID1FREQ, Effect::PARAM_FLOAT},
	{Effect::EQUALIZER_MID1BAND, Effect::PARAM_FLOAT},
	{Effect::EQUALIZER_MID2GAIN, Effect::PARAM_FLOAT},
	{Effect::EQUALIZER_MID2FREQ, Effect::PARAM_FLOAT},
	{Effect::EQUALIZER_MID2BAND, Effect::PARAM_FLOAT},
	{Effect::EQUALIZER_HIGHGAIN, Effect::PARAM_FLOAT},
	{Effect::EQUALIZER_HIGHCUT, Effect::PARAM_FLOAT}
};

} //audio
} //love
