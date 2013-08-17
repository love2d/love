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

#include "ddsHandler.h"

namespace love
{
namespace image
{
namespace magpie
{

CompressedData::CompressedData(love::filesystem::FileData *data)
{
	load(data);
}

CompressedData::~CompressedData()
{
	// We have ownership of the heap memory in dataImages, so we have to free it.
	for (size_t i = 0; i < dataImages.size(); i++)
		delete[] dataImages[i].data;
}

void CompressedData::load(love::filesystem::FileData *data)
{
	// SubImage vector will be populated by a parser.
	std::vector<SubImage> parsedimages;
	Format texformat = FORMAT_UNKNOWN;

	if (ddsHandler::canParse(data))
		texformat = ddsHandler::parse(data, parsedimages);

	if (texformat == FORMAT_UNKNOWN)
		throw love::Exception("Could not parse compressed data: Unknown format.");

	if (parsedimages.size() == 0)
		throw love::Exception("Could not parse compressed data: No valid data?");

	// Make sure to clean up any previously loaded data.
	for (size_t i = 0; i < dataImages.size(); i++)
	{
		delete[] dataImages[i].data;
		dataImages[i].data = 0;
	}

	dataImages = parsedimages;
	format = texformat;
}

bool CompressedData::isCompressed(love::filesystem::FileData *data)
{
	if (ddsHandler::canParse(data))
		return true;

	return false;
}

} // magpie
} // image
} // love
