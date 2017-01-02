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

CompressedImageData::CompressedImageData()
	: format(PIXELFORMAT_UNKNOWN)
	, sRGB(false)
	, data(nullptr)
	, dataSize(0)
{
}

CompressedImageData::~CompressedImageData()
{
}

size_t CompressedImageData::getSize() const
{
	return dataSize;
}

void *CompressedImageData::getData() const
{
	return data;
}

int CompressedImageData::getMipmapCount() const
{
	return (int) dataImages.size();
}

size_t CompressedImageData::getSize(int miplevel) const
{
	checkMipmapLevelExists(miplevel);

	return dataImages[miplevel].size;
}

void *CompressedImageData::getData(int miplevel) const
{
	checkMipmapLevelExists(miplevel);

	return &dataImages[miplevel].data[0];
}

int CompressedImageData::getWidth(int miplevel) const
{
	checkMipmapLevelExists(miplevel);

	return dataImages[miplevel].width;
}

int CompressedImageData::getHeight(int miplevel) const
{
	checkMipmapLevelExists(miplevel);

	return dataImages[miplevel].height;
}

PixelFormat CompressedImageData::getFormat() const
{
	return format;
}

bool CompressedImageData::isSRGB() const
{
	return sRGB;
}

void CompressedImageData::checkMipmapLevelExists(int miplevel) const
{
	if (miplevel < 0 || miplevel >= (int) dataImages.size())
		throw love::Exception("Mipmap level %d does not exist", miplevel + 1);
}

} // image
} // love
