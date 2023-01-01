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

#include "CompressedSlice.h"
#include "common/Exception.h"

namespace love
{
namespace image
{

CompressedMemory::CompressedMemory(size_t size)
	: data(nullptr)
	, size(size)
{
	try
	{
		data = new uint8[size];
	}
	catch (std::exception &)
	{
		throw love::Exception("Out of memory.");
	}
}

CompressedMemory::~CompressedMemory()
{
	delete[] data;
}

CompressedSlice::CompressedSlice(PixelFormat format, int width, int height, CompressedMemory *memory, size_t offset, size_t size)
	: ImageDataBase(format, width, height)
	, memory(memory)
	, offset(offset)
	, dataSize(size)
{
}

CompressedSlice::CompressedSlice(const CompressedSlice &s)
	: ImageDataBase(s.getFormat(), s.getWidth(), s.getHeight())
	, memory(s.memory)
	, offset(s.offset)
	, dataSize(s.dataSize)
{
}

CompressedSlice::~CompressedSlice()
{
}

CompressedSlice *CompressedSlice::clone() const
{
	return new CompressedSlice(*this);
}

} // image
} // love
