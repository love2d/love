/**
 * Copyright (c) 2006-2024 LOVE Development Team
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

bool EXRHandler::canEncode(PixelFormat rawFormat, EncodedFormat encodedFormat)
{
	if (encodedFormat != ENCODED_EXR)
		return false;

	switch (rawFormat)
	{
	case PIXELFORMAT_R16_FLOAT:
	case PIXELFORMAT_R32_FLOAT:
	case PIXELFORMAT_R32_UINT:
	case PIXELFORMAT_RG16_FLOAT:
	case PIXELFORMAT_RG32_FLOAT:
	case PIXELFORMAT_RG32_UINT:
	case PIXELFORMAT_RGBA16_FLOAT:
	case PIXELFORMAT_RGBA32_FLOAT:
	case PIXELFORMAT_RGBA32_UINT:
		return true;
	default:
		return false;
	}

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
static T *readEXRChannels(int width, int height, T *rgba[4], T one)
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

template <typename T>
static void writeEXRChannels(int width, int height, int components, const int *channelmapping, const T *pixels, T *rgba[4])
{
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			size_t offset = y * width + x;
			for (int c = 0; c < components; c++)
				rgba[channelmapping[c]][offset] = pixels[offset * components + c];
		}
	}
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
		{
			FreeEXRHeader(&exrHeader);
			throw love::Exception("Could not decode EXR image: %s", err);
		}
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
			FreeEXRHeader(&exrHeader);
			FreeEXRImage(&exrImage);
			throw love::Exception("Could not decode EXR image: all channels must have the same data type.");
		}
	}

	img.width  = exrImage.width;
	img.height = exrImage.height;

	if (pixelType == TINYEXR_PIXELTYPE_UINT)
	{
		img.format = PIXELFORMAT_RGBA32_UINT;

		uint32 *rgba[4] = {nullptr};
		getEXRChannels(exrHeader, exrImage, rgba);

		try
		{
			img.data = (unsigned char *) readEXRChannels(img.width, img.height, rgba, 1u);
		}
		catch (love::Exception &)
		{
			FreeEXRHeader(&exrHeader);
			FreeEXRImage(&exrImage);
			throw;
		}
	}
	else if (pixelType == TINYEXR_PIXELTYPE_HALF)
	{
		img.format = PIXELFORMAT_RGBA16_FLOAT;

		float16 *rgba[4] = {nullptr};
		getEXRChannels(exrHeader, exrImage, rgba);

		try
		{
			img.data = (unsigned char *) readEXRChannels(img.width, img.height, rgba, float32to16(1.0f));
		}
		catch (love::Exception &)
		{
			FreeEXRHeader(&exrHeader);
			FreeEXRImage(&exrImage);
			throw;
		}
	}
	else if (pixelType == TINYEXR_PIXELTYPE_FLOAT)
	{
		img.format = PIXELFORMAT_RGBA32_FLOAT;

		float *rgba[4] = {nullptr};
		getEXRChannels(exrHeader, exrImage, rgba);

		try
		{
			img.data = (unsigned char *) readEXRChannels(img.width, img.height, rgba, 1.0f);
		}
		catch (love::Exception &)
		{
			FreeEXRHeader(&exrHeader);
			FreeEXRImage(&exrImage);
			throw;
		}
	}
	else
	{
		FreeEXRHeader(&exrHeader);
		FreeEXRImage(&exrImage);
		throw love::Exception("Could not decode EXR image: unknown pixel format.");
	}

	img.size = getPixelFormatSliceSize(img.format, img.width, img.height);

	FreeEXRHeader(&exrHeader);
	FreeEXRImage(&exrImage);

	return img;
}

FormatHandler::EncodedImage EXRHandler::encode(const DecodedImage &img, EncodedFormat encodedFormat)
{
	if (!canEncode(img.format, encodedFormat))
	{
		if (encodedFormat != ENCODED_EXR)
			throw love::Exception("EXR encoder cannot encode to non-EXR format.");
		else
			throw love::Exception("EXR encoder cannot encode the given pixel format.");
	}

	const auto &formatinfo = getPixelFormatInfo(img.format);

	EXRHeader exrHeader;
	InitEXRHeader(&exrHeader);

	exrHeader.num_channels = formatinfo.components;

	// TODO: this could be configurable.
	exrHeader.compression_type = TINYEXR_COMPRESSIONTYPE_ZIP;

	// TinyEXR expects malloc here because FreeEXRHeader uses free().
	exrHeader.channels = (EXRChannelInfo *) malloc(sizeof(EXRChannelInfo) * exrHeader.num_channels);
	exrHeader.pixel_types = (int *) malloc(sizeof(int) * exrHeader.num_channels);
	exrHeader.requested_pixel_types = (int *) malloc(sizeof(int) * exrHeader.num_channels);

	int pixeltype = -1;
	if (formatinfo.dataType == PIXELFORMATTYPE_UINT)
	{
		pixeltype = TINYEXR_PIXELTYPE_UINT;
	}
	else if (formatinfo.dataType == PIXELFORMATTYPE_SFLOAT)
	{
		if (formatinfo.blockSize / formatinfo.components == 2)
			pixeltype = TINYEXR_PIXELTYPE_HALF;
		else if (formatinfo.blockSize / formatinfo.components == 4)
			pixeltype = TINYEXR_PIXELTYPE_FLOAT;
	}

	if (pixeltype == -1)
	{
		FreeEXRHeader(&exrHeader);
		throw love::Exception("Cannot convert the given pixel format to an EXR pixel type.");
	}

	// EXR parsers tend to only handle (A)BGR order,
	// the spec says channels should be stored alphabetically.
	const int channelmappings[4][4] = {
		{0},
		{1, 0},
		{2, 1, 0},
		{3, 2, 1, 0},
	};

	const int *channelmapping = channelmappings[exrHeader.num_channels - 1];

	for (int i = 0; i < exrHeader.num_channels; i++)
	{
		exrHeader.channels[i] = EXRChannelInfo();

		const char names[] = {'R', 'G', 'B', 'A'};
		exrHeader.channels[i].name[0] = names[channelmapping[i]];

		exrHeader.pixel_types[i] = pixeltype;
		exrHeader.requested_pixel_types[i] = pixeltype;
	}

	EXRImage exrImage;
	InitEXRImage(&exrImage);

	exrImage.width = img.width;
	exrImage.height = img.height;
	exrImage.num_channels = exrHeader.num_channels;

	exrImage.images = (unsigned char **) malloc(sizeof(unsigned char *) * exrImage.num_channels);

	for (int i = 0; i < exrImage.num_channels; i++)
	{
		size_t componentsize = pixeltype == TINYEXR_PIXELTYPE_HALF ? 2 : 4;

		exrImage.images[i] = (unsigned char *) malloc(img.width * img.height * componentsize);

		if (exrImage.images[i] == nullptr)
		{
			FreeEXRHeader(&exrHeader);
			FreeEXRImage(&exrImage);
			throw love::Exception("Out of memory.");
		}
	}

	if (pixeltype == TINYEXR_PIXELTYPE_UINT)
	{
		writeEXRChannels(img.width, img.height, formatinfo.components, channelmapping, (const uint32 *) img.data, (uint32 **) exrImage.images);
	}
	else if (pixeltype == TINYEXR_PIXELTYPE_HALF)
	{
		writeEXRChannels(img.width, img.height, formatinfo.components, channelmapping, (const float16 *) img.data, (float16 **) exrImage.images);
	}
	else if (pixeltype == TINYEXR_PIXELTYPE_FLOAT)
	{
		writeEXRChannels(img.width, img.height, formatinfo.components, channelmapping, (const float *) img.data, (float **) exrImage.images);
	}

	EncodedImage encimg;

	const char *err = nullptr;
	encimg.size = SaveEXRImageToMemory(&exrImage, &exrHeader, &encimg.data, &err);

	FreeEXRHeader(&exrHeader);
	FreeEXRImage(&exrImage);

	std::string errstring;
	if (err != nullptr)
	{
		errstring = err;
		FreeEXRErrorMessage(err);
	}

	if (encimg.size == 0)
	{
		if (!errstring.empty())
			throw love::Exception("Could not encode EXR image: %s", errstring.c_str());
		else
			throw love::Exception("Could not encode EXR image");
	}

	return encimg;
}

void EXRHandler::freeRawPixels(unsigned char *mem)
{
	delete[] mem;
}

void EXRHandler::freeEncodedImage(unsigned char *mem)
{
	// SaveEXRImageToMemory uses malloc.
	if (mem != nullptr)
		::free(mem);
}

} // magpie
} // image
} // love
