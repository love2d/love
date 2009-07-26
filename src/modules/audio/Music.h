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

#ifndef LOVE_AUDIO_MUSIC_H
#define LOVE_AUDIO_MUSIC_H

// LOVE
#include "Audible.h"
#include <sound/Decoder.h>

namespace love
{
namespace audio
{
	/**
	* A Music object represents a stream of sound samples which are gradually
	* acquired somehow. Compare with Sounds, which have all the needed sound
	* samples pre-decoded.
	* 
	* Typically, you would want to use love::sound::Decoder to decode samples
	* from some encoded format, like OGG or MP3, however, Music can come from 
	* other sources as well. 
	**/
	class Music : public Audible
	{
	public:

		/**
		* Destructor.
		**/
		virtual ~Music(){};
		
		/**
		* Creates a clone of the music stream. Music objects are gradually
		* decoded, so if the client wants to play the same Music object, we can't
		* use the same object. We must clone the entire stream.
		* @return A clone of this object, but rewound to the start.
		**/
		virtual Music * clone() = 0;

	}; // Music

} // audio
} // love

#endif // LOVE_AUDIO_MUSIC_H
