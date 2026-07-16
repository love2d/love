/**
 * Copyright (c) 2006-2026 LOVE Development Team
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
#include "QOIHandler.h"
#include "common/Exception.h"

#define QOI_IMPLEMENTATION
#define QOI_NO_STDIO
#include "libraries/qoi/qoi.h"

// C
#include <cstdint>

namespace love
{
namespace image
{
namespace magpie
{

// From qoi.h docs
struct QOIHeader {
	char     magic[4];   // magic bytes "qoif"
	uint32_t width;      // image width in pixels (BE)
	uint32_t height;     // image height in pixels (BE)
	uint8_t  channels;   // 3 = RGB, 4 = RGBA
	uint8_t  colorSpace; // 0 = sRGB with linear alpha, 1 = all channels linear
};

bool QOIHandler::canDecode(Data *data)
{
	if (data->getSize() < sizeof(QOIHeader)) {
		return false;
	}

	QOIHeader *header = (QOIHeader *) data->getData();
	bool magicOk = strncmp(header->magic, "qoif", 4) == 0;

	return magicOk && header->width > 0 && header->height > 0;
}

bool QOIHandler::canEncode(PixelFormat rawFormat, EncodedFormat encodedFormat)
{
	return encodedFormat == ENCODED_QOI && rawFormat == PIXELFORMAT_RGBA8_UNORM;
}

FormatHandler::DecodedImage QOIHandler::decode(Data *data)
{
	DecodedImage img;

	qoi_desc imageDesc;
	img.data = (unsigned char *) qoi_decode(data->getData(), data->getSize(), &imageDesc, 4);
	img.width = imageDesc.width;
	img.height = imageDesc.height;
	img.size = img.width * img.height * 4;
	img.format = PIXELFORMAT_RGBA8_UNORM;

	if (img.data == nullptr)
	{
		throw love::Exception("Could not decode image with QOI.");
	}

	return img;
}

FormatHandler::EncodedImage QOIHandler::encode(const DecodedImage &img, EncodedFormat format)
{
	EncodedImage encodedImg;

	qoi_desc desc;
	desc.width = img.width;
	desc.height = img.height;
	desc.channels = 4;
	desc.colorspace = QOI_SRGB;

	int len;
	encodedImg.data = (unsigned char *) qoi_encode((void *) img.data, &desc, &len);
	encodedImg.size = len;

	if (encodedImg.data == nullptr)
	{
		throw love::Exception("Could not encode image with QOI");
	}

	return encodedImg;
}

void QOIHandler::freeRawPixels(unsigned char *mem)
{
	free(mem);
}

void QOIHandler::freeEncodedImage(unsigned char *mem)
{
	free(mem);
}

} // magpie
} // image
} // love
