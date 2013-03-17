/**
 * Copyright (c) 2006-2013 LOVE Development Team
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

#include "common/config.h"

#include <iostream>

#include "common/utf8.h"
#include "common/b64.h"

#include "Filesystem.h"

namespace love
{
namespace filesystem
{
namespace physfs
{

Filesystem::Filesystem()
	: open_count(0)
	, buffer(0)
	, isInited(false)
	, release(false)
	, releaseSet(false)
{
}

Filesystem::~Filesystem()
{
	if (isInited)
	{
		isInited = false;
		PHYSFS_deinit();
	}
}

const char *Filesystem::getName() const
{
	return "love.filesystem.physfs";
}

void Filesystem::init(const char *arg0)
{
	if (!PHYSFS_init(arg0))
		throw Exception(PHYSFS_getLastError());
	isInited = true;
}

void Filesystem::setRelease(bool release)
{
	if (releaseSet)
		return;
	this->release = release;
	releaseSet = true;
}

bool Filesystem::isRelease() const
{
	if (!releaseSet)
		return false;
	return release;
}

bool Filesystem::setIdentity(const char *ident)
{
	if (!isInited)
		return false;

	// Store the save directory.
	save_identity = std::string(ident);

	// Generate the relative path to the game save folder.
	save_path_relative = std::string(LOVE_APPDATA_PREFIX LOVE_APPDATA_FOLDER LOVE_PATH_SEPARATOR) + save_identity;

	// Generate the full path to the game save folder.
	save_path_full = std::string(getAppdataDirectory()) + std::string(LOVE_PATH_SEPARATOR);
	if (release)
		save_path_full += std::string(LOVE_APPDATA_PREFIX) + save_identity;
	else
		save_path_full += save_path_relative;

	// We now have something like:
	// save_identity: game
	// save_path_relative: ./LOVE/game
	// save_path_full: C:\Documents and Settings\user\Application Data/LOVE/game

	// Try to add the save directory to the search path.
	// (No error on fail, it means that the path doesn't exist).
	PHYSFS_addToSearchPath(save_path_full.c_str(), 0);

	return true;
}

const char *Filesystem::getIdentity() const
{
	return save_identity.c_str();
}

bool Filesystem::setSource(const char *source)
{
	if (!isInited)
		return false;

	// Check whether directory is already set.
	if (!game_source.empty())
		return false;

	// Add the directory.
	if (!PHYSFS_addToSearchPath(source, 1))
		return false;

	// Save the game source.
	game_source = std::string(source);

	return true;
}

bool Filesystem::setupWriteDirectory()
{
	if (!isInited)
		return false;

	// These must all be set.
	if (save_identity.empty() || save_path_full.empty() || save_path_relative.empty())
		return false;

	// Set the appdata folder as writable directory.
	// (We must create the save folder before mounting it).
	if (!PHYSFS_setWriteDir(getAppdataDirectory()))
		return false;

	// Create the save folder. (We're now "at" %APPDATA%).
	bool success = false;
	if (release)
		success = mkdir(save_identity.c_str());
	else
		success = mkdir(save_path_relative.c_str());

	if (!success)
	{
		PHYSFS_setWriteDir(0); // Clear the write directory in case of error.
		return false;
	}

	// Set the final write directory.
	if (!PHYSFS_setWriteDir(save_path_full.c_str()))
		return false;

	// Add the directory. (Will not be readded if already present).
	if (!PHYSFS_addToSearchPath(save_path_full.c_str(), 0))
	{
		PHYSFS_setWriteDir(0); // Clear the write directory in case of error.
		return false;
	}

	return true;
}

File *Filesystem::newFile(const char *filename)
{
	return new File(filename);
}

FileData *Filesystem::newFileData(void *data, unsigned int size, const char *filename)
{
	FileData *fd = new FileData(size, std::string(filename));

	// Copy the data into FileData.
	memcpy(fd->getData(), data, size);

	return fd;
}

FileData *Filesystem::newFileData(const char *b64, const char *filename)
{
	int size = strlen(b64);
	int outsize = 0;
	char *dst = b64_decode(b64, size, outsize);
	FileData *fd = new FileData(outsize, std::string(filename));

	// Copy the data into FileData.
	memcpy(fd->getData(), dst, outsize);
	delete [] dst;

	return fd;
}

const char *Filesystem::getWorkingDirectory()
{
	if (cwd.empty())
	{
#ifdef LOVE_WINDOWS

		WCHAR w_cwd[LOVE_MAX_PATH];
		_wgetcwd(w_cwd, LOVE_MAX_PATH);
		cwd = to_utf8(w_cwd);
		replace_char(cwd, '\\', '/');
#else
		char *cwd_char = new char[LOVE_MAX_PATH];

		if (getcwd(cwd_char, LOVE_MAX_PATH))
			cwd = cwd_char; // if getcwd fails, cwd_char (and thus cwd) will still be empty

		delete [] cwd_char;
#endif
	}

	return cwd.c_str();
}

const char *Filesystem::getUserDirectory()
{
	return PHYSFS_getUserDir();
}

const char *Filesystem::getAppdataDirectory()
{
#ifdef LOVE_WINDOWS
	if (appdata.empty())
	{
		wchar_t *w_appdata = _wgetenv(L"APPDATA");
		appdata = to_utf8(w_appdata);
		replace_char(appdata, '\\', '/');
	}
	return appdata.c_str();
#elif defined(LOVE_MACOSX)
	if (appdata.empty())
	{
		std::string udir = getUserDirectory();
		udir.append("/Library/Application Support");
		appdata = udir;
	}
	return appdata.c_str();
#elif defined(LOVE_LINUX)
	if (appdata.empty())
	{
		char *xdgdatahome = getenv("XDG_DATA_HOME");
		if (!xdgdatahome)
			appdata = std::string(getUserDirectory()) + "/.local/share/";
		else
			appdata = xdgdatahome;
	}
	return appdata.c_str();
#else
	return getUserDirectory();
#endif
}


const char *Filesystem::getSaveDirectory()
{
	return save_path_full.c_str();
}

bool Filesystem::exists(const char *file)
{
	if (PHYSFS_exists(file))
		return true;
	return false;
}

bool Filesystem::isDirectory(const char *file)
{
	if (PHYSFS_isDirectory(file))
		return true;
	return false;
}

bool Filesystem::isFile(const char *file)
{
	return exists(file) && !isDirectory(file);
}

bool Filesystem::mkdir(const char *file)
{
	if (PHYSFS_getWriteDir() == 0 && !setupWriteDirectory())
		return false;

	if (!PHYSFS_mkdir(file))
		return false;
	return true;
}

bool Filesystem::remove(const char *file)
{
	if (PHYSFS_getWriteDir() == 0 && !setupWriteDirectory())
		return false;

	if (!PHYSFS_delete(file))
		return false;
	return true;
}

Data *Filesystem::read(const char *filename, int64 size) const
{
	File file(filename);

	file.open(File::READ);

	// close() is called in the File destructor.
	return file.read(size);
}

void Filesystem::write(const char *filename, const void *data, int64 size)
{
	File file(filename);

	file.open(File::WRITE);

	// close() is called in the File destructor.
	if (!file.write(data, size))
		throw love::Exception("Data could not be written.");
}

int Filesystem::enumerate(lua_State *L)
{
	int n = lua_gettop(L);

	if (n != 1)
		return luaL_error(L, "Function requires a single parameter.");

	int type = lua_type(L, 1);

	if (type != LUA_TSTRING)
		return luaL_error(L, "Function requires parameter of type string.");

	const char *dir = lua_tostring(L, 1);
	char **rc = PHYSFS_enumerateFiles(dir);
	char **i;
	int index = 1;

	lua_newtable(L);

	for (i = rc; *i != 0; i++)
	{
		lua_pushinteger(L, index);
		lua_pushstring(L, *i);
		lua_settable(L, -3);
		index++;
	}

	PHYSFS_freeList(rc);

	return 1;
}

int Filesystem::lines_i(lua_State *L)
{
	const int bufsize = 1024;
	char buf[bufsize];
	int linesize = 0;
	bool newline = false;

	File *file = luax_checktype<File>(L, lua_upvalueindex(1), "File", FILESYSTEM_FILE_T);

	// Only accept read mode at this point.
	if (file->getMode() != File::READ)
		return luaL_error(L, "File needs to stay in read mode.");

	int64 pos = file->tell();
	int64 userpos = -1;

	if (lua_isnoneornil(L, lua_upvalueindex(2)) == 0)
	{
		// User may have changed the file position.
		userpos = pos;
		pos = (int64) lua_tonumber(L, lua_upvalueindex(2));
		if (userpos != pos)
			file->seek(pos);
	}

	while (!newline && !file->eof())
	{
		// This 64-bit to 32-bit integer cast should be safe as it never exceeds bufsize.
		int read = (int) file->read(buf, bufsize);
		if (read < 0)
			return luaL_error(L, "Could not read from file.");

		linesize += read;

		for (int i = 0; i < read; i++)
		{
			if (buf[i] == '\n')
			{
				linesize -= read - i;
				newline = true;
				break;
			}
		}
	}

	if (newline || (file->eof() && linesize > 0))
	{
		if (linesize < bufsize)
		{
			// We have the line in the buffer on the stack. No 'new' and 'read' needed.
			lua_pushlstring(L, buf, linesize > 0 && buf[linesize - 1] == '\r' ? linesize - 1 : linesize);
			if (userpos < 0)
				file->seek(pos + linesize + 1);
		}
		else
		{
			char *str;
			try
			{
				str = new char[linesize + 1];
			}
			catch(std::bad_alloc &)
			{
				return luaL_error(L, "Out of memory");
			}
			file->seek(pos);

			// Read the \n anyway and save us a call to seek.
			if (file->read(str, linesize + 1) == -1)
			{
				delete [] str;
				return luaL_error(L, "Could not read from file.");
			}

			lua_pushlstring(L, str, str[linesize - 1] == '\r' ? linesize - 1 : linesize);
			delete [] str;
		}

		if (userpos >= 0)
		{
			// Save new position in upvalue.
			lua_pushnumber(L, (lua_Number)(pos + linesize + 1));
			lua_replace(L, lua_upvalueindex(2));
			file->seek(userpos);
		}

		return 1;
	}

	// EOF reached.
	if (userpos >= 0 && luax_toboolean(L, lua_upvalueindex(3)))
		file->seek(userpos);
	else
		file->close();

	return 0;
}

int Filesystem::load(lua_State *L)
{
	// Need only one arg.
	luax_assert_argc(L, 1, 1);

	// Must be string.
	if (!lua_isstring(L, -1))
		return luaL_error(L, "The argument must be a string.");

	std::string filename = lua_tostring(L, -1);

	// The file must exist.
	if (!exists(filename.c_str()))
		return luaL_error(L, "File %s does not exist.", filename.c_str());

	// Create the file.
	File *file = newFile(filename.c_str());

	// Get the data from the file.
	Data *data = file->read();

	int status = luaL_loadbuffer(L, (const char *)data->getData(), data->getSize(), ("@" + filename).c_str());

	data->release();
	file->release();

	// Load the chunk, but don't run it.
	switch (status)
	{
	case LUA_ERRMEM:
		return luaL_error(L, "Memory allocation error: %s\n", lua_tostring(L, -1));
	case LUA_ERRSYNTAX:
		return luaL_error(L, "Syntax error: %s\n", lua_tostring(L, -1));
	default: // success
		return 1;
	}
}

int Filesystem::getLastModified(lua_State *L)
{
	const char *filename = luaL_checkstring(L, 1);
	PHYSFS_sint64 time = PHYSFS_getLastModTime(filename);
	if (time == -1)
	{
		lua_pushnil(L);
		lua_pushstring(L, "Could not determine file modification date.");
		return 2;
	}
	lua_pushnumber(L, static_cast<lua_Number>(time));
	return 1;
}

int64 Filesystem::getSize(const char *filename)
{
	File file(filename);
	int64 size = file.getSize();
	return size;
}

} // physfs
} // filesystem
} // love
