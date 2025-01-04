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
#include "Filesystem.h"
#include "NativeFile.h"
#include "common/utf8.h"

// Assume POSIX or Visual Studio.
#include <sys/types.h>
#include <sys/stat.h>

#if defined(LOVE_MACOS) || defined(LOVE_IOS)
#include "common/apple.h"
#include <unistd.h>
#elif defined(LOVE_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fileapi.h>
#include "common/utf8.h"
#elif defined(LOVE_LINUX)
#include <unistd.h>
#endif

// C++17 std::filesystem
#include <filesystem>

namespace love
{
namespace filesystem
{

love::Type Filesystem::type("filesystem", &Module::type);

Filesystem::Filesystem(const char *name)
	: Module(M_FILESYSTEM, name)
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

File *Filesystem::openNativeFile(const char *path, File::Mode mode) const
{
	return new NativeFile(path, mode);
}

FileData *Filesystem::newFileData(const void *data, size_t size, const char *filename) const
{
	FileData *fd = new FileData(size, std::string(filename));
	memcpy(fd->getData(), data, size);
	return fd;
}

bool Filesystem::isRealDirectory(const std::string &path) const
{
	FileType ftype = FILETYPE_MAX_ENUM;
	if (!getRealPathType(path, ftype))
		return false;
	return ftype == FILETYPE_DIRECTORY;
}

bool Filesystem::getRealPathType(const std::string &path, FileType &ftype) const
{
#ifdef LOVE_WINDOWS
	// make sure non-ASCII paths work.
	std::wstring wpath = to_widestr(path);

	struct _stat buf;
	if (_wstat(wpath.c_str(), &buf) != 0)
		return false;

	if ((buf.st_mode & _S_IFREG) == _S_IFREG)
		ftype = FILETYPE_FILE;
	else if ((buf.st_mode & _S_IFDIR) == _S_IFDIR)
		ftype = FILETYPE_DIRECTORY;
	else
		ftype = FILETYPE_OTHER;
#else
	// Assume POSIX support...
	struct stat buf;
	if (stat(path.c_str(), &buf) != 0)
		return false;

	if (S_ISREG(buf.st_mode))
		ftype = FILETYPE_FILE;
	else if (S_ISDIR(buf.st_mode))
		ftype = FILETYPE_DIRECTORY;
	else if (S_ISLNK(buf.st_mode))
		ftype = FILETYPE_SYMLINK;
	else
		ftype = FILETYPE_OTHER;
#endif

	return true;
}

static bool getContainingDirectory(const std::string &path, std::string &newpath)
{
	size_t index = path.find_last_of("/\\");

	if (index == std::string::npos)
		return false;

	newpath = path.substr(0, index);

	// Bail if the root has been stripped out.
	return newpath.find_first_of("/\\") != std::string::npos;
}

static bool createDirectoryRaw(const std::string &path)
{
#ifdef LOVE_WINDOWS
	std::wstring wpath = to_widestr(path);
	return CreateDirectoryW(wpath.c_str(), nullptr) != 0;
#else
	int mode = S_IRWXU;

#ifdef LOVE_ANDROID
	// Need to create save directory with ugo+rwx and setgid bit if
	// t.externalstorage is set and it's for save directory.
	auto fs = Module::getInstance<Filesystem>(Module::M_FILESYSTEM);
	if (fs != nullptr && fs->isAndroidSaveExternal())
	{
		const std::string &savedir = fs->getFullCommonPath(Filesystem::COMMONPATH_APP_SAVEDIR);
		if (path.rfind(savedir, 0) == 0)
			mode |= S_IRWXG | S_IRWXO | S_ISGID;
	}
#endif

	return mkdir(path.c_str(), mode) == 0;
#endif
}

bool Filesystem::createRealDirectory(const std::string &path)
{
	FileType ftype = FILETYPE_MAX_ENUM;
	if (getRealPathType(path, ftype))
		return ftype == FILETYPE_DIRECTORY;

	std::vector<std::string> createpaths = {path};

	// Find the deepest subdirectory in the given path that actually exists.
	while (true)
	{
		std::string subpath;
		if (!getContainingDirectory(createpaths[0], subpath))
			break;

		if (isRealDirectory(subpath))
			break;

		createpaths.insert(createpaths.begin(), subpath);
	}

	// Try to create missing subdirectories starting from that existing one.
	for (const std::string &p : createpaths)
	{
		if (!createDirectoryRaw(p))
			return false;
	}

	return true;
}

std::string Filesystem::canonicalizeRealPath(const std::string &p) const
{
	try
	{
		return std::filesystem::weakly_canonical(p).string();
	}
	catch (std::exception &)
	{
		return p;
	}
}

std::string Filesystem::getExecutablePath() const
{
#if defined(LOVE_MACOS) || defined(LOVE_IOS)
	return love::apple::getExecutablePath();
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

STRINGMAP_CLASS_BEGIN(Filesystem, Filesystem::FileType, Filesystem::FILETYPE_MAX_ENUM, fileType)
{
	{ "file",      Filesystem::FILETYPE_FILE      },
	{ "directory", Filesystem::FILETYPE_DIRECTORY },
	{ "symlink",   Filesystem::FILETYPE_SYMLINK   },
	{ "other",     Filesystem::FILETYPE_OTHER     },
}
STRINGMAP_CLASS_END(Filesystem, Filesystem::FileType, Filesystem::FILETYPE_MAX_ENUM, fileType)

STRINGMAP_CLASS_BEGIN(Filesystem, Filesystem::CommonPath, Filesystem::COMMONPATH_MAX_ENUM, commonPath)
{
	{ "appsavedir",    Filesystem::COMMONPATH_APP_SAVEDIR    },
	{ "appdocuments",  Filesystem::COMMONPATH_APP_DOCUMENTS  },
	{ "userhome",      Filesystem::COMMONPATH_USER_HOME      },
	{ "userappdata",   Filesystem::COMMONPATH_USER_APPDATA   },
	{ "userdesktop",   Filesystem::COMMONPATH_USER_DESKTOP   },
	{ "userdocuments", Filesystem::COMMONPATH_USER_DOCUMENTS },
}
STRINGMAP_CLASS_END(Filesystem, Filesystem::CommonPath, Filesystem::COMMONPATH_MAX_ENUM, commonPath)

STRINGMAP_CLASS_BEGIN(Filesystem, Filesystem::MountPermissions, Filesystem::MOUNT_PERMISSIONS_MAX_ENUM, mountPermissions)
{
	{ "read",      Filesystem::MOUNT_PERMISSIONS_READ      },
	{ "readwrite", Filesystem::MOUNT_PERMISSIONS_READWRITE },
}
STRINGMAP_CLASS_END(Filesystem, Filesystem::MountPermissions, Filesystem::MOUNT_PERMISSIONS_MAX_ENUM, mountPermissions)

STRINGMAP_CLASS_BEGIN(Filesystem, Filesystem::LoadMode, Filesystem::LOADMODE_MAX_ENUM, loadMode)
{
	{ "b",  Filesystem::LOADMODE_BINARY},
	{ "t",  Filesystem::LOADMODE_TEXT  },
	{ "bt", Filesystem::LOADMODE_ANY   }
}
STRINGMAP_CLASS_END(Filesystem, Filesystem::LoadMode, Filesystem::LOADMODE_MAX_ENUM, loadMode)

} // filesystem
} // love
