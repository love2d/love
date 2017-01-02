/**
 * Copyright (c) 2006-2017 LOVE Development Team
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

int Filter::getParameterCount(Type in)
{
	return parameterCount[in];
}

int Filter::getParameterCount()
{
	return parameterCount[TYPE_MAX_ENUM];
}

StringMap<Filter::Type, Filter::TYPE_MAX_ENUM>::Entry Filter::typeEntries[] =
{
	{"lowpass", Filter::TYPE_LOWPASS},
	{"highpass", Filter::TYPE_HIGHPASS},
	{"bandpass", Filter::TYPE_BANDPASS},
};

StringMap<Filter::Type, Filter::TYPE_MAX_ENUM> Filter::types(Filter::typeEntries, sizeof(Filter::typeEntries));

std::map<Filter::Type, int> Filter::parameterCount =
{
	{Filter::TYPE_LOWPASS, 2},
	{Filter::TYPE_HIGHPASS, 2},
	{Filter::TYPE_BANDPASS, 3},
	{Filter::TYPE_MAX_ENUM, 3},
};

} //audio
} //love
