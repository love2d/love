/**
 * Copyright (c) 2006-2022 LOVE Development Team
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

#include <chrono>

// LOVE
#include "common/delay.h"
#include "DeviceWatcher.h"

#ifndef ALC_CONNECTED
#define ALC_CONNECTED 0x313
#endif

#ifndef ALC_ALL_DEVICES_SPECIFIER
#define ALC_ALL_DEVICES_SPECIFIER 0x1013
#endif

namespace love
{
namespace audio
{
namespace openal
{

DeviceWatcher::DeviceWatcher(ALCdevice *device, const std::vector<ALint> &attribs)
: deviceChanged(false)
, stopped(false)
, device(device)
, attribs(attribs)
, alcReopenDeviceSOFT(nullptr)
{
	if (
		alcIsExtensionPresent(nullptr, "ALC_ENUMERATE_ALL_EXT") &&
		alcIsExtensionPresent(device, "ALC_EXT_disconnect") &&
		alcIsExtensionPresent(device, "ALC_SOFT_reopen_device"))
	{
		alcReopenDeviceSOFT = (LPALCREOPENDEVICESOFT) alcGetProcAddress(nullptr, "alcReopenDeviceSOFT");
		currentDevice = getFirstDevice();
	}
}

DeviceWatcher::~DeviceWatcher()
{
}

void DeviceWatcher::stop()
{
	stopped = true;
}

bool DeviceWatcher::tryReconnect(const ALCchar* deviceName)
{
	if (!alcReopenDeviceSOFT)
		return false;

	ALint connected;
	alcGetIntegerv(device, ALC_CONNECTED, 1, &connected);

	if (!connected)
		// Force update
		update();

	if (deviceChanged || !connected)
	{
		if (!alcReopenDeviceSOFT(device, deviceName, attribs.data()))
			// Note: We can't simply throw exception here.
			fprintf(stderr, "Unable to reconnect device %p: %s\n", device, alcGetString(device, alcGetError(device)));

		deviceChanged = false;
	}

	return !connected;
}

void DeviceWatcher::update()
{
	thread::Lock lock(mutex);

	// Get first device
	std::string device = getFirstDevice();

	if (device != currentDevice)
	{
		// Mark as changed
		deviceChanged = true;
		currentDevice = device;
	}
}

std::string DeviceWatcher::getFirstDevice()
{
	// Assume first device is default device.
	// We can't use ALC_DEFAULT_ALL_DEVICES_SPECIFIER because
	// it only probes once whilst ALC_ALL_DEVICES_SPECIFIER
	// always probes.
	return std::string((const char *) alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER));
}

void DeviceWatcher::threadFunction()
{
	// Some constants
	constexpr std::chrono::milliseconds STATUS_POLL_INTERVAL {5};
	constexpr std::chrono::milliseconds ENUM_POLL_INTERVAL {1000};

	if (!alcReopenDeviceSOFT)
		return;

	auto currentTime = std::chrono::high_resolution_clock::now();

	while (!stopped)
	{
		auto timeDiff = std::chrono::high_resolution_clock::now() - currentTime;

		if (timeDiff >= ENUM_POLL_INTERVAL)
		{
			update();
			currentTime = std::chrono::high_resolution_clock::now();
		}

		sleep(STATUS_POLL_INTERVAL.count());
	}
}

} // openal
} // audio
} // love
