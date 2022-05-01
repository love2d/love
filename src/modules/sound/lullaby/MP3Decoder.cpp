/**
 * Copyright (c) 2006-2022 LOVE Development Team
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

#define DR_MP3_IMPLEMENTATION
#define DR_MP3_NO_STDIO
#include "MP3Decoder.h"
#include "common/Exception.h"

namespace love
{
namespace sound
{
namespace lullaby
{

static size_t onRead(void *pUserData, void *pBufferOut, size_t bytesToRead)
{
	auto stream = (Stream *) pUserData;
	int64 read = stream->read(pBufferOut, bytesToRead);
	return std::max<int64>(0, read);
}

static drmp3_bool32 onSeek(void *pUserData, int offset, drmp3_seek_origin origin)
{
	auto stream = (Stream *) pUserData;
	auto seekorigin = origin == drmp3_seek_origin_current ? Stream::SEEKORIGIN_CURRENT : Stream::SEEKORIGIN_BEGIN;
	return stream->seek(offset, seekorigin) ? DRMP3_TRUE : DRMP3_FALSE;
}

MP3Decoder::MP3Decoder(Stream *stream, int bufferSize)
	: Decoder(stream, bufferSize)
{
	// initialize mp3 handle
	if (!drmp3_init(&mp3, onRead, onSeek, stream, nullptr, nullptr))
		throw love::Exception("Could not read mp3 data.");

	sampleRate = mp3.sampleRate;

	// calculate duration
	drmp3_uint64 pcmCount, mp3FrameCount;
	if (!drmp3_get_mp3_and_pcm_frame_count(&mp3, &mp3FrameCount, &pcmCount))
	{
		drmp3_uninit(&mp3);
		throw love::Exception("Could not calculate mp3 duration.");
	}
	duration = ((double) pcmCount) / ((double) mp3.sampleRate);

	// create seek table
	uint32_t mp3FrameInt = mp3FrameCount;
	seekTable.resize(mp3FrameCount, {0ULL, 0ULL, 0, 0});
	if (!drmp3_calculate_seek_points(&mp3, &mp3FrameInt, seekTable.data()))
	{
		drmp3_uninit(&mp3);
		throw love::Exception("Could not calculate mp3 seek table");
	}
	mp3FrameInt = mp3FrameInt > mp3FrameCount ? mp3FrameCount : mp3FrameInt;

	// bind seek table
	if (!drmp3_bind_seek_table(&mp3, mp3FrameInt, seekTable.data()))
	{
		drmp3_uninit(&mp3);
		throw love::Exception("Could not bind mp3 seek table");
	}
}

MP3Decoder::~MP3Decoder()
{
	drmp3_uninit(&mp3);
}

love::sound::Decoder *MP3Decoder::clone()
{
	StrongRef<Stream> s(stream->clone(), Acquire::NORETAIN);
	return new MP3Decoder(s, bufferSize);
}

int MP3Decoder::decode()
{
	// bufferSize is in char
	int maxRead = bufferSize / sizeof(int16_t) / mp3.channels;
	int read = (int) drmp3_read_pcm_frames_s16(&mp3, maxRead, (drmp3_int16 *) buffer);

	if (read < maxRead)
		eof = true;

	return read * sizeof(int16_t) * mp3.channels;
}

bool MP3Decoder::seek(double s)
{
	drmp3_uint64 targetSample = s * mp3.sampleRate;
	drmp3_bool32 success = drmp3_seek_to_pcm_frame(&mp3, targetSample);

	if (success)
		eof = false;

	return success;
}

bool MP3Decoder::rewind()
{
	return seek(0.0);
}

bool MP3Decoder::isSeekable()
{
	return true;
}

int MP3Decoder::getChannelCount() const
{
	return mp3.channels;
}

int MP3Decoder::getBitDepth() const
{
	return 16;
}

double MP3Decoder::getDuration()
{
	return duration;
}

} // lullaby
} // sound
} // love
