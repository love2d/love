/**
* Copyright (c) 2006-2010 LOVE Development Team
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

#include <sound/Decoder.h>

namespace love
{
namespace audio
{
namespace openal
{
	Audio::Audio()
		: finish(false)
	{
		// Passing zero for default device.
		device = alcOpenDevice(0);

		if(device == 0)
			throw love::Exception("Could not open device.");

		context = alcCreateContext(device, 0);

		if(context == 0)
			throw love::Exception("Could not create context.");

		alcMakeContextCurrent(context);

		if(alcGetError(device) != ALC_NO_ERROR)
			throw love::Exception("Could not make context current.");
		
		/*std::string captureName(alcGetString(NULL, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER));
		const ALCchar * devices = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
		while (*devices) {
			std::string device(devices);
			devices += device.size() + 1;
			if (device.find("Mic") != std::string::npos || device.find("mic") != std::string::npos) {
				captureName = device;
			}
		}
		
		capture = alcCaptureOpenDevice(captureName.c_str(), 8000, AL_FORMAT_MONO16, 262144); // about 32 seconds
		
		if (!capture) {
			// We're not going to prevent LOVE from running without a microphone, but we should warn, at least
			std::cerr << "Warning, couldn't open capture device! No audio input!" << std::endl;
		}*/

		// pool must be allocated after AL context.
		pool = new Pool();

		thread = SDL_CreateThread(Audio::run, (void*)this);
	}

	Audio::~Audio()
	{
		finish = true;

		SDL_WaitThread(thread, 0);

		delete pool;
		
		alcMakeContextCurrent(0);
		alcDestroyContext(context);
		//if (capture) alcCaptureCloseDevice(capture);
		alcCloseDevice(device);
	}

	int Audio::run(void * d)
	{
		Audio * instance = (Audio*)d;
		
		while(!instance->finish)
		{
			instance->pool->update();
			SDL_Delay(5);
		}

		return 0;
	}

	const char * Audio::getName() const
	{
		return "love.audio.openal";
	}

	love::audio::Source * Audio::newSource(love::sound::Decoder * decoder)
	{
		return new Source(pool, decoder);
	}

	love::audio::Source * Audio::newSource(love::sound::SoundData * soundData)
	{
		return new Source(pool, soundData);
	}

	int Audio::getNumSources() const
	{
		return pool->getNumSources();
	}

	int Audio::getMaxSources() const
	{
		return pool->getMaxSources();
	}

	void Audio::play(love::audio::Source * source)
	{
		source->play();
	}

	void Audio::stop(love::audio::Source * source)
	{
		source->stop();
	}

	void Audio::stop()
	{
		pool->stop();
	}

	void Audio::pause(love::audio::Source * source)
	{
		source->pause();
	}

	void Audio::pause()
	{
		pool->pause();
	}

	void Audio::resume(love::audio::Source * source)
	{
		source->resume();
	}
	
	void Audio::resume()
	{
		pool->resume();
	}

	void Audio::rewind(love::audio::Source * source)
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

	void Audio::getPosition(float * v) const
	{
		alGetListenerfv(AL_POSITION, v);
	}

	void Audio::setPosition(float * v)
	{
		alListenerfv(AL_POSITION, v);
	}

	void Audio::getOrientation(float * v) const
	{
		alGetListenerfv(AL_ORIENTATION, v);
	}

	void Audio::setOrientation(float * v)
	{
		alListenerfv(AL_ORIENTATION, v);
	}

	void Audio::getVelocity(float * v) const
	{
		alGetListenerfv(AL_VELOCITY, v);
	}

	void Audio::setVelocity(float * v)
	{
		alListenerfv(AL_VELOCITY, v);
	}
	
	void Audio::record()
	{
		if (!canRecord()) return;
		alcCaptureStart(capture);
	}
	
	love::sound::SoundData * Audio::getRecordedData()
	{
		if (!canRecord()) return NULL;
		int samplerate = 8000;
		ALCint samples;
		alcGetIntegerv(capture, ALC_CAPTURE_SAMPLES, 4, &samples);
		void * data = malloc(samples * (2/sizeof(char)));
		alcCaptureSamples(capture, data, samples);
		love::sound::SoundData * sd = new love::sound::SoundData(data, samples, samplerate, 16, 1);
		free(data);
		return sd;
	}
	
	love::sound::SoundData * Audio::stopRecording(bool returnData)
	{
		if (!canRecord()) return NULL;
		love::sound::SoundData * sd = NULL;
		if (returnData) {
			sd = getRecordedData();
		}
		alcCaptureStop(capture);
		return sd;
	}
	
	bool Audio::canRecord()
	{
		return (capture != NULL);
	}

} // openal
} // audio
} // love
