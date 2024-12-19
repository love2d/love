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

namespace love
{
namespace filesystem
{

love::Type File::type("File", &Stream::type);

File::~File()
{
}

FileData *File::read()
{
	return read(SIZE_ALL);
}

FileData *File::read(int64 size)
{
	bool isopen = isOpen();

	if (!isopen && !open(MODE_READ))
		throw love::Exception("Could not read file %s.", getFilename().c_str());

	int64 max = getSize();
	int64 cur = tell();

	if (size == SIZE_ALL)
		size = max;
	else if (size < 0)
		throw love::Exception("Invalid read size.");

	// Clamping because the file offset may be in a weird position.
	if (cur < 0)
		cur = 0;
	else if (cur > max)
		cur = max;

	if (cur + size > max)
		size = max - cur;

	StrongRef<FileData> fileData(new FileData(size, getFilename()), Acquire::NORETAIN);
	int64 bytesRead = read(fileData->getData(), size);

	if (bytesRead < 0 || (bytesRead == 0 && bytesRead != size))
	{
		delete fileData;
		throw love::Exception("Could not read from file.");
	}

	if (bytesRead < size)
	{
		StrongRef<FileData> tmpFileData(new FileData(bytesRead, getFilename()), Acquire::NORETAIN);
		memcpy(tmpFileData->getData(), fileData->getData(), (size_t) bytesRead);
		fileData = tmpFileData;
	}

	if (!isopen)
		close();

	fileData->retain();
	return fileData;
}

std::string File::getExtension() const
{
	const std::string &filename = getFilename();
	std::string::size_type idx = filename.rfind('.');

	if (idx != std::string::npos)
		return filename.substr(idx+1);
	else
		return std::string();
}

STRINGMAP_CLASS_BEGIN(File, File::Mode, File::MODE_MAX_ENUM, mode)
{
	{ "c", File::MODE_CLOSED },
	{ "r", File::MODE_READ   },
	{ "w", File::MODE_WRITE  },
	{ "a", File::MODE_APPEND },
}
STRINGMAP_CLASS_END(File, File::Mode, File::MODE_MAX_ENUM, mode)

STRINGMAP_CLASS_BEGIN(File, File::BufferMode, File::BUFFER_MAX_ENUM, bufferMode)
{
	{ "none", File::BUFFER_NONE },
	{ "line", File::BUFFER_LINE },
	{ "full", File::BUFFER_FULL },
}
STRINGMAP_CLASS_END(File, File::BufferMode, File::BUFFER_MAX_ENUM, bufferMode)

} // filesystem
} // love
