/**
 * Copyright (c) 2006-2017 LOVE Development Team
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
#include "audio/Filter.h"
#include "sound/SoundData.h"
#include "sound/Decoder.h"
#include "Audio.h"
#include "Filter.h"

// STL
#include <vector>

// C
#include <float.h>

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

#ifdef LOVE_IOS
// OpenAL on iOS barfs if the max distance is +inf.
static const float MAX_ATTENUATION_DISTANCE = 1000000.0f;
#else
static const float MAX_ATTENUATION_DISTANCE = FLT_MAX;
#endif

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
	Source(Pool *pool, int sampleRate, int bitDepth, int channels);
	Source(const Source &s);
	virtual ~Source();

	virtual love::audio::Source *clone();
	virtual bool play();
	virtual void stop();
	virtual void pause();
	virtual bool isPlaying() const;
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

	virtual bool setFilter(love::audio::Filter::Type type, std::vector<float> &params);
	virtual bool setFilter();
	virtual bool getFilter(love::audio::Filter::Type &type, std::vector<float> &params);

	virtual int getFreeBufferCount() const;
	virtual bool queue(void *data, size_t length, int dataSampleRate, int dataBitDepth, int dataChannels);
	virtual bool queueAtomic(void *data, ALsizei length);

	void prepareAtomic();
	void teardownAtomic();

	bool playAtomic(ALuint source);
	void stopAtomic();
	void pauseAtomic();
	void resumeAtomic();

	static bool playAtomic(const std::vector<love::audio::Source*> &sources, const std::vector<ALuint> &ids, const std::vector<char> &wasPlaying);
	static void stopAtomic(const std::vector<love::audio::Source*> &sources);
	static void pauseAtomic(const std::vector<love::audio::Source*> &sources);

private:

	void reset();

	void setFloatv(float *dst, const float *src) const;

	int streamAtomic(ALuint buffer, love::sound::Decoder *d);

	ALuint unusedBufferPeek();
	ALuint unusedBufferPeekNext();
	ALuint *unusedBufferPop();
	void unusedBufferPush(ALuint buffer);
	void unusedBufferQueue(ALuint buffer);
	
	Pool *pool = nullptr;
	ALuint source = 0;
	bool valid = false;

	static const unsigned int MAX_BUFFERS = 8;
	ALuint streamBuffers[MAX_BUFFERS];
	ALuint unusedBuffers[MAX_BUFFERS];

	StrongRef<StaticDataBuffer> staticBuffer;

	float pitch = 1.0f;
	float volume = 1.0f;
	float position[3];
	float velocity[3];
	float direction[3];
	bool relative = false;
	bool looping = false;
	float minVolume = 0.0f;
	float maxVolume = 1.0f;
	float referenceDistance = 1.0f;
	float rolloffFactor = 1.0f;
	float maxDistance = MAX_ATTENUATION_DISTANCE;

	struct Cone
	{
		int innerAngle = 360; // degrees
		int outerAngle = 360; // degrees
		float outerVolume = 0.0f;
	} cone;

	float offsetSamples = 0.0f;
	float offsetSeconds = 0.0f;

	int sampleRate = 0;
	int channels = 0;
	int bitDepth = 0;

	StrongRef<love::sound::Decoder> decoder;

	unsigned int toLoop = 0;
	int unusedBufferTop = -1;
	ALsizei bufferedBytes = 0;

	Filter *filter = nullptr;

}; // Source

} // openal
} // audio
} // love

#endif // LOVE_AUDIO_OPENAL_SOURCE_H
