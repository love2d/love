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

#ifndef LOVE_AUDIO_NULL_SOURCE_H
#define LOVE_AUDIO_NULL_SOURCE_H

// LOVE
#include <common/Object.h>
#include <audio/Source.h>

namespace love
{
namespace audio
{
namespace null
{
	class Source : public love::audio::Source
	{
	private:

		float pitch;
		float volume; 

	public:
		Source();
		Source(Audible * audible);
		virtual ~Source();
		
		void play();
		void stop();
		void pause();
		void resume();
		void rewind();
		bool isFinished() const;
		void update();

		void setPitch(float pitch);
		float getPitch() const;

		void setVolume(float volume);
		float getVolume() const;

		void setPosition(float * v);
		void getPosition(float * v) const;
		void setVelocity(float * v);
		void getVelocity(float * v) const;
		void setDirection(float * v);
		void getDirection(float * v) const;

	}; // Source

} // null
} // audio
} // love

#endif // LOVE_AUDIO_NULL_SOURCE_H