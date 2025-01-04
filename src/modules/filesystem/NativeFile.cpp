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

// SDL
#include <SDL3/SDL_iostream.h>

namespace love
{
namespace filesystem
{

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

	file = SDL_IOFromFile(filename.c_str(), getModeString(newmode));
	if (file == nullptr)
		throw love::Exception("Could not open file %s: %s", filename.c_str(), SDL_GetError());

	mode = newmode;

	return file != nullptr;
}

bool NativeFile::close()
{
	if (file == nullptr || !SDL_CloseIO(file))
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
	int64 size = SDL_GetIOSize(file);
	return std::max(size, -1LL);
}

int64 NativeFile::read(void *dst, int64 size)
{
	if (!file || mode != MODE_READ)
		throw love::Exception("File is not opened for reading.");

	if (size < 0)
		throw love::Exception("Invalid read size.");

	size_t read = SDL_ReadIO(file, dst, (size_t) size);

	return (int64) read;
}

bool NativeFile::write(const void *data, int64 size)
{
	if (!file || (mode != MODE_WRITE && mode != MODE_APPEND))
		throw love::Exception("File is not opened for writing.");

	if (size < 0)
		throw love::Exception("Invalid write size.");

	int64 written = SDL_WriteIO(file, data, (size_t) size);

	return written == size;
}

bool NativeFile::flush()
{
	if (!file || (mode != MODE_WRITE && mode != MODE_APPEND))
		throw love::Exception("File is not opened for writing.");

	return SDL_FlushIO(file);
}

bool NativeFile::isEOF()
{
	return file == nullptr || tell() >= getSize();
}

int64 NativeFile::tell()
{
	if (file == nullptr)
		return -1;

	return SDL_TellIO(file);
}

bool NativeFile::seek(int64 pos, SeekOrigin origin)
{
	if (file == nullptr)
		return false;

	SDL_IOWhence whence = SDL_IO_SEEK_SET;
	if (origin == SEEKORIGIN_CURRENT)
		whence = SDL_IO_SEEK_CUR;
	else if (origin == SEEKORIGIN_END)
		whence = SDL_IO_SEEK_END;

	return SDL_SeekIO(file, pos, whence) >= 0;
}

bool NativeFile::setBuffer(BufferMode bufmode, int64 size)
{
	if (size < 0)
		return false;

	if (bufmode == BUFFER_NONE)
		size = 0;

	// FIXME: SDL doesn't have option to set buffering.

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
		// Note: PhysFS "append" allows the user to
		// seek the write pointer, but it's not possible
		// to do so with standard fopen-style modes.
		return "ab";
	}
}

} // filesystem
} // love
