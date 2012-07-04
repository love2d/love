/**
 * Copyright (c) 2006-2012 LOVE Development Team
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

#ifndef LOVE_FILESYSTEM_PHYSFS_FILE_H
#define LOVE_FILESYSTEM_PHYSFS_FILE_H

// LOVE
#include "filesystem/File.h"

// PhysFS
#ifdef LOVE_MACOSX // wacky Mac behavior means different #include syntax!
#include <physfs/physfs.h>
#else
#include <physfs.h>
#endif

// STD
#include <string>

namespace love
{
namespace filesystem
{
namespace physfs
{

class File : public love::filesystem::File
{
private:

	// filename
	std::string filename;

	// PHYSFS File handle.
	PHYSFS_file *file;

	// The current mode of the file.
	Mode mode;

public:

	/**
	 * Constructs an File with the given source and filename.
	 * @param source The source from which to load the file. (Archive or directory)
	 * @param filename The relative filepath of the file to load from the source.
	 **/
	File(std::string filename);

	virtual ~File();

	// Implements love::filesystem::File.
	bool open(Mode mode);
	bool close();
	int64 getSize();
	Data *read(int64 size = ALL);
	int64 read(void *dst, int64 size);
	bool write(const void *data, int64 size);
	bool write(const Data *data, int64 size = ALL);
	bool eof();
	int64 tell();
	bool seek(uint64 pos);
	Mode getMode();
	std::string getFilename() const;
	std::string getExtension() const;

}; // File

} // physfs
} // filesystem
} // love

#endif // LOVE_FILESYSTEM_PHYSFS_FILE_H
