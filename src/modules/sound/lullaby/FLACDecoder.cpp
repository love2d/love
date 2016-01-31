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

#if 0

#include "FLACDecoder.h"

#include <set>
#include "common/Exception.h"

namespace love
{
namespace sound
{
namespace lullaby
{

FLACDecoder::FLACDecoder(Data *data, const std::string &ext, int nbufferSize)
	: Decoder(data, ext, nbufferSize)
	, pos(0)
{
	init();
	init_ogg();
	process_until_end_of_metadata();
	process_single();
	seek(0);
	bufferSize = 256 * getBitDepth() * getChannels() * 2;
	delete[](char *) buffer;
	buffer = new char[bufferSize];
}

FLACDecoder::~FLACDecoder()
{
	finish();
}

bool FLACDecoder::accepts(const std::string &ext)
{
	static const std::string supported[] =
	{
		"flac", ""
	};

	for (int i = 0; !(supported[i].empty()); i++)
	{
		if (supported[i].compare(ext) == 0)
			return true;
	}

	return false;
}

love::sound::Decoder *FLACDecoder::clone()
{
	return new FLACDecoder(data.get(), ext, bufferSize, sampleRate);
}

int FLACDecoder::decode()
{
	process_single();
	return bufferSize;
}

bool FLACDecoder::seek(float s)
{
	return seek_absolute((int)(s*1000.0f));
}

bool FLACDecoder::rewind()
{
	return seek_absolute(0);
}

bool FLACDecoder::isSeekable()
{
	return true;
}

int FLACDecoder::getChannels() const
{
	return get_channels();
}

int FLACDecoder::getBitDepth() const
{
	return get_bits_per_sample();
}

int FLACDecoder::getSampleRate() const
{
	return get_sample_rate();
}

double FLACDecoder::getDuration()
{
	return -1;
}

FLAC__StreamDecoderReadStatus FLACDecoder::read_callback(FLAC__byte buffer[], size_t *bytes)
{
	int size = data->getSize();
	char *d = (char *) data->getData() + pos;
	if (pos >= size)
	{
		eof = true;
		return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
	}
	if (pos+*bytes <= (unsigned int)size)
	{
		memcpy(buffer, d, *bytes);
		pos = pos+*bytes;
	}
	else
	{
		memcpy(buffer, d, size-pos);
		 *bytes = size-pos;
		pos = size;
	}
	return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

FLAC__StreamDecoderSeekStatus FLACDecoder::seek_callback(FLAC__uint64 offset)
{
	pos = (int)offset;
	return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}

FLAC__StreamDecoderTellStatus FLACDecoder::tell_callback(FLAC__uint64 *offset)
{
	 *offset = pos;
	return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}

FLAC__StreamDecoderLengthStatus FLACDecoder::length_callback(FLAC__uint64 *length)
{
	 *length = data->getSize();
	return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}

bool FLACDecoder::eof_callback()
{
	eof = (pos >= data->getSize());
	return eof;
}

FLAC__StreamDecoderWriteStatus FLACDecoder::write_callback(const FLAC__Frame *frame, const FLAC__int32 *const fbuffer[])
{
	int i, j;
	for (i = 0, j = 0; i < bufferSize; i += 4, j++)
	{
		((char *)buffer)[i] = fbuffer[0][j];
		((char *)buffer)[i+1] = fbuffer[0][j] >> 8;
		((char *)buffer)[i+2] = fbuffer[1][j];
		((char *)buffer)[i+3] = fbuffer[1][j] >> 8;
	}
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void FLACDecoder::metadata_callback(const FLAC__StreamMetadata *metadata)
{
	//we do nothing with metadata...
	return;
}

void FLACDecoder::error_callback(FLAC__StreamDecoderErrorStatus status)
{
	//wow.. error, let's throw one (please clean this part up sometime)
	throw love::Exception("FLAC error: %s!", FLAC__StreamDecoderErrorStatusString[status]);
}
} // lullaby
} // sound
} // love

#endif // 0
