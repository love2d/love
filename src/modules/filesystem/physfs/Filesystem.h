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

#ifndef LOVE_FILESYSTEM_PHYSFS_FILESYSTEM_H
#define LOVE_FILESYSTEM_PHYSFS_FILESYSTEM_H

// STD
#include <cstdlib>
#include <cstring>

// LOVE
#include "filesystem/Filesystem.h"

namespace love
{
namespace filesystem
{
namespace physfs
{

class Filesystem : public love::filesystem::Filesystem
{
public:

	Filesystem();
	virtual ~Filesystem();

	// Implements Module.
	const char *getName() const;

	void init(const char *arg0);

	void setFused(bool fused);
	bool isFused() const;

	bool setupWriteDirectory();

	bool setIdentity(const char *ident, bool appendToPath = false);
	const char *getIdentity() const;

	bool setSource(const char *source);

	const char *getSource() const;

	bool mount(const char *archive, const char *mountpoint, bool appendToPath = false);
	bool unmount(const char *archive);

	File *newFile(const char *filename) const;

	FileData *newFileData(void *data, unsigned int size, const char *filename) const;
	FileData *newFileData(const char *b64, const char *filename) const;

	const char *getWorkingDirectory();
	std::string getUserDirectory();
	std::string getAppdataDirectory();
	const char *getSaveDirectory();
	std::string getSourceBaseDirectory() const;

	std::string getRealDirectory(const char *filename) const;

	bool exists(const char *path) const;
	bool isDirectory(const char *dir) const;
	bool isFile(const char *file) const;
	bool isSymlink(const char *filename) const;

	bool createDirectory(const char *dir);

	bool remove(const char *file);

	FileData *read(const char *filename, int64 size = File::ALL) const;
	void write(const char *filename, const void *data, int64 size) const;
	void append(const char *filename, const void *data, int64 size) const;

	void getDirectoryItems(const char *dir, std::vector<std::string> &items);

	int64 getLastModified(const char *filename) const;
	int64 getSize(const char *filename) const;

	void setSymlinksEnabled(bool enable);
	bool areSymlinksEnabled() const;

	std::vector<std::string> &getRequirePath();

	void allowMountingForPath(const std::string &path);

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

	// Allow saving outside of the LOVE_APPDATA_FOLDER
	// for release 'builds'
	bool fused;
	bool fusedSet;

	// Search path for require
	std::vector<std::string> requirePath;

	std::vector<std::string> allowedMountPaths;

}; // Filesystem

} // physfs
} // filesystem
} // love

#endif // LOVE_FILESYSTEM_PHYSFS_FILESYSTEM_H
