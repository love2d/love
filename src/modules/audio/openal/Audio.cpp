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

#include "sound/Decoder.h"

#include <cstdlib>

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

Audio::Audio()
	: device(nullptr)
	, capture(nullptr)
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

	/*std::string captureName(alcGetString(NULL, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER));
	const ALCchar * devices = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
	while (*devices)
	{
		std::string device(devices);
		devices += device.size() + 1;
		if (device.find("Mic") != std::string::npos || device.find("mic") != std::string::npos)
		{
			captureName = device;
		}
	}

	capture = alcCaptureOpenDevice(captureName.c_str(), 8000, AL_FORMAT_MONO16, 262144); // about 32 seconds

	if (!capture)
	{
		// We're not going to prevent LOVE from running without a microphone, but we should warn, at least
		std::cerr << "Warning, couldn't open capture device! No audio input!" << std::endl;
	}*/

	// pool must be allocated after AL context.
	try
	{
		pool = new Pool();
	}
	catch (love::Exception &)
	{
		alcMakeContextCurrent(nullptr);
		alcDestroyContext(context);
		//if (capture) alcCaptureCloseDevice(capture);
		alcCloseDevice(device);
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
	//if (capture) alcCaptureCloseDevice(capture);
	alcCloseDevice(device);
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

void Audio::stop(love::audio::Source *source)
{
	source->stop();
}

void Audio::stop()
{
	pool->stop();
}

void Audio::pause(love::audio::Source *source)
{
	source->pause();
}

void Audio::pause()
{
	pool->pause();
#ifdef LOVE_ANDROID
	alcDevicePauseSOFT(device);
#endif
}

void Audio::resume(love::audio::Source *source)
{
	source->resume();
}

void Audio::resume()
{
#ifdef LOVE_ANDROID
	alcDeviceResumeSOFT(device);
#endif
	pool->resume();
}

void Audio::rewind(love::audio::Source *source)
{
	source->rewind();
}

void Audio::rewind()
{
	pool->rewind();
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

void Audio::record()
{
	if (!canRecord()) return;
	alcCaptureStart(capture);
}

love::sound::SoundData *Audio::getRecordedData()
{
	if (!canRecord())
		return NULL;
	int samplerate = 8000;
	ALCint samples;
	alcGetIntegerv(capture, ALC_CAPTURE_SAMPLES, 4, &samples);
	void *data = malloc(samples * (2/sizeof(char)));
	alcCaptureSamples(capture, data, samples);
	love::sound::SoundData *sd = new love::sound::SoundData(data, samples, samplerate, 16, 1);
	free(data);
	return sd;
}

love::sound::SoundData *Audio::stopRecording(bool returnData)
{
	if (!canRecord())
		return NULL;
	love::sound::SoundData *sd = NULL;
	if (returnData)
	{
		sd = getRecordedData();
	}
	alcCaptureStop(capture);
	return sd;
}

bool Audio::canRecord()
{
	return (capture != NULL);
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

} // openal
} // audio
} // love
