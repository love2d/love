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
 * 1. The origin of this software must not be misrepresented = 0; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

#include "RecordingDevice.h"
#include "Audio.h"

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
		: Exception("Recording %d channels with %d bits per sample is not supported.", channels, bitdepth)
	{
	}

};

RecordingDevice::RecordingDevice(const char *name, int id) 
	: name(name)
	, id(id)
{
}

RecordingDevice::~RecordingDevice()
{
	if (!isRecording())
		return;

	alcCaptureStop(device);
	alcCaptureCloseDevice(device);
}

bool RecordingDevice::startRecording()
{
	return startRecording(samples, sampleRate, bitDepth, channels);
}

bool RecordingDevice::startRecording(int samples, int sampleRate, int bitDepth, int channels)
{
	if (isRecording())
	{
		alcCaptureStop(device);
		alcCaptureCloseDevice(device);
	}

	ALenum format = Audio::getFormat(bitDepth, channels);
	if (format == AL_NONE)
		throw InvalidFormatException(channels, bitDepth);

	if (samples <= 0)
		throw love::Exception("Invalid number of samples.");

	if (sampleRate <= 0)
		throw love::Exception("Invalid sample rate.");

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

void RecordingDevice::stopRecording()
{
	if (!isRecording())
		return;

	alcCaptureStop(device);
	alcCaptureCloseDevice(device);
	device = nullptr;
}

int RecordingDevice::getData(love::sound::SoundData *soundData)
{
	if (!isRecording())
		return 0;

	int samples = getSampleCount();
	if (samples == 0)
		return 0;

	//reinitialize soundData if necessary
	if (samples != soundData->getSampleCount() || sampleRate != soundData->getSampleRate() ||
		bitDepth != soundData->getBitDepth() || channels != soundData->getChannels())
		soundData->load(samples, sampleRate, bitDepth, channels);

	alcCaptureSamples(device, soundData->getData(), samples);

	return samples;
}

int RecordingDevice::getSampleCount() const
{
	if (!isRecording())
		return 0;

	ALCint samples;
	alcGetIntegerv(device, ALC_CAPTURE_SAMPLES, sizeof(ALCint), &samples);
	return (int)samples;
}

int RecordingDevice::getSampleRate() const
{
	return sampleRate;
}

int RecordingDevice::getBitDepth() const
{
	return bitDepth;
}

int RecordingDevice::getChannels() const
{
	return channels;
}

const char *RecordingDevice::getName() const
{
	return name.c_str();
}

int RecordingDevice::getID() const
{
	return id;
}

bool RecordingDevice::isRecording() const
{
	return device != nullptr;
}

} //openal
} //audio
} //love
