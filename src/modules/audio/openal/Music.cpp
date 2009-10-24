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

#include "Music.h"

// STD
#include <iostream>

namespace love
{
namespace audio
{
namespace openal
{
	Music::Music(Pool * pool, love::sound::Decoder * decoder)
		: pool(pool), decoder(decoder), source(0)
	{
		decoder->retain();
		alGenBuffers(NUM_BUFFERS, buffers);
	}

	Music::~Music()
	{
		decoder->release();
		alDeleteBuffers(NUM_BUFFERS, buffers);
	}

	love::audio::Music * Music::clone()
	{
		return new Music(pool, decoder->clone());
	}

	void Music::play(love::audio::Source * s)
	{
		source = pool->find(s);

		if(source)
		{
			for(int i = 0; i < NUM_BUFFERS; i++)
			{
				if(!stream(s, buffers[i]))
				{
					std::cout << "Could not stream music." << std::endl;
					return;
				}
			}

			alSourceQueueBuffers(source, NUM_BUFFERS, buffers);
		}
	}

	void Music::update(love::audio::Source * s)
	{
		if(source)
		{
			// Number of processed buffers.
			ALint processed;

			alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);

			while(processed--)
			{
				ALuint buffer;

				// Get a free buffer.
				alSourceUnqueueBuffers(source, 1, &buffer);

				if(stream(s, buffer))
					alSourceQueueBuffers(source, 1, &buffer);
			}
		}
	}

	void Music::stop(love::audio::Source * s)
	{
		if(source)
		{
			ALuint bufs[NUM_BUFFERS];
			alSourceStop(source);
			alSourceUnqueueBuffers(source, NUM_BUFFERS, bufs);
			source = 0;
			rewind(s);
		}
	}

	void Music::rewind(love::audio::Source * s)
	{
		decoder->rewind();
	}

	bool Music::stream(love::audio::Source * source, ALuint buffer)
	{
		if(source->isLooping() && decoder->isFinished())
			decoder->rewind();		

		// Get more sound data.
		int decoded = decoder->decode();

		int fmt = pool->getFormat(decoder->getChannels(), decoder->getBits());

		if(fmt == 0)
			return false;

		if(decoded > 0)
		{
			alBufferData(buffer, fmt, decoder->getBuffer(), 
				decoder->getSize(), decoder->getSampleRate());
			return true;

		}
		return false;
	}


} // openal
} // audio
} // love