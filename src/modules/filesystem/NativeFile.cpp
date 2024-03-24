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

// LOVE
#include "NativeFile.h"
#include "common/utf8.h"

#ifdef LOVE_ANDROID
#include "common/android.h"
#endif

// Assume POSIX or Visual Studio.
#include <sys/types.h>
#include <sys/stat.h>

#ifdef LOVE_WINDOWS
#include <wchar.h>
#else
#include <unistd.h> // POSIX.
#endif

namespace love
{
namespace filesystem
{

love::Type NativeFile::type("NativeFile", &File::type);

NativeFile::NativeFile(const std::string &filename, Mode mode)
	: filename(filename)
	, file(nullptr)
	, mode(MODE_CLOSED)
	, bufferMode(BUFFER_NONE)
	, bufferSize(0)
{
	if (!open(mode))
		throw love::Exception("Could not open file at path %s", filename.c_str());
}

NativeFile::NativeFile(const NativeFile &other)
	: filename(other.filename)
	, file(nullptr)
	, mode(MODE_CLOSED)
	, bufferMode(other.bufferMode)
	, bufferSize(other.bufferSize)
{
	if (!open(other.mode))
		throw love::Exception("Could not open file at path %s", filename.c_str());
}

NativeFile::~NativeFile()
{
	if (mode != MODE_CLOSED)
		close();
}

NativeFile *NativeFile::clone()
{
	return new NativeFile(*this);
}

bool NativeFile::open(Mode newmode)
{
	if (newmode == MODE_CLOSED)
	{
		close();
		return true;
	}

	// File already open?
	if (file != nullptr)
		return false;

#if defined(LOVE_ANDROID)
	// Try to handle content:// URI
	int fd = love::android::getFDFromContentProtocol(filename.c_str());
	if (fd != -1)
	{
		if (newmode != MODE_READ)
		{
			::close(fd);
			throw love::Exception("%s is read-only.", filename.c_str());
		}

		file = fdopen(fd, "rb");
	}
	else
		file = fopen(filename.c_str(), getModeString(newmode));
#elif defined(LOVE_WINDOWS)
	// make sure non-ASCII filenames work.
	std::wstring modestr = to_widestr(getModeString(newmode));
	std::wstring wfilename = to_widestr(filename);

	file = _wfopen(wfilename.c_str(), modestr.c_str());
#else
	file = fopen(filename.c_str(), getModeString(newmode));
#endif

	if (newmode == MODE_READ && file == nullptr)
		throw love::Exception("Could not open file %s. Does not exist.", filename.c_str());

	mode = newmode;

	if (file != nullptr && !setBuffer(bufferMode, bufferSize))
	{
		// Revert to buffer defaults if we don't successfully set the buffer.
		bufferMode = BUFFER_NONE;
		bufferSize = 0;
	}

	return file != nullptr;
}

bool NativeFile::close()
{
	if (file == nullptr || fclose(file) != 0)
		return false;

	mode = MODE_CLOSED;
	file = nullptr;

	return true;
}

bool NativeFile::isOpen() const
{
	return mode != MODE_CLOSED && file != nullptr;
}

int64 NativeFile::getSize()
{
	int fd = file ? fileno(file) : -1;

#ifdef LOVE_WINDOWS
	
	struct _stat64 buf;

	if (fd != -1)
	{
		if (_fstat64(fd, &buf) != 0)
			return -1;
	}
	else
	{
		// make sure non-ASCII filenames work.
		std::wstring wfilename = to_widestr(filename);

		if (_wstat64(wfilename.c_str(), &buf) != 0)
			return -1;
	}

	return (int64) buf.st_size;

#else

	// Assume POSIX support...
	struct stat buf;

	if (fd != -1)
	{
		if (fstat(fd, &buf) != 0)
			return -1;
	}
	else if (stat(filename.c_str(), &buf) != 0)
		return -1;

	return (int64) buf.st_size;

#endif
}

int64 NativeFile::read(void *dst, int64 size)
{
	if (!file || mode != MODE_READ)
		throw love::Exception("File is not opened for reading.");

	if (size < 0)
		throw love::Exception("Invalid read size.");

	size_t read = fread(dst, 1, (size_t) size, file);

	return (int64) read;
}

bool NativeFile::write(const void *data, int64 size)
{
	if (!file || (mode != MODE_WRITE && mode != MODE_APPEND))
		throw love::Exception("File is not opened for writing.");

	if (size < 0)
		throw love::Exception("Invalid write size.");

	int64 written = (int64) fwrite(data, 1, (size_t) size, file);

	return written == size;
}

bool NativeFile::flush()
{
	if (!file || (mode != MODE_WRITE && mode != MODE_APPEND))
		throw love::Exception("File is not opened for writing.");

	return fflush(file) == 0;
}

bool NativeFile::isEOF()
{
	return file == nullptr || tell() >= getSize();
}

int64 NativeFile::tell()
{
	if (file == nullptr)
		return -1;

#ifdef LOVE_WINDOWS
	return (int64) _ftelli64(file);
#else
	return (int64) ftello(file);
#endif
}

bool NativeFile::seek(int64 pos, SeekOrigin origin)
{
	if (file == nullptr)
		return false;

	int forigin = SEEK_SET;
	if (origin == SEEKORIGIN_CURRENT)
		forigin = SEEK_CUR;
	else if (origin == SEEKORIGIN_END)
		forigin = SEEK_END;

	// TODO
#ifdef LOVE_WINDOWS
	return _fseeki64(file, pos, forigin) == 0;
#else
	return fseeko(file, (off_t) pos, forigin) == 0;
#endif
}

bool NativeFile::setBuffer(BufferMode bufmode, int64 size)
{
	if (size < 0)
		return false;

	if (bufmode == BUFFER_NONE)
		size = 0;

	// If the file isn't open, we'll make sure the buffer values are set in
	// NativeFile::open.
	if (!isOpen())
	{
		bufferMode = bufmode;
		bufferSize = size;
		return true;
	}

	int vbufmode;
	switch (bufmode)
	{
	case File::BUFFER_NONE:
	default:
		vbufmode = _IONBF;
		break;
	case File::BUFFER_LINE:
		vbufmode = _IOLBF;
		break;
	case File::BUFFER_FULL:
		vbufmode = _IOFBF;
		break;
	}

	if (setvbuf(file, nullptr, vbufmode, (size_t) size) != 0)
		return false;

	bufferMode = bufmode;
	bufferSize = size;

	return true;
}

File::BufferMode NativeFile::getBuffer(int64 &size) const
{
	size = bufferSize;
	return bufferMode;
}

const std::string &NativeFile::getFilename() const
{
	return filename;
}

File::Mode NativeFile::getMode() const
{
	return mode;
}

const char *NativeFile::getModeString(Mode mode)
{
	switch (mode)
	{
	case File::MODE_CLOSED:
	default:
		return "c";
	case File::MODE_READ:
		return "rb";
	case File::MODE_WRITE:
		return "wb";
	case File::MODE_APPEND:
		return "ab";
	}
}

} // filesystem
} // love
