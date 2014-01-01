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

#ifndef LOVE_FILESYSTEM_PHYSFS_FILESYSTEM_H
#define LOVE_FILESYSTEM_PHYSFS_FILESYSTEM_H

// STD
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

// LOVE
#include "common/Module.h"
#include "common/config.h"
#include "common/int.h"
#include "filesystem/FileData.h"
#include "File.h"

// For great CWD. (Current Working Directory)
// Using this instead of boost::filesystem which totally
// cramped our style.
#ifdef LOVE_WINDOWS
#	include <windows.h>
#	include <direct.h>
#else
#	include <sys/param.h>
#	include <unistd.h>
#endif

// In Windows, we would like to use "LOVE" as the
// application folder, but in Linux, we like .love.
#define LOVE_APPDATA_PREFIX ""
#ifdef LOVE_WINDOWS
#	define LOVE_APPDATA_FOLDER "LOVE"
#	define LOVE_PATH_SEPARATOR "/"
#	define LOVE_MAX_PATH _MAX_PATH
#else
#	ifdef LOVE_MACOSX
#		define LOVE_APPDATA_FOLDER "LOVE"
#	elif defined(LOVE_LINUX)
#		define LOVE_APPDATA_FOLDER "love"
#	else
#		define LOVE_APPDATA_PREFIX "."
#		define LOVE_APPDATA_FOLDER "love"
#	endif
#	define LOVE_PATH_SEPARATOR "/"
#	define LOVE_MAX_PATH MAXPATHLEN
#endif

namespace love
{
namespace filesystem
{
namespace physfs
{

class Filesystem : public Module
{
public:

	Filesystem();
	virtual ~Filesystem();

	const char *getName() const;

	void init(const char *arg0);

	void setFused(bool fused);
	bool isFused() const;

	/**
	 * This sets up the save directory. If the
	 * it is already set up, nothing happens.
	 * @return True on success, false otherwise.
	 **/
	bool setupWriteDirectory();

	/**
	 * Sets the name of the save folder.
	 * @param ident The name of the game. Will be used to
	 * to create the folder in the LOVE data folder.
	 **/
	bool setIdentity(const char *ident, bool appendToPath = false);
	const char *getIdentity() const;

	/**
	 * Sets the path to the game source.
	 * This can only be set once.
	 * @param source Path to a directory or a .love-file.
	 **/
	bool setSource(const char *source);

	/**
	 * Gets the path to the game source.
	 * Returns a 0-length string if the source has not been set.
	 **/
	const char *getSource() const;

	bool mount(const char *archive, const char *mountpoint, bool appendToPath = false);
	bool unmount(const char *archive);

	/**
	 * Creates a new file.
	 **/
	File *newFile(const char *filename) const;

	/**
	 * Creates a new FileData object. Data will be copied.
	 * @param data Pointer to the data.
	 * @param size The size of the data.
	 * @param filename The full filename used to file type identification.
	 **/
	FileData *newFileData(void *data, unsigned int size, const char *filename) const;

	/**
	 * Creates a new FileData object from base64 data.
	 * @param b64 The base64 data.
	 **/
	FileData *newFileData(const char *b64, const char *filename) const;

	/**
	 * Gets the current working directory.
	 **/
	const char *getWorkingDirectory();

	/**
	 * Gets the user home directory.
	 **/
	const char *getUserDirectory();

	/**
	 * Gets the APPDATA directory. On Windows, this is the folder
	 * in the %APPDATA% enviroment variable. On Linux, this is the
	 * user home folder.
	 **/
	const char *getAppdataDirectory();

	/**
	 * Gets the full path of the save folder.
	 **/
	const char *getSaveDirectory();

	/**
	 * Gets the full path to the directory containing the game source.
	 * For example if the game source is C:\Games\mygame.love, this will return
	 * C:\Games.
	 **/
	std::string getSourceBaseDirectory() const;

	/**
	 * Checks whether a file exists in the current search path
	 * or not.
	 * @param file The filename to check.
	 **/
	bool exists(const char *file) const;

	/**
	 * Checks if an existing file really is a directory.
	 * @param file The filename to check.
	 **/
	bool isDirectory(const char *file) const;

	/**
	 * Checks if an existing file really is a file,
	 * and not a directory.
	 * @param file The filename to check.
	 **/
	bool isFile(const char *file) const;

	/**
	 * Creates a directory. Write dir must be set.
	 * @param dir The directory to create.
	 **/
	bool createDirectory(const char *dir);

	/**
	 * Removes a file (or directory).
	 * @param file The file or directory to remove.
	 **/
	bool remove(const char *file);

	/**
	 * Reads data from a file.
	 * @param filename The name of the file to read from.
	 * @param size The size in bytes of the data to read.
	 **/
	Data *read(const char *filename, int64 size = File::ALL) const;

	/**
	 * Write data to a file.
	 * @param filename The name of the file to write to.
	 * @param data The data to write.
	 * @param size The size in bytes of the data to write.
	 **/
	void write(const char *filename, const void *data, int64 size) const;

	/**
	 * Append data to a file, creating it if it doesn't exist.
	 * @param filename The name of the file to write to.
	 * @param data The data to append.
	 * @param size The size in bytes of the data to append.
	 **/
	void append(const char *filename, const void *data, int64 size) const;

	/**
	 * This "native" method returns a table of all
	 * files in a given directory.
	 **/
	int getDirectoryItems(lua_State *L);

	/**
	 * Gets the last modification time of a file, in seconds
	 * since the Unix epoch.
	 * @param filename The name of the file.
	 **/
	int64 getLastModified(const char *filename) const;

	/**
	 * Gets the size of a file in bytes.
	 * @param filename The name of the file.
	 **/
	int64 getSize(const char *filename) const;

	/**
	 * Text file line-reading iterator function used and
	 * pushed on the Lua stack by love.filesystem.lines
	 * and File:lines.
	 **/
	static int lines_i(lua_State *L);

private:

	// Contains the current working directory (UTF8).
	std::string cwd;

	// %APPDATA% on Windows.
	std::string appdata;

	// This name will be used to create the folder
	// in the appdata/userdata folder.
	std::string save_identity;

	// Full and relative paths of the game save folder.
	// (Relative to the %APPDATA% folder, meaning that the
	// relative string will look something like: ./LOVE/game)
	std::string save_path_relative, save_path_full;

	// The full path to the source of the game.
	std::string game_source;

	// Workaround for machines without PhysFS 2.0
	bool initialized;

	// Allow saving outside of the LOVE_APPDATA_FOLDER
	// for release 'builds'
	bool fused;
	bool fusedSet;

}; // Filesystem

} // physfs
} // filesystem
} // love

#endif // LOVE_FILESYSTEM_PHYSFS_FILESYSTEM_H
