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

// LOVE
#include "EXRHandler.h"
#include "common/floattypes.h"
#include "common/Exception.h"

// zlib (for tinyexr)
#include <zlib.h>

// tinyexr
#define TINYEXR_IMPLEMENTATION
#define TINYEXR_USE_MINIZ 0
#include "libraries/tinyexr/tinyexr.h"

// C
#include <cstdlib>

namespace love
{
namespace image
{
namespace magpie
{

bool EXRHandler::canDecode(Data *data)
{
	EXRVersion version;
	return ParseEXRVersionFromMemory(&version, (const unsigned char *) data->getData(), data->getSize()) == TINYEXR_SUCCESS;
}

bool EXRHandler::canEncode(PixelFormat /*rawFormat*/, EncodedFormat /*encodedFormat*/)
{
	return false;
}

template <typename T>
static void getEXRChannels(const EXRHeader &header, const EXRImage &image, T *rgba[4])
{
	for (int i = 0; i < header.num_channels; i++)
	{
		switch (*header.channels[i].name)
		{
		case 'R':
			rgba[0] = (T *) image.images[i];
			break;
		case 'G':
			rgba[1] = (T *) image.images[i];
			break;
		case 'B':
			rgba[2] = (T *) image.images[i];
			break;
		case 'A':
			rgba[3] = (T *) image.images[i];
			break;
		}
	}
}

template <typename T>
static T *loadEXRChannels(int width, int height, T *rgba[4], T one)
{
	T *data = nullptr;

	try
	{
		data = new T[width * height * 4];
	}
	catch (std::exception &)
	{
		throw love::Exception("Out of memory.");
	}

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			size_t offset = y * width + x;

			data[offset * 4 + 0] = rgba[0] != nullptr ? rgba[0][offset] : 0;
			data[offset * 4 + 1] = rgba[1] != nullptr ? rgba[1][offset] : 0;
			data[offset * 4 + 2] = rgba[2] != nullptr ? rgba[2][offset] : 0;
			data[offset * 4 + 3] = rgba[3] != nullptr ? rgba[3][offset] : one;
		}
	}

	return data;
}

FormatHandler::DecodedImage EXRHandler::decode(Data *data)
{
	const char *err = "unknown error";
	auto mem = (const unsigned char *) data->getData();
	size_t memsize = data->getSize();
	DecodedImage img;

	EXRHeader exrHeader;
	InitEXRHeader(&exrHeader);

	EXRImage exrImage;
	InitEXRImage(&exrImage);

	try
	{
		EXRVersion exrVersion;
		if (ParseEXRVersionFromMemory(&exrVersion, mem, memsize) != TINYEXR_SUCCESS)
			throw love::Exception("Could not parse EXR image header.");

		if (exrVersion.multipart || exrVersion.non_image || exrVersion.tiled)
			throw love::Exception("Multi-part, tiled, and non-image EXR files are not supported.");

		if (ParseEXRHeaderFromMemory(&exrHeader, &exrVersion, mem, memsize, &err) != TINYEXR_SUCCESS)
			throw love::Exception("Could not parse EXR image header: %s", err);

		if (LoadEXRImageFromMemory(&exrImage, &exrHeader, mem, memsize, &err) != TINYEXR_SUCCESS)
			throw love::Exception("Could not decode EXR image: %s", err);
	}
	catch (love::Exception &)
	{
		FreeEXRErrorMessage(err);
		throw;
	}

	int pixelType = exrHeader.pixel_types[0];

	for (int i = 1; i < exrHeader.num_channels; i++)
	{
		if (pixelType != exrHeader.pixel_types[i])
		{
			FreeEXRImage(&exrImage);
			throw love::Exception("Could not decode EXR image: all channels must have the same data type.");
		}
	}

	img.width  = exrImage.width;
	img.height = exrImage.height;

	if (pixelType == TINYEXR_PIXELTYPE_HALF)
	{
		img.format = PIXELFORMAT_RGBA16F;

		float16 *rgba[4] = {nullptr};
		getEXRChannels(exrHeader, exrImage, rgba);

		try
		{
			img.data = (unsigned char *) loadEXRChannels(img.width, img.height, rgba, float32to16(1.0f));
		}
		catch (love::Exception &)
		{
			FreeEXRImage(&exrImage);
			throw;
		}
	}
	else if (pixelType == TINYEXR_PIXELTYPE_FLOAT)
	{
		img.format = PIXELFORMAT_RGBA32F;

		float *rgba[4] = {nullptr};
		getEXRChannels(exrHeader, exrImage, rgba);

		try
		{
			img.data = (unsigned char *) loadEXRChannels(img.width, img.height, rgba, 1.0f);
		}
		catch (love::Exception &)
		{
			FreeEXRImage(&exrImage);
			throw;
		}
	}
	else
	{
		FreeEXRImage(&exrImage);
		throw love::Exception("Could not decode EXR image: unknown pixel format.");
	}

	img.size = img.width * img.height * getPixelFormatSize(img.format);

	FreeEXRImage(&exrImage);

	return img;
}

FormatHandler::EncodedImage EXRHandler::encode(const DecodedImage & /*img*/, EncodedFormat /*encodedFormat*/)
{
	throw love::Exception("Invalid format.");
}

void EXRHandler::freeRawPixels(unsigned char *mem)
{
	delete[] mem;
}

} // magpie
} // image
} // love
