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

#ifndef LOVE_AUDIO_RECORDING_DEVICE_H
#define LOVE_AUDIO_RECORDING_DEVICE_H

#include "common/Object.h"
#include "sound/SoundData.h"

#include <string>

namespace love
{
namespace audio
{

class RecordingDevice : public love::Object
{
public:

	static love::Type type;

	static const int DEFAULT_SAMPLES = 8192;
	static const int DEFAULT_SAMPLE_RATE = 8000;
	static const int DEFAULT_BIT_DEPTH = 16;
	static const int DEFAULT_CHANNELS = 1;

	RecordingDevice();
	virtual ~RecordingDevice();

	/**
	 * Begins audio input recording process.
	 * @param samples Number of samples to buffer.
	 * @param sampleRate Desired sample rate.
	 * @param bitDepth Desired bit depth (8 or 16).
	 * @param channels Desired number of channels. 
	 * @return True if recording started successfully.
	 **/
	virtual bool start(int samples, int sampleRate, int bitDepth, int channels) = 0;

	/** 
	 * Stops audio input recording.
	 **/
	virtual void stop() = 0;

	/**
	 * Retreives recorded data. 
	 * @return SoundData containing data obtained from recording device.
	 **/
	virtual love::sound::SoundData *getData() = 0;

	/**
	 * @return C string device name.
	 **/ 
	virtual const char *getName() const = 0;

	/**
	 * @return Number of samples currently recorded.
	 **/
	virtual int getSampleCount() const = 0;

	/**
	 * Gets the maximum number of samples that will be buffered, as set by start().
	 **/
	virtual int getMaxSamples() const = 0;

	/**
	 * @return Sample rate for recording.
	 **/
	virtual int getSampleRate() const = 0;

	/**
	 * @return Bit depth for recording.
	 **/
	virtual int getBitDepth() const = 0;

	/**
	 * @return Number of channels for recording.
	 **/
	virtual int getChannelCount() const = 0;

	/**
	 * @return True if currently recording.
	 **/
	virtual bool isRecording() const = 0;

}; //RecordingDevice

} //audio
} //love

#endif //LOVE_AUDIO_RECORDING_DEVICE_H
