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

#include "Audio.h"
#include "common/delay.h"
#include "RecordingDevice.h"
#include "sound/Decoder.h"

#include <cstdlib>
#include <iostream>

namespace love
{
namespace audio
{
namespace openal
{

Audio::PoolThread::PoolThread(Pool *pool)
	: pool(pool)
	, finish(false)
{
	threadName = "AudioPool";
}

Audio::PoolThread::~PoolThread()
{
}


void Audio::PoolThread::threadFunction()
{
	while (true)
	{
		{
			thread::Lock lock(mutex);
			if (finish)
			{
				return;
			}
		}

		pool->update();
		sleep(5);
	}
}

void Audio::PoolThread::setFinish()
{
	thread::Lock lock(mutex);
	finish = true;
}

ALenum Audio::getFormat(int bitDepth, int channels)
{
	if (bitDepth != 8 && bitDepth != 16)
		return AL_NONE;

	if (channels == 1)
		return bitDepth == 8 ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
	else if (channels == 2)
		return bitDepth == 8 ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;
	#ifdef AL_EXT_MCFORMATS
	else if (alIsExtensionPresent("AL_EXT_MCFORMATS"))
	{
		if (channels == 6)
			return bitDepth == 8 ? AL_FORMAT_51CHN8 : AL_FORMAT_51CHN16;
		else if (channels == 8)
			return bitDepth == 8 ? AL_FORMAT_71CHN8 : AL_FORMAT_71CHN16;
	}
	#endif
	return AL_NONE;
}

Audio::Audio()
	: device(nullptr)
	, context(nullptr)
	, pool(nullptr)
	, poolThread(nullptr)
	, distanceModel(DISTANCE_INVERSE_CLAMPED)
{
	// Passing null for default device.
	device = alcOpenDevice(nullptr);

	if (device == nullptr)
		throw love::Exception("Could not open device.");

	context = alcCreateContext(device, nullptr);

	if (context == nullptr)
		throw love::Exception("Could not create context.");

	if (!alcMakeContextCurrent(context) || alcGetError(device) != ALC_NO_ERROR)
		throw love::Exception("Could not make context current.");

	//find and push default capture device
	//AL may not return actual device name string when asked directly
	std::string defaultname;
	ALCdevice *defaultdevice = alcCaptureOpenDevice(NULL, 8000, 8, 1);
	if (alGetError() == AL_NO_ERROR)
	{
		defaultname = alcGetString(defaultdevice, ALC_CAPTURE_DEVICE_SPECIFIER);
		alcCaptureCloseDevice(defaultdevice);
		capture.push_back(new RecordingDevice(defaultname.c_str(), 0));
	}

	const ALCchar *devstr = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
	size_t offset = 0;
	while (true)
	{
		if (devstr[offset] == '\0')
			break;
		std::string str((ALCchar*)&devstr[offset]);
		if (str != defaultname)
			capture.push_back(new RecordingDevice(str.c_str(), capture.size()));
		offset += str.length() + 1;
	}

	// pool must be allocated after AL context.
	try
	{
		pool = new Pool();
	}
	catch (love::Exception &)
	{
		alcMakeContextCurrent(nullptr);
		alcDestroyContext(context);
		alcCloseDevice(device);
		for (unsigned int i = 0; i < capture.size(); i++)
			delete capture[i];

		throw;
	}

	poolThread = new PoolThread(pool);
	poolThread->start();
}

Audio::~Audio()
{
	poolThread->setFinish();
	poolThread->wait();

	delete poolThread;
	delete pool;

	alcMakeContextCurrent(nullptr);
	alcDestroyContext(context);
	alcCloseDevice(device);
	for (unsigned int i = 0; i < capture.size(); i++)
		delete capture[i];
}


const char *Audio::getName() const
{
	return "love.audio.openal";
}

love::audio::Source *Audio::newSource(love::sound::Decoder *decoder)
{
	return new Source(pool, decoder);
}

love::audio::Source *Audio::newSource(love::sound::SoundData *soundData)
{
	return new Source(pool, soundData);
}

love::audio::Source *Audio::newSource(int sampleRate, int bitDepth, int channels)
{
	return new Source(pool, sampleRate, bitDepth, channels);
}

int Audio::getSourceCount() const
{
	return pool->getSourceCount();
}

int Audio::getMaxSources() const
{
	return pool->getMaxSources();
}

bool Audio::play(love::audio::Source *source)
{
	return source->play();
}

bool Audio::play(const std::vector<love::audio::Source*> &sources)
{
	return pool->play(sources);
}

void Audio::stop(love::audio::Source *source)
{
	source->stop();
}

void Audio::stop(const std::vector<love::audio::Source*> &sources)
{
	return pool->stop(sources);
}

void Audio::stop()
{
	pool->stop();
}

void Audio::pause(love::audio::Source *source)
{
	source->pause();
}

void Audio::pause(const std::vector<love::audio::Source*> &sources)
{
	return pool->pause(sources);
}

std::vector<love::audio::Source*> Audio::pause()
{
	return pool->pause();
}

void Audio::setVolume(float volume)
{
	alListenerf(AL_GAIN, volume);
}

float Audio::getVolume() const
{
	ALfloat volume;
	alGetListenerf(AL_GAIN, &volume);
	return volume;
}

void Audio::getPosition(float *v) const
{
	alGetListenerfv(AL_POSITION, v);
}

void Audio::setPosition(float *v)
{
	alListenerfv(AL_POSITION, v);
}

void Audio::getOrientation(float *v) const
{
	alGetListenerfv(AL_ORIENTATION, v);
}

void Audio::setOrientation(float *v)
{
	alListenerfv(AL_ORIENTATION, v);
}

void Audio::getVelocity(float *v) const
{
	alGetListenerfv(AL_VELOCITY, v);
}

void Audio::setVelocity(float *v)
{
	alListenerfv(AL_VELOCITY, v);
}

void Audio::setDopplerScale(float scale)
{
	if (scale >= 0.0f)
		alDopplerFactor(scale);
}

float Audio::getDopplerScale() const
{
	return alGetFloat(AL_DOPPLER_FACTOR);
}

Audio::DistanceModel Audio::getDistanceModel() const
{
	return distanceModel;
}

void Audio::setDistanceModel(DistanceModel distanceModel)
{
	this->distanceModel = distanceModel;

	switch (distanceModel)
	{
	case DISTANCE_NONE:
		alDistanceModel(AL_NONE);
		break;

	case DISTANCE_INVERSE:
		alDistanceModel(AL_INVERSE_DISTANCE);
		break;

	case DISTANCE_INVERSE_CLAMPED:
		alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
		break;

	case DISTANCE_LINEAR:
		alDistanceModel(AL_LINEAR_DISTANCE);
		break;

	case DISTANCE_LINEAR_CLAMPED:
		alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
		break;

	case DISTANCE_EXPONENT:
		alDistanceModel(AL_EXPONENT_DISTANCE);
		break;

	case DISTANCE_EXPONENT_CLAMPED:
		alDistanceModel(AL_EXPONENT_DISTANCE_CLAMPED);
		break;

	default:
		break;
	}
}

int Audio::getRecordingDeviceCount() const
{
	return capture.size();
}

love::audio::RecordingDevice *Audio::getRecordingDevice(int index) const
{
	if (index < 0 || (unsigned int)index >= capture.size())
		return nullptr;

	return capture[index];
}

int Audio::getRecordingDeviceIndex(love::audio::RecordingDevice *device) const
{
	for (unsigned int i = 0; i < capture.size(); i++)
		if (device == capture[i])
			return i;

	return -1;
}

} // openal
} // audio
} // love
