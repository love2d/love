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

#ifndef LOVE_MATH_COMPRESSED_DATA_H
#define LOVE_MATH_COMPRESSED_DATA_H

// LOVE
#include "common/Data.h"
#include "Compressor.h"

namespace love
{
namespace math
{

/**
 * Stores byte data compressed via Math::compress.
 **/
class CompressedData : public love::Data
{
public:

	/**
	 * Constructor just stores already-compressed data in the object.
	 **/
	CompressedData(Compressor::Format format, char *cdata, size_t compressedsize, size_t rawsize, bool own = true);
	virtual ~CompressedData();

	/**
	 * Gets the format that was used to compress the data.
	 **/
	Compressor::Format getFormat() const;

	/**
	 * Gets the original (uncompressed) size of the compressed data. May return
	 * 0 if the uncompressed size is unknown.
	 **/
	size_t getDecompressedSize() const;

	// Implements Data.
	void *getData() const override;
	size_t getSize() const override;

private:

	Compressor::Format format;

	char *data;
	size_t dataSize;

	size_t originalSize;

}; // CompressedData

} // math
} // love

#endif // LOVE_MATH_COMPRESSED_DATA_H
