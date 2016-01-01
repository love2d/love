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

#ifndef LOVE_IMAGE_MAGPIE_IMAGE_H
#define LOVE_IMAGE_MAGPIE_IMAGE_H

// LOVE
#include "image/Image.h"
#include "FormatHandler.h"
#include "CompressedFormatHandler.h"

// C++
#include <list>

namespace love
{
namespace image
{
namespace magpie
{

/**
 * Similar to love.sound's Lullaby module, love.image.magpie interfaces with
 * multiple image libraries and determines the correct one to use on a
 * per-image basis at runtime.
 **/
class Image : public love::image::Image
{
public:

	Image();
	~Image();

	// Implements Module.
	const char *getName() const;

	love::image::ImageData *newImageData(love::filesystem::FileData *data);
	love::image::ImageData *newImageData(int width, int height);
	love::image::ImageData *newImageData(int width, int height, void *data, bool own = false);

	love::image::CompressedImageData *newCompressedData(love::filesystem::FileData *data);

	bool isCompressed(love::filesystem::FileData *data);

private:

	// Image format handlers we can use for decoding and encoding ImageData.
	std::list<FormatHandler *> formatHandlers;

	// Compressed image format handers we can use for parsing CompressedImageData.
	std::list<CompressedFormatHandler *> compressedFormatHandlers;

}; // Image

} // magpie
} // image
} // love

#endif // LOVE_IMAGE_MAGPIE_IMAGE_H
