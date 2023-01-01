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

#ifndef LOVE_AUDIO_OPENAL_RECORDING_DEVICE_H
#define LOVE_AUDIO_OPENAL_RECORDING_DEVICE_H

#include "common/config.h"

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

#include "audio/RecordingDevice.h"
#include "sound/SoundData.h"

namespace love
{
namespace audio
{
namespace openal
{

class RecordingDevice : public love::audio::RecordingDevice
{
public:

	RecordingDevice(const char *name);
	virtual ~RecordingDevice();
	virtual bool start(int samples, int sampleRate, int bitDepth, int channels);
	virtual void stop();
	virtual love::sound::SoundData *getData();
	virtual const char *getName() const;
	virtual int getSampleCount() const;
	virtual int getMaxSamples() const;
	virtual int getSampleRate() const;
	virtual int getBitDepth() const;
	virtual int getChannelCount() const;
	virtual bool isRecording() const;

private:

	int samples = DEFAULT_SAMPLES;
	int sampleRate = DEFAULT_SAMPLE_RATE;
	int bitDepth = DEFAULT_BIT_DEPTH;
	int channels = DEFAULT_CHANNELS;

	std::string name;
	ALCdevice *device = nullptr;

}; //RecordingDevice

} //openal
} //audio
} //love

#endif //LOVE_AUDIO_OPENAL_RECORDING_DEVICE_H
