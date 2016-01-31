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

#include "WaveDecoder.h"

#include <string.h>
#include "common/config.h"
#include "common/Exception.h"

namespace love
{
namespace sound
{
namespace lullaby
{

// Callbacks
static wuff_sint32 read_callback(void *userdata, wuff_uint8 *buffer, size_t *size)
{
	WaveFile *input = (WaveFile *) userdata;
	size_t bytes_left = input->size - input->offset;
	size_t target_size = *size < bytes_left ? *size : bytes_left;
	memcpy(buffer, input->data + input->offset, target_size);
	input->offset += target_size;
	*size = target_size;
	return WUFF_SUCCESS;
}

static wuff_sint32 seek_callback(void *userdata, wuff_uint64 offset)
{
	WaveFile *input = (WaveFile *)userdata;
	input->offset = (size_t) (offset < input->size ? offset : input->size);
	return WUFF_SUCCESS;
}

static wuff_sint32 tell_callback(void *userdata, wuff_uint64 *offset)
{
	WaveFile *input = (WaveFile *)userdata;
	*offset = input->offset;
	return WUFF_SUCCESS;
}

wuff_callback WaveDecoderCallbacks = {read_callback, seek_callback, tell_callback};


WaveDecoder::WaveDecoder(Data *data, const std::string &ext, int bufferSize)
	: Decoder(data, ext, bufferSize)
{
	dataFile.data = (char *) data->getData();
	dataFile.size = data->getSize();
	dataFile.offset = 0;

	int wuff_status = wuff_open(&handle, &WaveDecoderCallbacks, &dataFile);
	if (wuff_status < 0)
		throw love::Exception("Could not open WAVE");

	try
	{
		wuff_status = wuff_stream_info(handle, &info);
		if (wuff_status < 0)
			throw love::Exception("Could not retrieve WAVE stream info");

		if (info.channels > 2)
			throw love::Exception("Multichannel audio not supported");

		if (info.format != WUFF_FORMAT_PCM_U8 && info.format != WUFF_FORMAT_PCM_S16)
		{
			wuff_status = wuff_format(handle, WUFF_FORMAT_PCM_S16);
			if (wuff_status < 0)
				throw love::Exception("Could not set output format");
		}
	}
	catch (love::Exception &)
	{
		wuff_close(handle);
		throw;
	}
}

WaveDecoder::~WaveDecoder()
{
	wuff_close(handle);
}

bool WaveDecoder::accepts(const std::string &ext)
{
	static const std::string supported[] =
	{
		"wav", ""
	};

	for (int i = 0; !(supported[i].empty()); i++)
	{
		if (supported[i].compare(ext) == 0)
			return true;
	}

	return false;
}

love::sound::Decoder *WaveDecoder::clone()
{
	return new WaveDecoder(data.get(), ext, bufferSize);
}

int WaveDecoder::decode()
{
	size_t size = 0;

	while (size < (size_t) bufferSize)
	{
		size_t bytes = bufferSize-size;
		int wuff_status = wuff_read(handle, (wuff_uint8 *) buffer+size, &bytes);

		if (wuff_status < 0)
			return 0;
		else if (bytes == 0)
		{
			eof = true;
			break;
		}

		size += bytes;
	}

	return (int) size;
}

bool WaveDecoder::seek(float s)
{
	int wuff_status = wuff_seek(handle, (wuff_uint64) (s * info.sample_rate));

	if (wuff_status >= 0)
	{
		eof = false;
		return true;
	}

	return false;
}

bool WaveDecoder::rewind()
{
	int wuff_status = wuff_seek(handle, 0);

	if (wuff_status >= 0)
	{
		eof = false;
		return true;
	}

	return false;
}

bool WaveDecoder::isSeekable()
{
	return true;
}

int WaveDecoder::getChannels() const
{
	return info.channels;
}

int WaveDecoder::getBitDepth() const
{
	return info.bits_per_sample == 8 ? 8 : 16;
}

int WaveDecoder::getSampleRate() const
{
	return info.sample_rate;
}

double WaveDecoder::getDuration()
{
	return (double) info.length / (double) info.sample_rate;
}

} // lullaby
} // sound
} // love
