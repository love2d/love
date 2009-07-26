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

#ifndef LOVE_AUDIO_OPENAL_MUSIC_H
#define LOVE_AUDIO_OPENAL_MUSIC_H

// LOVE
#include <audio/Music.h>
#include <sound/Decoder.h>
#include "Pool.h"

// OpenAL
#include <AL/alc.h>
#include <AL/al.h>

namespace love
{
namespace audio
{
namespace openal
{

	// Forward declarations.
	class Audio;

	class Music : public love::audio::Music
	{
	private:
		static const unsigned int NUM_BUFFERS = 32;
		ALuint buffers[NUM_BUFFERS];
		Pool * pool;
		love::sound::Decoder * decoder;
		ALuint source;
	public:
		Music(Pool * pool, love::sound::Decoder * decoder);
		virtual ~Music();
		

		// Implements Audible.
		void play(love::audio::Source * source);
		void update(love::audio::Source * source);
		void stop(love::audio::Source * source);
		void rewind(love::audio::Source * source);

		// Implements Music.
		love::audio::Music * clone();

	private:
		bool stream(ALuint buffer);
	}; // Sound

} // openal
} // audio
} // love

#endif // LOVE_AUDIO_OPENAL_MUSIC_H