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

#ifndef LOVE_IMAGE_MAGPIE_FORMAT_HANDLER_H
#define LOVE_IMAGE_MAGPIE_FORMAT_HANDLER_H

// LOVE
#include "image/ImageData.h"
#include "filesystem/FileData.h"
#include "common/Object.h"

namespace love
{
namespace image
{
namespace magpie
{

/**
 * Base class for all ImageData encoder/decoder library interfaces.
 * We inherit from love::Object to take advantage of reference counting...
 **/
class FormatHandler : public love::Object
{
public:

	// Raw RGBA pixel data.
	struct DecodedImage
	{
		int width   = 0;
		int height  = 0;
		size_t size = 0;
		unsigned char *data = nullptr;
	};

	// Pixel data encoded in a particular format.
	struct EncodedImage
	{
		size_t size = 0;
		unsigned char *data = nullptr;
	};

	/**
	 * The default constructor is called when the Image module is initialized.
	 **/
	FormatHandler();

	/**
	 * The destructor is called when the Image module is uninitialized.
	 **/
	virtual ~FormatHandler();

	/**
	 * Whether this format handler can decode a particular FileData.
	 **/
	virtual bool canDecode(love::filesystem::FileData *data);

	/**
	 * Whether this format handler can encode to a particular format.
	 **/
	virtual bool canEncode(ImageData::EncodedFormat format);

	/**
	 * Decodes an image from its encoded form into raw pixel data.
	 * @param data The encoded data to decode.
	 * @return The decoded pixel data.
	 **/
	virtual DecodedImage decode(love::filesystem::FileData *data);

	/**
	 * Encodes an image from raw pixel data into a particular format.
	 * @param img The raw image data to encode.
	 * @param format The format to encode to.
	 * @return The encoded image data.
	 **/
	virtual EncodedImage encode(const DecodedImage &img, ImageData::EncodedFormat format);

	/**
	 * Frees memory allocated by the format handler.
	 **/
	virtual void free(unsigned char *mem);

}; // FormatHandler

} // magpie
} // image
} // love

#endif // LOVE_IMAGE_MAGPIE_FORMAT_HANDLER_H
