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
#include "common/Object.h"
#include "common/Data.h"
#include "common/pixelformat.h"
#include "CompressedSlice.h"

namespace love
{
namespace image
{

/**
 * Base class for all ImageData and CompressedImageData encoder/decoder library
 * interfaces. We inherit from love::Object to take advantage of ref coounting.
 **/
class FormatHandler : public love::Object
{
public:

	enum EncodedFormat
	{
		ENCODED_TGA,
		ENCODED_PNG,
		ENCODED_MAX_ENUM
	};

	// Raw RGBA pixel data.
	struct DecodedImage
	{
		PixelFormat format = PIXELFORMAT_RGBA8;
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
	virtual ~FormatHandler();

	/**
	 * Whether this format handler can decode the given Data into raw pixels.
	 **/
	virtual bool canDecode(Data *data);

	/**
	 * Whether this format handler can encode raw pixels to a particular format.
	 **/
	virtual bool canEncode(PixelFormat rawFormat, EncodedFormat encodedFormat);

	/**
	 * Decodes an image from its encoded form into raw pixel data.
	 **/
	virtual DecodedImage decode(Data *data);

	/**
	 * Encodes an image from raw pixel data into a particular format.
	 **/
	virtual EncodedImage encode(const DecodedImage &img, EncodedFormat format);

	/**
	 * Whether this format handler can parse the given Data into a
	 * CompressedImageData object.
	 **/
	virtual bool canParseCompressed(Data *data);

	/**
	 * Parses compressed image data into a list of sub-images and returns a
	 * single block of memory containing all the images.
	 *
	 * @param[in] filedata The data to parse.
	 * @param[out] images The list of sub-images generated. Byte data is a
	 *             pointer to the returned data.
	 * @param[out] format The format of the Compressed Data.
	 * @param[out] sRGB Whether the texture is sRGB-encoded.
	 *
	 * @return The single block of memory containing the parsed images.
	 **/
	virtual StrongRef<CompressedMemory> parseCompressed(Data *filedata,
	        std::vector<StrongRef<CompressedSlice>> &images,
	        PixelFormat &format, bool &sRGB);

	/**
	 * Frees raw pixel memory allocated by the format handler.
	 **/
	virtual void freeRawPixels(unsigned char *mem);

}; // FormatHandler

} // image
} // love
