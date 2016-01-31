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

#include "PNGHandler.h"

// LOVE
#include "common/Exception.h"
#include "common/math.h"

// LodePNG
#include "lodepng/lodepng.h"

// zlib
#include <zlib.h>

// C++
#include <algorithm>

// C
#include <cstdlib>

namespace love
{
namespace image
{
namespace magpie
{

// Custom PNG decompression function for LodePNG, using zlib.
static unsigned zlibDecompress(unsigned char **out, size_t *outsize, const unsigned char *in,
                               size_t insize, const LodePNGDecompressSettings* /*settings*/)
{
	int status = Z_OK;

	uLongf outdatasize = insize;
	size_t sizemultiplier = 0;
	unsigned char *outdata = out != nullptr ? *out : nullptr;

	while (true)
	{
		// Enough size to hold the decompressed data, hopefully.
		outdatasize = insize << (++sizemultiplier);

		// LodePNG uses malloc, realloc, and free.
		// Since version 2014-08-23, LodePNG passes in an existing pointer in
		// the 'out' argument that it expects to be realloc'd. Not doing so can
		// result in a memory leak.
		if (outdata != nullptr)
			outdata = (unsigned char *) realloc(outdata, outdatasize);
		else
			outdata = (unsigned char *) malloc(outdatasize);

		if (!outdata)
			return 83; // "Memory allocation failed" error code for LodePNG.

		// Use zlib to decompress the PNG data.
		status = uncompress(outdata, &outdatasize, in, insize);

		// If the out buffer was big enough, break out of the loop.
		if (status != Z_BUF_ERROR)
			break;

		// Otherwise delete the out buffer and try again with a larger size...
		free(outdata);
		outdata = nullptr;
	}

	if (status != Z_OK)
	{
		free(outdata);
		return 10000; // "Unknown error code" for LodePNG.
	}

	if (out != nullptr)
		*out = outdata;

	if (outsize != nullptr)
		*outsize = outdatasize;

	return 0; // Success.
}

// Custom PNG compression function for LodePNG, using zlib.
static unsigned zlibCompress(unsigned char **out, size_t *outsize, const unsigned char *in,
                             size_t insize, const LodePNGCompressSettings* /*settings*/)
{
	// Get the maximum compressed size of the data.
	uLongf outdatasize = compressBound(insize);

	// LodePNG uses malloc, realloc, and free.
	unsigned char *outdata = (unsigned char *) malloc(outdatasize);

	if (!outdata)
		return 83; // "Memory allocation failed" error code for LodePNG.

	// Use zlib to compress the PNG data.
	int status = compress(outdata, &outdatasize, in, insize);

	if (status != Z_OK)
	{
		free(outdata);
		return 10000; // "Unknown error code" for LodePNG.
	}

	if (out != nullptr)
		*out = outdata;

	if (outsize != nullptr)
		*outsize = (size_t) outdatasize;

	return 0; // Success.
}

bool PNGHandler::canDecode(love::filesystem::FileData *data)
{
	unsigned int width = 0, height = 0;
	unsigned char *indata = (unsigned char *) data->getData();
	size_t insize = data->getSize();

	lodepng::State state;
	unsigned status = lodepng_inspect(&width, &height, &state, indata, insize);

	return status == 0 && width > 0 && height > 0;
}

bool PNGHandler::canEncode(ImageData::EncodedFormat format)
{
	return format == ImageData::ENCODED_PNG;
}

PNGHandler::DecodedImage PNGHandler::decode(love::filesystem::FileData *fdata)
{
	unsigned int width = 0, height = 0;
	unsigned char *indata = (unsigned char *) fdata->getData();
	size_t insize = fdata->getSize();

	DecodedImage img;

	lodepng::State state;

	state.info_raw.colortype = LCT_RGBA;
	state.info_raw.bitdepth = 8;

	state.decoder.zlibsettings.custom_zlib = zlibDecompress;

	unsigned status = lodepng_decode(&img.data, &width, &height,
	                                 &state, indata, insize);

	if (status != 0)
	{
		const char *err = lodepng_error_text(status);
		throw love::Exception("Could not decode PNG image (%s)", err);
	}

	img.width  = (int) width;
	img.height = (int) height;
	img.size   = width * height * 4;

	return img;
}

PNGHandler::EncodedImage PNGHandler::encode(const DecodedImage &img, ImageData::EncodedFormat format)
{
	if (format != ImageData::ENCODED_PNG)
		throw love::Exception("PNG encoder cannot encode to non-PNG format.");

	EncodedImage encimg;

	lodepng::State state;

	state.info_raw.colortype = LCT_RGBA;
	state.info_raw.bitdepth = 8;

	// TODO: support plain RGB (24-bit) encoding in the future?
	state.info_png.color.colortype = LCT_RGBA;
	state.info_png.color.bitdepth = 8;

	state.encoder.zlibsettings.custom_zlib = zlibCompress;

	unsigned status = lodepng_encode(&encimg.data, &encimg.size,
	                                 img.data, img.width, img.height, &state);

	if (status != 0)
	{
		const char *err = lodepng_error_text(status);
		throw love::Exception("Could not encode PNG image (%s)", err);
	}

	return encimg;
}

void PNGHandler::free(unsigned char *mem)
{
	// LodePNG uses malloc, realloc, and free.
	if (mem)
		::free(mem);
}

} // magpie
} // image
} // love
