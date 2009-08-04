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
* 1. The origin of this software must not be misrepresented = 0; you must not
*    claim that you wrote the original software. If you use this software
*    in a product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
**/

#ifndef LOVE_AUDIO_AUDIO_H
#define LOVE_AUDIO_AUDIO_H

#include <common/Module.h>
#include "Source.h"
#include "Sound.h"
#include "Music.h"

namespace love
{
namespace audio
{
	/**
	* The Audio module is responsible for playing back raw sound samples. 
	**/
	class Audio : public Module
	{
	public:

		/**
		* Destructor.
		**/
		virtual ~Audio(){};

		/**
		* Creates a new Sound with the specified SoundData.
		* @param data The SoundData from which to create the sound.
		* @return A new Sound if successful, zero otherwise.
		**/
		virtual Sound * newSound(love::sound::SoundData * data) = 0;

		/**
		* Creates a new Music (stream) using the specified SoundData.
		* @param decoder The object to use to decode the sound stream.
		**/
		virtual Music * newMusic(love::sound::Decoder * decoder) = 0;

		/**
		* Creates a new Source.
		* @returns A new Source.
		**/
		virtual Source * newSource(Audible * audible) = 0;

		/**
		* Gets the current number of simulatenous playing sources.
		* @return The current number of simulatenous playing sources.
		**/
		virtual int getNumSources() const = 0;

		/**
		* Gets the maximum supported number of simulatenous playing sources.
		* @return The maximum supported number of simulatenous playing sources.
		**/
		virtual int getMaxSources() const = 0;

		/**
		* Play the specified Source.
		* @param source The Source to play.
		**/
		virtual void play(Source * source) = 0;

		/**
		* Plays one Sound on the specified Source. We need separate
		* Sound and Music play functions because Music must be cloned, 
		* whereas Sound needs not be.
		* 
		* @param sound The Sound to play.
		* @param source The Source on which to play the Sound.
		**/
		virtual void play(Sound * sound) = 0;

		/**
		* Plays one Music on the specified Source. We need separate
		* Sound and Music play functions because Music must be cloned, 
		* whereas Sound needs not be.
		* 
		* @param music The Music to play.
		* @param source The Source on which to play the Music.
		**/
		virtual void play(Music * music) = 0;

		/**
		* Stops playback on the specified source.
		* @param source The source on which to stop the playback.
		**/
		virtual void stop(Source * source) = 0;

		/**
		* Stops all playing audio.
		**/
		virtual void stop() = 0;

		/**
		* Pauses playback on the specified source.
		* @param source The source on which to pause the playback.
		**/
		virtual void pause(Source * source) = 0;

		/**
		* Pauses all audio.
		**/
		virtual void pause() = 0;

		/**
		* Resumes playback on the specified source.
		* @param source The source on which to resume the playback.
		**/
		virtual void resume(Source * source) = 0;

		/**
		* Resumes all audio.
		**/
		virtual void resume() = 0;

		/**
		* Rewinds the specified source. Whatever is playing on this
		* source gets rewound to the start.
		* @param source The source to rewind.
		**/
		virtual void rewind(Source * source) = 0;

		/**
		* Rewinds all playing audio.
		**/
		virtual void rewind() = 0;

		/**
		* Sets the master volume, where 0.0f is min (off) and 1.0f is max.
		* @param volume The new master volume.
		**/
		virtual void setVolume(float volume) = 0;

		/**
		* Gets the master volume.
		* @return The current master volume.
		**/
		virtual float getVolume() const = 0;

		/**
		* Gets the position of the listener.
		* @param v A float array of size 3 containing (x,y,z) in that order. 
		**/
		virtual void getPosition(float * v) const = 0;

		/**
		* Sets the position of the listener.
		* @param v A float array of size 3 containing [x,y,z] in that order. 
		**/
		virtual void setPosition(float * v) = 0;

		/**
		* Gets the orientation of the listener.
		* @param v A float array of size 6 containing [x,y,z] for the forward
		* vector, followed by [x,y,z] for the up vector.
		**/
		virtual void getOrientation(float * v) const = 0;

		/**
		* Sets the orientation of the listener.
		* @param v A float array of size 6 containing [x,y,z] for the forward
		* vector, followed by [x,y,z] for the up vector.
		**/
		virtual void setOrientation(float * v) = 0;

		/**
		* Gets the velocity of the listener.
		* @param v A float array of size 3 containing [x,y,z] in that order. 
		**/
		virtual void getVelocity(float * v) const = 0;

		/**
		* Sets the velocity of the listener.
		* @param v A float array of size 3 containing [x,y,z] in that order. 
		**/
		virtual void setVelocity(float * v) = 0;

	}; // Audio

} // audio
} // love

#endif // LOVE_AUDIO_AUDIO_H
