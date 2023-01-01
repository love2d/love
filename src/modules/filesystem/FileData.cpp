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

#include "FileData.h"

// C++
#include <iostream>
#include <limits>

namespace love
{
namespace filesystem
{

love::Type FileData::type("FileData", &Data::type);

FileData::FileData(uint64 size, const std::string &filename)
	: data(nullptr)
	, size((size_t) size)
	, filename(filename)
{
	try
	{
		data = new char[(size_t) size];
	}
	catch (std::bad_alloc &)
	{
		throw love::Exception("Out of memory.");
	}

	size_t dotpos = filename.rfind('.');

	if (dotpos != std::string::npos)
	{
		extension = filename.substr(dotpos + 1);
		name = filename.substr(0, dotpos);
	}
	else
		name = filename;
}

FileData::FileData(const FileData &c)
	: data(nullptr)
	, size(c.size)
	, filename(c.filename)
	, extension(c.extension)
	, name(c.name)
{
	try
	{
		data = new char[(size_t) size];
	}
	catch (std::bad_alloc &)
	{
		throw love::Exception("Out of memory.");
	}
	memcpy(data, c.data, size);
}

FileData::~FileData()
{
	delete [] data;
}

FileData *FileData::clone() const
{
	return new FileData(*this);
}

void *FileData::getData() const
{
	return data;
}

size_t FileData::getSize() const
{
	size_t sizemax = std::numeric_limits<size_t>::max();
	return size > sizemax ? sizemax : (size_t) size;
}

const std::string &FileData::getFilename() const
{
	return filename;
}

const std::string &FileData::getExtension() const
{
	return extension;
}

const std::string &FileData::getName() const
{
	return name;
}

} // filesystem
} // love
