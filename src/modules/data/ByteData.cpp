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

#include "ByteData.h"
#include "common/Exception.h"
#include "common/int.h"

#include <string.h>

namespace love
{
namespace data
{

love::Type ByteData::type("ByteData", &Data::type);

ByteData::ByteData(size_t size)
	: size(size)
{
	create();
	memset(data, 0, size);
}

ByteData::ByteData(const void *d, size_t size)
	: size(size)
{
	create();
	memcpy(data, d, size);
}

ByteData::ByteData(void *d, size_t size, bool own)
	: size(size)
{
	if (own)
		data = (char *) d;
	else
	{
		create();
		memcpy(data, d, size);
	}
}

ByteData::ByteData(const ByteData &d)
	: size(d.size)
{
	create();
	memcpy(data, d.data, size);
}

ByteData::~ByteData()
{
	delete[] data;
}

void ByteData::create()
{
	if (size == 0)
		throw love::Exception("ByteData size must be greater than 0.");

	try
	{
		data = new char[size];
	}
	catch (std::exception &)
	{
		throw love::Exception("Out of memory.");
	}
}

ByteData *ByteData::clone() const
{
	return new ByteData(*this);
}

void *ByteData::getData() const
{
	return data;
}

size_t ByteData::getSize() const
{
	return size;
}

} // data
} // love
