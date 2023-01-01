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

#include "File.h"

// STD
#include <cstring>

// LOVE
#include "Filesystem.h"
#include "filesystem/FileData.h"

namespace love
{
namespace filesystem
{

extern bool hack_setupWriteDirectory();

namespace physfs
{

File::File(const std::string &filename)
	: filename(filename)
	, file(nullptr)
	, mode(MODE_CLOSED)
	, bufferMode(BUFFER_NONE)
	, bufferSize(0)
{
}

File::~File()
{
	if (mode != MODE_CLOSED)
		close();
}

bool File::open(Mode mode)
{
	if (mode == MODE_CLOSED)
		return true;

	if (!PHYSFS_isInit())
		throw love::Exception("PhysFS is not initialized.");

	// File must exist if read mode.
	if ((mode == MODE_READ) && !PHYSFS_exists(filename.c_str()))
		throw love::Exception("Could not open file %s. Does not exist.", filename.c_str());

	// Check whether the write directory is set.
	if ((mode == MODE_APPEND || mode == MODE_WRITE) && (PHYSFS_getWriteDir() == nullptr) && !hack_setupWriteDirectory())
		throw love::Exception("Could not set write directory.");

	// File already open?
	if (file != nullptr)
		return false;

	PHYSFS_getLastErrorCode();
	PHYSFS_File *handle = nullptr;

	switch (mode)
	{
	case MODE_READ:
		handle = PHYSFS_openRead(filename.c_str());
		break;
	case MODE_APPEND:
		handle = PHYSFS_openAppend(filename.c_str());
		break;
	case MODE_WRITE:
		handle = PHYSFS_openWrite(filename.c_str());
		break;
	default:
		break;
	}

	if (handle == nullptr)
	{
		const char *err = PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
		if (err == nullptr)
			err = "unknown error";
		throw love::Exception("Could not open file %s (%s)", filename.c_str(), err);
	}

	file = handle;

	this->mode = mode;

	if (file != nullptr && !setBuffer(bufferMode, bufferSize))
	{
		// Revert to buffer defaults if we don't successfully set the buffer.
		bufferMode = BUFFER_NONE;
		bufferSize = 0;
	}

	return (file != nullptr);
}

bool File::close()
{
	if (file == nullptr || !PHYSFS_close(file))
		return false;

	mode = MODE_CLOSED;
	file = nullptr;

	return true;
}

bool File::isOpen() const
{
	return mode != MODE_CLOSED && file != nullptr;
}

int64 File::getSize()
{
	// If the file is closed, open it to
	// check the size.
	if (file == nullptr)
	{
		open(MODE_READ);
		int64 size = (int64) PHYSFS_fileLength(file);
		close();
		return size;
	}

	return (int64) PHYSFS_fileLength(file);
}

int64 File::read(void *dst, int64 size)
{
	if (!file || mode != MODE_READ)
		throw love::Exception("File is not opened for reading.");

	int64 max = (int64)PHYSFS_fileLength(file);
	size = (size == ALL) ? max : size;
	size = (size > max) ? max : size;

	if (size < 0)
		throw love::Exception("Invalid read size.");

	return PHYSFS_readBytes(file, dst, (PHYSFS_uint64) size);
}

bool File::write(const void *data, int64 size)
{
	if (!file || (mode != MODE_WRITE && mode != MODE_APPEND))
		throw love::Exception("File is not opened for writing.");

	if (size < 0)
		throw love::Exception("Invalid write size.");

	// Try to write.
	int64 written = PHYSFS_writeBytes(file, data, (PHYSFS_uint64) size);

	// Check that correct amount of data was written.
	if (written != size)
		return false;

	// Manually flush the buffer in BUFFER_LINE mode if we find a newline.
	if (bufferMode == BUFFER_LINE && bufferSize > size)
	{
		if (memchr(data, '\n', (size_t) size) != nullptr)
			flush();
	}

	return true;
}

bool File::flush()
{
	if (!file || (mode != MODE_WRITE && mode != MODE_APPEND))
		throw love::Exception("File is not opened for writing.");

	return PHYSFS_flush(file) != 0;
}

#ifdef LOVE_WINDOWS
// MSVC doesn't like the 'this' keyword
// well, we'll use 'that'.
// It zigs, we zag.
inline bool test_eof(File *that, PHYSFS_File *)
{
	int64 pos = that->tell();
	int64 size = that->getSize();
	return pos == -1 || size == -1 || pos >= size;
}
#else
inline bool test_eof(File *, PHYSFS_File *file)
{
	return PHYSFS_eof(file);
}
#endif

bool File::isEOF()
{
	return file == nullptr || test_eof(this, file);
}

int64 File::tell()
{
	if (file == nullptr)
		return -1;

	return (int64) PHYSFS_tell(file);
}

bool File::seek(uint64 pos)
{
	return file != nullptr && PHYSFS_seek(file, (PHYSFS_uint64) pos) != 0;
}

bool File::setBuffer(BufferMode bufmode, int64 size)
{
	if (size < 0)
		return false;

	// If the file isn't open, we'll make sure the buffer values are set in
	// File::open.
	if (!isOpen())
	{
		bufferMode = bufmode;
		bufferSize = size;
		return true;
	}

	int ret = 1;

	switch (bufmode)
	{
	case BUFFER_NONE:
	default:
		ret = PHYSFS_setBuffer(file, 0);
		size = 0;
		break;
	case BUFFER_LINE:
	case BUFFER_FULL:
		ret = PHYSFS_setBuffer(file, size);
		break;
	}

	if (ret == 0)
		return false;

	bufferMode = bufmode;
	bufferSize = size;

	return true;
}

File::BufferMode File::getBuffer(int64 &size) const
{
	size = bufferSize;
	return bufferMode;
}

const std::string &File::getFilename() const
{
	return filename;
}

filesystem::File::Mode File::getMode() const
{
	return mode;
}

} // physfs
} // filesystem
} // love
