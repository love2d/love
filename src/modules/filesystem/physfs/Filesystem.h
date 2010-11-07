/**
* Copyright (c) 2006-2010 LOVE Development Team
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
#include <common/Module.h>
#include <common/config.h>
#include <filesystem/FileData.h>
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
#		define LOVE_APPDATA_FOLDER ".love"
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
	private:

		// Counts open files.
		int open_count;

		// Pointer used for file reads.
		char * buffer;

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
		bool isInited;

	protected:

	public:

		Filesystem();

		~Filesystem();

		const char * getName() const;

		void init(const char * arg0);

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
		bool setIdentity(const char * ident);

		/**
		* Sets the path to the game source.
		* This can only be set once.
		* @param source Path to a directory or a .love-file.
		**/
		bool setSource(const char * source);

		/**
		* Creates a new file.
		**/
		File * newFile(const char* filename);

		/**
		* Creates a new FileData object. Data will be copied.
		* @param data Pointer to the data.
		* @param size The size of the data.
		* @param filename The full filename used to file type identification.
		**/
		FileData * newFileData(void * data, int size, const char * filename);

		/**
		* Creates a new FileData object from base64 data.
		* @param b64 The base64 data.
		**/
		FileData * newFileData(const char * b64, const char * filename);

		/**
		* Gets the current working directory.
		**/
		const char * getWorkingDirectory();

		/**
		* Gets the user home directory.
		**/
		const char * getUserDirectory();

		/**
		* Gets the APPDATA directory. On Windows, this is the folder
		* in the %APPDATA% enviroment variable. On Linux, this is the
		* user home folder.
		**/
		const char * getAppdataDirectory();

		/**
		* Gets the full path of the save folder.
		**/
		const char * getSaveDirectory();

		/**
		* Checks whether a file exists in the current search path
		* or not.
		* @param file The filename to check.
		**/
		bool exists(const char * file);

		/**
		* Checks if an existing file really is a directory.
		* @param file The filename to check.
		**/
		bool isDirectory(const char * file);

		/**
		* Checks if an existing file really is a file,
		* and not a directory.
		* @param file The filename to check.
		**/
		bool isFile(const char * file);

		/**
		* Creates a directory. Write dir must be set.
		* @param file The directory to create.
		**/
		bool mkdir(const char * file);

		/**
		* Removes a file (or directory).
		* @param file The file or directory to remove.
		**/
		bool remove(const char * file);

		/**
		* Opens a file for reading or writing. (Depends
		* on the mode chosen at the time of creation).
		* @param file The file to open.
		* @param mode The mode to open the file in.
		**/
		bool open(File * file, File::Mode mode);

		/**
		* Closes a file.
		* @param file The file to close.
		**/
		bool close(File * file);

		/**
		* Reads count bytes from an open file.
		* The first parameter is either a File or
		* a string. An optional second parameter specified the
		* max number of bytes to read.
		**/
		int read(lua_State * L);

		/**
		* Write the bytes in data to the file. File
		* must be opened for write.
		* The first parameter is either a File or
		* a string.
		**/
		int write(lua_State * L);

		/**
		* Check if end-of-file is reached.
		* @return True if EOF, false otherwise.
		**/
		bool eof(File * file);

		/**
		* Gets the current position in a file.
		* @param file An open File.
		**/
		int tell(File * file);

		/**
		* Seek to a position within a file.
		* @param pos The position to seek to.
		**/
		bool seek(File * file, int pos);

		/**
		* This "native" method returns a table of all
		* files in a given directory.
		**/
		int enumerate(lua_State * L);

		/**
		* Returns an iterator which iterates over
		* lines in files.
		**/
		int lines(lua_State * L);

		/**
		* The line iterator function.
		**/
		static int lines_i(lua_State * L);

		/**
		* Loads a file without running it. The loaded
		* chunk is returned as a function.
		* @param filename The filename of the file to load.
		* @return A function.
		**/
		int load(lua_State * L);

	}; // Filesystem

} // physfs
} // filesystem
} // love

#endif // LOVE_FILESYSTEM_PHYSFS_FILESYSTEM_H
