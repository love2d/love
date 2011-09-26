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

#include "Source.h"

#include "Pool.h"

// STD
#include <iostream>

namespace love
{
namespace audio
{
namespace openal
{

	Source::Source(Pool * pool, love::sound::SoundData * soundData)
		: love::audio::Source(Source::TYPE_STATIC), pool(pool), valid(false),
		pitch(1.0f), volume(1.0f), looping(false), paused(false), offsetSamples(0),
		offsetSeconds(0), decoder(0), toLoop(0)
	{
		alGenBuffers(1, buffers);
		ALenum fmt = getFormat(soundData->getChannels(), soundData->getBits());
		alBufferData(buffers[0], fmt, soundData->getData(), soundData->getSize(), soundData->getSampleRate());

		static float z[3] = {0, 0, 0};

		setFloatv(position, z);
		setFloatv(velocity, z);
		setFloatv(direction, z);
	}

	Source::Source(Pool * pool, love::sound::Decoder * decoder)
		: love::audio::Source(Source::TYPE_STREAM), pool(pool), valid(false),
		pitch(1.0f), volume(1.0f), looping(false), paused(false), offsetSamples(0),
		offsetSeconds(0), decoder(decoder), toLoop(0)
	{
		decoder->retain();
		alGenBuffers(MAX_BUFFERS, buffers);

		static float z[3] = {0, 0, 0};

		setFloatv(position, z);
		setFloatv(velocity, z);
		setFloatv(direction, z);
	}

	Source::~Source()
	{
		if (valid)
			pool->stop(this);
		alDeleteBuffers((type == TYPE_STATIC) ? 1 : MAX_BUFFERS, buffers);
		if (decoder)
			decoder->release();
	}

	love::audio::Source * Source::copy()
	{
		return 0;
	}

	void Source::play()
	{
		if (valid && paused)
		{
			pool->resume(this);
			return;
		}

		valid = pool->play(this, source);

		if(valid)
			reset(source);
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
		if(valid)
		{
			ALenum state;
			alGetSourcei(source, AL_SOURCE_STATE, &state);
			return (state == AL_STOPPED);
		}

		return true;
	}

	bool Source::isPaused() const
	{
		if(valid)
		{
			ALenum state;
			alGetSourcei(source, AL_SOURCE_STATE, &state);
			return (state == AL_PAUSED);
		}

		return false;
	}

	bool Source::isFinished() const
	{
		return type == TYPE_STATIC ? isStopped() : isStopped() && !isLooping() && decoder->isFinished();
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
			ALint processed;

			alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);

			while(processed--)
			{
				ALuint buffer;

				float curOffsetSamples, curOffsetSecs;

				alGetSourcef(source, AL_SAMPLE_OFFSET, &curOffsetSamples);

				ALint b;
				alGetSourcei(source, AL_BUFFER, &b);
				int freq;
				alGetBufferi(b, AL_FREQUENCY, &freq);
				curOffsetSecs = curOffsetSamples / freq;

				// Get a free buffer.
				alSourceUnqueueBuffers(source, 1, &buffer);

				float newOffsetSamples, newOffsetSecs;

				alGetSourcef(source, AL_SAMPLE_OFFSET, &newOffsetSamples);
				newOffsetSecs = newOffsetSamples / freq;

				offsetSamples += (curOffsetSamples - newOffsetSamples);
				offsetSeconds += (curOffsetSecs - newOffsetSecs);

				streamAtomic(buffer, decoder);
				alSourceQueueBuffers(source, 1, &buffer);
			}
			return true;
		}
		return false;
	}

	void Source::setPitch(float pitch)
	{
		if(valid)
			alSourcef(source, AL_PITCH, pitch);

		this->pitch = pitch;
	}

	float Source::getPitch() const
	{
		if(valid)
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
		if(valid)
		{
			alSourcef(source, AL_GAIN, volume);
		}

		this->volume = volume;
	}

	float Source::getVolume() const
	{
		if(valid)
		{
			ALfloat f;
			alGetSourcef(source, AL_GAIN, &f);
			return f;
		}

		// In case the Source isn't playing.
		return volume;
	}
	
	void Source::seekAtomic(float offset, void * unit)
	{
		if (valid)
		{
			switch (*((Source::Unit*) unit)) {
				case Source::UNIT_SAMPLES:
					if (type == TYPE_STREAM) {
						offsetSamples = offset;
						ALint buffer;
						alGetSourcei(source, AL_BUFFER, &buffer);
						int freq;
						alGetBufferi(buffer, AL_FREQUENCY, &freq);
						offset /= freq;
						offsetSeconds = offset;
						decoder->seek(offset);
					} else {
						alSourcef(source, AL_SAMPLE_OFFSET, offset);
					}
					break;
				case Source::UNIT_SECONDS:	
				default:
					if (type == TYPE_STREAM) {
						offsetSeconds = offset;
						decoder->seek(offset);
						ALint buffer;
						alGetSourcei(source, AL_BUFFER, &buffer);
						int freq;
						alGetBufferi(buffer, AL_FREQUENCY, &freq);
						offsetSamples = offset*freq;
					} else {
						alSourcef(source, AL_SEC_OFFSET, offset);
					}
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
	
	float Source::tellAtomic(void * unit) const
	{
		if (valid)
		{
			float offset;
			switch (*((Source::Unit*) unit)) {
				case Source::UNIT_SAMPLES:
					alGetSourcef(source, AL_SAMPLE_OFFSET, &offset);
					if (type == TYPE_STREAM) offset += offsetSamples;
					break;
				case Source::UNIT_SECONDS:
				default:
					alGetSourcef(source, AL_SAMPLE_OFFSET, &offset);
					ALint buffer;
					alGetSourcei(source, AL_BUFFER, &buffer);
					int freq;
					alGetBufferi(buffer, AL_FREQUENCY, &freq);
					offset /= freq;
					if (type == TYPE_STREAM) offset += offsetSeconds;
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

	void Source::setPosition(float * v)
	{
		if(valid)
			alSourcefv(source, AL_POSITION, v);

		setFloatv(position, v);
	}

	void Source::getPosition(float * v) const
	{
		if(valid)
			alGetSourcefv(source, AL_POSITION, v);
		else
			setFloatv(v, position);
	}

	void Source::setVelocity(float * v)
	{
		if(valid)
			alSourcefv(source, AL_VELOCITY, v);

		setFloatv(velocity, v);
	}

	void Source::getVelocity(float * v) const
	{
		if(valid)
			alGetSourcefv(source, AL_VELOCITY, v);
		else
			setFloatv(v, velocity);
	}

	void Source::setDirection(float * v)
	{
		if(valid)
			alSourcefv(source, AL_DIRECTION, v);
		else
			setFloatv(direction, v);
	}

	void Source::getDirection(float * v) const
	{
		if(valid)
			alGetSourcefv(source, AL_DIRECTION, v);
		else
			setFloatv(v, direction);
	}

	void Source::setLooping(bool looping)
	{
		if(valid && type == TYPE_STATIC)
			alSourcei(source, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);

		this->looping = looping;
	}

	bool Source::isLooping() const
	{
		return looping;
	}

	void Source::playAtomic()
	{
		if(type == TYPE_STATIC)
		{
			alSourcei(source, AL_BUFFER, buffers[0]);
		}
		else if(type == TYPE_STREAM)
		{
			int usedBuffers = 0;

			for(unsigned int i = 0; i < MAX_BUFFERS; i++)
			{
				streamAtomic(buffers[i], decoder);
				++usedBuffers;
				if(decoder->isFinished())
					break;
			}

			if(usedBuffers > 0)
				alSourceQueueBuffers(source, usedBuffers, buffers);
		}

		// Set these properties. These may have changed while we've
		// been without an AL source.
		alSourcef(source, AL_PITCH, pitch);
		alSourcef(source, AL_GAIN, volume);

		alSourcePlay(source);

		valid = true; //if it fails it will be set to false again
		//but this prevents a horrible, horrible bug
	}

	void Source::stopAtomic()
	{
		if(valid)
		{
			if(type == TYPE_STATIC)
			{
				alSourceStop(source);
			}
			else if(type == TYPE_STREAM)
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
		if(valid)
		{
			alSourcePause(source);
			paused = true;
		}
	}

	void Source::resumeAtomic()
	{
		if(valid && paused)
		{
			alSourcePlay(source);
			paused = false;
		}
	}

	void Source::rewindAtomic()
	{
		if(valid && type == TYPE_STATIC)
		{
			alSourceRewind(source);
			if (!paused)
				alSourcePlay(source);
		}
		else if(valid && type == TYPE_STREAM)
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

	void Source::reset(ALenum source)
	{
		alSourcefv(source, AL_POSITION, position);
		alSourcefv(source, AL_VELOCITY, velocity);
		alSourcefv(source, AL_DIRECTION, direction);
		alSourcef(source, AL_PITCH, pitch);
		alSourcef(source, AL_GAIN, volume);
	}

	void Source::setFloatv(float * dst, const float * src) const
	{
		dst[0] = src[0];
		dst[1] = src[1];
		dst[2] = src[2];
	}

	ALenum Source::getFormat(int channels, int bits) const
	{
		if(channels == 1 && bits == 8)
			return AL_FORMAT_MONO8;
		else if(channels == 1 && bits == 16)
			return AL_FORMAT_MONO16;
		else if(channels == 2 && bits == 8)
			return AL_FORMAT_STEREO8;
		else if(channels == 2 && bits == 16)
			return AL_FORMAT_STEREO16;
		else
			return 0;
	}

	int Source::streamAtomic(ALuint buffer, love::sound::Decoder * d)
	{
		// Get more sound data.
		int decoded = d->decode();

		int fmt = getFormat(d->getChannels(), d->getBits());

		if(fmt != 0)
			alBufferData(buffer, fmt, d->getBuffer(), decoded, d->getSampleRate());

		if(decoder->isFinished() && isLooping()) {
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

	bool Source::isStatic() const
	{
		return (type == TYPE_STATIC);
	}

} // openal
} // audio
} // love
