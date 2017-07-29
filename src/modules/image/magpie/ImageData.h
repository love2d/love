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

#ifndef LOVE_IMAGE_MAGPIE_IMAGE_DATA_H
#define LOVE_IMAGE_MAGPIE_IMAGE_DATA_H

// LOVE
#include "FormatHandler.h"
#include "image/ImageData.h"

// C++
#include <list>

namespace love
{
namespace image
{
namespace magpie
{

class ImageData : public love::image::ImageData
{
public:

	ImageData(love::filesystem::FileData *data);
	ImageData(int width, int height, PixelFormat format = PIXELFORMAT_RGBA8);
	ImageData(int width, int height, PixelFormat format, void *data, bool own);
	ImageData(const ImageData &c);
	virtual ~ImageData();

	// Implements image::ImageData.
	virtual love::image::ImageData *clone() const;
	virtual love::filesystem::FileData *encode(EncodedFormat encodedFormat, const char *filename, bool writefile);

private:

	// Create imagedata. Initialize with data if not null.
	void create(int width, int height, PixelFormat format, void *data = nullptr);

	// Decode and load an encoded format.
	void decode(love::filesystem::FileData *data);

	// The format handler that was used to decode the ImageData. We need to know
	// this so we can properly delete memory allocated by the decoder.
	StrongRef<FormatHandler> decodeHandler;

}; // ImageData

} // magpie
} // image
} // love

#endif // LOVE_IMAGE_MAGPIE_IMAGE_DATA_H
