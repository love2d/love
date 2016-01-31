/**
 * Copyright (c) 2006-2016 LOVE Development Team
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
#include "CompressedData.h"

namespace love
{
namespace math
{

CompressedData::CompressedData(Compressor::Format format, char *cdata, size_t compressedsize, size_t rawsize, bool own)
	: format(format)
	, data(nullptr)
	, dataSize(compressedsize)
	, originalSize(rawsize)
{
	if (own)
		data = cdata;
	else
	{
		try
		{
			data = new char[dataSize];
		}
		catch (std::bad_alloc &)
		{
			throw love::Exception("Out of memory.");
		}

		memcpy(data, cdata, dataSize);
	}
}

CompressedData::~CompressedData()
{
	delete[] data;
}

Compressor::Format CompressedData::getFormat() const
{
	return format;
}

size_t CompressedData::getDecompressedSize() const
{
	return originalSize;
}

void *CompressedData::getData() const
{
	return data;
}

size_t CompressedData::getSize() const
{
	return dataSize;
}

} // math
} // love
