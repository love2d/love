/**
 * Copyright (c) 2006-2014 LOVE Development Team
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
	size_t bufferSize = 524288; // 0x80000
	int decoded = decoder->decode();

	while (decoded > 0)
	{
		// Expand or allocate buffer. Note that realloc may move
		// memory to other locations.
		if (!data || bufferSize < (size_t) size + decoded)
		{
			while (bufferSize < (size_t) size + decoded)
				bufferSize <<= 1;
			data = (int8 *) realloc(data, bufferSize);
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
		data = (int8 *) realloc(data, size);

	channels = decoder->getChannels();
	bitDepth = decoder->getBitDepth();
	sampleRate = decoder->getSampleRate();
}

SoundData::SoundData(int samples, int sampleRate, int bitDepth, int channels)
	: data(0)
	, size(0)
	, sampleRate(0)
	, bitDepth(0)
	, channels(0)
{
	load(samples, sampleRate, bitDepth, channels);
}

SoundData::SoundData(void *d, int samples, int sampleRate, int bitDepth, int channels)
	: data(0)
	, size(0)
	, sampleRate(0)
	, bitDepth(0)
	, channels(0)
{
	load(samples, sampleRate, bitDepth, channels, d);
}

SoundData::~SoundData()
{
	if (data != 0)
		free(data);
}

void SoundData::load(int samples, int sampleRate, int bitDepth, int channels, void *newData)
{
	if (samples <= 0)
		throw love::Exception("Invalid sample count: %d", samples);

	if (sampleRate <= 0)
		throw love::Exception("Invalid sample rate: %d", sampleRate);

	if (bitDepth <= 0)
		throw love::Exception("Invalid bit depth: %d", bitDepth);

	if (channels <= 0)
		throw love::Exception("Invalid channel count: %d", channels);

	if (data != 0)
	{
		free(data);
		data = 0;
	}

	size = samples * (bitDepth / 8) * channels;
	this->sampleRate = sampleRate;
	this->bitDepth = bitDepth;
	this->channels = channels;

	double realsize = samples;
	realsize *= (bitDepth / 8) * channels;
	if (realsize > INT_MAX)
		throw love::Exception("Data is too big!");

	data = (int8 *) malloc(size);
	if (!data)
		throw love::Exception("Not enough memory.");

	if (newData)
		memcpy(data, newData, size);
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
		int16 *s = (int16 *) data;
		s[i] = (int16) (sample * (float) LOVE_INT16_MAX);
	}
	else
	{
		data[i] = (int8) (sample * (float) LOVE_INT8_MAX);
	}
}

float SoundData::getSample(int i) const
{
	// Check range.
	if (i < 0 || i >= size/(bitDepth/8))
		throw love::Exception("Attempt to get out-of-range sample!");

	if (bitDepth == 16)
	{
		int16 *s = (int16 *) data;
		return (float) s[i] / (float) LOVE_INT16_MAX;
	}
	else
	{
		return (float) data[i] / (float) LOVE_INT8_MAX;
	}
}

} // sound
} // love
