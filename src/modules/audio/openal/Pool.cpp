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

#include "Pool.h"

#define MUTEX_ASSERT(fn, sval) \
	if(fn != sval) \
	{ \
		std::cout << "Mutex lock/unlock failure. " << SDL_GetError() << std::endl; \
		exit(-1); \
	} \

#define LOCK(m) MUTEX_ASSERT(SDL_LockMutex(m), 0)
#define UNLOCK(m) MUTEX_ASSERT(SDL_UnlockMutex(m), 0)

namespace love
{
namespace audio
{
namespace openal
{
	Pool::Pool()
	{
		// Generate sources.
		alGenSources(NUM_SOURCES, sources);

		// Create the mutex.
		mutex = SDL_CreateMutex();

		if(alGetError() != AL_NO_ERROR)
			throw love::Exception("Could not generate sources.");

		// Make all sources available initially.
		for(int i = 0; i < NUM_SOURCES; i++)
			available.push(sources[i]);
	}

	Pool::~Pool()
	{
		stop();

		SDL_DestroyMutex(mutex);

		// Free all sources.
		alDeleteSources(NUM_SOURCES, sources);
	}

	ALenum Pool::getFormat(int channels, int bits) const
	{
		if(channels == 1 && bits == 8)
			return AL_FORMAT_MONO8;
		else if(channels == 1 && bits == 16)
			return AL_FORMAT_MONO16;
		else if(channels == 2 && bits == 8)
			return AL_FORMAT_STEREO8;
		else if(channels == 2 && bits == 16)
			return AL_FORMAT_STEREO16;
		else
			return 0;
	}

	bool Pool::isAvailable() const
	{
		bool has = false;
		LOCK(mutex);
		has = !available.empty();
		UNLOCK(mutex);
		return has;
	}

	ALuint Pool::claim(love::audio::Source * source)
	{
		ALuint s = 0;
		LOCK(mutex);
		if(!available.empty())
		{
			// Get the first available source.
			s = available.front();

			// Remove it.
			available.pop();

			// Insert into map of playing sources.
			playing.insert(std::pair<love::audio::Source *, ALuint>(source, s));

			source->retain();
		}
		UNLOCK(mutex);

		return s;
	}

	ALuint Pool::find(const love::audio::Source * source) const
	{
		ALuint r = 0;
		LOCK(mutex);
		r = findi(source);
		UNLOCK(mutex);
		return r;
	}

	bool Pool::isPlaying(love::audio::Source * s)
	{
		bool p = false;
		LOCK(mutex);
		for(std::map<love::audio::Source *, ALuint>::iterator i = playing.begin(); i != playing.end(); i++)
		{
			if(i->first == s)
				p = true;
		}
		UNLOCK(mutex);
		return p;
	}

	void Pool::update()
	{
		LOCK(mutex);

		std::map<love::audio::Source *, ALuint>::iterator i = playing.begin();
	
		while(i != playing.end())
		{
			if(i->first->isStopped())
			{
				i->first->stop();
				i->first->release();
				available.push(i->second);
				playing.erase(i++);
			}
			else
			{
				i->first->update();
				i++;
			}
		}

		UNLOCK(mutex);
	}

	int Pool::getNumSources() const
	{
		return playing.size();
	}

	int Pool::getMaxSources() const
	{
		return NUM_SOURCES;
	}

	void Pool::stop()
	{
		LOCK(mutex);
		for(std::map<love::audio::Source *, ALuint>::iterator i = playing.begin(); i != playing.end(); i++)
		{
			i->first->stop();
			available.push(i->second);
		}

		playing.clear();

		UNLOCK(mutex);
	}

	void Pool::pause()
	{
		LOCK(mutex);
		for(std::map<love::audio::Source *, ALuint>::iterator i = playing.begin(); i != playing.end(); i++)
			i->first->pause();
		UNLOCK(mutex);
	}

	void Pool::resume()
	{
		LOCK(mutex);
		for(std::map<love::audio::Source *, ALuint>::iterator i = playing.begin(); i != playing.end(); i++)
			i->first->resume();
		UNLOCK(mutex);
	}

	void Pool::rewind()
	{
		LOCK(mutex);
		for(std::map<love::audio::Source *, ALuint>::iterator i = playing.begin(); i != playing.end(); i++)
			i->first->rewind();
		UNLOCK(mutex);
	}

	void Pool::release(love::audio::Source * source)
	{
		ALuint s = findi(source);

		if(s != 0)
		{
			available.push(s);
			playing.erase(source);
		}
	}

	ALuint Pool::findi(const love::audio::Source * source) const
	{
		std::map<love::audio::Source *, ALuint>::const_iterator i = playing.find((love::audio::Source *)source);

		if(i != playing.end())
			return i->second;

		return 0;
	}

} // openal
} // audio
} // love
