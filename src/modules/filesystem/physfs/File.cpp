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

#include "File.h"

// STD
#include <cstring>

// LOVE
#include "Filesystem.h"
#include "filesystem/FileData.h"

#ifdef LOVE_ANDROID
#include "common/android.h"
#endif

namespace love
{
namespace filesystem
{

namespace physfs
{

static bool setupWriteDirectory()
{
	auto fs = Module::getInstance<love::filesystem::Filesystem>(Module::M_FILESYSTEM);
	return fs != nullptr && fs->setupWriteDirectory();
}

File::File(const std::string &filename, Mode mode)
	: filename(filename)
	, file(nullptr)
	, mode(MODE_CLOSED)
	, bufferMode(BUFFER_NONE)
	, bufferSize(0)
{
	if (!open(mode))
		throw love::Exception("Could not open file at path %s", filename.c_str());

#ifdef LOVE_ANDROID
	// In Android with t.externalstorage = true, make sure the file opened or
	// created in the save directory has permissions of ugo+rw (0666) so that
	// it's accessible through MTP.
	auto fs = Module::getInstance<love::filesystem::Filesystem>(Module::M_FILESYSTEM);
	if (fs != nullptr && fs->isAndroidSaveExternal())
	{
		const char *realdir = PHYSFS_getRealDir(filename.c_str());
		const std::string &savedir = fs->getFullCommonPath(Filesystem::COMMONPATH_APP_SAVEDIR);

		if (realdir != nullptr && strcmp(realdir, savedir.c_str()) == 0)
			love::android::fixupPermissionSingleFile(savedir, filename);
	}
#endif
}

File::File(const File &other)
	: filename(other.filename)
	, file(nullptr)
	, mode(MODE_CLOSED)
	, bufferMode(other.bufferMode)
	, bufferSize(other.bufferSize)
{
	if (!open(other.mode))
		throw love::Exception("Could not open file at path %s", filename.c_str());
}

File::~File()
{
	if (mode != MODE_CLOSED)
		close();
}

File *File::clone()
{
	return new File(*this);
}

bool File::open(Mode mode)
{
	if (mode == MODE_CLOSED)
	{
		close();
		return true;
	}

	if (!PHYSFS_isInit())
		throw love::Exception("PhysFS is not initialized.");

	// File must exist if read mode.
	if ((mode == MODE_READ) && !PHYSFS_exists(filename.c_str()))
		throw love::Exception("Could not open file %s. Does not exist.", filename.c_str());

	// Check whether the write directory is set.
	if ((mode == MODE_APPEND || mode == MODE_WRITE) && !setupWriteDirectory())
		throw love::Exception("Could not set write directory.");

	// File already open?
	if (file != nullptr)
		return false;

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

bool File::isEOF()
{
	return file == nullptr || PHYSFS_eof(file);
}

int64 File::tell()
{
	if (file == nullptr)
		return -1;

	return (int64) PHYSFS_tell(file);
}

bool File::seek(int64 pos, SeekOrigin origin)
{
	if (file != nullptr)
	{
		if (origin == SEEKORIGIN_CURRENT)
			pos += tell();
		else if (origin == SEEKORIGIN_END)
			pos += getSize();
	}

	if (pos < 0)
		return false;

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
