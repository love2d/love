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

#pragma once

// LOVE
#include "common/Data.h"
#include "common/StringMap.h"
#include "common/int.h"
#include "common/pixelformat.h"
#include "CompressedSlice.h"
#include "FormatHandler.h"

// STL
#include <vector>
#include <list>

namespace love
{
namespace image
{

/**
 * CompressedImageData represents image data which is designed to be uploaded to
 * the GPU and rendered in its compressed form, without being decompressed.
 * http://renderingpipeline.com/2012/07/texture-compression/
 **/
class CompressedImageData : public Data
{
public:

	static love::Type type;

	CompressedImageData(const std::list<FormatHandler *> &formats, Data *filedata);
	CompressedImageData(const CompressedImageData &c);
	virtual ~CompressedImageData();

	// Implements Data.
	CompressedImageData *clone() const override;
	void *getData() const override;
	size_t getSize() const override;

	/**
	 * Gets the number of mipmaps in this Compressed Image Data.
	 * Includes the base image level.
	 **/
	int getMipmapCount() const;

	/**
	 * Gets the number of slices (array layers, cube faces, 3D layers, etc.)
	 **/
	int getSliceCount(int mip = 0) const;

	/**
	 * Gets the size in bytes of a sub-image at the specified mipmap level.
	 **/
	size_t getSize(int miplevel) const;

	/**
	 * Gets the byte data of a sub-image at the specified mipmap level.
	 **/
	void *getData(int miplevel) const;

	/**
	 * Gets the width of a sub-image at the specified mipmap level.
	 **/
	int getWidth(int miplevel = 0) const;

	/**
	 * Gets the height of a sub-image at the specified mipmap level.
	 **/
	int getHeight(int miplevel = 0) const;

	/**
	 * Gets the format of the compressed data.
	 **/
	PixelFormat getFormat() const;

	bool isSRGB() const;

	CompressedSlice *getSlice(int slice, int miplevel) const;

protected:

	PixelFormat format;
	bool sRGB;

	// Single block of memory containing all of the sub-images.
	StrongRef<CompressedMemory> memory;

	// Texture info for each mipmap level.
	std::vector<StrongRef<CompressedSlice>> dataImages;

	void checkSliceExists(int slice, int miplevel) const;

}; // CompressedImageData

} // image
} // love
