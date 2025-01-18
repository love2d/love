/**
 * Copyright (c) 2006-2024 LOVE Development Team
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

// LOVE
#include "common/config.h"
#include "System.h"

#if defined(LOVE_IOS)
#include "common/ios.h"
#elif defined(LOVE_ANDROID)
#include "common/android.h"
#endif

namespace love
{
namespace system
{

System::System(const char *name)
	: Module(M_SYSTEM, name)
{
}

const char *System::getOS()
{
#if defined(LOVE_MACOS)
	return "OS X";
#elif defined(LOVE_IOS)
	return "iOS";
#elif defined(LOVE_WINDOWS_UWP)
	return "UWP";
#elif defined(LOVE_WINDOWS)
	return "Windows";
#elif defined(LOVE_ANDROID)
	return "Android";
#elif defined(LOVE_LINUX)
	return "Linux";
#else
	return "Unknown";
#endif
}

void System::vibrate(double seconds) const
{
#ifdef LOVE_ANDROID
	love::android::vibrate(seconds);
#elif defined(LOVE_IOS)
	love::ios::vibrate();
#else
	LOVE_UNUSED(seconds);
#endif
}

bool System::hasBackgroundMusic() const
{
#if defined(LOVE_ANDROID)
	return love::android::hasBackgroundMusic();
#elif defined(LOVE_IOS)
	return love::ios::hasBackgroundMusic();
#else
	return false;
#endif
}

bool System::getConstant(const char *in, System::PowerState &out)
{
	return powerStates.find(in, out);
}

bool System::getConstant(System::PowerState in, const char *&out)
{
	return powerStates.find(in, out);
}

StringMap<System::PowerState, System::POWER_MAX_ENUM>::Entry System::powerEntries[] =
{
	{"unknown", System::POWER_UNKNOWN},
	{"battery", System::POWER_BATTERY},
	{"nobattery", System::POWER_NO_BATTERY},
	{"charging", System::POWER_CHARGING},
	{"charged", System::POWER_CHARGED},
};

StringMap<System::PowerState, System::POWER_MAX_ENUM> System::powerStates(System::powerEntries, sizeof(System::powerEntries));

} // system
} // love
