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

// LOVE
#include "common/config.h"
#include "System.h"

#if defined(LOVE_MACOSX)
#include <CoreServices/CoreServices.h>
#elif defined(LOVE_IOS)
#include "common/ios.h"
#elif defined(LOVE_LINUX) || defined(LOVE_ANDROID)
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#elif defined(LOVE_WINDOWS)
#include "common/utf8.h"
#include <shlobj.h>
#include <shellapi.h>
#pragma comment(lib, "shell32.lib")
#endif
#if defined(LOVE_ANDROID)
#include "common/android.h"
#elif defined(LOVE_LINUX)

#ifdef __has_include
#if __has_include(<spawn.h>)
#define LOVE_HAS_POSIX_SPAWN
#endif
#endif

#ifdef LOVE_HAS_POSIX_SPAWN
#include <spawn.h>
#else
#include <unistd.h>
#endif
#endif

namespace love
{
namespace system
{

System::System()
{
}

std::string System::getOS() const
{
#if defined(LOVE_MACOSX)
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

#ifdef LOVE_HAS_POSIX_SPAWN
extern "C"
{
	extern char **environ; // The environment, always available
}
#endif

bool System::openURL(const std::string &url) const
{

#if defined(LOVE_MACOSX)

	bool success = false;
	CFURLRef cfurl = CFURLCreateWithBytes(nullptr,
	                                      (const UInt8 *) url.c_str(),
	                                      url.length(),
	                                      kCFStringEncodingUTF8,
	                                      nullptr);

	success = LSOpenCFURLRef(cfurl, nullptr) == noErr;
	CFRelease(cfurl);
	return success;

#elif defined(LOVE_IOS)

	return love::ios::openURL(url);

#elif defined(LOVE_ANDROID)

	return love::android::openURL(url);

#elif defined(LOVE_LINUX)

	pid_t pid;
	const char *argv[] = {"xdg-open", url.c_str(), nullptr};

#ifdef LOVE_HAS_POSIX_SPAWN
	// Note: at the moment this process inherits our file descriptors.
	// Note: the below const_cast is really ugly as well.
	if (posix_spawnp(&pid, "xdg-open", nullptr, nullptr, const_cast<char **>(argv), environ) != 0)
		return false;
#else
	pid = fork();
	if (pid == 0)
	{
		execvp("xdg-open", const_cast<char **>(argv));
		return false;
	}
#endif

	// Check if xdg-open already completed (or failed.)
	int status = 0;
	if (waitpid(pid, &status, WNOHANG) > 0)
		return (status == 0);
	else
		// We can't tell what actually happens without waiting for
		// the process to finish, which could take forever (literally).
		return true;

#elif defined(LOVE_WINDOWS)

	// Unicode-aware WinAPI functions don't accept UTF-8, so we need to convert.
	std::wstring wurl = to_widestr(url);

	HINSTANCE result = 0;

#if defined(LOVE_WINDOWS_UWP)
	
	Platform::String^ urlString = ref new Platform::String(wurl.c_str());
	auto uwpUri = ref new Windows::Foundation::Uri(urlString);
	Windows::System::Launcher::LaunchUriAsync(uwpUri);

#else

	result = ShellExecuteW(nullptr,
		L"open",
		wurl.c_str(),
		nullptr,
		nullptr,
		SW_SHOW);

#endif

	return (ptrdiff_t) result > 32;

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
