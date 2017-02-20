/**
 * Copyright (c) 2006-2017 LOVE Development Team
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

#include "CompressedImageData.h"

namespace love
{
namespace image
{

love::Type CompressedImageData::type("CompressedImageData", &Data::type);

CompressedImageData::Memory::Memory(size_t size)
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

CompressedImageData::Memory::~Memory()
{
	delete[] data;
}

CompressedImageData::Slice::Slice(PixelFormat format, int width, int height, Memory *memory, size_t offset, size_t size)
	: memory(memory)
	, offset(offset)
	, dataSize(size)
{
	this->format = format;
	this->width = width;
	this->height = height;
}

CompressedImageData::Slice::Slice(const Slice &s)
	: memory(s.memory)
	, offset(s.offset)
	, dataSize(s.dataSize)
{
	this->format = s.getFormat();
	this->width = s.getWidth();
	this->height = s.getHeight();
}

CompressedImageData::Slice::~Slice()
{
}

CompressedImageData::Slice *CompressedImageData::Slice::clone() const
{
	return new Slice(*this);
}

CompressedImageData::CompressedImageData()
	: format(PIXELFORMAT_UNKNOWN)
	, sRGB(false)
{
}

CompressedImageData::~CompressedImageData()
{
}

size_t CompressedImageData::getSize() const
{
	return memory->size;
}

void *CompressedImageData::getData() const
{
	return memory->data;
}

int CompressedImageData::getMipmapCount(int /*slice*/) const
{
	return (int) dataImages.size();
}

int CompressedImageData::getSliceCount(int /*mip*/) const
{
	return 1;
}

size_t CompressedImageData::getSize(int miplevel) const
{
	checkSliceExists(0, miplevel);

	return dataImages[miplevel]->getSize();
}

void *CompressedImageData::getData(int miplevel) const
{
	checkSliceExists(0, miplevel);

	return dataImages[miplevel]->getData();
}

int CompressedImageData::getWidth(int miplevel) const
{
	checkSliceExists(0, miplevel);

	return dataImages[miplevel]->getWidth();
}

int CompressedImageData::getHeight(int miplevel) const
{
	checkSliceExists(0, miplevel);

	return dataImages[miplevel]->getHeight();
}

PixelFormat CompressedImageData::getFormat() const
{
	return format;
}

bool CompressedImageData::isSRGB() const
{
	return sRGB;
}

CompressedImageData::Slice *CompressedImageData::getSlice(int slice, int miplevel) const
{
	checkSliceExists(slice, miplevel);

	return dataImages[miplevel].get();
}

void CompressedImageData::checkSliceExists(int slice, int miplevel) const
{
	if (slice != 0)
		throw love::Exception("Slice index %d does not exists", slice + 1);

	if (miplevel < 0 || miplevel >= (int) dataImages.size())
		throw love::Exception("Mipmap level %d does not exist", miplevel + 1);
}

} // image
} // love
