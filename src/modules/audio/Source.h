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

#ifndef LOVE_AUDIO_SOURCE_H
#define LOVE_AUDIO_SOURCE_H

// LOVE
#include <common/Object.h>
#include "Audible.h"

namespace love
{
namespace audio
{
	class Source : public Object
	{
	protected:
		Audible * audible;
		bool looping;
	public:
		Source();
		virtual ~Source();
		void setAudible(Audible * audible);
		Audible * getAudible() const;
		
		virtual void play() = 0;
		virtual void stop() = 0;
		virtual void pause() = 0;
		virtual void resume() = 0;
		virtual void rewind() = 0;
		virtual bool isStopped() const = 0;
		virtual void update() = 0;

		virtual void setPitch(float pitch) = 0;
		virtual float getPitch() const = 0;

		virtual void setVolume(float volume) = 0;
		virtual float getVolume() const = 0;

		// all float * v must be of size 3
		virtual void setPosition(float * v) = 0;
		virtual void getPosition(float * v) const = 0;
		virtual void setVelocity(float * v) = 0;
		virtual void getVelocity(float * v) const = 0;
		virtual void setDirection(float * v) = 0;
		virtual void getDirection(float * v) const = 0;
		
		void setLooping(bool looping);
		bool isLooping() const;

	}; // Source

} // audio
} // love

#endif // LOVE_AUDIO_SOURCE_H