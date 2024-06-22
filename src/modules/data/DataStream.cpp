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

#include "DataStream.h"
#include "common/Exception.h"
#include "common/int.h"
#include "common/Data.h"

#include <algorithm>

namespace love
{
namespace data
{

love::Type DataStream::type("DataStream", &Stream::type);

DataStream::DataStream(Data *data)
	: data(data)
	, memory((const uint8 *) data->getData())
	, writableMemory((uint8 *) data->getData()) // TODO: disallow writing sometimes?
	, offset(0)
	, size(data->getSize())
{
}

DataStream::DataStream(const DataStream &other)
	: data(other.data)
	, memory(other.memory)
	, writableMemory(other.writableMemory)
	, offset(0)
	, size(other.size)
{
}

DataStream::~DataStream()
{
}

DataStream *DataStream::clone()
{
	return new DataStream(*this);
}

bool DataStream::isReadable() const
{
	return true;
}

bool DataStream::isWritable() const
{
	return writableMemory != nullptr;
}

bool DataStream::isSeekable() const
{
	return true;
}

int64 DataStream::read(void* data, int64 size)
{
	if (size <= 0)
		return 0;

	if ((int64) offset >= getSize())
		return 0;

	int64 readsize = std::min<int64>(size, getSize() - offset);

	memcpy(data, memory + offset, readsize);

	offset += readsize;
	return readsize;
}

bool DataStream::write(const void* data, int64 size)
{
	if (size <= 0 || writableMemory == nullptr)
		return false;

	if ((int64) offset >= getSize())
		return false;

	int64 writesize = std::min<int64>(size, getSize() - offset);

	memcpy(writableMemory + offset, data, writesize);

	offset += writesize;
	return true;
}

bool DataStream::flush()
{
	return true;
}

int64 DataStream::getSize()
{
	return size;
}

bool DataStream::seek(int64 pos, SeekOrigin origin)
{
	if (origin == SEEKORIGIN_CURRENT)
		pos += offset;
	else if (origin == SEEKORIGIN_END)
		pos += size;

	if (pos < 0 || pos > (int64) size)
		return false;

	offset = pos;
	return true;
}

int64 DataStream::tell()
{
	return offset;
}

} // data
} // love
