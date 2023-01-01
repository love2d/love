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
#include "common/Exception.h"

#include <cmath>

namespace love
{
namespace audio
{
namespace openal
{

//base class
Filter::Filter()
{
	generateFilter();
}

Filter::Filter(const Filter &s)
	: Filter()
{
	setParams(s.getParams());
}

Filter::~Filter()
{
	deleteFilter();
}

Filter *Filter::clone()
{
	return new Filter(*this);
}

bool Filter::generateFilter()
{
#ifdef ALC_EXT_EFX
	if (!alGenFilters)
		return false;

	if (filter != AL_FILTER_NULL)
		return true;

	alGenFilters(1, &filter);
	if (alGetError() != AL_NO_ERROR)
		throw love::Exception("Failed to create sound Filter.");

	return true;
#else
	return false;
#endif
}

void Filter::deleteFilter()
{
#ifdef ALC_EXT_EFX
	if (filter != AL_FILTER_NULL)
		alDeleteFilters(1, &filter);
#endif
	filter = AL_FILTER_NULL;
}

ALuint Filter::getFilter() const
{
	return filter;
}

bool Filter::setParams(const std::map<Parameter, float> &params)
{
	this->params = params;
	type = (Type)(int) this->params[FILTER_TYPE];

	if (!generateFilter())
		return false;

#ifdef ALC_EXT_EFX
	switch (type)
	{
	case TYPE_LOWPASS:
		alFilteri(filter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
		break;
	case TYPE_HIGHPASS:
		alFilteri(filter, AL_FILTER_TYPE, AL_FILTER_HIGHPASS);
		break;
	case TYPE_BANDPASS:
		alFilteri(filter, AL_FILTER_TYPE, AL_FILTER_BANDPASS);
		break;
	case TYPE_BASIC:
	case TYPE_MAX_ENUM:
		break;
	}

	//failed to make filter specific type - not supported etc.
	if (alGetError() != AL_NO_ERROR)
	{
		deleteFilter();
		return false;
	}

#define clampf(v,l,h) fmax(fmin((v),(h)),(l))
#define PARAMSTR(i,e,v) filter,AL_##e##_##v,clampf(getValue(i,AL_##e##_DEFAULT_##v),AL_##e##_MIN_##v,AL_##e##_MAX_##v)
	switch (type)
	{
	case TYPE_LOWPASS:
		alFilterf(PARAMSTR(FILTER_VOLUME,LOWPASS,GAIN));
		alFilterf(PARAMSTR(FILTER_HIGHGAIN,LOWPASS,GAINHF));
		break;
	case TYPE_HIGHPASS:
		alFilterf(PARAMSTR(FILTER_VOLUME,HIGHPASS,GAIN));
		alFilterf(PARAMSTR(FILTER_LOWGAIN,HIGHPASS,GAINLF));
		break;
	case TYPE_BANDPASS:
		alFilterf(PARAMSTR(FILTER_VOLUME,BANDPASS,GAIN));
		alFilterf(PARAMSTR(FILTER_LOWGAIN,BANDPASS,GAINLF));
		alFilterf(PARAMSTR(FILTER_HIGHGAIN,BANDPASS,GAINHF));
		break;
	case TYPE_BASIC:
	case TYPE_MAX_ENUM:
		break;
	}
#undef clampf
#undef PARAMSTR
	//alGetError();

	return true;
#else
	return false;
#endif
}

const std::map<Filter::Parameter, float> &Filter::getParams() const
{
	return params;
}

float Filter::getValue(Parameter in, float def) const
{
	return params.find(in) == params.end() ? def : params.at(in);
}

int Filter::getValue(Parameter in, int def) const
{
	return params.find(in) == params.end() ? def : static_cast<int>(params.at(in));
}

} //openal
} //audio
} //love
