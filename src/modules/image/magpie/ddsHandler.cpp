/**
 * Copyright (c) 2006-2015 LOVE Development Team
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

namespace love
{
namespace image
{
namespace magpie
{

bool DDSHandler::canParse(const filesystem::FileData *data)
{
	return dds::isCompressedDDS(data->getData(), data->getSize());
}

uint8 *DDSHandler::parse(filesystem::FileData *filedata, std::vector<CompressedData::SubImage> &images, size_t &dataSize, CompressedData::Format &format)
{
	if (!dds::isDDS(filedata->getData(), filedata->getSize()))
		throw love::Exception("Could not decode compressed data (not a DDS file?)");

	CompressedData::Format texformat = CompressedData::FORMAT_UNKNOWN;

	uint8 *data = nullptr;
	dataSize = 0;
	images.clear();

	try
	{
		// Attempt to parse the dds file.
		dds::Parser parser(filedata->getData(), filedata->getSize());

		texformat = convertFormat(parser.getFormat());

		if (texformat == CompressedData::FORMAT_UNKNOWN)
			throw love::Exception("Could not parse compressed data: Unsupported format.");

		if (parser.getMipmapCount() == 0)
			throw love::Exception("Could not parse compressed data: No readable texture data.");

		// Calculate the size of the block of memory we're returning.
		for (size_t i = 0; i < parser.getMipmapCount(); i++)
		{
			const dds::Image *img = parser.getImageData(i);
			dataSize += img->dataSize;
		}

		data = new uint8[dataSize];

		size_t dataOffset = 0;

		// Copy the parsed mipmap levels from the FileData to our CompressedData.
		for (size_t i = 0; i < parser.getMipmapCount(); i++)
		{
			// Fetch the data for this mipmap level.
			const dds::Image *img = parser.getImageData(i);

			CompressedData::SubImage mip;

			mip.width = img->width;
			mip.height = img->height;
			mip.size = img->dataSize;

			// Copy the mipmap image from the FileData to our block of memory.
			memcpy(data + dataOffset, img->data, mip.size);
			mip.data = data + dataOffset;

			dataOffset += mip.size;

			images.push_back(mip);
		}
	}
	catch (std::exception &e)
	{
		delete[] data;
		images.clear();
		throw love::Exception("%s", e.what());
	}

	format = texformat;
	return data;
}

CompressedData::Format DDSHandler::convertFormat(dds::Format ddsformat)
{
	switch (ddsformat)
	{
	case dds::FORMAT_DXT1:
		return CompressedData::FORMAT_DXT1;
	case dds::FORMAT_DXT3:
		return CompressedData::FORMAT_DXT3;
	case dds::FORMAT_DXT5:
		return CompressedData::FORMAT_DXT5;
	case dds::FORMAT_BC4:
		return CompressedData::FORMAT_BC4;
	case dds::FORMAT_BC4s:
		return CompressedData::FORMAT_BC4s;
	case dds::FORMAT_BC5:
		return CompressedData::FORMAT_BC5;
	case dds::FORMAT_BC5s:
		return CompressedData::FORMAT_BC5s;
	case dds::FORMAT_BC6H:
		return CompressedData::FORMAT_BC6H;
	case dds::FORMAT_BC6Hs:
		return CompressedData::FORMAT_BC6Hs;
	case dds::FORMAT_BC7:
		return CompressedData::FORMAT_BC7;
	case dds::FORMAT_BC7srgb:
		return CompressedData::FORMAT_BC7SRGB;
	default:
		return CompressedData::FORMAT_UNKNOWN;
	}
}

} // magpie
} // image
} // love
