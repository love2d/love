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

#ifndef LOVE_AUDIO_OPENAL_SOURCE_H
#define LOVE_AUDIO_OPENAL_SOURCE_H

// LOVE
#include "common/config.h"
#include "common/Object.h"
#include "audio/Source.h"
#include "sound/SoundData.h"
#include "sound/Decoder.h"

// OpenAL
#ifdef LOVE_APPLE_USE_FRAMEWORKS
#ifdef LOVE_IOS
#include <OpenAL/alc.h>
#include <OpenAL/al.h>
#else
#include <OpenAL-Soft/alc.h>
#include <OpenAL-Soft/al.h>
#endif
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

// Basically just a reference-counted non-streaming OpenAL buffer object.
class StaticDataBuffer : public love::Object
{
public:

	StaticDataBuffer(ALenum format, const ALvoid *data, ALsizei size, ALsizei freq);
	virtual ~StaticDataBuffer();

	inline ALuint getBuffer() const
	{
		return buffer;
	}

	inline ALsizei getSize() const
	{
		return size;
	}

private:

	ALuint buffer;
	ALsizei size;

}; // StaticDataBuffer

class Source : public love::audio::Source
{
public:

	Source(Pool *pool, love::sound::SoundData *soundData);
	Source(Pool *pool, love::sound::Decoder *decoder);
	Source(const Source &s);
	virtual ~Source();

	virtual love::audio::Source *clone();
	virtual bool play();
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
	virtual void seekAtomic(float offset, void *unit);
	virtual void seek(float offset, Unit unit);
	virtual float tellAtomic(void *unit) const;
	virtual float tell(Unit unit);
	virtual double getDurationAtomic(void *unit);
	virtual double getDuration(Unit unit);
	virtual void setPosition(float *v);
	virtual void getPosition(float *v) const;
	virtual void setVelocity(float *v);
	virtual void getVelocity(float *v) const;
	virtual void setDirection(float *v);
	virtual void getDirection(float *v) const;
	virtual void setCone(float innerAngle, float outerAngle, float outerVolume);
	virtual void getCone(float &innerAngle, float &outerAngle, float &outerVolume) const;
	virtual void setRelative(bool enable);
	virtual bool isRelative() const;
	void setLooping(bool looping);
	bool isLooping() const;
	virtual void setMinVolume(float volume);
	virtual float getMinVolume() const;
	virtual void setMaxVolume(float volume);
	virtual float getMaxVolume() const;
	virtual void setReferenceDistance(float distance);
	virtual float getReferenceDistance() const;
	virtual void setRolloffFactor(float factor);
	virtual float getRolloffFactor() const;
	virtual void setMaxDistance(float distance);
	virtual float getMaxDistance() const;
	virtual int getChannels() const;

	bool playAtomic();
	void stopAtomic();
	void pauseAtomic();
	void resumeAtomic();
	void rewindAtomic();

private:

	void reset();

	void setFloatv(float *dst, const float *src) const;

	/**
	 * Gets the OpenAL format identifier based on number of
	 * channels and bits.
	 * @param channels Either 1 (mono) or 2 (stereo).
	 * @param bitDepth Either 8-bit samples, or 16-bit samples.
	 * @return One of AL_FORMAT_*, or 0 if unsupported format.
	 **/
	ALenum getFormat(int channels, int bitDepth) const;

	int streamAtomic(ALuint buffer, love::sound::Decoder *d);

	Pool *pool;
	ALuint source;
	bool valid;

	static const unsigned int MAX_BUFFERS = 8;
	ALuint streamBuffers[MAX_BUFFERS];

	StrongRef<StaticDataBuffer> staticBuffer;

	float pitch;
	float volume;
	float position[3];
	float velocity[3];
	float direction[3];
	bool relative;
	bool looping;
	bool paused;
	float minVolume;
	float maxVolume;
	float referenceDistance;
	float rolloffFactor;
	float maxDistance;

	struct Cone
	{
		int innerAngle = 360; // degrees
		int outerAngle = 360; // degrees
		float outerVolume = 0.0f;
	} cone;

	float offsetSamples;
	float offsetSeconds;

	int sampleRate;
	int channels;
	int bitDepth;

	StrongRef<love::sound::Decoder> decoder;

	unsigned int toLoop;

}; // Source

} // openal
} // audio
} // love

#endif // LOVE_AUDIO_OPENAL_SOURCE_H
