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
}

void CompressedData::load(love::filesystem::FileData *data)
{
	std::vector<SubImage> imageMipmaps;
	TextureType textype = TYPE_UNKNOWN;

	if (ddsHandler::canParse(data))
		textype = ddsHandler::parse(data, imageMipmaps);

	if (textype == TYPE_UNKNOWN)
		throw (love::Exception("Could not parse compressed data: Unknown format."));

	dataImages = imageMipmaps;
	type = textype;
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
