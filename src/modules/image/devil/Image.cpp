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

#include "Image.h"

#include "ImageData.h"
#include "CompressedData.h"

// DevIL
#include <IL/il.h>

namespace love
{
namespace image
{
namespace devil
{

const std::string Image::compressedExts[] =
{
	".dds", ""
};

Image::Image()
{
	ilInit();
	ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
	ilEnable(IL_ORIGIN_SET);
}

Image::~Image()
{
	ilShutDown();
}

const char *Image::getName() const
{
	return "love.image.devil";
}

love::image::ImageData *Image::newImageData(love::filesystem::File *file)
{
	return new ImageData(file);
}

love::image::ImageData *Image::newImageData(Data *data)
{
	return new ImageData(data);
}

love::image::ImageData *Image::newImageData(int width, int height)
{
	return new ImageData(width, height);
}

love::image::ImageData *Image::newImageData(int width, int height, void *data)
{
	return new ImageData(width, height, data);
}

love::image::CompressedData *Image::newCompressedData(love::filesystem::File *file)
{
	return new CompressedData(file);
}

love::image::CompressedData *Image::newCompressedData(love::Data *data)
{
	return new CompressedData(data);
}

bool Image::isCompressed(love::filesystem::File *file)
{
	bool hasExt = false;

	// Check whether the file has an extension known to contain compressed data.
	const std::string &ext = file->getExtension();
	for (int i = 0; !(compressedExts[i].empty()); i++)
	{
		if (compressedExts[i].compare(ext))
		{
			hasExt = true;
			break;
		}
	}

	if (!hasExt)
		return false;

	// Check whether the actual data is compressed.
	Data *data = file->read();
	bool compressed = CompressedData::isCompressed(data);
	data->release();

	return compressed;
}

bool Image::isCompressed(love::Data *data)
{
	return CompressedData::isCompressed(data);
}

} // devil
} // image
} // love
