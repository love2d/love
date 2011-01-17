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

#ifndef LOVE_AUDIO_OPENAL_SOURCE_H
#define LOVE_AUDIO_OPENAL_SOURCE_H

// LOVE
#include <common/config.h>
#include <common/Object.h>
#include <audio/Source.h>
#include <sound/SoundData.h>
#include <sound/Decoder.h>

// OpenAL
#ifdef LOVE_MACOSX
#include <OpenAL/alc.h>
#include <OpenAL/al.h>
#else
#include <AL/alc.h>
#include <AL/al.h>
#endif

namespace love
{
namespace audio
{
namespace openal
{
	class Audio;
	class Pool;

	class Source : public love::audio::Source
	{
	private:

		Pool * pool;
		ALuint source;
		bool valid;
		static const unsigned int MAX_BUFFERS = 32;
		ALuint buffers[MAX_BUFFERS];

		float pitch;
		float volume;
		float position[3];
		float velocity[3];
		float direction[3];
		bool looping;

		love::sound::Decoder * decoder;

	public:
		Source(Pool * pool, love::sound::SoundData * soundData);
		Source(Pool * pool, love::sound::Decoder * decoder);
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
		virtual void update();
		virtual void setPitch(float pitch);
		virtual float getPitch() const;
		virtual void setVolume(float volume);
		virtual float getVolume() const;
		virtual void setPosition(float * v);
		virtual void getPosition(float * v) const;
		virtual void setVelocity(float * v);
		virtual void getVelocity(float * v) const;
		virtual void setDirection(float * v);
		virtual void getDirection(float * v) const;
		void setLooping(bool looping);
		bool isLooping() const;
		bool isStatic() const;

		void playAtomic();
		void stopAtomic();
		void pauseAtomic();
		void resumeAtomic();
		void rewindAtomic();

	private:

		void reset(ALenum source);

		void setFloatv(float * dst, const float * src) const;

		/**
		* Gets the OpenAL format identifier based on number of
		* channels and bits.
		* @param channels Either 1 (mono) or 2 (stereo).
		* @param bits Either 8-bit samples, or 16-bit samples.
		* @return One of AL_FORMAT_*, or 0 if unsupported format.
		**/
		ALenum getFormat(int channels, int bits) const;

		int streamAtomic(ALuint buffer, love::sound::Decoder * d);

	}; // Source

} // openal
} // audio
} // love

#endif // LOVE_AUDIO_OPENAL_SOURCE_H
