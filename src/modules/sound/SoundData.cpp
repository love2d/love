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

#include "SoundData.h"

// C
#include <cstdlib>
#include <cstring>

// C++
#include <limits>
#include <iostream>
#include <vector>

namespace love
{
namespace sound
{

love::Type SoundData::type("SoundData", &Data::type);

SoundData::SoundData(Decoder *decoder)
	: data(0)
	, size(0)
	, sampleRate(Decoder::DEFAULT_SAMPLE_RATE)
	, bitDepth(0)
	, channels(0)
{
	if (decoder->getBitDepth() != 8 && decoder->getBitDepth() != 16)
		throw love::Exception("Invalid bit depth: %d", decoder->getBitDepth());

	size_t bufferSize = 524288; // 0x80000
	int decoded = decoder->decode();

	while (decoded > 0)
	{
		// Expand or allocate buffer. Note that realloc may move
		// memory to other locations.
		if (!data || bufferSize < size + decoded)
		{
			while (bufferSize < size + decoded)
				bufferSize <<= 1;
			data = (uint8 *) realloc(data, bufferSize);
		}

		if (!data)
			throw love::Exception("Not enough memory.");

		// Copy memory into new part of memory.
		memcpy(data + size, decoder->getBuffer(), decoded);

		// Overflow check.
		if (size > std::numeric_limits<size_t>::max() - decoded)
		{
			free(data);
			throw love::Exception("Not enough memory.");
		}

		// Keep this up to date.
		size += decoded;

		decoded = decoder->decode();
	}

	// Shrink buffer if necessary.
	if (data && bufferSize > size)
		data = (uint8 *) realloc(data, size);

	channels = decoder->getChannelCount();
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

SoundData::SoundData(const SoundData &c)
	: data(0)
	, size(0)
	, sampleRate(0)
	, bitDepth(0)
	, channels(0)
{
	load(c.getSampleCount(), c.getSampleRate(), c.getBitDepth(), c.getChannelCount(), c.getData());
}

SoundData::~SoundData()
{
	if (data != 0)
		free(data);
}

SoundData *SoundData::clone() const
{
	return new SoundData(*this);
}

void SoundData::load(int samples, int sampleRate, int bitDepth, int channels, void *newData)
{
	if (samples <= 0)
		throw love::Exception("Invalid sample count: %d", samples);

	if (sampleRate <= 0)
		throw love::Exception("Invalid sample rate: %d", sampleRate);

	if (bitDepth != 8 && bitDepth != 16)
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
	if (realsize > std::numeric_limits<size_t>::max())
		throw love::Exception("Data is too big!");

	data = (uint8 *) malloc(size);
	if (!data)
		throw love::Exception("Not enough memory.");

	if (newData)
		memcpy(data, newData, size);
	else
		memset(data, bitDepth == 8 ? 128 : 0, size);
}

void *SoundData::getData() const
{
	return (void *)data;
}

size_t SoundData::getSize() const
{
	return size;
}

int SoundData::getChannelCount() const
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
	return (int) ((size/channels)/(bitDepth/8));
}

float SoundData::getDuration() const
{
	return float(size) / (channels*sampleRate*bitDepth/8);
}

void SoundData::setSample(int i, float sample)
{
	// Check range.
	if (i < 0 || (size_t) i >= size/(bitDepth/8))
		throw love::Exception("Attempt to set out-of-range sample!");

	if (bitDepth == 16)
	{
		// 16-bit sample values are signed.
		int16 *s = (int16 *) data;
		s[i] = (int16) (sample * (float) LOVE_INT16_MAX);
	}
	else
	{
		// 8-bit sample values are unsigned internally.
		data[i] = (uint8) ((sample * 127.0f) + 128.0f);
	}
}

void SoundData::setSample(int i, int channel, float sample)
{
	if (channel < 1 || channel > channels)
		throw love::Exception("Attempt to set sample from out-of-range channel!");

	return setSample(i * channels + (channel - 1), sample);
}

float SoundData::getSample(int i) const
{
	// Check range.
	if (i < 0 || (size_t) i >= size/(bitDepth/8))
		throw love::Exception("Attempt to get out-of-range sample!");

	if (bitDepth == 16)
	{
		// 16-bit sample values are signed.
		int16 *s = (int16 *) data;
		return (float) s[i] / (float) LOVE_INT16_MAX;
	}
	else
	{
		// 8-bit sample values are unsigned internally.
		return ((float) data[i] - 128.0f) / 127.0f;
	}
}

float SoundData::getSample(int i, int channel) const
{
	if (channel < 1 || channel > channels)
		throw love::Exception("Attempt to get sample from out-of-range channel!");

	return getSample(i * channels + (channel - 1));
}

} // sound
} // love
