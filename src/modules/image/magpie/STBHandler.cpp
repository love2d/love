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

// LOVE
#include "STBHandler.h"

static void loveSTBIAssert(bool test, const char *teststr)
{
	if (!test)
		throw love::Exception("Could not decode image (stb_image assertion '%s' failed)", teststr);
}

// stb_image
 #define STBI_ONLY_JPEG
// #define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#define STBI_ONLY_TGA
#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT(A) loveSTBIAssert((A), #A)
#include "libraries/stb/stb_image.h"

// C
#include <cstdlib>

namespace love
{
namespace image
{
namespace magpie
{

bool STBHandler::canDecode(love::filesystem::FileData *data)
{
	int w = 0;
	int h = 0;
	int comp = 0;

	int status = stbi_info_from_memory((const stbi_uc *) data->getData(),
	                                   (int) data->getSize(), &w, &h, &comp);

	return status == 1 && w > 0 && h > 0;
}

bool STBHandler::canEncode(ImageData::EncodedFormat format)
{
	return format == ImageData::ENCODED_TGA;
}

FormatHandler::DecodedImage STBHandler::decode(love::filesystem::FileData *data)
{
	DecodedImage img;

	int comp = 0;
	img.data = stbi_load_from_memory((const stbi_uc *) data->getData(),
	                                 (int) data->getSize(),
	                                 &img.width, &img.height,
	                                 &comp, 4);

	if (img.data == nullptr || img.width <= 0 || img.height <= 0)
	{
		const char *err = stbi_failure_reason();
		if (err == nullptr)
			err = "unknown error";
		throw love::Exception("Could not decode image with stb_image (%s).", err);
	}

	img.size = img.width * img.height * 4;

	return img;
}

FormatHandler::EncodedImage STBHandler::encode(const DecodedImage &img, ImageData::EncodedFormat format)
{
	if (!canEncode(format))
		throw love::Exception("Invalid format.");

	// We don't actually use stb_image for encoding, but this code is small
	// enough that it might as well stay here.

	EncodedImage encimg;

	const size_t headerlen = 18;
	const size_t bpp = 4;

	encimg.size = (img.width * img.height * bpp) + headerlen;

	// We need to use malloc because we use stb_image_free (which uses free())
	// as our custom free() function, which is called by the ImageData after
	// encode() is complete.
	// stb_image's source code is compiled with this source, so calling malloc()
	// directly is fine.
	encimg.data = (unsigned char *) malloc(encimg.size);

	if (encimg.data == nullptr)
		throw love::Exception("Out of memory.");

	// here's the header for the Targa file format.
	encimg.data[0] = 0; // ID field size
	encimg.data[1] = 0; // colormap type
	encimg.data[2] = 2; // image type
	encimg.data[3] = encimg.data[4] = 0; // colormap start
	encimg.data[5] = encimg.data[6] = 0; // colormap length
	encimg.data[7] = 32; // colormap bits
	encimg.data[8] = encimg.data[9] = 0; // x origin
	encimg.data[10] = encimg.data[11] = 0; // y origin
	// Targa is little endian, so:
	encimg.data[12] = img.width & 255; // least significant byte of width
	encimg.data[13] = img.width >> 8; // most significant byte of width
	encimg.data[14] = img.height & 255; // least significant byte of height
	encimg.data[15] = img.height >> 8; // most significant byte of height
	encimg.data[16] = bpp * 8; // bits per pixel
	encimg.data[17] = 0x20; // descriptor bits (flip bits: 0x10 horizontal, 0x20 vertical)

	// header done. write the pixel data to TGA:
	memcpy(encimg.data + headerlen, img.data, img.width * img.height * bpp);

	// convert the pixels from RGBA to BGRA.
	pixel *encodedpixels = (pixel *) (encimg.data + headerlen);
	for (int y = 0; y < img.height; y++)
	{
		for (int x = 0; x < img.width; x++)
		{
			unsigned char r = encodedpixels[y * img.width + x].r;
			unsigned char b = encodedpixels[y * img.width + x].b;
			encodedpixels[y * img.width + x].r = b;
			encodedpixels[y * img.width + x].b = r;
		}
	}

	return encimg;
}

void STBHandler::free(unsigned char *mem)
{
	// The STB decoder gave memory allocated directly by stb_image to the
	// ImageData, so we use stb_image_free to delete it.
	stbi_image_free(mem);
}

} // magpie
} // image
} // love
