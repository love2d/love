/**
 * Copyright (c) 2006-2024 LOVE Development Team
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

#include "VorbisDecoder.h"

#include <string.h>
#include "common/config.h"
#include "common/Exception.h"

namespace love
{
namespace sound
{
namespace lullaby
{

static int vorbisClose(void *)
{
	// Does nothing (handled elsewhere)
	return 1;
}

static size_t vorbisRead(void *ptr, size_t byteSize, size_t sizeToRead, void *datasource)
{
	auto stream = (Stream *) datasource;
	return stream->read(ptr, byteSize * sizeToRead);
}

static int vorbisSeek(void *datasource, ogg_int64_t offset, int whence)
{
	auto stream = (Stream *) datasource;
	auto origin = Stream::SEEKORIGIN_BEGIN;

	switch (whence)
	{
	case SEEK_SET:
		origin = Stream::SEEKORIGIN_BEGIN;
		break;
	case SEEK_CUR:
		origin = Stream::SEEKORIGIN_CURRENT;
		break;
	case SEEK_END:
		origin = Stream::SEEKORIGIN_END;
		break;
	default:
		break;
	};

	return stream->seek(offset, origin) ? 0 : -1;
}

static long vorbisTell(void *datasource)
{
	auto stream = (Stream *) datasource;
	return (long) stream->tell();
}
/**
 * END CALLBACK FUNCTIONS
 **/

VorbisDecoder::VorbisDecoder(Stream *stream, int bufferSize)
	: Decoder(stream, bufferSize)
	, duration(-2.0)
{
	ov_callbacks callbacks = {};
	callbacks.close_func = vorbisClose;
	callbacks.seek_func  = vorbisSeek;
	callbacks.read_func  = vorbisRead;
	callbacks.tell_func  = vorbisTell;

	// Open Vorbis handle
	if (ov_open_callbacks(stream, &handle, nullptr, 0, callbacks) < 0)
		throw love::Exception("Could not read Ogg bitstream");

	vorbisInfo = ov_info(&handle, -1);
}

VorbisDecoder::~VorbisDecoder()
{
	ov_clear(&handle);
}

love::sound::Decoder *VorbisDecoder::clone()
{
	StrongRef<Stream> s(stream->clone(), Acquire::NORETAIN);
	return new VorbisDecoder(s, bufferSize);
}

int VorbisDecoder::decode()
{
	int size = 0;

#ifdef LOVE_BIG_ENDIAN
	int endian = 1;
#else
	int endian = 0;
#endif

	while (size < bufferSize)
	{
		long result = ov_read(&handle, (char *) buffer + size, bufferSize - size, endian, (getBitDepth() == 16 ? 2 : 1), 1, 0);

		if (result == OV_HOLE)
			continue;
		else if (result <= OV_EREAD)
			return -1;
		else if (result == 0)
		{
			eof = true;
			break;
		}
		else if (result > 0)
			size += result;
	}

	return size;
}

bool VorbisDecoder::seek(double s)
{
	int result = 0;

	// Avoid ov_time_seek (which calls ov_pcm_seek) when seeking to 0, to avoid
	// a bug in libvorbis <= 1.3.4 when seeking to PCM 0 in multiplexed streams.
	if (s <= 0.000001)
		result = ov_raw_seek(&handle, 0);
	else
		result = ov_time_seek(&handle, s);

	if (result == 0)
	{
		eof = false;
		return true;
	}

	return false;
}

bool VorbisDecoder::rewind()
{
	// Avoid ov_time_seek to avoid a bug in libvorbis <= 1.3.4 when seeking to
	// PCM 0 in multiplexed streams.
	int result = ov_raw_seek(&handle, 0);

	if (result == 0)
	{
		eof = false;
		return true;
	}

	return false;
}

bool VorbisDecoder::isSeekable()
{
	long result = ov_seekable(&handle);
	return (result != 0);
}

int VorbisDecoder::getChannelCount() const
{
	return vorbisInfo->channels;
}

int VorbisDecoder::getBitDepth() const
{
	return 16;
}

int VorbisDecoder::getSampleRate() const
{
	return (int) vorbisInfo->rate;
}

double VorbisDecoder::getDuration()
{
	// Only calculate the duration if we haven't done so already.
	if (duration == -2.0)
	{
		duration = ov_time_total(&handle, -1);

		if (duration == OV_EINVAL || duration < 0.0)
			duration = -1.0;
	}

	return duration;
}

} // lullaby
} // sound
} // love
