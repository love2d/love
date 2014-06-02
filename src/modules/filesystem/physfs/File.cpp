/**
 * Copyright (c) 2006-2014 LOVE Development Team
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
	, file(0)
	, mode(CLOSED)
	, bufferMode(BUFFER_NONE)
	, bufferSize(0)
{
}

File::~File()
{
	if (mode != CLOSED)
		close();
}

bool File::open(Mode mode)
{
	if (mode == CLOSED)
		return true;

	// File must exist if read mode.
	if ((mode == READ) && !PHYSFS_exists(filename.c_str()))
		throw love::Exception("Could not open file %s. Does not exist.", filename.c_str());

	// Check whether the write directory is set.
	if ((mode == APPEND || mode == WRITE) && (PHYSFS_getWriteDir() == 0) && !hack_setupWriteDirectory())
		throw love::Exception("Could not set write directory.");

	// File already open?
	if (file != 0)
		return false;

	this->mode = mode;

	switch (mode)
	{
	case READ:
		file = PHYSFS_openRead(filename.c_str());
		break;
	case APPEND:
		file = PHYSFS_openAppend(filename.c_str());
		break;
	case WRITE:
		file = PHYSFS_openWrite(filename.c_str());
		break;
	default:
		break;
	}

	if (file != 0 && !setBuffer(bufferMode, bufferSize))
	{
		// Revert to buffer defaults if we don't successfully set the buffer.
		bufferMode = BUFFER_NONE;
		bufferSize = 0;
	}

	return (file != 0);
}

bool File::close()
{
	if (!PHYSFS_close(file))
		return false;
	mode = CLOSED;
	file = 0;
	return true;
}

bool File::isOpen() const
{
	return mode != CLOSED && file != 0;
}

int64 File::getSize()
{
	// If the file is closed, open it to
	// check the size.
	if (file == 0)
	{
		open(READ);
		int64 size = (int64)PHYSFS_fileLength(file);
		close();
		return size;
	}

	return (int64)PHYSFS_fileLength(file);
}


FileData *File::read(int64 size)
{
	bool isOpen = (file != 0);

	if (!isOpen && !open(READ))
		throw love::Exception("Could not read file %s.", filename.c_str());

	int64 max = getSize();
	int64 cur = tell();
	size = (size == ALL) ? max : size;

	if (size < 0)
		throw love::Exception("Invalid read size.");

	// Clamping because the file offset may be in a weird position.
	if (cur < 0)
		cur = 0;
	else if (cur > max)
		cur = max;

	if (cur + size > max)
		size = max - cur;

	FileData *fileData = new FileData(size, getFilename());
	int64 bytesRead = read(fileData->getData(), size);

	if (bytesRead < 0 || (bytesRead == 0 && bytesRead != size))
	{
		delete fileData;
		throw love::Exception("Could not read from file.");
	}
	if (bytesRead < size)
	{
		FileData *tmpFileData = new FileData(bytesRead, getFilename());
		memcpy(tmpFileData->getData(), fileData->getData(), (size_t) bytesRead);
		delete fileData;
		fileData = tmpFileData;
	}

	if (!isOpen)
		close();

	return fileData;
}

int64 File::read(void *dst, int64 size)
{
	if (!file || mode != READ)
		throw love::Exception("File is not opened for reading.");

	int64 max = (int64)PHYSFS_fileLength(file);
	size = (size == ALL) ? max : size;
	size = (size > max) ? max : size;
	// Sadly, we'll have to clamp to 32 bits here
	size = (size > LOVE_UINT32_MAX) ? LOVE_UINT32_MAX : size;

	if (size < 0)
		throw love::Exception("Invalid read size.");

	int64 read = (int64)PHYSFS_read(file, dst, 1, (PHYSFS_uint32) size);

	return read;
}

bool File::write(const void *data, int64 size)
{
	if (!file || (mode != WRITE && mode != APPEND))
		throw love::Exception("File is not opened for writing.");

	// Another clamp, for the time being.
	size = (size > LOVE_UINT32_MAX) ? LOVE_UINT32_MAX : size;

	if (size < 0)
		throw love::Exception("Invalid write size.");

	// Try to write.
	int64 written = static_cast<int64>(PHYSFS_write(file, data, 1, (PHYSFS_uint32) size));

	// Check that correct amount of data was written.
	if (written != size)
		return false;

	// Manually flush the buffer in BUFFER_LINE mode if we find a newline.
	if (bufferMode == BUFFER_LINE && bufferSize > size)
	{
		if (memchr(data, '\n', (size_t) size) != NULL)
			flush();
	}

	return true;
}

bool File::write(const Data *data, int64 size)
{
	return write(data->getData(), (size == ALL) ? data->getSize() : size);
}

bool File::flush()
{
	if (!file || (mode != WRITE && mode != APPEND))
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

bool File::eof()
{
	if (file == 0 || test_eof(this, file))
		return true;
	return false;
}

int64 File::tell()
{
	if (file == 0)
		return -1;

	return (int64) PHYSFS_tell(file);
}

bool File::seek(uint64 pos)
{
	if (file == 0)
		return false;

	if (!PHYSFS_seek(file, (PHYSFS_uint64) pos))
		return false;
	return true;
}

bool File::setBuffer(BufferMode bufmode, int64 size)
{
	// No negativity allowed!
	if (size < 0)
		return false;

	// If the file isn't open, we'll make sure the buffer values are set in
	// File::open.
	if (file == 0 || mode == CLOSED)
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

std::string File::getFilename() const
{
	return filename;
}

std::string File::getExtension() const
{
	std::string::size_type idx = filename.rfind('.');

	if (idx != std::string::npos)
		return filename.substr(idx+1);
	else
		return std::string();
}

filesystem::File::Mode File::getMode() const
{
	return mode;
}

} // physfs
} // filesystem
} // love
