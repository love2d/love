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

#include "Sound.h"

#include <common/Exception.h>

namespace love
{
namespace audio
{
namespace openal
{
	Sound::Sound(Pool * pool, love::sound::SoundData * data)
		: pool(pool), buffer(0), source(source)
	{
		
		// Generate the buffer.
		alGenBuffers(1, &buffer);

		int fmt = pool->getFormat(data->getChannels(), data->getBits());

		if(fmt == 0)
			throw love::Exception("Unsupported audio format.");

		alBufferData(buffer, fmt, data->getData(), data->getSize(), data->getSampleRate());

		// Note: we're done with the sound data
		// at this point. No need to retain.
	}

	Sound::~Sound()
	{
		if(buffer)
			alDeleteBuffers(1, &buffer);
	}

	void Sound::play(love::audio::Source * s)
	{
		// Set the buffer for the sound.
		source = pool->find(s);

		if(source)
		{
			alSourcei(source, AL_BUFFER, buffer);
			alSourcei(source, AL_LOOPING, s->isLooping() ? AL_TRUE : AL_FALSE);
		}
	}

	void Sound::update(love::audio::Source * s)
	{
		// Looping mode could have changed.
		alSourcei(source, AL_LOOPING, s->isLooping() ? AL_TRUE : AL_FALSE);
	}

	void Sound::stop(love::audio::Source * s)
	{
		// Also no need.
	}

	void Sound::rewind(love::audio::Source * s)
	{
		if(source)
			alSourceRewind(source);
	}

} // openal
} // audio
} // love
