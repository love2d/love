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

#ifndef LOVE_IMAGE_MAGPIE_FORMAT_HANDLER_H
#define LOVE_IMAGE_MAGPIE_FORMAT_HANDLER_H

// LOVE
#include "image/ImageData.h"
#include "filesystem/FileData.h"

namespace love
{
namespace image
{
namespace magpie
{

/**
 * Base class for all ImageData encoder/decoder library interfaces.
 **/
class FormatHandler
{
public:

	// Raw RGBA pixel data.
	struct DecodedImage
	{
		int width, height;
		size_t size;
		unsigned char *data;
		DecodedImage() : width(0), height(0), size(0), data(0) {}
	};

	// Pixel data encoded in a particular format.
	struct EncodedImage
	{
		size_t size;
		unsigned char *data;
		EncodedImage() : size(0), data(0) {}
	};

	// Lets pretend we have virtual static methods...

	/**
	 * Determines whether a particular FileData can be decoded by this handler.
	 * @param data The data to decode.
	 **/
	// virtual static bool canDecode(love::filesystem::FileData *data) = 0;

	/**
	 * Determines whether this handler can encode to a particular format.
	 * @param format The format to encode to.
	 **/
	// virtual static bool canEncode(ImageData::Format format) = 0;

	/**
	 * Decodes an image from its encoded form into raw pixel data.
	 * @param data The encoded data to decode.
	 * @return The decoded pixel data.
	 **/
	// virtual static DecodedImage decode(love::filesystem::FileData *data) = 0;

	/**
	 * Encodes an image from raw pixel data into a particular format.
	 * @param img The raw image data to encode.
	 * @param format The format to encode to.
	 * @return The encoded image data.
	 **/
	// virtual static EncodedImage encode(const DecodedImage &img, ImageData::Format format) = 0;

}; // FormatHandler

} // magpie
} // image
} // love

#endif // LOVE_IMAGE_MAGPIE_FORMAT_HANDLER_H
