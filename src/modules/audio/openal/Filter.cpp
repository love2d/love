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
#include "common/Exception.h"

namespace love
{
namespace audio
{
namespace openal
{

//clamp values silently to avoid randomly throwing errors due to implementation differences
float clampf(float val, float min, float max)
{
	if      (val < min) val = min;
	else if (val > max) val = max;
	return val;
}

//base class
Filter::Filter()
{
	#ifdef ALC_EXT_EFX
	if (!alGenFilters)
		return;

	alGenFilters(1, &filter);
	if (alGetError() != AL_NO_ERROR)
		throw love::Exception("Failed to create sound Filter.");
	#endif
}

Filter::Filter(const Filter &s)
	: Filter()
{
	setParams(s.getType(), s.getParams());
}

Filter::~Filter()
{
	#ifdef ALC_EXT_EFX
	if (filter != AL_FILTER_NULL)
		alDeleteFilters(1, &filter);
	#endif
}

Filter *Filter::clone()
{
	return new Filter(*this);
}

ALuint Filter::getFilter() const
{
	return filter;
}

bool Filter::setParams(Type type, const std::vector<float> &params)
{
	this->type = type;
	this->params = params;

	if (filter == AL_FILTER_NULL)
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
	case TYPE_MAX_ENUM:
		break;
	}

	//failed to make filter specific type - not supported etc.
	if (alGetError() != AL_NO_ERROR)
	{
		filter = AL_FILTER_NULL;
		return false;
	}

	switch (type)
	{
	case TYPE_LOWPASS:
		alFilterf(filter, AL_LOWPASS_GAIN, clampf(params[0], AL_LOWPASS_MIN_GAIN, AL_LOWPASS_MAX_GAIN));
		alFilterf(filter, AL_LOWPASS_GAINHF, clampf(params[1], AL_LOWPASS_MIN_GAINHF, AL_LOWPASS_MAX_GAINHF));
		break;
	case TYPE_HIGHPASS:
		alFilterf(filter, AL_HIGHPASS_GAIN, clampf(params[0], AL_HIGHPASS_MIN_GAIN, AL_HIGHPASS_MAX_GAIN));
		alFilterf(filter, AL_HIGHPASS_GAINLF, clampf(params[1], AL_HIGHPASS_MIN_GAINLF, AL_HIGHPASS_MAX_GAINLF));
		break;
	case TYPE_BANDPASS:
		alFilterf(filter, AL_BANDPASS_GAIN, clampf(params[0], AL_BANDPASS_MIN_GAIN, AL_BANDPASS_MAX_GAIN));
		alFilterf(filter, AL_BANDPASS_GAINLF, clampf(params[1], AL_BANDPASS_MIN_GAINLF, AL_BANDPASS_MAX_GAINLF));
		alFilterf(filter, AL_BANDPASS_GAINHF, clampf(params[2], AL_BANDPASS_MIN_GAINHF, AL_BANDPASS_MAX_GAINHF));
		break;
	case TYPE_MAX_ENUM:
		break;
	}
	#endif

	return true;
}

const std::vector<float> &Filter::getParams() const
{
	return params;
}

} //openal
} //audio
} //love
