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
		bool looping;

	public:
		Source();
		virtual ~Source();

		virtual love::audio::Source * copy();
		virtual void play();
		virtual void stop();
		virtual void pause();
		virtual void resume();
		virtual void rewind();
		virtual bool isStopped() const;
		virtual bool isPaused() const;
		virtual bool isFinished() const;
		virtual bool update();
		virtual void setPitch(float pitch);
		virtual float getPitch() const;
		virtual void setVolume(float volume);
		virtual float getVolume() const;
		virtual void seek(float offset, Unit unit);
		virtual float tell(Unit unit);
		virtual void setPosition(float * v);
		virtual void getPosition(float * v) const;
		virtual void setVelocity(float * v);
		virtual void getVelocity(float * v) const;
		virtual void setDirection(float * v);
		virtual void getDirection(float * v) const;
		void setLooping(bool looping);
		bool isLooping() const;
		bool isStatic() const;

	}; // Source

} // null
} // audio
} // love

#endif // LOVE_AUDIO_NULL_SOURCE_H
