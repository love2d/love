/**
 * Copyright (c) 2006-2016 LOVE Development Team
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

#include "CompressedImageData.h"

namespace love
{
namespace image
{
namespace magpie
{

CompressedImageData::CompressedImageData(std::list<CompressedFormatHandler *> formats, love::filesystem::FileData *filedata)
{
	CompressedFormatHandler *parser = nullptr;

	for (CompressedFormatHandler *handler : formats)
	{
		if (handler->canParse(filedata))
		{
			parser = handler;
			break;
		}
	}

	if (parser == nullptr)
		throw love::Exception("Could not parse compressed data: Unknown format.");

	data = parser->parse(filedata, dataImages, dataSize, format, sRGB);

	if (data == nullptr)
		throw love::Exception("Could not parse compressed data.");

	if (format == FORMAT_UNKNOWN)
	{
		delete[] data;
		throw love::Exception("Could not parse compressed data: Unknown format.");
	}

	if (dataImages.size() == 0 || dataSize == 0)
	{
		delete[] data;
		throw love::Exception("Could not parse compressed data: No valid data?");
	}
}

CompressedImageData::~CompressedImageData()
{
	delete[] data;
}

} // magpie
} // image
} // love
