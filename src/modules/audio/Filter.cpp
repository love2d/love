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

#include "Filter.h"

namespace love
{
namespace audio
{

Filter::Filter()
{
}

Filter::~Filter()
{
}

Filter::Type Filter::getType() const
{
	return type;
}

bool Filter::getConstant(const char *in, Type &out)
{
	return types.find(in, out);
}

bool Filter::getConstant(Type in, const char *&out)
{
	return types.find(in, out);
}

std::vector<std::string> Filter::getConstants(Type)
{
	return types.getNames();
}

bool Filter::getConstant(const char *in, Parameter &out, Type t)
{
	return parameterNames[t].find(in, out);
}

bool Filter::getConstant(Parameter in, const char *&out, Type t)
{
	return parameterNames[t].find(in, out);
}

Filter::ParameterType Filter::getParameterType(Parameter in)
{
	return parameterTypes[in];
}

StringMap<Filter::Type, Filter::TYPE_MAX_ENUM>::Entry Filter::typeEntries[] =
{
	{"lowpass", Filter::TYPE_LOWPASS},
	{"highpass", Filter::TYPE_HIGHPASS},
	{"bandpass", Filter::TYPE_BANDPASS},
};

StringMap<Filter::Type, Filter::TYPE_MAX_ENUM> Filter::types(Filter::typeEntries, sizeof(Filter::typeEntries));

#define StringMap LazierAndSlowerButEasilyArrayableStringMap2
std::vector<StringMap<Filter::Parameter>::Entry> Filter::basicParameters =
{
	{"type", Filter::FILTER_TYPE},
	{"volume", Filter::FILTER_VOLUME}
};

std::vector<StringMap<Filter::Parameter>::Entry> Filter::lowpassParameters =
{
	{"highgain", Filter::FILTER_HIGHGAIN}
};

std::vector<StringMap<Filter::Parameter>::Entry> Filter::highpassParameters =
{
	{"lowgain", Filter::FILTER_LOWGAIN}
};

std::vector<StringMap<Filter::Parameter>::Entry> Filter::bandpassParameters =
{
	{"lowgain", Filter::FILTER_LOWGAIN},
	{"highgain", Filter::FILTER_HIGHGAIN}
};

std::map<Filter::Type, StringMap<Filter::Parameter>> Filter::parameterNames =
{
	{Filter::TYPE_BASIC, Filter::basicParameters},
	{Filter::TYPE_LOWPASS, Filter::lowpassParameters},
	{Filter::TYPE_HIGHPASS, Filter::highpassParameters},
	{Filter::TYPE_BANDPASS, Filter::bandpassParameters},
};
#undef StringMap

std::map<Filter::Parameter, Filter::ParameterType> Filter::parameterTypes =
{
	{Filter::FILTER_TYPE, Filter::PARAM_TYPE},
	{Filter::FILTER_VOLUME, Filter::PARAM_FLOAT},
	{Filter::FILTER_LOWGAIN, Filter::PARAM_FLOAT},
	{Filter::FILTER_HIGHGAIN, Filter::PARAM_FLOAT}
};

} //audio
} //love
