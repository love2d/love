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

#ifndef LOVE_FILESYSTEM_FILESYSTEM_H
#define LOVE_FILESYSTEM_FILESYSTEM_H

// LOVE
#include "common/config.h"
#include "common/Module.h"
#include "common/int.h"
#include "common/StringMap.h"
#include "FileData.h"
#include "File.h"

// C++
#include <string>
#include <vector>

// In Windows, we would like to use "LOVE" as the
// application folder, but in Linux, we like .love.
#define LOVE_APPDATA_PREFIX ""
#ifdef LOVE_WINDOWS
#	define LOVE_APPDATA_FOLDER "LOVE"
#	define LOVE_PATH_SEPARATOR "/"
#	define LOVE_MAX_PATH _MAX_PATH
#else
#	if defined(LOVE_MACOS) || defined(LOVE_IOS)
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

class Filesystem : public Module
{
public:

	enum FileType
	{
		FILETYPE_FILE,
		FILETYPE_DIRECTORY,
		FILETYPE_SYMLINK,
		FILETYPE_OTHER,
		FILETYPE_MAX_ENUM
	};

	enum CommonPath
	{
		COMMONPATH_APP_SAVEDIR,
		COMMONPATH_APP_DOCUMENTS,
		COMMONPATH_USER_HOME,
		COMMONPATH_USER_APPDATA,
		COMMONPATH_USER_DESKTOP,
		COMMONPATH_USER_DOCUMENTS,
		COMMONPATH_MAX_ENUM
	};

	enum MountPermissions
	{
		MOUNT_PERMISSIONS_READ,
		MOUNT_PERMISSIONS_READWRITE,
		MOUNT_PERMISSIONS_MAX_ENUM
	};

	enum LoadMode
	{
		LOADMODE_BINARY,
		LOADMODE_TEXT,
		LOADMODE_ANY,
		LOADMODE_MAX_ENUM
	};

	struct Info
	{
		// Numbers will be -1 if they cannot be determined.
		int64 size;
		int64 modtime;
		FileType type;
		bool readonly;
	};

	static love::Type type;

	virtual ~Filesystem();

	virtual void init(const char *arg0) = 0;

	virtual void setFused(bool fused) = 0;
	virtual bool isFused() const = 0;

	/**
	 * This sets up the save directory. If the
	 * it is already set up, nothing happens.
	 * @return True on success, false otherwise.
	 **/
	virtual bool setupWriteDirectory() = 0;

	/**
	 * This sets the save location on Android. 
	 * False for internal, true for external
	 * @param external Bool for whether 
	 * Android should use external file storage.
	**/
	virtual void setAndroidSaveExternal(bool useExternal = false);

	/**
	 * Gets whether the Android save is external.
	 * Returns a bool.
	**/
	virtual bool isAndroidSaveExternal() const; 

	/**
	 * Sets the name of the save folder.
	 * @param ident The name of the game. Will be used to
	 * to create the folder in the LOVE data folder.
	 **/
	virtual bool setIdentity(const char *ident, bool appendToPath = false) = 0;
	virtual const char *getIdentity() const = 0;

	/**
	 * Sets the path to the game source.
	 * This can only be set once.
	 * @param source Path to a directory or a .love-file.
	 **/
	virtual bool setSource(const char *source) = 0;

	/**
	 * Gets the path to the game source.
	 * Returns a 0-length string if the source has not been set.
	 **/
	virtual const char *getSource() const = 0;

	virtual bool mount(const char *archive, const char *mountpoint, bool appendToPath = false) = 0;
	virtual bool mount(Data *data, const char *archivename, const char *mountpoint, bool appendToPath = false) = 0;

	virtual bool mountFullPath(const char *archive, const char *mountpoint, MountPermissions permissions, bool appendToPath = false) = 0;
	virtual bool mountCommonPath(CommonPath path, const char *mountpoint, MountPermissions permissions, bool appendToPath = false) = 0;

	virtual bool unmount(const char *archive) = 0;
	virtual bool unmount(Data *data) = 0;
	virtual bool unmount(CommonPath path) = 0;
	virtual bool unmountFullPath(const char *fullpath) = 0;

	/**
	 * Opens a new File object from the specified path, using the given mode.
	 **/
	virtual File *openFile(const char *filename, File::Mode mode) const = 0;

	/**
	 * Creates a new FileData object. Data will be copied.
	 * @param data Pointer to the data.
	 * @param size The size of the data.
	 * @param filename The full filename used to file type identification.
	 **/
	virtual FileData *newFileData(const void *data, size_t size, const char *filename) const;

	/**
	 * Gets the full path for the given common path.
	 */
	virtual std::string getFullCommonPath(CommonPath path) = 0;

	/**
	 * Gets the current working directory.
	 **/
	virtual const char *getWorkingDirectory() = 0;

	/**
	 * Gets the user home directory.
	 **/
	virtual std::string getUserDirectory() = 0;

	/**
	 * Gets the APPDATA directory. On Windows, this is the folder
	 * in the %APPDATA% enviroment variable. On Linux, this is the
	 * user home folder.
	 **/
	virtual std::string getAppdataDirectory() = 0;

	/**
	 * Gets the full path of the save folder.
	 **/
	virtual std::string getSaveDirectory() = 0;

	/**
	 * Gets the full path to the directory containing the game source.
	 * For example if the game source is C:\Games\mygame.love, this will return
	 * C:\Games.
	 **/
	virtual std::string getSourceBaseDirectory() const = 0;

	/**
	 * Gets the real directory path containing the file.
	 **/
	virtual std::string getRealDirectory(const char *filename) const = 0;

	/**
	 * Gets whether anything exists at the specified path.
	 **/
	virtual bool exists(const char *filepath) const = 0;

	/**
	 * Gets information about the item at the specified filepath. Returns false
	 * if nothing exists at the path.
	 **/
	virtual bool getInfo(const char *filepath, Info &info) const = 0;

	/**
	 * Creates a directory. Write dir must be set.
	 * @param dir The directory to create.
	 **/
	virtual bool createDirectory(const char *dir) = 0;

	/**
	 * Removes a file (or directory).
	 * @param file The file or directory to remove.
	 **/
	virtual bool remove(const char *file) = 0;

	/**
	 * Reads data from a file.
	 * @param filename The name of the file to read from.
	 * @param size The size in bytes of the data to read.
	 **/
	virtual FileData *read(const char *filename, int64 size) const = 0;
	virtual FileData *read(const char *filename) const = 0;

	/**
	 * Write data to a file.
	 * @param filename The name of the file to write to.
	 * @param data The data to write.
	 * @param size The size in bytes of the data to write.
	 **/
	virtual void write(const char *filename, const void *data, int64 size) const = 0;

	/**
	 * Append data to a file, creating it if it doesn't exist.
	 * @param filename The name of the file to write to.
	 * @param data The data to append.
	 * @param size The size in bytes of the data to append.
	 **/
	virtual void append(const char *filename, const void *data, int64 size) const = 0;

	/**
	 * This "native" method returns a table of all
	 * files in a given directory.
	 **/
	virtual bool getDirectoryItems(const char *dir, std::vector<std::string> &items) = 0;

	/**
	 * Enable or disable symbolic link support in love.filesystem.
	 **/
	virtual void setSymlinksEnabled(bool enable) = 0;

	/**
	 * Gets whether symbolic link support is enabled.
	 **/
	virtual bool areSymlinksEnabled() const = 0;

	// Require path accessors
	// Not const because it's R/W
	virtual std::vector<std::string> &getRequirePath() = 0;
	virtual std::vector<std::string> &getCRequirePath() = 0;

	/**
	 * Allows a full (OS-dependent) path to be used with Filesystem::mount.
	 **/
	virtual void allowMountingForPath(const std::string &path) = 0;

	/**
	 * Gets whether the given full (OS-dependent) path is a directory.
	 **/
	virtual bool isRealDirectory(const std::string &path) const;

	/**
	 * Recursively creates a directory at the given full OS-dependent path.
	 **/
	virtual bool createRealDirectory(const std::string &path);

	/**
	 * Converts the given real path to its canonical version (e.g. resolving
	 * '..', '.', relative paths, etc).
	 **/
	virtual std::string canonicalizeRealPath(const std::string &path) const;

	/**
	 * Gets the full platform-dependent path to the executable.
	 **/
	virtual std::string getExecutablePath() const;

	STRINGMAP_CLASS_DECLARE(FileType);
	STRINGMAP_CLASS_DECLARE(CommonPath);
	STRINGMAP_CLASS_DECLARE(MountPermissions);
	STRINGMAP_CLASS_DECLARE(LoadMode);

protected:

	Filesystem(const char *name);

private:

	bool getRealPathType(const std::string &path, FileType &ftype) const;

	// Should we save external or internal for Android
	bool useExternal;

}; // Filesystem

} // filesystem
} // love

#endif // LOVE_FILESYSTEM_FILESYSTEM_H
