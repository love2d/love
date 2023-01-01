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

namespace love
{
namespace audio
{
namespace null
{

const char *RecordingDevice::name = "null";

RecordingDevice::RecordingDevice(const char *)
{
}

RecordingDevice::~RecordingDevice()
{
}

bool RecordingDevice::start(int, int, int, int)
{
	return false;
}

void RecordingDevice::stop()
{
}

love::sound::SoundData *RecordingDevice::getData()
{
	return nullptr;
}

int RecordingDevice::getSampleCount() const
{
	return 0;
}

int RecordingDevice::getMaxSamples() const
{
	return 0;
}

int RecordingDevice::getSampleRate() const
{
	return 0;
}

int RecordingDevice::getBitDepth() const
{
	return 0;
}

int RecordingDevice::getChannelCount() const
{
	return 0;
}

const char *RecordingDevice::getName() const
{
	return name;
}

bool RecordingDevice::isRecording() const
{
	return false;
}

} //null
} //audio
} //love
