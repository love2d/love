/**
 * Copyright (c) 2006-2020 LOVE Development Team
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

#include "WEBPHandler.h"

// LOVE
#include "common/Exception.h"
#include "common/math.h"

// libwebp
#include <webp/decode.h>
#include <webp/encode.h>

// C++

// C
#include <cstdlib>

namespace love
{
namespace image
{
namespace magpie
{

bool WEBPHandler::canDecode(Data *data)
{
	WebPBitstreamFeatures features;
	bool ok = (WebPGetFeatures((uint8_t*)data->getData(), data->getSize(), &features) == VP8_STATUS_OK);
	return ok && features.width > 0 && features.height > 0;
}

bool WEBPHandler::canEncode(PixelFormat rawFormat, EncodedFormat encodedFormat)
{
	return encodedFormat == ENCODED_WEBP && rawFormat == PIXELFORMAT_RGBA8;
}

WEBPHandler::DecodedImage WEBPHandler::decode(Data *data)
{
	int width, height;
	uint8_t *pdata = WebPDecodeRGBA((uint8_t*)data->getData(), data->getSize(), &width, &height); 

	if(pdata){
		DecodedImage img;
		img.width = width;
		img.height = height;
		img.size = (size_t)width*height*4;
		img.format = PIXELFORMAT_RGBA8;
		img.data = pdata;
		return img;
	}
	else
		throw love::Exception("Could not decode WebP image");
}

FormatHandler::EncodedImage WEBPHandler::encode(const DecodedImage &img, EncodedFormat encodedFormat)
{
	if (!canEncode(img.format, encodedFormat))
		throw love::Exception("WebP encoder cannot encode to non-WebP format.");

	uint8_t *data = nullptr;
  // export using lossless encoding
	size_t size = WebPEncodeLosslessRGBA((uint8_t*)img.data, img.width, img.height, img.width*4, &data);
	if(size > 0){
		EncodedImage out_img;
		out_img.size = size;
		out_img.data = data;
		return out_img;
	}
	else{
		if(data) WebPFree(data); // doc ambiguity: data may need to be freed on error
		throw love::Exception("Could not encode WebP image");
	}
}

void WEBPHandler::freeRawPixels(unsigned char *mem)
{
	WebPFree(mem);
}

} // magpie
} // image
} // love
