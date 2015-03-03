/**
 * Copyright (c) 2006-2015 LOVE Development Team
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

bool Filesystem::isRealDirectory(const std::string &path) const
{
#ifdef LOVE_WINDOWS
	// make sure non-ASCII paths work.
	struct _stat buf;
	if (_wstat(to_widestr(path).c_str(), &buf) != 0)
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

} // filesystem
} // love
