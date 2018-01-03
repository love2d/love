/**
 * Copyright (c) 2006-2018 LOVE Development Team
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

namespace love
{
namespace image
{
namespace magpie
{

bool DDSHandler::canParseCompressed(Data *data)
{
	return dds::isCompressedDDS(data->getData(), data->getSize());
}

StrongRef<CompressedMemory> DDSHandler::parseCompressed(Data *filedata, std::vector<StrongRef<CompressedSlice>> &images, PixelFormat &format, bool &sRGB)
{
	if (!dds::isDDS(filedata->getData(), filedata->getSize()))
		throw love::Exception("Could not decode compressed data (not a DDS file?)");

	PixelFormat texformat = PIXELFORMAT_UNKNOWN;
	bool isSRGB = false;

	StrongRef<CompressedMemory> memory;
	size_t dataSize = 0;

	images.clear();

	// Attempt to parse the dds file.
	dds::Parser parser(filedata->getData(), filedata->getSize());

	texformat = convertFormat(parser.getFormat(), isSRGB);

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

PixelFormat DDSHandler::convertFormat(dds::Format ddsformat, bool &sRGB)
{
	sRGB = false;

	switch (ddsformat)
	{
	case dds::FORMAT_DXT1:
		return PIXELFORMAT_DXT1;
	case dds::FORMAT_DXT3:
		return PIXELFORMAT_DXT3;
	case dds::FORMAT_DXT5:
		return PIXELFORMAT_DXT5;
	case dds::FORMAT_BC4:
		return PIXELFORMAT_BC4;
	case dds::FORMAT_BC4s:
		return PIXELFORMAT_BC4s;
	case dds::FORMAT_BC5:
		return PIXELFORMAT_BC5;
	case dds::FORMAT_BC5s:
		return PIXELFORMAT_BC5s;
	case dds::FORMAT_BC6H:
		return PIXELFORMAT_BC6H;
	case dds::FORMAT_BC6Hs:
		return PIXELFORMAT_BC6Hs;
	case dds::FORMAT_BC7:
		return PIXELFORMAT_BC7;
	case dds::FORMAT_BC7srgb:
		sRGB = true;
		return PIXELFORMAT_BC7;
	default:
		return PIXELFORMAT_UNKNOWN;
	}
}

} // magpie
} // image
} // love
