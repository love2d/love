/**
 * Copyright (c) 2006-2013 LOVE Development Team
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

#ifndef LOVE_IMAGE_COMPRESSED_DATA_H
#define LOVE_IMAGE_COMPRESSED_DATA_H

// LOVE
#include "common/Data.h"
#include "common/StringMap.h"
#include "thread/threads.h"

using love::thread::Mutex;

// STL
#include <vector>

namespace love
{
namespace image
{

// CompressedData represents image data which is designed to be uploaded to the
// GPU and rendered in its compressed form, without being un-compressed.
// http://renderingpipeline.com/2012/07/texture-compression/

class CompressedData : public Data
{
public:

	// Types of compressed image data.
	enum TextureType
	{
		TYPE_UNKNOWN,
		TYPE_DXT1,
		TYPE_DXT3,
		TYPE_DXT5,
		TYPE_BC5,
		TYPE_BC5s,
		TYPE_BC7,
		TYPE_BC7srgb,
		TYPE_MAX_ENUM
	};

	// Compressed image data can have multiple mipmap levels, each represented
	// by a sub-image.
	struct SubImage
	{
		int width, height;
		size_t size;
		std::vector<unsigned char> data;
	};

	CompressedData();
	virtual ~CompressedData();

	// Implements Data.
	virtual void *getData() const;
	virtual int getSize() const;

	/**
	 * Gets the number of mipmaps in this CompressedData.
	 * Includes the base image level.
	 **/
	int getNumMipmaps() const;

	/**
	 * Gets the size in bytes of the sub-image at the specified mipmap level.
	 **/
	int getSize(int miplevel) const;

	/**
	 * Gets the byte data of the sub-image at the specified mipmap level.
	 **/
	void *getData(int miplevel) const;

	/**
	 * Gets the width of the sub-image at the specified mipmap level.
	 **/
	int getWidth(int miplevel) const;

	/**
	 * Gets the height of the sub-image at the specified mipmap level.
	 **/
	int getHeight(int miplevel) const;

	/**
	 * Gets the type of the compressed data.
	 **/
	TextureType getType() const;

	static bool getConstant(const char *in, TextureType &out);
	static bool getConstant(TextureType in, const char *&out);

protected:

	TextureType type;

	// Texture info for each mipmap level.
	std::vector<SubImage> dataImages;

	void checkMipmapLevelExists(int miplevel) const;

private:

	static StringMap<TextureType, TYPE_MAX_ENUM>::Entry typeEntries[];
	static StringMap<TextureType, TYPE_MAX_ENUM> types;

}; // CompressedData

} // image
} // love

#endif // LOVE_IMAGE_COMPRESSED_DATA_H
