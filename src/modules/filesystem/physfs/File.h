/**
 * Copyright (c) 2006-2023 LOVE Development Team
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
#include "common/config.h"
#include "filesystem/File.h"

// PhysFS
#include "libraries/physfs/physfs.h"

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
public:

	/**
	 * Constructs an File with the given ilename.
	 * @param filename The relative filepath of the file to load.
	 **/
	File(const std::string &filename);

	virtual ~File();

	// Implements love::filesystem::File.
	using love::filesystem::File::read;
	using love::filesystem::File::write;
	bool open(Mode mode) override;
	bool close() override;
	bool isOpen() const override;
	int64 getSize() override;
	virtual int64 read(void *dst, int64 size) override;
	bool write(const void *data, int64 size) override;
	bool flush() override;
	bool isEOF() override;
	int64 tell() override;
	bool seek(uint64 pos) override;
	bool setBuffer(BufferMode bufmode, int64 size) override;
	BufferMode getBuffer(int64 &size) const override;
	Mode getMode() const override;
	const std::string &getFilename() const override;

private:

	// filename
	std::string filename;

	// PHYSFS File handle.
	PHYSFS_File *file;

	// The current mode of the file.
	Mode mode;

	BufferMode bufferMode;
	int64 bufferSize;

}; // File

} // physfs
} // filesystem
} // love

#endif // LOVE_FILESYSTEM_PHYSFS_FILE_H
