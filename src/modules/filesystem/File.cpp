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

namespace love
{
namespace filesystem
{

love::Type File::type("File", &Object::type);

File::~File()
{
}

FileData *File::read(int64 size)
{
	bool isopen = isOpen();

	if (!isopen && !open(MODE_READ))
		throw love::Exception("Could not read file %s.", getFilename().c_str());

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
		fileData->release();
		fileData = tmpFileData;
	}

	if (!isopen)
		close();

	return fileData;
}

bool File::write(const Data *data, int64 size)
{
	return write(data->getData(), (size == ALL) ? data->getSize() : size);
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

bool File::getConstant(const char *in, Mode &out)
{
	return modes.find(in, out);
}

bool File::getConstant(Mode in, const char *&out)
{
	return modes.find(in, out);
}

std::vector<std::string> File::getConstants(Mode)
{
	return modes.getNames();
}

bool File::getConstant(const char *in, BufferMode &out)
{
	return bufferModes.find(in, out);
}

bool File::getConstant(BufferMode in, const char *&out)
{
	return bufferModes.find(in, out);
}

std::vector<std::string> File::getConstants(BufferMode)
{
	return bufferModes.getNames();
}

StringMap<File::Mode, File::MODE_MAX_ENUM>::Entry File::modeEntries[] =
{
	{ "c", MODE_CLOSED },
	{ "r", MODE_READ   },
	{ "w", MODE_WRITE  },
	{ "a", MODE_APPEND },
};

StringMap<File::Mode, File::MODE_MAX_ENUM> File::modes(File::modeEntries, sizeof(File::modeEntries));

StringMap<File::BufferMode, File::BUFFER_MAX_ENUM>::Entry File::bufferModeEntries[] =
{
	{ "none", BUFFER_NONE },
	{ "line", BUFFER_LINE },
	{ "full", BUFFER_FULL },
};

StringMap<File::BufferMode, File::BUFFER_MAX_ENUM> File::bufferModes(File::bufferModeEntries, sizeof(File::bufferModeEntries));

} // filesystem
} // love
