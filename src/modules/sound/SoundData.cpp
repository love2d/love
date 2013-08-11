/**
 * Copyright (c) 2006-2013 LOVE Development Team
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

#include "SoundData.h"

// C
#include <climits>
#include <cstdlib>
#include <cstring>

// STL
#include <iostream>
#include <vector>

namespace love
{
namespace sound
{

SoundData::SoundData(Decoder *decoder)
	: data(0)
	, size(0)
	, sampleRate(Decoder::DEFAULT_SAMPLE_RATE)
	, bitDepth(0)
	, channels(0)
{
	size_t bufferSize = 524288;
	int decoded = decoder->decode();

	while (decoded > 0)
	{
		// Expand or allocate buffer. Note that realloc may move
		// memory to other locations.
		if (!data || bufferSize < (size_t) size + decoded)
		{
			while (bufferSize < (size_t) size + decoded)
				bufferSize <<= 1;
			data = (char *)realloc(data, bufferSize);
		}

		if (!data)
			throw love::Exception("Not enough memory.");

		// Copy memory into new part of memory.
		memcpy(data + size, decoder->getBuffer(), decoded);

		// Keep this up to date.
		size += decoded;

		// Overflow check.
		if (size < 0)
		{
			free(data);
			throw love::Exception("Not enough memory.");
		}

		decoded = decoder->decode();
	}

	// Shrink buffer if necessary.
	if (data && bufferSize > (size_t) size)
		data = (char *) realloc(data, size);

	channels = decoder->getChannels();
	bitDepth = decoder->getBitDepth();
	sampleRate = decoder->getSampleRate();
}

SoundData::SoundData(int samples, int sampleRate, int bitDepth, int channels)
	: data(0)
	, size(samples*(bitDepth/8)*channels)
	, sampleRate(sampleRate)
	, bitDepth(bitDepth)
	, channels(channels)
{
	double realsize = samples;
	realsize *= (bitDepth/8)*channels;
	if (realsize > INT_MAX)
		throw love::Exception("Data is too big!");
	data = (char *)malloc(size);
	if (!data)
		throw love::Exception("Not enough memory.");
}

SoundData::SoundData(void *d, int samples, int sampleRate, int bitDepth, int channels)
	: data(0)
	, size(samples*(bitDepth/8)*channels)
	, sampleRate(sampleRate)
	, bitDepth(bitDepth)
	, channels(channels)
{
	double realsize = samples;
	realsize *= (bitDepth/8)*channels;
	if (realsize > INT_MAX)
		throw love::Exception("Data is too big!");
	data = (char *)malloc(size);
	if (!data)
		throw love::Exception("Not enough memory.");
	memcpy(data, d, size);
}

SoundData::~SoundData()
{
	if (data != 0)
		free(data);
}

void *SoundData::getData() const
{
	return (void *)data;
}

int SoundData::getSize() const
{
	return (int)size;
}

int SoundData::getChannels() const
{
	return channels;
}

int SoundData::getBitDepth() const
{
	return bitDepth;
}

int SoundData::getSampleRate() const
{
	return sampleRate;
}

int SoundData::getSampleCount() const
{
	return (size/channels)/(bitDepth/8);
}

float SoundData::getDuration() const
{
	return float(size) / (channels*sampleRate*bitDepth/8);
}

void SoundData::setSample(int i, float sample)
{
	// Check range.
	if (i < 0 || i >= size/(bitDepth/8))
		throw love::Exception("Attempt to set out-of-range sample!");

	if (bitDepth == 16)
	{
		short *s = (short *)data;
		s[i] = (short)(sample*(float)SHRT_MAX);
		return;
	}
	else
	{
		data[i] = (char)(sample*(float)CHAR_MAX);
		return;
	}
}

float SoundData::getSample(int i) const
{
	// Check range.
	if (i < 0 || i >= size/(bitDepth/8))
		throw love::Exception("Attempt to get out-of-range sample!");

	if (bitDepth == 16)
	{
		short *s = (short *)data;
		return (float)s[i]/(float)SHRT_MAX;
	}
	else
	{
		return (float)data[i]/(float)CHAR_MAX;
	}
}

} // sound
} // love
