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

#include <string>

// OpenAL
#ifdef LOVE_APPLE_USE_FRAMEWORKS
#ifdef LOVE_IOS
#include <OpenAL/alc.h>
#include <OpenAL/al.h>
#include <OpenAL/oalMacOSX_OALExtensions.h>
#else
#include <OpenAL-Soft/alc.h>
#include <OpenAL-Soft/al.h>
#include <OpenAL-Soft/alext.h>
#endif
#else
#include <AL/alc.h>
#include <AL/al.h>
#include <AL/alext.h>
#endif

// LOVE
#include "thread/threads.h"

#ifndef ALC_SOFT_reopen_device
typedef ALCboolean (ALC_APIENTRY*LPALCREOPENDEVICESOFT)(ALCdevice *device,
	const ALCchar *deviceName, const ALCint *attribs);
#endif

namespace love
{
namespace audio
{
namespace openal
{

class DeviceWatcher: public thread::Threadable
{
public:
	DeviceWatcher(ALCdevice *device, const std::vector<ALint> &attribs);
	~DeviceWatcher() override;

	void threadFunction() override;

	// Stop device watcher thread.
	void stop();
	// Returns true if replaying a source is necessary.
	bool tryReconnect(const ALCchar *deviceName);

private:
	bool deviceChanged;

	// Boolean to signal enum thread to stop.
	bool stopped;

	// Current device and attributes.
	ALCdevice *device;
	std::vector<ALint> attribs;

	// Pointer to alcReopenDeviceSOFT function.
	LPALCREOPENDEVICESOFT alcReopenDeviceSOFT;

	// Device name string.
	std::string currentDevice;

	// Mutex to ensure only 1 thread can call update().
	love::thread::MutexRef mutex;

	void update();
	static std::string getFirstDevice();
};

} // openal
} // audio
} // love
