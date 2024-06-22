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

#ifndef LOVE_FILESYSTEM_PHYSFS_FILESYSTEM_H
#define LOVE_FILESYSTEM_PHYSFS_FILESYSTEM_H

// STD
#include <cstdlib>
#include <cstring>
#include <map>

// LOVE
#include "filesystem/Filesystem.h"

namespace love
{
namespace filesystem
{
namespace physfs
{

class Filesystem final : public love::filesystem::Filesystem
{
public:

	Filesystem();
	virtual ~Filesystem();

	void init(const char *arg0) override;

	void setFused(bool fused) override;
	bool isFused() const override;

	bool setupWriteDirectory() override;

	bool setIdentity(const char *ident, bool appendToPath = false) override;
	const char *getIdentity() const override;

	bool setSource(const char *source) override;

	const char *getSource() const override;

	bool mount(const char *archive, const char *mountpoint, bool appendToPath = false) override;
	bool mount(Data *data, const char *archivename, const char *mountpoint, bool appendToPath = false) override;

	bool mountFullPath(const char *archive, const char *mountpoint, MountPermissions permissions, bool appendToPath = false) override;
	bool mountCommonPath(CommonPath path, const char *mountpoint, MountPermissions permissions, bool appendToPath = false) override;

	bool unmount(const char *archive) override;
	bool unmount(Data *data) override;
	bool unmount(CommonPath path) override;
	bool unmountFullPath(const char *fullpath) override;

	love::filesystem::File *openFile(const char *filename, File::Mode mode) const override;

	std::string getFullCommonPath(CommonPath path) override;
	const char *getWorkingDirectory() override;
	std::string getUserDirectory() override;
	std::string getAppdataDirectory() override;
	std::string getSaveDirectory() override;
	std::string getSourceBaseDirectory() const override;

	std::string getRealDirectory(const char *filename) const override;

	bool exists(const char *filepath) const override;
	bool getInfo(const char *filepath, Info &info) const override;

	bool createDirectory(const char *dir) override;

	bool remove(const char *file) override;

	FileData *read(const char *filename, int64 size) const override;
	FileData *read(const char *filename) const override;
	void write(const char *filename, const void *data, int64 size) const override;
	void append(const char *filename, const void *data, int64 size) const override;

	bool getDirectoryItems(const char *dir, std::vector<std::string> &items) override;

	void setSymlinksEnabled(bool enable) override;
	bool areSymlinksEnabled() const override;

	std::vector<std::string> &getRequirePath() override;
	std::vector<std::string> &getCRequirePath() override;

	void allowMountingForPath(const std::string &path) override;

private:

	struct CommonPathMountInfo
	{
		bool mounted;
		std::string mountPoint;
		MountPermissions permissions;
	};

	bool mountCommonPathInternal(CommonPath path, const char *mountpoint, MountPermissions permissions, bool appendToPath, bool createDir);

	// Contains the current working directory (UTF8).
	std::string cwd;

	// This name will be used to create the folder in the appdata folder.
	std::string saveIdentity;
	bool appendIdentityToPath;

	// The full path to the source of the game.
	std::string gameSource;

	// Allow saving outside of the LOVE_APPDATA_FOLDER for release 'builds'
	bool fused;
	bool fusedSet;

	// Search path for require
	std::vector<std::string> requirePath;
	std::vector<std::string> cRequirePath;

	std::vector<std::string> allowedMountPaths;

	std::map<std::string, StrongRef<Data>> mountedData;

	std::string fullPaths[COMMONPATH_MAX_ENUM];

	CommonPathMountInfo commonPathMountInfo[COMMONPATH_MAX_ENUM];

	bool saveDirectoryNeedsMounting;

}; // Filesystem

} // physfs
} // filesystem
} // love

#endif // LOVE_FILESYSTEM_PHYSFS_FILESYSTEM_H
