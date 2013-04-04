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

#include "CompressedData.h"

namespace love
{
namespace image
{
namespace devil
{

CompressedData::CompressedData(Data *data)
{
	load(data);
}

CompressedData::CompressedData(love::filesystem::File *file)
{
	Data *data = file->read();
	try
	{
		load(data);
	}
	catch (love::Exception &)
	{
		data->release();
		throw;
	}
}

CompressedData::~CompressedData()
{
	
}

bool CompressedData::convertFormat(dds::Format ddsformat)
{
	switch (ddsformat)
	{
	case dds::FORMAT_DXT1:
		type = TYPE_DXT1;
		break;
	case dds::FORMAT_DXT3:
		type = TYPE_DXT3;
		break;
	case dds::FORMAT_DXT5:
		type = TYPE_DXT5;
		break;
	case dds::FORMAT_BC5s:
		type = TYPE_BC5s;
		break;
	case dds::FORMAT_BC5u:
		type = TYPE_BC5u;
		break;
	case dds::FORMAT_BC7:
		type = TYPE_BC7;
		break;
	case dds::FORMAT_BC7srgb:
		type = TYPE_BC7srgb;
		break;
	default:
		return false;
	}

	return true;
}

void CompressedData::load(Data *data)
{
	if (!dds::Parser::isDDS(data->getData(), data->getSize()))
		throw love::Exception("Could not decode compressed data (not a DDS file?)");

	try
	{
		dds::Parser parser(data->getData(), data->getSize());

		dds::Format format = parser.getFormat();

		if (format == dds::FORMAT_UNKNOWN || !convertFormat(format))
			throw love::Exception("Could not parse compressed data: Unsupported format.");

		if (parser.getNumMipmaps() == 0)
			throw love::Exception("Could not parse compressed data: No readable texture data.");

		for (size_t i = 0; i < parser.getNumMipmaps(); i++)
		{
			const dds::Parser::Image *img = parser.getImageData(i);

			MipmapInfo mip;

			mip.width = img->width;
			mip.height = img->height;
			mip.size = img->dataSize;

			mip.data.resize(mip.size);
			memcpy(&mip.data[0], img->data, mip.size);

			dataMipmapInfo.push_back(mip);
		}
		
	}
	catch (std::exception &e)
	{
		throw love::Exception(e.what());
	}
}

bool CompressedData::isCompressed(const Data *data)
{
	return dds::Parser::isDDS(data->getData(), data->getSize());
}

} // devil
} // image
} // love
