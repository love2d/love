/**
 * Copyright (c) 2006-2013 LOVE Development Team
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

bool ddsHandler::canParse(const filesystem::FileData *data)
{
	if (data->getExtension().compare("dds") != 0)
		return false;

	return dds::isCompressedDDS(data->getData(), data->getSize());
}

CompressedData::TextureType ddsHandler::parse(filesystem::FileData *data, std::vector<CompressedData::SubImage> &images)
{
	if (!dds::isDDS(data->getData(), data->getSize()))
		throw love::Exception("Could not decode compressed data (not a DDS file?)");

	CompressedData::TextureType textype = CompressedData::TYPE_UNKNOWN;

	try
	{
		dds::Parser parser(data->getData(), data->getSize());

		textype = convertFormat(parser.getFormat());

		if (textype == CompressedData::TYPE_UNKNOWN)
			throw love::Exception("Could not parse compressed data: Unsupported format.");

		if (parser.getMipmapCount() == 0)
			throw love::Exception("Could not parse compressed data: No readable texture data.");

		for (size_t i = 0; i < parser.getMipmapCount(); i++)
		{
			const dds::Image *img = parser.getImageData(i);

			CompressedData::SubImage mip;

			mip.width = img->width;
			mip.height = img->height;
			mip.size = img->dataSize;
			mip.data.insert(mip.data.begin(), &img->data[0], &img->data[mip.size]);

			images.push_back(mip);
		}
	}
	catch (std::exception &e)
	{
		throw love::Exception(e.what());
	}

	return textype;
}

CompressedData::TextureType ddsHandler::convertFormat(dds::Format ddsformat)
{
	switch (ddsformat)
	{
	case dds::FORMAT_DXT1:
		return CompressedData::TYPE_DXT1;
	case dds::FORMAT_DXT3:
		return CompressedData::TYPE_DXT3;
	case dds::FORMAT_DXT5:
		return CompressedData::TYPE_DXT5;
	case dds::FORMAT_BC5:
		return CompressedData::TYPE_BC5;
	case dds::FORMAT_BC5s:
		return CompressedData::TYPE_BC5s;
	default:
		return CompressedData::TYPE_UNKNOWN;
	}

	return CompressedData::TYPE_UNKNOWN;
}

} // magpie
} // image
} // love
