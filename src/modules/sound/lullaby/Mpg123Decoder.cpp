/**
* Copyright (c) 2006-2009 LOVE Development Team
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

	Mpg123Decoder::Mpg123Decoder(Data * data, const std::string & ext, int bufferSize, int sampleRate)
		: Decoder(data, ext, bufferSize, sampleRate), handle(0)
	{

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
		ret = mpg123_format(handle, sampleRate, MPG123_STEREO, MPG123_ENC_SIGNED_16);
		if (ret != MPG123_OK)
			throw love::Exception("Could not set output format.");
		size_t numbytes = 0;
		ret = mpg123_feed(handle, (unsigned char*)data->getData(), data->getSize());
		
		if(ret == MPG123_NEW_FORMAT)
		{
			long rate;
			int channels;
			int encoding;
			mpg123_getformat(handle, &rate, &channels, &encoding);
		}
		else if(ret != 0)
		{
			throw love::Exception(mpg123_strerror(handle));
		}

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
		return new Mpg123Decoder(data, ext, bufferSize, sampleRate);
	}

	int Mpg123Decoder::decode()
	{
		size_t numbytes;
		
		for(int i = 0; i < 2; ++i)
		{
			int r = mpg123_read(handle, (unsigned char*) buffer, bufferSize, &numbytes);

			switch(r)
			{
			case MPG123_NEW_FORMAT:
				continue;
			case MPG123_DONE:
				eof = false;
			case MPG123_OK:
			default:
				return numbytes;
			}
		}


		// If we're done, then EOF. If some error occurred, pretend we EOF'd.
		/*
		if (r == MPG123_DONE || r != MPG123_OK)
		{
			if ( r == MPG123_NEED_MORE )
			{
				size_t numbytes = 0;
				mpg123_decode(handle, (unsigned char*) data->getData(), data->getSize(), (unsigned char*) buffer, bufferSize, &numbytes);
			}
			else
			{
				eof = true;
				numbytes = 0;
			}
		}
		*/

		return numbytes;
	}

	bool Mpg123Decoder::seek(float s)
	{
		off_t r = mpg123_seek(handle, (off_t)(s*1000.0f), SEEK_SET);

		return (r >= 0);
	}

	bool Mpg123Decoder::rewind()
	{
		if(mpg123_seek(handle, 0, SEEK_SET) >= 0)
			return false;
		return true;
	}

	bool Mpg123Decoder::isSeekable()
	{
		return true;
	}

	int Mpg123Decoder::getChannels() const
	{
		return 2;
	}

	int Mpg123Decoder::getBits() const
	{
		return 16;
	}


} // lullaby
} // sound
} // love
