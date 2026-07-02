/**
 * Copyright (c) 2006-2026 LOVE Development Team
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

#include "OpusDecoder.h"

#include "common/Exception.h"

namespace love
{
namespace sound
{
namespace lullaby
{

static int opusRead(void *_stream, unsigned char *ptr, int sizeToRead)
{
	auto stream = (Stream *) _stream;
	return stream->read(ptr, sizeToRead);
}

static int opusSeek(void *_stream, opus_int64 offset, int whence)
{
	auto stream = (Stream *) _stream;
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

static opus_int64 opusTell(void *_stream)
{
	auto stream = (Stream *) _stream;
	return (opus_int64) stream->tell();
}
/**
 * END CALLBACK FUNCTIONS
 **/

OpusDecoder::OpusDecoder(Stream *stream, int bufferSize)
: Decoder(stream, bufferSize)
, duration(-2.0)
{
	OpusFileCallbacks callbacks = {};
	callbacks.close = NULL;
	callbacks.seek  = opusSeek;
	callbacks.read  = opusRead;
	callbacks.tell  = opusTell;

	constexpr size_t LOOKUP_SIZE = 512;

	std::vector<uint8> data(LOOKUP_SIZE);
	uint8 *dataPtr = data.data();
	stream->read(dataPtr, LOOKUP_SIZE);

	handle = op_open_callbacks(stream, &callbacks, dataPtr, LOOKUP_SIZE, NULL);

	if (handle == NULL)
		throw love::Exception("Could not read Opus bitstream");

	opusHead = op_head(handle, -1);
}

OpusDecoder::~OpusDecoder()
{
	op_free(handle);
}

love::sound::Decoder *OpusDecoder::clone()
{
	StrongRef<Stream> s(stream->clone(), Acquire::NORETAIN);
	return new OpusDecoder(s, bufferSize);
}

int OpusDecoder::decode()
{
	int size = 0;

	while (size < bufferSize)
	{
		// libopus wants and returns per-channel samples
		int samplesToRead = (bufferSize - size) / (getChannelCount() * sizeof(opus_int16));
		if (samplesToRead <= 0)	break;

		int result = op_read(handle, (opus_int16 *)((char *)buffer + size), samplesToRead, NULL);

		if (result == OP_HOLE)
			continue;
		else if (result <= OP_EREAD)
			return -1;
		else if (result == 0)
		{
			eof = true;
			break;
		}
		else if (result > 0)
		{
			int bytesRead = result * getChannelCount() * sizeof(opus_int16);
			size += bytesRead;
		}
	}

	return size;
}

bool OpusDecoder::seek(double s)
{
	int result = op_pcm_seek(handle, (ogg_int64_t)(s * 48000.0));

	if (result == 0)
	{
		eof = false;
		return true;
	}

	return false;
}

bool OpusDecoder::rewind()
{
	int result = op_raw_seek(handle, 0);

	if (result == 0)
	{
		eof = false;
		return true;
	}

	return false;
}

bool OpusDecoder::isSeekable()
{
	return (op_seekable(handle) != 0);
}

int OpusDecoder::getChannelCount() const
{
	return opusHead->channel_count;
}

int OpusDecoder::getBitDepth() const
{
	return 16;
}

int OpusDecoder::getSampleRate() const
{
	// return (int) opusHead->input_sample_rate;
	return 48000;
}

double OpusDecoder::getDuration()
{
	// Only calculate the duration if we haven't done so already.
	if (duration == -2.0)
	{
		duration = op_pcm_total(handle, -1);

		if (duration == OP_EINVAL || duration < 0.0)
			duration = -1.0;
		else
			duration = duration / 48000.0;
	}

	return duration;
}

} // lullaby
} // sound
} // love
