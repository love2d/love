/**
* Copyright (c) 2006-2011 LOVE Development Team
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

#include <common/Exception.h>

#include <iostream>

namespace love
{
namespace sound
{
namespace lullaby
{

	bool Mpg123Decoder::inited = false;

	Mpg123Decoder::Mpg123Decoder(Data * data, const std::string & ext, int bufferSize)
		: Decoder(data, ext, bufferSize), handle(0), channels(MPG123_STEREO)
	{

		data_size = data->getSize();
		data_offset = 0;

		int ret;

		if(!inited)
		{
			ret = mpg123_init();
			if (ret != MPG123_OK)
				throw love::Exception("Could not initialize mpg123.");
			inited = (ret == MPG123_OK);
		}

		//Intialize the handle
		handle = mpg123_new(NULL, &ret);
		if (handle == NULL)
			throw love::Exception("Could not create handle.");
		ret = mpg123_open_feed(handle);
		if (ret != MPG123_OK)
			throw love::Exception("Could not open feed.");

		ret = feed(16384);

		if (ret != MPG123_OK && ret != MPG123_DONE)
			throw love::Exception("Could not feed!");
	}

	Mpg123Decoder::~Mpg123Decoder()
	{
		mpg123_close(handle);

	}

	bool Mpg123Decoder::accepts(const std::string & ext)
	{
		static const std::string supported[] = {
			"mp3", ""
		};

		for(int i = 0; !(supported[i].empty()); i++)
		{
			if(supported[i].compare(ext) == 0)
				return true;
		}

		return false;
	}

	void Mpg123Decoder::quit()
	{
		if(inited)
			mpg123_exit();
	}

	love::sound::Decoder * Mpg123Decoder::clone()
	{
		return new Mpg123Decoder(data, ext, bufferSize);
	}

	int Mpg123Decoder::decode()
	{
		int size = 0;
		bool done = false;

		while(size < bufferSize && !done && !eof)
		{
			size_t numbytes = 0;

			int r = mpg123_read(handle, (unsigned char*) buffer + size, bufferSize - size, &numbytes);

			switch(r)
			{
			case MPG123_NEW_FORMAT:
				{
					long rate = 0;
					int encoding = 0;
					int ret = mpg123_getformat(handle, &rate, &channels, &encoding);
					if (rate == 0)
						rate = sampleRate;
					else
						sampleRate = rate;
					if (channels == 0)
						channels = MPG123_STEREO;
					if (encoding == 0)
						encoding = MPG123_ENC_SIGNED_16;
					ret = mpg123_format(handle, rate, channels, encoding);
					if (ret != MPG123_OK)
						throw love::Exception("Could not set output format.");
				}
				continue;
			case MPG123_NEED_MORE:
				{
					size += numbytes;
					int v = feed(8192);

					switch(v)
					{
					case MPG123_OK:
						continue;
					case MPG123_DONE:
						if(numbytes == 0)
							eof = true;
						break;
					default:
						done = true;
					}

					continue;
				}
			case MPG123_OK:
				size += numbytes;
				continue;
			case MPG123_DONE:
				// Apparently, mpg123_read does not return MPG123_DONE, but
				// let's keep it here anyway.
				eof = true;
			default:
				done = true;
				break;
			}
		}

		return size;
	}

	bool Mpg123Decoder::seek(float s)
	{
		off_t offset = static_cast<off_t>(s * static_cast<float>(sampleRate));

		if(offset < 0)
			return false;

		if(mpg123_feedseek(handle, offset, SEEK_SET, &offset) >= 0)
		{
			data_offset = offset;
			eof = false;
			return true;
		}
		else
			return false;
	}

	bool Mpg123Decoder::rewind()
	{
		eof = false;
		off_t offset;

		if(mpg123_feedseek(handle, 0, SEEK_SET, &offset) >= 0)
		{
			data_offset = offset;
			return true;
		}
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

	int Mpg123Decoder::getBits() const
	{
		return 16;
	}

	int Mpg123Decoder::feed(int bytes)
	{
		int remaining = data_size - data_offset;

		if(remaining <= 0)
			return MPG123_DONE;

		int feed_bytes = remaining < bytes ? remaining : bytes;

		int r = mpg123_feed(handle, (unsigned char*)data->getData() + data_offset, feed_bytes);

		if(r == MPG123_OK || r == MPG123_DONE)
			data_offset += feed_bytes;

		return r;
	}

} // lullaby
} // sound
} // love
