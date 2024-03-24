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

#define DR_FLAC_IMPLEMENTATION
#include "FLACDecoder.h"

#include <set>
#include <algorithm>
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

static drflac_bool32 onSeek(void* pUserData, int offset, drflac_seek_origin origin)
{
	auto stream = (Stream *) pUserData;
	auto seekorigin = origin == drflac_seek_origin_current ? Stream::SEEKORIGIN_CURRENT : Stream::SEEKORIGIN_BEGIN;
	return stream->seek(offset, seekorigin) ? DRFLAC_TRUE : DRFLAC_FALSE;
}

FLACDecoder::FLACDecoder(Stream *stream, int nbufferSize)
	: Decoder(stream, nbufferSize)
{
	flac = drflac_open(onRead, onSeek, stream, nullptr);
	if (flac == nullptr)
		throw love::Exception("Could not load FLAC file");
}

FLACDecoder::~FLACDecoder()
{
	drflac_close(flac);
}

love::sound::Decoder *FLACDecoder::clone()
{
	StrongRef<Stream> s(stream->clone(), Acquire::NORETAIN);
	return new FLACDecoder(s, bufferSize);
}

int FLACDecoder::decode()
{
	// `bufferSize` is in bytes, so divide by 2.
	drflac_uint64 read = drflac_read_pcm_frames_s16(flac, bufferSize / 2 / flac->channels, (drflac_int16 *) buffer);
	read *= 2 * flac->channels;

	if ((int) read < bufferSize)
		eof = true;

	return (int) read;
}

bool FLACDecoder::seek(double s)
{
	drflac_uint64 seekPosition = (drflac_uint64) (s * flac->sampleRate);

	drflac_bool32 result = drflac_seek_to_pcm_frame(flac, seekPosition);
	if (result)
		eof = false;

	return result;
}

bool FLACDecoder::rewind()
{
	return seek(0);
}

bool FLACDecoder::isSeekable()
{
	return true;
}

int FLACDecoder::getChannelCount() const
{
	return flac->channels;
}

int FLACDecoder::getBitDepth() const
{
	return 16;
}

int FLACDecoder::getSampleRate() const
{
	return flac->sampleRate;
}

double FLACDecoder::getDuration()
{
	return ((double) flac->totalPCMFrameCount) / ((double) flac->sampleRate);
}

} // lullaby
} // sound
} // love
