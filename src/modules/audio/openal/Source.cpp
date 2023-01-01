/**
 * Copyright (c) 2006-2023 LOVE Development Team
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

#define audiomodule() (Module::getInstance<Audio>(Module::M_AUDIO))

using love::thread::Lock;

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
	, channels(soundData->getChannelCount())
	, bitDepth(soundData->getBitDepth())
{
	ALenum fmt = Audio::getFormat(soundData->getBitDepth(), soundData->getChannelCount());
	if (fmt == AL_NONE)
		throw InvalidFormatException(soundData->getChannelCount(), soundData->getBitDepth());

	staticBuffer.set(new StaticDataBuffer(fmt, soundData->getData(), (ALsizei) soundData->getSize(), sampleRate), Acquire::NORETAIN);

	float z[3] = {0, 0, 0};

	setFloatv(position, z);
	setFloatv(velocity, z);
	setFloatv(direction, z);

	for (int i = 0; i < audiomodule()->getMaxSourceEffects(); i++)
		slotlist.push(i);
}

Source::Source(Pool *pool, love::sound::Decoder *decoder)
	: love::audio::Source(Source::TYPE_STREAM)
	, pool(pool)
	, sampleRate(decoder->getSampleRate())
	, channels(decoder->getChannelCount())
	, bitDepth(decoder->getBitDepth())
	, decoder(decoder)
	, buffers(DEFAULT_BUFFERS)
{
	if (Audio::getFormat(decoder->getBitDepth(), decoder->getChannelCount()) == AL_NONE)
		throw InvalidFormatException(decoder->getChannelCount(), decoder->getBitDepth());

	for (int i = 0; i < buffers; i++)
	{
		ALuint buf;
		alGenBuffers(1, &buf);
		if (alGetError() == AL_NO_ERROR)
			unusedBuffers.push(buf);
		else
		{
			buffers = i;
			break;
		}
	}

	float z[3] = {0, 0, 0};

	setFloatv(position, z);
	setFloatv(velocity, z);
	setFloatv(direction, z);

	for (int i = 0; i < audiomodule()->getMaxSourceEffects(); i++)
		slotlist.push(i);
}

Source::Source(Pool *pool, int sampleRate, int bitDepth, int channels, int buffers)
	: love::audio::Source(Source::TYPE_QUEUE)
	, pool(pool)
	, sampleRate(sampleRate)
	, channels(channels)
	, bitDepth(bitDepth)
	, buffers(buffers)
{
	ALenum fmt = Audio::getFormat(bitDepth, channels);
	if (fmt == AL_NONE)
		throw InvalidFormatException(channels, bitDepth);

	if (buffers < 1)
		buffers = DEFAULT_BUFFERS;
	if (buffers > MAX_BUFFERS)
		buffers = MAX_BUFFERS;

	for (int i = 0; i < buffers; i++)
	{
		ALuint buf;
		alGenBuffers(1, &buf);
		if (alGetError() == AL_NO_ERROR)
			unusedBuffers.push(buf);
		else
		{
			buffers = i;
			break;
		}
	}

	float z[3] = {0, 0, 0};

	setFloatv(position, z);
	setFloatv(velocity, z);
	setFloatv(direction, z);

	for (int i = 0; i < audiomodule()->getMaxSourceEffects(); i++)
		slotlist.push(i);
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
	, sampleRate(s.sampleRate)
	, channels(s.channels)
	, bitDepth(s.bitDepth)
	, decoder(nullptr)
	, toLoop(0)
	, buffers(s.buffers)
{
	if (sourceType == TYPE_STREAM)
	{
		if (s.decoder.get())
			decoder.set(s.decoder->clone(), Acquire::NORETAIN);
	}
	if (sourceType != TYPE_STATIC)
	{
		for (int i = 0; i < buffers; i++)
		{
			ALuint buf;
			alGenBuffers(1, &buf);
			if (alGetError() == AL_NO_ERROR)
				unusedBuffers.push(buf);
			else
			{
				buffers = i;
				break;
			}
		}
	}

	if (s.directfilter)
		directfilter = s.directfilter->clone();

	for (auto e : s.effectmap)
	{
		Filter *filter = e.second.filter ? e.second.filter->clone() : nullptr;
		effectmap[e.first] = { filter, e.second.slot, e.second.target };
	}

	setFloatv(position, s.position);
	setFloatv(velocity, s.velocity);
	setFloatv(direction, s.direction);

	for (int i = 0; i < audiomodule()->getMaxSourceEffects(); i++)
	{
		// filter out already taken slots
		bool push = true;
		for (auto e : effectmap)
		{
			if (e.second.slot)
			{
				push = false;
				break;
			}
		}
		if (push)
			slotlist.push(i);
	}
}

Source::~Source()
{
	stop();

	if (sourceType != TYPE_STATIC)
	{
		while (!streamBuffers.empty())
		{
			alDeleteBuffers(1, &streamBuffers.front());
			streamBuffers.pop();
		}
		while (!unusedBuffers.empty())
		{
			alDeleteBuffers(1, &unusedBuffers.top());
			unusedBuffers.pop();
		}
	}

	if (directfilter)
		delete directfilter;

	for (auto e : effectmap)
	{
		if (e.second.filter)
			delete e.second.filter;
	}
}

love::audio::Source *Source::clone()
{
	return new Source(*this);
}

bool Source::play()
{
	Lock l = pool->lock();
	ALuint out;

	char wasPlaying;
	if (!pool->assignSource(this, out, wasPlaying))
		return valid = false;

	if (!wasPlaying)
		return valid = playAtomic(out);

	resumeAtomic();
	return valid = true;
}

void Source::stop()
{
	if (!valid)
		return;

	Lock l = pool->lock();
	pool->releaseSource(this);
}

void Source::pause()
{
	Lock l = pool->lock();
	if (pool->isPlaying(this))
		pauseAtomic();
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
				alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);

				// It would theoretically be better to unqueue all processed
				// buffers in a single call to alSourceUnqueueBuffers, but on
				// iOS I observed occasional (every ~5-10 seconds) pops in the
				// streaming source test I was using, when doing that. Perhaps
				// there was a bug in this code when I was testing, or maybe
				// this code runs into the same problem but now it's much harder
				// to reproduce. The test I used is the play-stop-play .love
				// from https://bitbucket.org/rude/love/issues/1484/
				while (processed--)
				{
					int curOffsetSamples;
					alGetSourcei(source, AL_SAMPLE_OFFSET, &curOffsetSamples);

					ALuint buffer;
					alSourceUnqueueBuffers(source, 1, &buffer);

					int newOffsetSamples;
					alGetSourcei(source, AL_SAMPLE_OFFSET, &newOffsetSamples);

					offsetSamples += (curOffsetSamples - newOffsetSamples);

					if (streamAtomic(buffer, decoder.get()) > 0)
						alSourceQueueBuffers(source, 1, &buffer);
					else
						unusedBuffers.push(buffer);
				}

				while (!unusedBuffers.empty())
				{
					ALuint b = unusedBuffers.top();
					if (streamAtomic(b, decoder.get()) > 0)
					{
						alSourceQueueBuffers(source, 1, &b);
						unusedBuffers.pop();
					}
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

			for (int i = 0; i < processed; i++)
			{
				ALint size;
				alGetBufferi(buffers[i], AL_SIZE, &size);
				bufferedBytes -= size;
				unusedBuffers.push(buffers[i]);
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

void Source::seek(double offset, Source::Unit unit)
{
	Lock l = pool->lock();

	int offsetSamples = 0;
	double offsetSeconds = 0.0f;

	switch (unit)
	{
	case Source::UNIT_SAMPLES:
		offsetSamples = (int) offset;
		offsetSeconds = offset / (double) sampleRate;
		break;
	case Source::UNIT_SECONDS:
	default:
		offsetSeconds = offset;
		offsetSamples = (int) (offset * sampleRate);
		break;
	}

	bool wasPlaying = isPlaying();
	switch (sourceType)
	{
		case TYPE_STATIC:
			if (valid)
			{
				alSourcei(source, AL_SAMPLE_OFFSET, offsetSamples);
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
				alSourcei(source, AL_SAMPLE_OFFSET, offsetSamples);
				offsetSamples = offsetSeconds = 0;
			}
			else
			{
				//emulate AL behavior, discarding buffer once playback head is past one
				while (!unusedBuffers.empty())
				{
					ALint size;
					auto buffer = unusedBuffers.top();
					alGetBufferi(buffer, AL_SIZE, &size);

					if (offsetSamples < size / (bitDepth / 8 * channels))
						break;

					unusedBuffers.pop();
					bufferedBytes -= size;
					offsetSamples -= size / (bitDepth / 8 * channels);
				}
				if (unusedBuffers.empty())
					offsetSamples = 0;
				offsetSeconds = offsetSamples / (double) sampleRate;
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
}

double Source::tell(Source::Unit unit)
{
	Lock l = pool->lock();

	int offset = 0;

	if (valid)
		alGetSourcei(source, AL_SAMPLE_OFFSET, &offset);

	offset += offsetSamples;

	if (unit == UNIT_SECONDS)
		return offset / (double) sampleRate;
	else
		return offset;
}

double Source::getDuration(Unit unit)
{
	Lock l = pool->lock();

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

void Source::setCone(float innerAngle, float outerAngle, float outerVolume, float outerHighGain)
{
	if (channels > 1)
		throw SpatialSupportException();

	cone.innerAngle  = (int) LOVE_TODEG(innerAngle);
	cone.outerAngle  = (int) LOVE_TODEG(outerAngle);
	cone.outerVolume = outerVolume;
	cone.outerHighGain = outerHighGain;

	if (valid)
	{
		alSourcei(source, AL_CONE_INNER_ANGLE, cone.innerAngle);
		alSourcei(source, AL_CONE_OUTER_ANGLE, cone.outerAngle);
		alSourcef(source, AL_CONE_OUTER_GAIN, cone.outerVolume);
#ifdef ALC_EXT_EFX
		alSourcef(source, AL_CONE_OUTER_GAINHF, cone.outerHighGain);
#endif
	}
}

void Source::getCone(float &innerAngle, float &outerAngle, float &outerVolume, float &outerHighGain) const
{
	if (channels > 1)
		throw SpatialSupportException();

	innerAngle  = LOVE_TORAD(cone.innerAngle);
	outerAngle  = LOVE_TORAD(cone.outerAngle);
	outerVolume = cone.outerVolume;
	outerHighGain = cone.outerHighGain;
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

	Lock l = pool->lock();

	if (unusedBuffers.empty())
		return false;

	auto buffer = unusedBuffers.top();
	unusedBuffers.pop();
	alBufferData(buffer, Audio::getFormat(bitDepth, channels), data, length, sampleRate);
	bufferedBytes += length;

	if (valid)
		alSourceQueueBuffers(source, 1, &buffer);
	else
		streamBuffers.push(buffer);

	return true;
}

int Source::getFreeBufferCount() const
{
	switch (sourceType) //why not :^)
	{
	case TYPE_STATIC:
		return 0;
	case TYPE_STREAM:
		return unusedBuffers.size();
	case TYPE_QUEUE:
		return unusedBuffers.size();
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
		while (!unusedBuffers.empty())
		{
			auto b = unusedBuffers.top();
			if (streamAtomic(b, decoder.get()) == 0)
				break;

			alSourceQueueBuffers(source, 1, &b);
			unusedBuffers.pop();

			if (decoder->isFinished())
				break;
		}
		break;
	case TYPE_QUEUE:
	{
		while (!streamBuffers.empty())
		{
			alSourceQueueBuffers(source, 1, &streamBuffers.front());
			streamBuffers.pop();
		}
		break;
	}
	case TYPE_MAX_ENUM:
		break;
	}

	// Seek to the current/pending offset.
	alSourcei(source, AL_SAMPLE_OFFSET, offsetSamples);
}

void Source::teardownAtomic()
{
	switch (sourceType)
	{
	case TYPE_STATIC:
		break;
	case TYPE_STREAM:
	{
		ALint queued = 0;
		ALuint buffers[MAX_BUFFERS];

		// Some decoders (e.g. ModPlug) can rewind() more reliably than seek(0).
		decoder->rewind();

		// Drain buffers.
		// NOTE: The Apple implementation of OpenAL on iOS doesn't return
		// correct buffer ids for single alSourceUnqueueBuffers calls past the
		// first queued buffer, so we must unqueue them all at once.
		alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);
		alSourceUnqueueBuffers(source, queued, buffers);

		for (int i = 0; i < queued; i++)
			unusedBuffers.push(buffers[i]);
		break;
	}
	case TYPE_QUEUE:
	{
		ALint queued;
		ALuint buffers[MAX_BUFFERS];

		alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);
		alSourceUnqueueBuffers(source, queued, buffers);

		for (int i = 0; i < queued; i++)
			unusedBuffers.push(buffers[i]);
		break;
	}
	case TYPE_MAX_ENUM:
		break;
	}

	alSourcei(source, AL_BUFFER, AL_NONE);

	toLoop = 0;
	valid = false;
	offsetSamples = 0;
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

	if (!success)
	{
		valid = true; //stop() needs source to be valid
		stop();
	}

	// Static sources: reset the pending offset since it's not valid anymore.
	if (sourceType != TYPE_STREAM)
		offsetSamples = 0;

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

		//failed to play or nothing to play
		if (alGetError() == AL_INVALID_VALUE || (sourceType == TYPE_STREAM && (int) unusedBuffers.size() == buffers))
			stop();
	}
}

bool Source::play(const std::vector<love::audio::Source*> &sources)
{
	if (sources.size() == 0)
		return true;

	Pool *pool = ((Source*) sources[0])->pool;
	Lock l = pool->lock();

	// NOTE: not bool, because std::vector<bool> is implemented as a bitvector
	// which means no bool references can be created.
	std::vector<char> wasPlaying(sources.size());
	std::vector<ALuint> ids(sources.size());

	for (size_t i = 0; i < sources.size(); i++)
	{
		if (!pool->assignSource((Source*) sources[i], ids[i], wasPlaying[i]))
		{
			for (size_t j = 0; j < i; j++)
				if (!wasPlaying[j])
					pool->releaseSource((Source*) sources[j], false);
			return false;
		}
	}

	std::vector<ALuint> toPlay;
	toPlay.reserve(sources.size());
	for (size_t i = 0; i < sources.size(); i++)
	{
		// If the source was paused, wasPlaying[i] will be true but we still
		// want to resume it. We don't want to call alSourcePlay on sources
		// that are actually playing though.
		if (wasPlaying[i] && sources[i]->isPlaying())
			continue;

		if (!wasPlaying[i])
		{
			Source *source = (Source*) sources[i];
			source->source = ids[i];
			source->prepareAtomic();
		}

		toPlay.push_back(ids[i]);
	}

	alGetError();
	alSourcePlayv((ALsizei) toPlay.size(), &toPlay[0]);
	bool success = alGetError() == AL_NO_ERROR;

	for (auto &_source : sources)
	{
		Source *source = (Source*) _source;
		source->valid = source->valid || success;

		if (success && source->sourceType != TYPE_STREAM)
			source->offsetSamples = 0;
	}

	return success;
}

void Source::stop(const std::vector<love::audio::Source*> &sources)
{
	if (sources.size() == 0)
		return;

	Pool *pool = ((Source*) sources[0])->pool;
	Lock l = pool->lock();

	std::vector<ALuint> sourceIds;
	sourceIds.reserve(sources.size());
	for (auto &_source : sources)
	{
		Source *source = (Source*) _source;
		if (source->valid)
			sourceIds.push_back(source->source);
	}

	alSourceStopv((ALsizei) sourceIds.size(), &sourceIds[0]);

	for (auto &_source : sources)
	{
		Source *source = (Source*) _source;
		if (source->valid)
			source->teardownAtomic();
		pool->releaseSource(source, false);
	}
}

void Source::pause(const std::vector<love::audio::Source*> &sources)
{
	if (sources.size() == 0)
		return;

	Lock l = ((Source*) sources[0])->pool->lock();

	std::vector<ALuint> sourceIds;
	sourceIds.reserve(sources.size());
	for (auto &_source : sources)
	{
		Source *source = (Source*) _source;
		if (source->valid)
			sourceIds.push_back(source->source);
	}

	alSourcePausev((ALsizei) sourceIds.size(), &sourceIds[0]);
}

std::vector<love::audio::Source*> Source::pause(Pool *pool)
{
	Lock l = pool->lock();
	std::vector<love::audio::Source*> sources = pool->getPlayingSources();

	auto newend = std::remove_if(sources.begin(), sources.end(), [](love::audio::Source* s) {
		return !s->isPlaying();
	});
	sources.erase(newend, sources.end());

	pause(sources);
	return sources;
}

void Source::stop(Pool *pool)
{
	Lock l = pool->lock();
	stop(pool->getPlayingSources());
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
	alSourcef(source, AL_AIR_ABSORPTION_FACTOR, absorptionFactor);
	alSourcef(source, AL_CONE_OUTER_GAINHF, cone.outerHighGain);
	alSourcef(source, AL_ROOM_ROLLOFF_FACTOR, rolloffFactor); //reverb-specific rolloff
	alSourcei(source, AL_DIRECT_FILTER, directfilter ? directfilter->getFilter() : AL_FILTER_NULL);
	// clear all send slots, then re-enable applied ones
	for (int i = 0; i < audiomodule()->getMaxSourceEffects(); i++)
		alSource3i(source, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, i, AL_FILTER_NULL);
	for (auto i : effectmap)
		alSource3i(source, AL_AUXILIARY_SEND_FILTER, i.second.target, i.second.slot, i.second.filter ? i.second.filter->getFilter() : AL_FILTER_NULL);
	//alGetError();
#endif
}

void Source::setFloatv(float *dst, const float *src) const
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
}

int Source::streamAtomic(ALuint buffer, love::sound::Decoder *d)
{
	// Get more sound data.
	int decoded = std::max(d->decode(), 0);

	// OpenAL implementations are allowed to ignore 0-size alBufferData calls.
	if (decoded > 0)
	{
		int fmt = Audio::getFormat(d->getBitDepth(), d->getChannelCount());

		if (fmt != AL_NONE)
			alBufferData(buffer, fmt, d->getBuffer(), decoded, d->getSampleRate());
		else
			decoded = 0;
	}

	// This shouldn't run after toLoop is calculated in this streamAtomic call,
	// otherwise it'll decrease too quickly.
	// TODO: this code is hard to understand, can it be made more clear?
	// It's meant to reset offsetSamples once OpenAL starts processing the first
	// queued buffer after a loop.
	if (toLoop > 0)
	{
		if (--toLoop == 0)
		{
			offsetSamples = 0;
		}
	}

	if (decoder->isFinished() && isLooping())
	{
		int queued, processed;
		alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);
		alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
		if (queued > processed)
			toLoop = queued-processed;
		else
			toLoop = buffers-processed;
		d->rewind();
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

void Source::setAirAbsorptionFactor(float factor)
{
	if (channels > 1)
		throw SpatialSupportException();

	absorptionFactor = factor;
#ifdef ALC_EXT_EFX
	if (valid)
	{
		alSourcef(source, AL_AIR_ABSORPTION_FACTOR, absorptionFactor);
		//alGetError();
	}
#endif
}

float Source::getAirAbsorptionFactor() const
{
	if (channels > 1)
		throw SpatialSupportException();

	return absorptionFactor;
}

int Source::getChannelCount() const
{
	return channels;
}

bool Source::setFilter(const std::map<Filter::Parameter, float> &params)
{
	if (!directfilter)
		directfilter = new Filter();

	bool result = directfilter->setParams(params);

#ifdef ALC_EXT_EFX
	if (valid)
	{
		//in case of failure contains AL_FILTER_NULL, a valid non-filter
		alSourcei(source, AL_DIRECT_FILTER, directfilter->getFilter());
		//alGetError();
	}
#endif

	return result;
}

bool Source::setFilter()
{
	if (directfilter)
		delete directfilter;

	directfilter = nullptr;

#ifdef ALC_EXT_EFX
	if (valid)
	{
		alSourcei(source, AL_DIRECT_FILTER, AL_FILTER_NULL);
		//alGetError();
	}
#endif

	return true;
}

bool Source::getFilter(std::map<Filter::Parameter, float> &params)
{
	if (!directfilter)
		return false;

	params = directfilter->getParams();

	return true;
}

bool Source::setEffect(const char *name)
{
	ALuint slot, target;
	Filter *filter;

	// effect with this name doesn't exist
	if (!dynamic_cast<Audio*>(audiomodule())->getEffectID(name, target))
		return false;

	auto iter = effectmap.find(name);
	if (iter == effectmap.end())
	{
		// new send target needed but no more room
		if (slotlist.empty())
			return false;

		slot = slotlist.top();
		slotlist.pop();
	}
	else
	{
		slot = iter->second.slot;
		filter = iter->second.filter;

		if (filter)
			delete filter;
	}
	effectmap[name] = {nullptr, slot, target};

#ifdef ALC_EXT_EFX
	if (valid)
	{
		alSource3i(source, AL_AUXILIARY_SEND_FILTER, target, slot, AL_FILTER_NULL);
		//alGetError();
	}
#endif

	return true;
}

bool Source::setEffect(const char *name, const std::map<Filter::Parameter, float> &params)
{
	ALuint slot, target;
	Filter *filter = nullptr;

	// effect with this name doesn't exist
	if (!dynamic_cast<Audio*>(audiomodule())->getEffectID(name, target))
		return false;

	auto iter = effectmap.find(name);
	if (iter == effectmap.end())
	{
		// new send target needed but no more room
		if (slotlist.empty())
			return false;

		slot = slotlist.top();
		slotlist.pop();
	}
	else
	{
		slot = iter->second.slot;
		filter = iter->second.filter;
	}
	if (!filter)
		filter = new Filter();

	effectmap[name] = {filter, slot, target};

	filter->setParams(params);

#ifdef ALC_EXT_EFX
	if (valid)
	{
		//in case of failure contains AL_FILTER_NULL, a valid non-filter
		alSource3i(source, AL_AUXILIARY_SEND_FILTER, target, slot, filter->getFilter());
		//alGetError();
	}
#endif
	return true;
}

bool Source::unsetEffect(const char *name)
{
	auto iter = effectmap.find(name);
	if (iter == effectmap.end())
		return false;

	ALuint slot = iter->second.slot;
	Filter *filter = iter->second.filter;

	if (filter)
		delete filter;

#ifdef ALC_EXT_EFX
	if (valid)
	{
		alSource3i(source, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, slot, AL_FILTER_NULL);
		//alGetError();
	}
#endif
	effectmap.erase(iter);
	slotlist.push(slot);
	return true;
}

bool Source::getEffect(const char *name, std::map<Filter::Parameter, float> &params)
{
	auto iter = effectmap.find(name);
	if (iter == effectmap.end())
		return false;

	if (iter->second.filter)
		params = iter->second.filter->getParams();

	return true;
}

bool Source::getActiveEffects(std::vector<std::string> &list) const
{
	if (effectmap.empty())
		return false;

	list.reserve(effectmap.size());

	for (auto i : effectmap)
		list.push_back(i.first);

	return true;
}

} // openal
} // audio
} // love
