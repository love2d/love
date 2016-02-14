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

#include "Source.h"
#include "Pool.h"
#include "common/math.h"

// STD
#include <iostream>
#include <float.h>
#include <algorithm>

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

class InvalidFormatException : public love::Exception
{
public:

	InvalidFormatException(int channels, int bitdepth)
		: Exception("%d-channel Sources with %d bits per sample are not supported.", channels, bitdepth)
	{
	}

};

class SpatialSupportException : public love::Exception
{
public:

	SpatialSupportException()
		: Exception("This spatial audio functionality is only available for mono Sources. \
Ensure the Source is not multi-channel before calling this function.")
	{
	}

};

StaticDataBuffer::StaticDataBuffer(ALenum format, const ALvoid *data, ALsizei size, ALsizei freq)
	: size(size)
{
	alGenBuffers(1, &buffer);
	alBufferData(buffer, format, data, size, freq);
}

StaticDataBuffer::~StaticDataBuffer()
{
	alDeleteBuffers(1, &buffer);
}

Source::Source(Pool *pool, love::sound::SoundData *soundData)
	: love::audio::Source(Source::TYPE_STATIC)
	, pool(pool)
	, valid(false)
	, staticBuffer(nullptr)
	, pitch(1.0f)
	, volume(1.0f)
	, relative(false)
	, looping(false)
	, paused(false)
	, minVolume(0.0f)
	, maxVolume(1.0f)
	, referenceDistance(1.0f)
	, rolloffFactor(1.0f)
	, maxDistance(MAX_ATTENUATION_DISTANCE)
	, cone()
	, offsetSamples(0)
	, offsetSeconds(0)
	, sampleRate(soundData->getSampleRate())
	, channels(soundData->getChannels())
	, bitDepth(soundData->getBitDepth())
	, decoder(nullptr)
	, toLoop(0)
{
	ALenum fmt = getFormat(soundData->getChannels(), soundData->getBitDepth());
	if (fmt == 0)
		throw InvalidFormatException(soundData->getChannels(), soundData->getBitDepth());

	staticBuffer.set(new StaticDataBuffer(fmt, soundData->getData(), (ALsizei) soundData->getSize(), soundData->getSampleRate()));

	// The buffer has a +2 retain count right now, but we want it to have +1.
	staticBuffer->release();

	float z[3] = {0, 0, 0};

	setFloatv(position, z);
	setFloatv(velocity, z);
	setFloatv(direction, z);
}

Source::Source(Pool *pool, love::sound::Decoder *decoder)
	: love::audio::Source(Source::TYPE_STREAM)
	, pool(pool)
	, valid(false)
	, staticBuffer(nullptr)
	, pitch(1.0f)
	, volume(1.0f)
	, relative(false)
	, looping(false)
	, paused(false)
	, minVolume(0.0f)
	, maxVolume(1.0f)
	, referenceDistance(1.0f)
	, rolloffFactor(1.0f)
	, maxDistance(MAX_ATTENUATION_DISTANCE)
	, cone()
	, offsetSamples(0)
	, offsetSeconds(0)
	, sampleRate(decoder->getSampleRate())
	, channels(decoder->getChannels())
	, bitDepth(decoder->getBitDepth())
	, decoder(decoder)
	, toLoop(0)
{
	if (getFormat(decoder->getChannels(), decoder->getBitDepth()) == 0)
		throw InvalidFormatException(decoder->getChannels(), decoder->getBitDepth());

	alGenBuffers(MAX_BUFFERS, streamBuffers);

	float z[3] = {0, 0, 0};

	setFloatv(position, z);
	setFloatv(velocity, z);
	setFloatv(direction, z);
}

Source::Source(const Source &s)
	: love::audio::Source(s.type)
	, pool(s.pool)
	, valid(false)
	, staticBuffer(s.staticBuffer)
	, pitch(s.pitch)
	, volume(s.volume)
	, relative(s.relative)
	, looping(s.looping)
	, paused(false)
	, minVolume(s.minVolume)
	, maxVolume(s.maxVolume)
	, referenceDistance(s.referenceDistance)
	, rolloffFactor(s.rolloffFactor)
	, maxDistance(s.maxDistance)
	, cone(s.cone)
	, offsetSamples(0)
	, offsetSeconds(0)
	, sampleRate(s.sampleRate)
	, channels(s.channels)
	, bitDepth(s.bitDepth)
	, decoder(nullptr)
	, toLoop(0)
{
	if (type == TYPE_STREAM)
	{
		if (s.decoder.get())
		{
			love::sound::Decoder *dec = s.decoder->clone();
			decoder.set(dec);
			dec->release();
		}

		alGenBuffers(MAX_BUFFERS, streamBuffers);
	}

	setFloatv(position, s.position);
	setFloatv(velocity, s.velocity);
	setFloatv(direction, s.direction);
}

Source::~Source()
{
	if (valid)
		pool->stop(this);

	if (type == TYPE_STREAM)
		alDeleteBuffers(MAX_BUFFERS, streamBuffers);
}

love::audio::Source *Source::clone()
{
	return new Source(*this);
}

bool Source::play()
{
	if (valid && paused)
	{
		pool->resume(this);
		return true;
	}

	valid = pool->play(this, source);
	return valid;
}

void Source::stop()
{
	if (!isStopped())
	{
		pool->stop(this);
		pool->softRewind(this);
	}
}

void Source::pause()
{
	pool->pause(this);
}

void Source::resume()
{
	pool->resume(this);
}

void Source::rewind()
{
	pool->rewind(this);
}

bool Source::isStopped() const
{
	if (valid)
	{
		ALenum state;
		alGetSourcei(source, AL_SOURCE_STATE, &state);
		return (state == AL_STOPPED);
	}

	return true;
}

bool Source::isPaused() const
{
	if (valid)
	{
		ALenum state;
		alGetSourcei(source, AL_SOURCE_STATE, &state);
		return (state == AL_PAUSED);
	}

	return false;
}

bool Source::isFinished() const
{
	return type == TYPE_STATIC ? isStopped() : (isStopped() && !isLooping() && decoder->isFinished());
}

bool Source::update()
{
	if (!valid)
		return false;

	if (type == TYPE_STATIC)
	{
		// Looping mode could have changed.
		alSourcei(source, AL_LOOPING, isLooping() ? AL_TRUE : AL_FALSE);
		return !isStopped();
	}
	else if (type == TYPE_STREAM && (isLooping() || !isFinished()))
	{
		// Number of processed buffers.
		ALint processed = 0;

		alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);

		while (processed--)
		{
			ALuint buffer;

			float curOffsetSamples, curOffsetSecs;

			alGetSourcef(source, AL_SAMPLE_OFFSET, &curOffsetSamples);

			int freq = decoder->getSampleRate();
			curOffsetSecs = curOffsetSamples / freq;

			// Get a free buffer.
			alSourceUnqueueBuffers(source, 1, &buffer);

			float newOffsetSamples, newOffsetSecs;

			alGetSourcef(source, AL_SAMPLE_OFFSET, &newOffsetSamples);
			newOffsetSecs = newOffsetSamples / freq;

			offsetSamples += (curOffsetSamples - newOffsetSamples);
			offsetSeconds += (curOffsetSecs - newOffsetSecs);

			// FIXME: We should put freed buffers into a list that we later
			// consume here, so we can keep track of all free buffers even if we
			// tried to stream data to one but the decoder didn't have data for it.
			if (streamAtomic(buffer, decoder.get()) > 0)
				alSourceQueueBuffers(source, 1, &buffer);
		}

		return true;
	}

	return false;
}

void Source::setPitch(float pitch)
{
	if (valid)
		alSourcef(source, AL_PITCH, pitch);

	this->pitch = pitch;
}

float Source::getPitch() const
{
	if (valid)
	{
		ALfloat f;
		alGetSourcef(source, AL_PITCH, &f);
		return f;
	}

	// In case the Source isn't playing.
	return pitch;
}

void Source::setVolume(float volume)
{
	if (valid)
		alSourcef(source, AL_GAIN, volume);

	this->volume = volume;
}

float Source::getVolume() const
{
	if (valid)
	{
		ALfloat f;
		alGetSourcef(source, AL_GAIN, &f);
		return f;
	}

	// In case the Source isn't playing.
	return volume;
}

void Source::seekAtomic(float offset, void *unit)
{
	if (valid)
	{
		switch (*((Source::Unit *) unit))
		{
		case Source::UNIT_SAMPLES:
			if (type == TYPE_STREAM)
			{
				offsetSamples = offset;
				offset /= decoder->getSampleRate();
				offsetSeconds = offset;
				decoder->seek(offset);
			}
			else
				alSourcef(source, AL_SAMPLE_OFFSET, offset);
			break;
		case Source::UNIT_SECONDS:
		default:
			if (type == TYPE_STREAM)
			{
				offsetSeconds = offset;
				decoder->seek(offset);
				offsetSamples = offset * decoder->getSampleRate();
			}
			else
				alSourcef(source, AL_SEC_OFFSET, offset);
			break;
		}
		if (type == TYPE_STREAM)
		{
			bool waspaused = paused;
			// Because we still have old data
			// from before the seek in the buffers
			// let's empty them.
			stopAtomic();
			playAtomic();
			if (waspaused)
				pauseAtomic();
		}
	}
}

void Source::seek(float offset, Source::Unit unit)
{
	return pool->seek(this, offset, &unit);
}

float Source::tellAtomic(void *unit) const
{
	if (valid)
	{
		float offset;
		switch (*((Source::Unit *) unit))
		{
		case Source::UNIT_SAMPLES:
			alGetSourcef(source, AL_SAMPLE_OFFSET, &offset);
			if (type == TYPE_STREAM) offset += offsetSamples;
			break;
		case Source::UNIT_SECONDS:
		default:
			{
				alGetSourcef(source, AL_SAMPLE_OFFSET, &offset);
				offset /= sampleRate;
				if (type == TYPE_STREAM) offset += offsetSeconds;
			}
			break;
		}
		return offset;
	}
	return 0.0f;
}

float Source::tell(Source::Unit unit)
{
	return pool->tell(this, &unit);
}

double Source::getDurationAtomic(void *vunit)
{
	Unit unit = *(Unit *) vunit;

	if (type == TYPE_STREAM)
	{
		double seconds = decoder->getDuration();

		if (unit == UNIT_SECONDS)
			return seconds;
		else
			return seconds * decoder->getSampleRate();
	}
	else
	{
		ALsizei size = staticBuffer->getSize();
		ALsizei samples = (size / channels) / (bitDepth / 8);

		if (unit == UNIT_SAMPLES)
			return (double) samples;
		else
			return (double) samples / (double) sampleRate;
	}
}

double Source::getDuration(Unit unit)
{
	return pool->getDuration(this, &unit);
}

void Source::setPosition(float *v)
{
	if (channels > 1)
		throw SpatialSupportException();

	if (valid)
		alSourcefv(source, AL_POSITION, v);

	setFloatv(position, v);
}

void Source::getPosition(float *v) const
{
	if (channels > 1)
		throw SpatialSupportException();

	if (valid)
		alGetSourcefv(source, AL_POSITION, v);
	else
		setFloatv(v, position);
}

void Source::setVelocity(float *v)
{
	if (channels > 1)
		throw SpatialSupportException();

	if (valid)
		alSourcefv(source, AL_VELOCITY, v);

	setFloatv(velocity, v);
}

void Source::getVelocity(float *v) const
{
	if (channels > 1)
		throw SpatialSupportException();

	if (valid)
		alGetSourcefv(source, AL_VELOCITY, v);
	else
		setFloatv(v, velocity);
}

void Source::setDirection(float *v)
{
	if (channels > 1)
		throw SpatialSupportException();

	if (valid)
		alSourcefv(source, AL_DIRECTION, v);
	else
		setFloatv(direction, v);
}

void Source::getDirection(float *v) const
{
	if (channels > 1)
		throw SpatialSupportException();

	if (valid)
		alGetSourcefv(source, AL_DIRECTION, v);
	else
		setFloatv(v, direction);
}

void Source::setCone(float innerAngle, float outerAngle, float outerVolume)
{
	if (channels > 1)
		throw SpatialSupportException();

	cone.innerAngle  = (int) LOVE_TODEG(innerAngle);
	cone.outerAngle  = (int) LOVE_TODEG(outerAngle);
	cone.outerVolume = outerVolume;

	if (valid)
	{
		alSourcei(source, AL_CONE_INNER_ANGLE, cone.innerAngle);
		alSourcei(source, AL_CONE_OUTER_ANGLE, cone.outerAngle);
		alSourcef(source, AL_CONE_OUTER_GAIN, cone.outerVolume);
	}
}

void Source::getCone(float &innerAngle, float &outerAngle, float &outerVolume) const
{
	if (channels > 1)
		throw SpatialSupportException();

	innerAngle  = LOVE_TORAD(cone.innerAngle);
	outerAngle  = LOVE_TORAD(cone.outerAngle);
	outerVolume = cone.outerVolume;
}

void Source::setRelative(bool enable)
{
	if (channels > 1)
		throw SpatialSupportException();

	if (valid)
		alSourcei(source, AL_SOURCE_RELATIVE, enable ? AL_TRUE : AL_FALSE);

	relative = enable;
}

bool Source::isRelative() const
{
	if (channels > 1)
		throw SpatialSupportException();

	return relative;
}

void Source::setLooping(bool enable)
{
	if (valid && type == TYPE_STATIC)
		alSourcei(source, AL_LOOPING, enable ? AL_TRUE : AL_FALSE);

	looping = enable;
}

bool Source::isLooping() const
{
	return looping;
}

bool Source::playAtomic()
{
	if (type == TYPE_STATIC)
	{
		alSourcei(source, AL_BUFFER, staticBuffer->getBuffer());
	}
	else if (type == TYPE_STREAM)
	{
		int usedBuffers = 0;

		for (unsigned int i = 0; i < MAX_BUFFERS; i++)
		{
			if (streamAtomic(streamBuffers[i], decoder.get()) == 0)
				break;

			++usedBuffers;

			if (decoder->isFinished())
				break;
		}

		if (usedBuffers > 0)
			alSourceQueueBuffers(source, usedBuffers, streamBuffers);
	}

	// This Source may now be associated with an OpenAL source that still has
	// the properties of another love Source. Let's reset it to the settings
	// of the new one.
	reset();

	// Clear errors.
	alGetError();

	alSourcePlay(source);

	// alSourcePlay may fail if the system has reached its limit of simultaneous
	// playing sources.
	bool success = alGetError() == AL_NO_ERROR;

	valid = true; //if it fails it will be set to false again
	//but this prevents a horrible, horrible bug

	return success;
}

void Source::stopAtomic()
{
	if (valid)
	{
		if (type == TYPE_STATIC)
			alSourceStop(source);
		else if (type == TYPE_STREAM)
		{
			alSourceStop(source);
			int queued = 0;
			alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);

			while (queued--)
			{
				ALuint buffer;
				alSourceUnqueueBuffers(source, 1, &buffer);
			}
		}

		alSourcei(source, AL_BUFFER, AL_NONE);
	}

	toLoop = 0;
	valid = false;
}

void Source::pauseAtomic()
{
	if (valid)
	{
		alSourcePause(source);
		paused = true;
	}
}

void Source::resumeAtomic()
{
	if (valid && paused)
	{
		alSourcePlay(source);
		paused = false;
	}
}

void Source::rewindAtomic()
{
	if (valid && type == TYPE_STATIC)
	{
		alSourceRewind(source);
		if (!paused)
			alSourcePlay(source);
	}
	else if (valid && type == TYPE_STREAM)
	{
		bool waspaused = paused;
		decoder->rewind();
		// Because we still have old data
		// from before the seek in the buffers
		// let's empty them.
		stopAtomic();
		playAtomic();
		if (waspaused)
			pauseAtomic();
		offsetSamples = 0;
		offsetSeconds = 0;
	}
	else if (type == TYPE_STREAM)
	{
		decoder->rewind();
		offsetSamples = 0;
		offsetSeconds = 0;
	}
}

void Source::reset()
{
	alSourcefv(source, AL_POSITION, position);
	alSourcefv(source, AL_VELOCITY, velocity);
	alSourcefv(source, AL_DIRECTION, direction);
	alSourcef(source, AL_PITCH, pitch);
	alSourcef(source, AL_GAIN, volume);
	alSourcef(source, AL_MIN_GAIN, minVolume);
	alSourcef(source, AL_MAX_GAIN, maxVolume);
	alSourcef(source, AL_REFERENCE_DISTANCE, referenceDistance);
	alSourcef(source, AL_ROLLOFF_FACTOR, rolloffFactor);
	alSourcef(source, AL_MAX_DISTANCE, maxDistance);
	alSourcei(source, AL_LOOPING, (type == TYPE_STATIC) && isLooping() ? AL_TRUE : AL_FALSE);
	alSourcei(source, AL_SOURCE_RELATIVE, relative ? AL_TRUE : AL_FALSE);
	alSourcei(source, AL_CONE_INNER_ANGLE, cone.innerAngle);
	alSourcei(source, AL_CONE_OUTER_ANGLE, cone.outerAngle);
	alSourcef(source, AL_CONE_OUTER_GAIN, cone.outerVolume);
}

void Source::setFloatv(float *dst, const float *src) const
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
}

ALenum Source::getFormat(int channels, int bitDepth) const
{
	if (channels == 1 && bitDepth == 8)
		return AL_FORMAT_MONO8;
	else if (channels == 1 && bitDepth == 16)
		return AL_FORMAT_MONO16;
	else if (channels == 2 && bitDepth == 8)
		return AL_FORMAT_STEREO8;
	else if (channels == 2 && bitDepth == 16)
		return AL_FORMAT_STEREO16;

#ifdef AL_EXT_MCFORMATS
	if (alIsExtensionPresent("AL_EXT_MCFORMATS"))
	{
		if (channels == 6 && bitDepth == 8)
			return AL_FORMAT_51CHN8;
		else if (channels == 6 && bitDepth == 16)
			return AL_FORMAT_51CHN16;
		else if (channels == 8 && bitDepth == 8)
			return AL_FORMAT_71CHN8;
		else if (channels == 8 && bitDepth == 16)
			return AL_FORMAT_71CHN16;
	}
#endif

	return 0;
}

int Source::streamAtomic(ALuint buffer, love::sound::Decoder *d)
{
	// Get more sound data.
	int decoded = std::max(d->decode(), 0);

	// OpenAL implementations are allowed to ignore 0-size alBufferData calls.
	if (decoded > 0)
	{
		int fmt = getFormat(d->getChannels(), d->getBitDepth());

		if (fmt != 0)
			alBufferData(buffer, fmt, d->getBuffer(), decoded, d->getSampleRate());
		else
			decoded = 0;
	}

	if (decoder->isFinished() && isLooping())
	{
		int queued, processed;
		alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);
		alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
		if (queued > processed)
			toLoop = queued-processed;
		else
			toLoop = MAX_BUFFERS-processed;
		d->rewind();
	}

	if (toLoop > 0)
	{
		if (--toLoop == 0)
		{
			offsetSamples = 0;
			offsetSeconds = 0;
		}
	}

	return decoded;
}

void Source::setMinVolume(float volume)
{
	if (valid)
		alSourcef(source, AL_MIN_GAIN, volume);

	minVolume = volume;
}

float Source::getMinVolume() const
{
	if (valid)
	{
		ALfloat f;
		alGetSourcef(source, AL_MIN_GAIN, &f);
		return f;
	}

	// In case the Source isn't playing.
	return this->minVolume;
}

void Source::setMaxVolume(float volume)
{
	if (valid)
		alSourcef(source, AL_MAX_GAIN, volume);

	maxVolume = volume;
}

float Source::getMaxVolume() const
{
	if (valid)
	{
		ALfloat f;
		alGetSourcef(source, AL_MAX_GAIN, &f);
		return f;
	}

	// In case the Source isn't playing.
	return maxVolume;
}

void Source::setReferenceDistance(float distance)
{
	if (channels > 1)
		throw SpatialSupportException();

	if (valid)
		alSourcef(source, AL_REFERENCE_DISTANCE, distance);

	referenceDistance = distance;
}

float Source::getReferenceDistance() const
{
	if (channels > 1)
		throw SpatialSupportException();

	if (valid)
	{
		ALfloat f;
		alGetSourcef(source, AL_REFERENCE_DISTANCE, &f);
		return f;
	}

	// In case the Source isn't playing.
	return referenceDistance;
}

void Source::setRolloffFactor(float factor)
{
	if (channels > 1)
		throw SpatialSupportException();

	if (valid)
		alSourcef(source, AL_ROLLOFF_FACTOR, factor);

	rolloffFactor = factor;
}

float Source::getRolloffFactor() const
{
	if (channels > 1)
		throw SpatialSupportException();

	if (valid)
	{
		ALfloat f;
		alGetSourcef(source, AL_ROLLOFF_FACTOR, &f);
		return f;
	}

	// In case the Source isn't playing.
	return rolloffFactor;
}

void Source::setMaxDistance(float distance)
{
	if (channels > 1)
		throw SpatialSupportException();

	distance = std::min(distance, MAX_ATTENUATION_DISTANCE);

	if (valid)
		alSourcef(source, AL_MAX_DISTANCE, distance);

	maxDistance = distance;
}

float Source::getMaxDistance() const
{
	if (channels > 1)
		throw SpatialSupportException();

	if (valid)
	{
		ALfloat f;
		alGetSourcef(source, AL_MAX_DISTANCE, &f);
		return f;
	}

	// In case the Source isn't playing.
	return maxDistance;
}

int Source::getChannels() const
{
	return channels;
}

} // openal
} // audio
} // love
