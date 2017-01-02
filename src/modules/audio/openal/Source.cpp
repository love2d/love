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

#include "Source.h"
#include "Filter.h"
#include "Pool.h"
#include "Audio.h"
#include "common/math.h"

// STD
#include <iostream>
#include <algorithm>

namespace love
{
namespace audio
{
namespace openal
{

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

class QueueFormatMismatchException : public love::Exception
{
public:

	QueueFormatMismatchException()
		: Exception("Queued sound data must have same format as sound Source.")
	{
	}

};

class QueueTypeMismatchException : public love::Exception
{
public:

	QueueTypeMismatchException()
		: Exception("Only queueable Sources can be queued with sound data.")
	{
	}

};

class QueueMalformedLengthException : public love::Exception
{
public:

	QueueMalformedLengthException(int bytes)
		: Exception("Data length must be a multiple of sample size (%d bytes).", bytes)
	{
	}

};

class QueueLoopingException : public love::Exception
{
public:

	QueueLoopingException()
		: Exception("Queueable Sources can not be looped.")
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
	, sampleRate(soundData->getSampleRate())
	, channels(soundData->getChannels())
	, bitDepth(soundData->getBitDepth())
{
	ALenum fmt = Audio::getFormat(soundData->getBitDepth(), soundData->getChannels());
	if (fmt == AL_NONE)
		throw InvalidFormatException(soundData->getChannels(), soundData->getBitDepth());

	staticBuffer.set(new StaticDataBuffer(fmt, soundData->getData(), (ALsizei) soundData->getSize(), sampleRate), Acquire::NORETAIN);

	float z[3] = {0, 0, 0};

	setFloatv(position, z);
	setFloatv(velocity, z);
	setFloatv(direction, z);
}

Source::Source(Pool *pool, love::sound::Decoder *decoder)
	: love::audio::Source(Source::TYPE_STREAM)
	, pool(pool)
	, sampleRate(decoder->getSampleRate())
	, channels(decoder->getChannels())
	, bitDepth(decoder->getBitDepth())
	, decoder(decoder)
	, unusedBufferTop(MAX_BUFFERS - 1)
{
	if (Audio::getFormat(decoder->getBitDepth(), decoder->getChannels()) == AL_NONE)
		throw InvalidFormatException(decoder->getChannels(), decoder->getBitDepth());

	alGenBuffers(MAX_BUFFERS, streamBuffers);
	for (unsigned int i = 0; i < MAX_BUFFERS; i++)
		unusedBuffers[i] = streamBuffers[i];

	float z[3] = {0, 0, 0};

	setFloatv(position, z);
	setFloatv(velocity, z);
	setFloatv(direction, z);
}

Source::Source(Pool *pool, int sampleRate, int bitDepth, int channels)
	: love::audio::Source(Source::TYPE_QUEUE)
	, pool(pool)
	, sampleRate(sampleRate)
	, channels(channels)
	, bitDepth(bitDepth)
{
	ALenum fmt = Audio::getFormat(bitDepth, channels);
	if (fmt == AL_NONE)
		throw InvalidFormatException(channels, bitDepth);

	alGenBuffers(MAX_BUFFERS, streamBuffers);
	for (unsigned int i = 0; i < MAX_BUFFERS; i++)
		unusedBuffers[i] = streamBuffers[i];

	float z[3] = {0, 0, 0};

	setFloatv(position, z);
	setFloatv(velocity, z);
	setFloatv(direction, z);
}

Source::Source(const Source &s)
	: love::audio::Source(s.sourceType)
	, pool(s.pool)
	, valid(false)
	, staticBuffer(s.staticBuffer)
	, pitch(s.pitch)
	, volume(s.volume)
	, relative(s.relative)
	, looping(s.looping)
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
	, unusedBufferTop(s.sourceType == TYPE_STREAM ? MAX_BUFFERS - 1 : -1)
{
	if (sourceType == TYPE_STREAM)
	{
		if (s.decoder.get())
			decoder.set(s.decoder->clone(), Acquire::NORETAIN);
	}
	if (sourceType != TYPE_STATIC)
	{
		alGenBuffers(MAX_BUFFERS, streamBuffers);
		for (unsigned int i = 0; i < MAX_BUFFERS; i++)
			unusedBuffers[i] = streamBuffers[i];
	}
	if (s.filter)
		filter = s.filter->clone();

	setFloatv(position, s.position);
	setFloatv(velocity, s.velocity);
	setFloatv(direction, s.direction);
}

Source::~Source()
{
	if (valid)
		pool->stop(this);

	if (sourceType != TYPE_STATIC)
		alDeleteBuffers(MAX_BUFFERS, streamBuffers);

	if (filter)
		delete filter;
}

love::audio::Source *Source::clone()
{
	return new Source(*this);
}

bool Source::play()
{
	valid = pool->play(this);
	return valid;
}

void Source::stop()
{
	if (valid)
		pool->stop(this);
}

void Source::pause()
{
	pool->pause(this);
}

bool Source::isPlaying() const
{
	if (!valid)
		return false;

	ALenum state;
	alGetSourcei(source, AL_SOURCE_STATE, &state);
	return state == AL_PLAYING;
}

bool Source::isFinished() const
{
	if (!valid)
		return false;

	if (sourceType == TYPE_STREAM && (isLooping() || !decoder->isFinished()))
		return false;

	ALenum state;
	alGetSourcei(source, AL_SOURCE_STATE, &state);
	return state == AL_STOPPED;
}

bool Source::update()
{
	if (!valid)
		return false;

	switch (sourceType)
	{
		case TYPE_STATIC:
		{
			// Looping mode could have changed.
			// FIXME: make looping mode change atomically so this is not needed
			alSourcei(source, AL_LOOPING, isLooping() ? AL_TRUE : AL_FALSE);
			return !isFinished();
		}
		case TYPE_STREAM:
			if (!isFinished())
			{
				ALint processed;
				ALuint buffers[MAX_BUFFERS];
				float curOffsetSamples, curOffsetSecs, newOffsetSamples, newOffsetSecs;
				int freq = decoder->getSampleRate();

				alGetSourcef(source, AL_SAMPLE_OFFSET, &curOffsetSamples);
				curOffsetSecs = curOffsetSamples / freq;

				alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
				alSourceUnqueueBuffers(source, processed, buffers);

				alGetSourcef(source, AL_SAMPLE_OFFSET, &newOffsetSamples);
				newOffsetSecs = newOffsetSamples / freq;

				offsetSamples += (curOffsetSamples - newOffsetSamples);
				offsetSeconds += (curOffsetSecs - newOffsetSecs);

				for (unsigned int i = 0; i < (unsigned int)processed; i++)
					unusedBufferPush(buffers[i]);

				while (unusedBufferPeek() != AL_NONE)
				{
					if(streamAtomic(unusedBufferPeek(), decoder.get()) > 0)
						alSourceQueueBuffers(source, 1, unusedBufferPop());
					else
						break;
				}

				return true;
			}
			return false;
		case TYPE_QUEUE: 
		{
			ALint processed;
			ALuint buffers[MAX_BUFFERS];

			alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
			alSourceUnqueueBuffers(source, processed, buffers);

			for (unsigned int i = 0; i < (unsigned int)processed; i++)
			{
				ALint size;
				alGetBufferi(buffers[i], AL_SIZE, &size);
				bufferedBytes -= size;
				unusedBufferPush(buffers[i]);
			}
			return !isFinished();
		}
		case TYPE_MAX_ENUM:
			break;
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
	float offsetSamples, offsetSeconds;

	switch (*((Source::Unit *) unit))
	{
	case Source::UNIT_SAMPLES:
		offsetSamples = offset;
		offsetSeconds = offset / sampleRate;
		break;
	case Source::UNIT_SECONDS:
	default:
		offsetSeconds = offset;
		offsetSamples = offset * sampleRate;
		break;
	}

	bool wasPlaying = isPlaying();
	switch (sourceType)
	{
		case TYPE_STATIC:
			if (valid)
			{
				alSourcef(source, AL_SAMPLE_OFFSET, offsetSamples);
				offsetSamples = offsetSeconds = 0;
			}
			break;
		case TYPE_STREAM:
		{
			// To drain all buffers
			if (valid)
				stop();

			decoder->seek(offsetSeconds);

			if (wasPlaying)
				play();

			break;
		}
		case TYPE_QUEUE:
			if (valid)
			{
				alSourcef(source, AL_SAMPLE_OFFSET, offsetSamples);
				offsetSamples = offsetSeconds = 0;
			}
			else
			{
				ALint size;
				ALuint buffer = unusedBufferPeek();

				//emulate AL behavior, discarding buffer once playback head is past one
				while (buffer != AL_NONE)
				{
					alGetBufferi(buffer, AL_SIZE, &size);

					if (offsetSamples < size / (bitDepth / 8 * channels))
						break;

					unusedBufferPop();
					buffer = unusedBufferPeek();
					bufferedBytes -= size;
					offsetSamples -= size / (bitDepth / 8 * channels);
				}
				if (buffer == AL_NONE)
					offsetSamples = 0;
				offsetSeconds = offsetSamples / sampleRate;
			}
			break;
		case TYPE_MAX_ENUM:
			break;
	}
	if (wasPlaying && (alGetError() == AL_INVALID_VALUE || (sourceType == TYPE_STREAM && !isPlaying())))
	{
		stop();
		if (isLooping())
			play();
		return;
	}
	this->offsetSamples = offsetSamples;
	this->offsetSeconds = offsetSeconds;
}

void Source::seek(float offset, Source::Unit unit)
{
	return pool->seek(this, offset, &unit);
}

float Source::tellAtomic(void *unit) const
{
	float offset = 0.0f;

	switch (*((Source::Unit *) unit))
	{
	case Source::UNIT_SAMPLES:
		if (valid)
			alGetSourcef(source, AL_SAMPLE_OFFSET, &offset);
		offset += offsetSamples;
		break;
	case Source::UNIT_SECONDS:
	default:
		if (valid)
			alGetSourcef(source, AL_SEC_OFFSET, &offset);
		offset += offsetSeconds;
		break;
	}

	return offset;
}

float Source::tell(Source::Unit unit)
{
	return pool->tell(this, &unit);
}

double Source::getDurationAtomic(void *vunit)
{
	Unit unit = *(Unit *) vunit;

	switch (sourceType)
	{
		case TYPE_STATIC:
		{
			ALsizei size = staticBuffer->getSize();
			ALsizei samples = (size / channels) / (bitDepth / 8);

			if (unit == UNIT_SAMPLES)
				return (double) samples;
			else
				return (double) samples / (double) sampleRate;
		}
		case TYPE_STREAM:
		{
			double seconds = decoder->getDuration();

			if (unit == UNIT_SECONDS)
				return seconds;
			else
				return seconds * decoder->getSampleRate();
		}
		case TYPE_QUEUE:
		{
			ALsizei samples = (bufferedBytes / channels) / (bitDepth / 8);

			if (unit == UNIT_SAMPLES)
				return (double)samples;
			else
				return (double)samples / (double)sampleRate;
		}
		case TYPE_MAX_ENUM:
			return 0.0;
	}
	return 0.0;
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
	if (sourceType == TYPE_QUEUE)
		throw QueueLoopingException();

	if (valid && sourceType == TYPE_STATIC)
		alSourcei(source, AL_LOOPING, enable ? AL_TRUE : AL_FALSE);

	looping = enable;
}

bool Source::isLooping() const
{
	return looping;
}

bool Source::queue(void *data, size_t length, int dataSampleRate, int dataBitDepth, int dataChannels)
{
	if (sourceType != TYPE_QUEUE)
		throw QueueTypeMismatchException();

	if (dataSampleRate != sampleRate || dataBitDepth != bitDepth || dataChannels != channels )
		throw QueueFormatMismatchException();

	if (length % (bitDepth / 8 * channels) != 0)
		throw QueueMalformedLengthException(bitDepth / 8 * channels);

	if (length == 0)
		return true;

	return pool->queue(this, data, (ALsizei)length);
}

bool Source::queueAtomic(void *data, ALsizei length)
{
	if (valid)
	{
		ALuint buffer = unusedBufferPeek();
		if (buffer == AL_NONE)
			return false;

		alBufferData(buffer, Audio::getFormat(bitDepth, channels), data, length, sampleRate);
		alSourceQueueBuffers(source, 1, &buffer);
		unusedBufferPop();
	}
	else
	{
		ALuint buffer = unusedBufferPeekNext();
		if (buffer == AL_NONE)
			return false;

		//stack acts as queue while stopped
		alBufferData(buffer, Audio::getFormat(bitDepth, channels), data, length, sampleRate);
		unusedBufferQueue(buffer);
	}
	bufferedBytes += length;

	return true;
}

int Source::getFreeBufferCount() const
{
	switch (sourceType) //why not :^)
	{
		case TYPE_STATIC: 
			return 0;
		case TYPE_STREAM:
			return unusedBufferTop + 1;
		case TYPE_QUEUE:
			return valid ? unusedBufferTop + 1 : (int)MAX_BUFFERS - unusedBufferTop - 1;
		case TYPE_MAX_ENUM:
			return 0; 
	}
	return 0;
}

void Source::prepareAtomic()
{
	// This Source may now be associated with an OpenAL source that still has
	// the properties of another love Source. Let's reset it to the settings
	// of the new one.
	reset();

	switch (sourceType)
	{
		case TYPE_STATIC:
			alSourcei(source, AL_BUFFER, staticBuffer->getBuffer());
			break;
		case TYPE_STREAM:
			while (unusedBufferPeek() != AL_NONE)
			{
				if(streamAtomic(unusedBufferPeek(), decoder.get()) == 0)
					break;

				alSourceQueueBuffers(source, 1, unusedBufferPop());

				if (decoder->isFinished())
					break;
			}
			break;
		case TYPE_QUEUE:
		{
			int top = unusedBufferTop;
			//when queue source is stopped, loaded buffers are stored in unused buffers stack
			while (unusedBufferPeek() != AL_NONE)
				alSourceQueueBuffers(source, 1, unusedBufferPop());
			//construct a stack of unused buffers (beyond the end of stack)
			for (unsigned int i = top + 1; i < MAX_BUFFERS; i++)
				unusedBufferPush(unusedBuffers[i]);

			break;
		}
		case TYPE_MAX_ENUM:
			break;
	}
}

void Source::teardownAtomic()
{
	switch (sourceType)
	{
		case TYPE_STATIC:
			break;
		case TYPE_STREAM:
		{
			ALint queued;
			ALuint buffer;

			decoder->seek(0);
			// drain buffers
			//since we only unqueue 1 buffer, it's OK to use singular variable pointer instead of array
			alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);
			for (unsigned int i = 0; i < (unsigned int)queued; i++)
				alSourceUnqueueBuffers(source, 1, &buffer);

			// generate unused buffers list
			for (unsigned int i = 0; i < MAX_BUFFERS; i++)
				unusedBuffers[i] = streamBuffers[i];

			unusedBufferTop = MAX_BUFFERS - 1;
			break;
		}
		case TYPE_QUEUE:
		{
			ALint queued;
			ALuint buffer;

			alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);
			for (unsigned int i = (unsigned int)queued; i > 0; i--)
				alSourceUnqueueBuffers(source, 1, &buffer);

			// generate unused buffers list
			for (unsigned int i = 0; i < MAX_BUFFERS; i++)
				unusedBuffers[i] = streamBuffers[i];

			unusedBufferTop = -1;
			break;
		}
		case TYPE_MAX_ENUM:
			break;
	}

	alSourcei(source, AL_BUFFER, AL_NONE);

	toLoop = 0;
	valid = false;
	offsetSamples = offsetSeconds = 0;
}

bool Source::playAtomic(ALuint source)
{
	this->source = source;
	prepareAtomic();

	// Clear errors.
	alGetError();

	alSourcePlay(source);

	bool success = alGetError() == AL_NO_ERROR;

	if (sourceType == TYPE_STREAM)
	{
		valid = true; //isPlaying() needs source to be valid
		if (!isPlaying())
			success = false;
	}
	else if (success)
	{
		alSourcef(source, AL_SAMPLE_OFFSET, offsetSamples);
		success = alGetError() == AL_NO_ERROR;
	}

	if (!success)
	{
		valid = true; //stop() needs source to be valid
		stop();
	}
	if (sourceType != TYPE_STREAM)
		offsetSamples = offsetSeconds = 0;

	//this is set to success state afterwards anyway, but setting it here
	//to true preemptively avoids race condition with update bug
	valid = true; 

	return success;
}

void Source::stopAtomic()
{
	if (!valid)
		return;
	alSourceStop(source);
	teardownAtomic();
}

void Source::pauseAtomic()
{
	if (valid)
		alSourcePause(source);
}

void Source::resumeAtomic()
{
	if (valid && !isPlaying())
	{
		alSourcePlay(source);

		if (alGetError() == AL_INVALID_VALUE || (sourceType == TYPE_STREAM && unusedBufferTop == MAX_BUFFERS - 1))
			stop();
	}
}

bool Source::playAtomic(const std::vector<love::audio::Source*> &sources, const std::vector<ALuint> &ids, const std::vector<char> &wasPlaying)
{
	if (sources.size() == 0)
		return true;

	std::vector<ALuint> toPlay;
	toPlay.reserve(sources.size());
	for (size_t i = 0; i < sources.size(); i++)
	{
		Source *source = (Source*) sources[i];
		// Paused sources have wasPlaying set to true, so do this first
		if (!source->isPlaying())
			toPlay.push_back(ids[i]);

		// If it wasn't playing (nor paused), we have just allocated a new
		// source
		if (wasPlaying[i])
			continue;
		source->source = ids[i];
		source->prepareAtomic();
	}

	alGetError();
	alSourcePlayv((ALsizei) toPlay.size(), &toPlay[0]);
	bool success = alGetError() == AL_NO_ERROR;

	for (auto &_source : sources)
	{
		Source *source = (Source*) _source;
		source->valid = source->valid || success;

		if (success && source->sourceType != TYPE_STREAM)
			source->offsetSamples = source->offsetSeconds = 0;
	}

	return success;
}

void Source::stopAtomic(const std::vector<love::audio::Source*> &sources)
{
	if (sources.size() == 0)
		return;

	std::vector<ALuint> sourceIds;
	sourceIds.reserve(sources.size());
	for (auto &_source : sources)
	{
		Source *source = (Source*) _source;
		if (source->valid)
			sourceIds.push_back(source->source);
	}

	alSourceStopv((ALsizei) sources.size(), &sourceIds[0]);

	for (auto &_source : sources)
	{
		Source *source = (Source*) _source;
		if (source->valid)
			source->teardownAtomic();
	}
}

void Source::pauseAtomic(const std::vector<love::audio::Source*> &sources)
{
	if (sources.size() == 0)
		return;

	std::vector<ALuint> sourceIds;
	sourceIds.reserve(sources.size());
	for (auto &_source : sources)
	{
		Source *source = (Source*) _source;
		if (source->valid)
			sourceIds.push_back(source->source);
	}

	alSourcePausev((ALsizei) sources.size(), &sourceIds[0]);
}

void Source::reset()
{
	alSourcei(source, AL_BUFFER, AL_NONE);
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
	alSourcei(source, AL_LOOPING, (sourceType == TYPE_STATIC) && isLooping() ? AL_TRUE : AL_FALSE);
	alSourcei(source, AL_SOURCE_RELATIVE, relative ? AL_TRUE : AL_FALSE);
	alSourcei(source, AL_CONE_INNER_ANGLE, cone.innerAngle);
	alSourcei(source, AL_CONE_OUTER_ANGLE, cone.outerAngle);
	alSourcef(source, AL_CONE_OUTER_GAIN, cone.outerVolume);
	#ifdef ALC_EXT_EFX
	alSourcei(source, AL_DIRECT_FILTER, filter ? filter->getFilter() : AL_FILTER_NULL);
	#endif
}

void Source::setFloatv(float *dst, const float *src) const
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
}

ALuint Source::unusedBufferPeek()
{
	return (unusedBufferTop < 0) ? AL_NONE : unusedBuffers[unusedBufferTop];
}

ALuint Source::unusedBufferPeekNext()
{
	return (unusedBufferTop >= (int)MAX_BUFFERS - 1) ? AL_NONE : unusedBuffers[unusedBufferTop + 1];
}

ALuint *Source::unusedBufferPop()
{
	return &unusedBuffers[unusedBufferTop--];
}

void Source::unusedBufferPush(ALuint buffer)
{
	unusedBuffers[++unusedBufferTop] = buffer;
}

void Source::unusedBufferQueue(ALuint buffer)
{
	for (unsigned int i = ++unusedBufferTop; i > 0; i--)
		unusedBuffers[i] = unusedBuffers[i - 1];
	unusedBuffers[0] = buffer;
}

int Source::streamAtomic(ALuint buffer, love::sound::Decoder *d)
{
	// Get more sound data.
	int decoded = std::max(d->decode(), 0);

	// OpenAL implementations are allowed to ignore 0-size alBufferData calls.
	if (decoded > 0)
	{
		int fmt = Audio::getFormat(d->getBitDepth(), d->getChannels());

		if (fmt != AL_NONE)
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

bool Source::setFilter(love::audio::Filter::Type type, std::vector<float> &params)
{
	if (!filter)
		filter = new Filter();

	bool result = filter->setParams(type, params);

	#ifdef ALC_EXT_EFX
	if (valid)
		alSourcei(source, AL_DIRECT_FILTER, filter->getFilter());
	#endif

	return result;
}

bool Source::setFilter()
{
	if (filter)
		delete filter;

	filter = nullptr;

	#ifdef ALC_EXT_EFX
	if (valid)
		alSourcei(source, AL_DIRECT_FILTER, AL_FILTER_NULL);
	#endif

	return true;
}

bool Source::getFilter(love::audio::Filter::Type &type, std::vector<float> &params)
{
	if (!filter)
		return false;

	type = filter->getType();
	params = filter->getParams();
	return true;
}

} // openal
} // audio
} // love
