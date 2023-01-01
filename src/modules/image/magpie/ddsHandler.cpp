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

#include "ddsHandler.h"
#include "common/Exception.h"
#include "image/ImageData.h"

// dds parser
#include "ddsparse/ddsparse.h"

namespace love
{
namespace image
{
namespace magpie
{

static PixelFormat convertFormat(dds::dxinfo::DXGIFormat dxformat, bool &sRGB, bool &bgra)
{
	using namespace dds::dxinfo;

	sRGB = false;
	bgra = false;

	switch (dxformat)
	{
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
		return PIXELFORMAT_RGBA32F;

	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
		return PIXELFORMAT_RGBA16F;

	case DXGI_FORMAT_R16G16B16A16_UNORM:
		return PIXELFORMAT_RGBA16;

	case DXGI_FORMAT_R32G32_TYPELESS:
	case DXGI_FORMAT_R32G32_FLOAT:
		return PIXELFORMAT_RG32F;

	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
	case DXGI_FORMAT_R10G10B10A2_UNORM:
		return PIXELFORMAT_RGB10A2;

	case DXGI_FORMAT_R11G11B10_FLOAT:
		return PIXELFORMAT_RG11B10F;

	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		sRGB = (dxformat == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
		return PIXELFORMAT_RGBA8;

	case DXGI_FORMAT_R16G16_TYPELESS:
	case DXGI_FORMAT_R16G16_FLOAT:
		return PIXELFORMAT_RG16F;

	case DXGI_FORMAT_R16G16_UNORM:
		return PIXELFORMAT_RG16;

	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_R32_FLOAT:
		return PIXELFORMAT_R32F;

	case DXGI_FORMAT_R8G8_TYPELESS:
	case DXGI_FORMAT_R8G8_UNORM:
		return PIXELFORMAT_RG8;

	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_R16_FLOAT:
		return PIXELFORMAT_R16F;

	case DXGI_FORMAT_R16_UNORM:
		return PIXELFORMAT_R16;

	case DXGI_FORMAT_R8_TYPELESS:
	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_A8_UNORM:
		return PIXELFORMAT_R8;

	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
		sRGB = (dxformat == DXGI_FORMAT_BC1_UNORM_SRGB);
		return PIXELFORMAT_DXT1;

	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
		sRGB = (dxformat == DXGI_FORMAT_BC2_UNORM_SRGB);
		return PIXELFORMAT_DXT3;

	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
		sRGB = (dxformat == DXGI_FORMAT_BC3_UNORM_SRGB);
		return PIXELFORMAT_DXT5;

	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
		return PIXELFORMAT_BC4;

	case DXGI_FORMAT_BC4_SNORM:
		return PIXELFORMAT_BC4s;

	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
		return PIXELFORMAT_BC5;

	case DXGI_FORMAT_BC5_SNORM:
		return PIXELFORMAT_BC5s;

	case DXGI_FORMAT_B5G6R5_UNORM:
		return PIXELFORMAT_RGB565;

	case DXGI_FORMAT_B5G5R5A1_UNORM:
		return PIXELFORMAT_RGB5A1;

	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		sRGB = (dxformat == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);
		bgra = true;
		return PIXELFORMAT_RGBA8;

	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC6H_UF16:
		return PIXELFORMAT_BC6H;

	case DXGI_FORMAT_BC6H_SF16:
		return PIXELFORMAT_BC6Hs;

	case DXGI_FORMAT_BC7_TYPELESS:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		sRGB = (dxformat == DXGI_FORMAT_BC7_UNORM_SRGB);
		return PIXELFORMAT_BC7;

	default:
		return PIXELFORMAT_UNKNOWN;
	}
}

bool DDSHandler::canDecode(Data *data)
{
	using namespace dds::dxinfo;

	DXGIFormat dxformat = dds::getDDSPixelFormat(data->getData(), data->getSize());
	bool isSRGB = false;
	bool bgra = false;
	PixelFormat format = convertFormat(dxformat, isSRGB, bgra);

	return ImageData::validPixelFormat(format);
}

FormatHandler::DecodedImage DDSHandler::decode(Data *data)
{
	DecodedImage img;

	dds::Parser parser(data->getData(), data->getSize());

	bool isSRGB = false;
	bool bgra = false;
	img.format = convertFormat(parser.getFormat(), isSRGB, bgra);

	if (!ImageData::validPixelFormat(img.format))
		throw love::Exception("Could not parse DDS pixel data: Unsupported format.");

	if (parser.getMipmapCount() == 0)
		throw love::Exception("Could not parse DDS pixel data: No readable texture data.");

	// We only support the top mip level through this codepath.
	const dds::Image *ddsimg = parser.getImageData(0);

	try
	{
		img.data = new uint8[ddsimg->dataSize];
	}
	catch (std::exception &)
	{
		throw love::Exception("Out of memory.");
	}

	memcpy(img.data, ddsimg->data, ddsimg->dataSize);

	img.size = ddsimg->dataSize;
	img.width = ddsimg->width;
	img.height = ddsimg->height;

	// Swap red and blue channels for incoming BGRA data.
	if (bgra)
	{
		for (int y = 0; y < img.height; y++)
		{
			for (int x = 0; x < img.width; x++)
			{
				size_t offset = (y * img.width + x) * 4;
				uint8 b = img.data[offset + 0];
				uint8 r = img.data[offset + 2];
				img.data[offset + 0] = r;
				img.data[offset + 2] = b;
			}
		}
	}

	return img;
}

bool DDSHandler::canParseCompressed(Data *data)
{
	return dds::isCompressedDDS(data->getData(), data->getSize());
}

StrongRef<CompressedMemory> DDSHandler::parseCompressed(Data *filedata, std::vector<StrongRef<CompressedSlice>> &images, PixelFormat &format, bool &sRGB)
{
	if (!dds::isCompressedDDS(filedata->getData(), filedata->getSize()))
		throw love::Exception("Could not decode compressed data (not a DDS file?)");

	PixelFormat texformat = PIXELFORMAT_UNKNOWN;
	bool isSRGB = false;
	bool bgra = false;

	StrongRef<CompressedMemory> memory;
	size_t dataSize = 0;

	images.clear();

	// Attempt to parse the dds file.
	dds::Parser parser(filedata->getData(), filedata->getSize());

	texformat = convertFormat(parser.getFormat(), isSRGB, bgra);

	if (texformat == PIXELFORMAT_UNKNOWN)
		throw love::Exception("Could not parse compressed data: Unsupported format.");

	if (parser.getMipmapCount() == 0)
		throw love::Exception("Could not parse compressed data: No readable texture data.");

	// Calculate the size of the block of memory we're returning.
	for (size_t i = 0; i < parser.getMipmapCount(); i++)
	{
		const dds::Image *img = parser.getImageData(i);
		dataSize += img->dataSize;
	}

	memory.set(new CompressedMemory(dataSize), Acquire::NORETAIN);

	size_t dataOffset = 0;

	// Copy the parsed mipmap levels from the FileData to our CompressedImageData.
	for (size_t i = 0; i < parser.getMipmapCount(); i++)
	{
		// Fetch the data for this mipmap level.
		const dds::Image *img = parser.getImageData(i);

		// Copy the mipmap image from the FileData to our block of memory.
		memcpy(memory->data + dataOffset, img->data, img->dataSize);

		auto slice = new CompressedSlice(texformat, img->width, img->height, memory, dataOffset, img->dataSize);
		images.emplace_back(slice, Acquire::NORETAIN);

		dataOffset += img->dataSize;
	}

	format = texformat;
	sRGB = isSRGB;
	return memory;
}

} // magpie
} // image
} // love
