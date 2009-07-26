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

#ifndef LOVE_AUDIO_OPENAL_POOL_H
#define LOVE_AUDIO_OPENAL_POOL_H

// STD
#include <queue>
#include <map>
#include <iostream>
#include <cmath>

// SDL
#include <SDL.h>

// OpenAL
#include <AL/alc.h>
#include <AL/al.h>

// LOVE
#include <audio/Source.h>
#include <common/Exception.h>

namespace love
{
namespace audio
{
namespace openal
{
	class Pool
	{
	private:

		// Number of OpenAL sources.
		static const int NUM_SOURCES = 16;

		// OpenAL sources
		ALuint sources[NUM_SOURCES];

		// A queue of available sources.
		std::queue<ALuint> available;

		// A map of playing sources.
		std::map<love::audio::Source *, ALuint> playing;

		// Only one thread can access this object at the same time. This mutex will
		// make sure of that.
		SDL_mutex * mutex;

	public:

		Pool();
		~Pool();

		/**
		* Gets the OpenAL format identifier based on number of
		* channels and bits.
		* @param channels Either 1 (mono) or 2 (stereo). 
		* @param bits Either 8-bit samples, or 16-bit samples.
		* @return One of AL_FORMAT_*, or 0 if unsupported format.
		**/
		ALenum getFormat(int channels, int bits) const;

		/**
		* Checks whether an OpenAL source is available.
		* @return True if at least one is available, false otherwise.
		**/
		bool isAvailable() const;

		/**
		* Checks whether a Source is currently in the playing list.
		**/
		bool isPlaying(love::audio::Source * s);

		/**
		* Returns an available OpenAL source identifier, or 0 if
		* none is available.
		* @return An OpenAL source ID, or 0 if unavailable.
		**/
		ALuint claim(love::audio::Source * source);

		/**
		* Makes the specified OpenAL source available for use.
		* @param source The OpenAL source.
		**/
		void release(love::audio::Source * source);

		ALuint find(const love::audio::Source * source) const;

		void update();

		int getNumSources() const;
		int getMaxSources() const;

		void stop();
		void pause();
		void resume();
		void rewind();

	private:

		ALuint findi(const love::audio::Source * source) const;
	}; // Pool

} // openal
} // audio
} // love

#endif // LOVE_AUDIO_OPENAL_POOL_H
