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
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

// LOVE
#include "Filesystem.h"
#include "common/utf8.h"

// Assume POSIX or Visual Studio.
#include <sys/types.h>
#include <sys/stat.h>

#if defined(LOVE_MACOSX)
#include "common/macosx.h"
#elif defined(LOVE_IOS)
#include "common/ios.h"
#elif defined(LOVE_WINDOWS)
#include <windows.h>
#include "common/utf8.h"
#elif defined(LOVE_LINUX)
#include <unistd.h>
#endif

namespace love
{
namespace filesystem
{

Filesystem::Filesystem()
{
}

Filesystem::~Filesystem()
{
}

void Filesystem::setAndroidSaveExternal(bool useExternal)
{	
	this->useExternal = useExternal;
}

bool Filesystem::isAndroidSaveExternal() const
{ 
	return useExternal;
}

bool Filesystem::isRealDirectory(const std::string &path) const
{
#ifdef LOVE_WINDOWS
	// make sure non-ASCII paths work.
	std::wstring wpath = to_widestr(path);

	struct _stat buf;
	if (_wstat(wpath.c_str(), &buf) != 0)
		return false;

	return (buf.st_mode & _S_IFDIR) == _S_IFDIR;
#else
	// Assume POSIX support...
	struct stat buf;
	if (stat(path.c_str(), &buf) != 0)
		return false;

	return S_ISDIR(buf.st_mode) != 0;
#endif
}

std::string Filesystem::getExecutablePath() const
{
#if defined(LOVE_MACOSX)
	return love::macosx::getExecutablePath();
#elif defined(LOVE_IOS)
	return love::ios::getExecutablePath();
#elif defined(LOVE_WINDOWS)

	wchar_t buffer[MAX_PATH + 1] = {0};

	if (GetModuleFileNameW(nullptr, buffer, MAX_PATH) == 0)
		return "";

	return to_utf8(buffer);

#elif defined(LOVE_LINUX)

	char buffer[2048] = {0};

	ssize_t len = readlink("/proc/self/exe", buffer, 2048);
	if (len <= 0)
		return "";

	return std::string(buffer, len);

#else
#error Missing implementation for Filesystem::getExecutablePath!
#endif
}

} // filesystem
} // love
