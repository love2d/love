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

#ifndef LOVE_AUDIO_OPENAL_POOL_H
#define LOVE_AUDIO_OPENAL_POOL_H

// STD
#include <queue>
#include <map>
#include <vector>
#include <cmath>

// LOVE
#include "common/config.h"
#include "common/Exception.h"
#include "thread/threads.h"
#include "audio/Source.h"

// OpenAL
#ifdef LOVE_APPLE_USE_FRAMEWORKS
#ifdef LOVE_IOS
#include <OpenAL/alc.h>
#include <OpenAL/al.h>
#include <OpenAL/oalMacOSX_OALExtensions.h>
#include <OpenAL/oalStaticBufferExtension.h>
#else
#include <OpenAL-Soft/alc.h>
#include <OpenAL-Soft/al.h>
#include <OpenAL-Soft/alext.h>
#endif
#else
#include <AL/alc.h>
#include <AL/al.h>
#include <AL/alext.h>
#endif

namespace love
{
namespace audio
{
namespace openal
{

class Source;

class Pool
{
public:

	Pool();
	~Pool();

	/**
	 * Checks whether an OpenAL source is available.
	 * @return True if at least one is available, false otherwise.
	 **/
	bool isAvailable() const;

	/**
	 * Checks whether a Source is currently in the playing list.
	 **/
	bool isPlaying(Source *s);

	void update();

	int getSourceCount() const;
	int getMaxSources() const;

	bool play(Source *source);
	void stop();
	void stop(Source *source);
	std::vector<love::audio::Source*> pause();
	void pause(Source *source);
	void seek(Source *source, float offset, void *unit);
	float tell(Source *source, void *unit);
	double getDuration(Source *source, void *unit);

	bool play(const std::vector<love::audio::Source*> &sources);
	void stop(const std::vector<love::audio::Source*> &sources);
	void pause(const std::vector<love::audio::Source*> &sources);

private:

	/**
	 * Makes the specified OpenAL source available for use.
	 * @param source The OpenAL source.
	 **/
	void release(Source *source);

	ALuint findi(const Source *source) const;

	bool assignSource(Source *source, ALuint &out, char *wasPlaying = nullptr);
	bool findSource(Source *source, ALuint &out);
	bool removeSource(Source *source);

	// Maximum possible number of OpenAL sources the pool attempts to generate.
	static const int MAX_SOURCES = 64;

	// OpenAL sources
	ALuint sources[MAX_SOURCES];

	// Total number of created sources in the pool.
	int totalSources;

	// A queue of available sources.
	std::queue<ALuint> available;

	// A map of playing sources.
	std::map<Source *, ALuint> playing;

	// Only one thread can access this object at the same time. This mutex will
	// make sure of that.
	love::thread::MutexRef mutex;

}; // Pool

} // openal
} // audio
} // love

#endif // LOVE_AUDIO_OPENAL_POOL_H
