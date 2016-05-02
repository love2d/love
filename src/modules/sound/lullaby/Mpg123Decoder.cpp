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

#include "Mpg123Decoder.h"

#include "common/Exception.h"

#include <iostream>

#ifndef LOVE_NOMPG123

namespace love
{
namespace sound
{
namespace lullaby
{

/**
 * mpg123 callbacks for seekable streams.
 **/

static ssize_t read_callback(void *udata, void *buffer, size_t count)
{
	DecoderFile *file = (DecoderFile *) udata;

	// Calculates how much data is still left and takes that value or
	// the buffer size, whichever is lower, as the number of bytes to write.
	size_t countLeft = file->size - file->offset;
	size_t countWrite = countLeft < count ? countLeft : count;

	if (countWrite > 0)
	{
		memcpy(buffer, file->data + file->offset, countWrite);
		file->offset += countWrite;
	}

	// Returns the number of written bytes. 0 means EOF.
	return countWrite;
}

static off_t seek_callback(void *udata, off_t offset, int whence)
{
	DecoderFile *file = (DecoderFile *) udata;

	switch (whence)
	{
	case SEEK_SET:
		// Negative values are invalid at this point.
		if (offset < 0)
			return -1;

		// Prevents the offset from going over EOF.
		if (file->size > (size_t) offset)
			file->offset = offset;
		else
			file->offset = file->size;
		break;
	case SEEK_END:
		// Offset is set to EOF. Offset calculation is just like SEEK_CUR.
		file->offset = file->size;
	case SEEK_CUR:
		// Prevents the offset from going over EOF or below 0.
		if (offset > 0)
		{
			if (file->size > file->offset + (size_t) offset)
				file->offset = file->offset + offset;
			else
				file->offset = file->size;
		}
		else if (offset < 0)
		{
			if (file->offset >= (size_t) (-offset))
				file->offset = file->offset - (size_t) (-offset);
			else
				file->offset = 0;
		}
		break;
	default:
		return -1;
	};
	
	return file->offset;
}

static void cleanup_callback(void *)
{
	// Cleanup is done by the Decoder class.
}

bool Mpg123Decoder::inited = false;

Mpg123Decoder::Mpg123Decoder(Data *data, const std::string &ext, int bufferSize)
	: Decoder(data, ext, bufferSize)
	, decoder_file(data)
	, handle(0)
	, channels(MPG123_STEREO)
	, duration(-2.0)
{
	int ret = 0;

	if (!inited)
	{
		ret = mpg123_init();
		if (ret != MPG123_OK)
			throw love::Exception("Could not initialize mpg123.");
		inited = (ret == MPG123_OK);
	}

	// Intialize the handle.
	handle = mpg123_new(nullptr, nullptr);
	if (handle == nullptr)
		throw love::Exception("Could not create decoder.");

	// Suppressing all mpg123 messages.
	mpg123_param(handle, MPG123_ADD_FLAGS, MPG123_QUIET, 0);

	try
	{
		ret = mpg123_replace_reader_handle(handle, &read_callback, &seek_callback, &cleanup_callback);
		if (ret != MPG123_OK)
			throw love::Exception("Could not set decoder callbacks.");

		ret = mpg123_open_handle(handle, &decoder_file);
		if (ret != MPG123_OK)
			throw love::Exception("Could not open decoder.");

		// mpg123_getformat should be able to tell us the properties of the stream's first frame.
		long rate = 0;
		ret = mpg123_getformat(handle, &rate, &channels, nullptr);
		if (ret == MPG123_ERR)
			throw love::Exception("Could not get stream information.");

		// I forgot what this was about.
		if (channels == 0)
			channels = 2;

		// Force signed 16-bit output.
		mpg123_param(handle, MPG123_FLAGS, (channels == 2 ? MPG123_FORCE_STEREO : MPG123_MONO_MIX), 0);
		mpg123_format_none(handle);
		mpg123_format(handle, rate, channels, MPG123_ENC_SIGNED_16);

		sampleRate = rate;
	}
	catch (love::Exception &)
	{
		mpg123_delete(handle);
		throw;
	}
}

Mpg123Decoder::~Mpg123Decoder()
{
	mpg123_delete(handle);

}

bool Mpg123Decoder::accepts(const std::string &ext)
{
	static const std::string supported[] =
	{
		"mp3", ""
	};

	for (int i = 0; !(supported[i].empty()); i++)
	{
		if (supported[i].compare(ext) == 0)
			return true;
	}

	return false;
}

void Mpg123Decoder::quit()
{
	if (inited)
		mpg123_exit();
}

love::sound::Decoder *Mpg123Decoder::clone()
{
	return new Mpg123Decoder(data.get(), ext, bufferSize);
}

int Mpg123Decoder::decode()
{
	int size = 0;

	while (size < bufferSize && !eof)
	{
		size_t numbytes = 0;
		int res = mpg123_read(handle, (unsigned char *) buffer + size, bufferSize - size, &numbytes);

		switch (res)
		{
		case MPG123_NEED_MORE:
		case MPG123_NEW_FORMAT:
		case MPG123_OK:
			size += numbytes;
			continue;
		case MPG123_DONE:
			size += numbytes;
			eof = true;
		default:
			return size;
		}
	}

	return size;
}

bool Mpg123Decoder::seek(float s)
{
	off_t offset = (off_t) (s * (double) sampleRate);

	if (offset < 0)
		return false;

	if (mpg123_seek(handle, offset, SEEK_SET) >= 0)
	{
		eof = false;
		return true;
	}
	else
		return false;
}

bool Mpg123Decoder::rewind()
{
	eof = false;

	if (mpg123_seek(handle, 0, SEEK_SET) >= 0)
		return true;
	else
		return false;
}

bool Mpg123Decoder::isSeekable()
{
	return true;
}

int Mpg123Decoder::getChannels() const
{
	return channels;
}

int Mpg123Decoder::getBitDepth() const
{
	return 16;
}

double Mpg123Decoder::getDuration()
{
	// Only calculate the duration if we haven't done so already.
	if (duration == -2.0)
	{
		mpg123_scan(handle);

		off_t length = mpg123_length(handle);

		if (length == MPG123_ERR || length < 0)
			duration = -1.0;
		else
			duration = (double) length / (double) sampleRate;
	}

	return duration;
}

} // lullaby
} // sound
} // love

#endif // LOVE_NOMPG123
