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

#include "RecordingDevice.h"
#include "Audio.h"
#include "sound/Sound.h"

namespace love
{
namespace audio
{
namespace openal
{

#define soundInstance() (Module::getInstance<love::sound::Sound>(Module::M_SOUND))

class InvalidFormatException : public love::Exception
{
public:

	InvalidFormatException(int channels, int bitdepth)
		: Exception("Recording %d channels with %d bits per sample is not supported.", channels, bitdepth)
	{
	}

};

RecordingDevice::RecordingDevice(const char *name) 
	: name(name)
{
}

RecordingDevice::~RecordingDevice()
{
	stop();
}

bool RecordingDevice::start(int samples, int sampleRate, int bitDepth, int channels)
{
	ALenum format = Audio::getFormat(bitDepth, channels);
	if (format == AL_NONE)
		throw InvalidFormatException(channels, bitDepth);

	if (samples <= 0)
		throw love::Exception("Invalid number of samples.");

	if (sampleRate <= 0)
		throw love::Exception("Invalid sample rate.");

	if (isRecording())
		stop();

	device = alcCaptureOpenDevice(name.c_str(), sampleRate, format, samples);
	if (device == nullptr)
		return false;

	alcCaptureStart(device);

	this->samples = samples;
	this->sampleRate = sampleRate;
	this->bitDepth = bitDepth;
	this->channels = channels;

	return true;
}

void RecordingDevice::stop()
{
	if (!isRecording())
		return;

	alcCaptureStop(device);
	alcCaptureCloseDevice(device);
	device = nullptr;
}

love::sound::SoundData *RecordingDevice::getData()
{
	if (!isRecording())
		return nullptr;

	int samples = getSampleCount();
	if (samples == 0)
		return nullptr;

	love::sound::SoundData *soundData = soundInstance()->newSoundData(samples, sampleRate, bitDepth, channels);

	alcCaptureSamples(device, soundData->getData(), samples);

	return soundData;
}

int RecordingDevice::getSampleCount() const
{
	if (!isRecording())
		return 0;

	ALCint samples;
	alcGetIntegerv(device, ALC_CAPTURE_SAMPLES, sizeof(ALCint), &samples);
	return (int)samples;
}

int RecordingDevice::getMaxSamples() const
{
	return samples;
}

int RecordingDevice::getSampleRate() const
{
	return sampleRate;
}

int RecordingDevice::getBitDepth() const
{
	return bitDepth;
}

int RecordingDevice::getChannelCount() const
{
	return channels;
}

const char *RecordingDevice::getName() const
{
	return name.c_str();
}

bool RecordingDevice::isRecording() const
{
	return device != nullptr;
}

} //openal
} //audio
} //love
