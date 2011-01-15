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

#include "Pool.h"

#include "Source.h"

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

	bool Pool::isAvailable() const
	{
		bool has = false;
		LOCK(mutex);
		has = !available.empty();
		UNLOCK(mutex);
		return has;
	}

	bool Pool::isPlaying(Source * s)
	{
		bool p = false;
		LOCK(mutex);
		for(std::map<Source *, ALuint>::iterator i = playing.begin(); i != playing.end(); i++)
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

		std::map<Source *, ALuint>::iterator i = playing.begin();

		while(i != playing.end())
		{
			if(i->first->isStopped())
			{
				i->first->stopAtomic();
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

	bool Pool::play(Source * source, ALuint & out)
	{
		bool ok;
		out = 0;

		LOCK(mutex);

		bool alreadyPlaying = findSource(source, out);

		if(!alreadyPlaying)
		{
			// Try to play.
			if(!available.empty())
			{
				// Get the first available source.
				out = available.front();

				// Remove it.
				available.pop();

				// Insert into map of playing sources.
				playing.insert(std::pair<Source *, ALuint>(source, out));

				source->retain();

				source->playAtomic();

				ok = true;
			}
			else
			{
				ok = false;
			}
		}
		else
		{
			ok = true;
		}

		UNLOCK(mutex);

		return ok;
	}

	void Pool::stop()
	{
		LOCK(mutex);
		for(std::map<Source *, ALuint>::iterator i = playing.begin(); i != playing.end(); i++)
		{
			i->first->stopAtomic();
			available.push(i->second);
		}

		playing.clear();

		UNLOCK(mutex);
	}

	void Pool::stop(Source * source)
	{
		LOCK(mutex);
		removeSource(source);
		UNLOCK(mutex);
	}

	void Pool::pause()
	{
		LOCK(mutex);
		for(std::map<Source *, ALuint>::iterator i = playing.begin(); i != playing.end(); i++)
			i->first->pauseAtomic();
		UNLOCK(mutex);
	}

	void Pool::pause(Source * source)
	{
		LOCK(mutex);
		ALuint out;
		if(findSource(source, out))
			source->pauseAtomic();
		UNLOCK(mutex);
	}

	void Pool::resume()
	{
		LOCK(mutex);
		for(std::map<Source *, ALuint>::iterator i = playing.begin(); i != playing.end(); i++)
			i->first->resumeAtomic();
		UNLOCK(mutex);
	}

	void Pool::resume(Source * source)
	{
		LOCK(mutex);
		ALuint out;
		if(findSource(source, out))
			source->resumeAtomic();
		UNLOCK(mutex);
	}

	void Pool::rewind()
	{
		LOCK(mutex);
		for(std::map<Source *, ALuint>::iterator i = playing.begin(); i != playing.end(); i++)
			i->first->rewindAtomic();
		UNLOCK(mutex);
	}

	void Pool::rewind(Source * source)
	{
		LOCK(mutex);
		ALuint out;
		if(findSource(source, out))
			source->rewindAtomic();
		UNLOCK(mutex);
	}

	void Pool::release(Source * source)
	{
		ALuint s = findi(source);

		if(s != 0)
		{
			available.push(s);
			playing.erase(source);
		}
	}

	ALuint Pool::findi(const Source * source) const
	{
		std::map<Source *, ALuint>::const_iterator i = playing.find((Source *)source);

		if(i != playing.end())
			return i->second;

		return 0;
	}

	bool Pool::findSource(Source * source, ALuint & out)
	{
		std::map<Source *, ALuint>::const_iterator i = playing.find((Source *)source);

		bool found = i != playing.end();

		if(found)
			out = i->second;

		return found;
	}

	bool Pool::removeSource(Source * source)
	{
		std::map<Source *, ALuint>::iterator i = playing.find((Source *)source);

		if(i != playing.end())
		{
			source->stopAtomic();
			source->release();
			available.push(i->second);
			playing.erase(i++);
			return true;
		}

		return false;
	}

} // openal
} // audio
} // love
