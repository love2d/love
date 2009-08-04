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

#include "Source.h"

// STD
#include <iostream>

namespace love
{
namespace audio
{
namespace openal
{

	Source::Source(Pool * pool)
		: pool(pool), source(0), pitch(1.0f), volume(1.0f)
	{
	}

	Source::Source(Pool * pool, Audible * audible)
		: pool(pool), source(0), pitch(1.0f), volume(1.0f)
	{
		setAudible(audible);
	}

	Source::~Source()
	{
	}

	void Source::play()
	{
		if(source != 0)
			return; // Already playing.

		if(pool->isAvailable())
			source = pool->claim(this);

		if(source != 0)
		{
			audible->play(this);

			// Set these properties. These may have changed while we've
			// been without an AL source.
			alSourcef(source, AL_PITCH, pitch);
			alSourcef(source, AL_GAIN, volume);

			alSourcePlay(source);
		}

	}

	void Source::stop()
	{
		if(source)
		{
			alSourceStop(source);
			audible->stop(this);
			source = 0;
		}
	}

	void Source::pause()
	{
		if(source)
		{
			alSourcePause(source);
		}
	}

	void Source::resume()
	{
		if(source)
		{
			alSourcePlay(source);
		}
	}

	void Source::rewind()
	{
		if(audible != 0)
			audible->rewind(this);	
	}

	bool Source::isFinished() const
	{
		if(source)
		{
			ALenum state;
			alGetSourcei(source, AL_SOURCE_STATE, &state);
			return (state == AL_STOPPED);
		}

		return true;		
	}

	void Source::update()
	{
		if(audible != 0)
			audible->update(this);
	}

	void Source::setPitch(float pitch)
	{
		if(source)
		{
			alSourcef(source, AL_PITCH, pitch);
		}

		this->pitch = pitch;
	}

	float Source::getPitch() const
	{
		if(source)
		{
			ALfloat f;
			alGetSourcef(source, AL_PITCH, &f);
			return f;
		}

		// In case the Source isn't playing.
		return pitch;
	}

	void Source::setVolume(float volume)
	{
		if(source)
		{
			alSourcef(source, AL_GAIN, volume);
		}

		this->volume = volume;
	}

	float Source::getVolume() const
	{
		if(source)
		{
			ALfloat f;
			alGetSourcef(source, AL_GAIN, &f);
			return f;
		}

		// In case the Source isn't playing.
		return volume;
	}

	void Source::setPosition(float * v)
	{
		if(source)
			alSourcefv(source, AL_POSITION, v);
	}

	void Source::getPosition(float * v) const
	{
		if(source)
			alGetSourcefv(source, AL_POSITION, v);
	}

	void Source::setVelocity(float * v)
	{
		if(source)
			alSourcefv(source, AL_VELOCITY, v);
	}

	void Source::getVelocity(float * v) const
	{
		if(source)
			alGetSourcefv(source, AL_VELOCITY, v);
	}

	void Source::setDirection(float * v)
	{
		if(source)
			alSourcefv(source, AL_DIRECTION, v);
	}

	void Source::getDirection(float * v) const
	{
		if(source)
			alGetSourcefv(source, AL_DIRECTION, v);
	}

} // openal
} // audio
} // love
