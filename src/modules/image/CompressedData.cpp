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

CompressedData::CompressedData()
	: type(TYPE_UNKNOWN)
{
}

CompressedData::~CompressedData()
{
}

int CompressedData::getSize() const
{
	// Adding up the total size for all mipmap levels would make more sense, but
	// it's probably better for getSize() to match getData() so no bad memory
	// accesses happen...
	if (dataImages.size() > 0)
		return dataImages[0].size;
	else
		return 0;
}

void *CompressedData::getData() const
{
	// Data for different mipmap levels is not stored contiguously in memory, so
	// getData() won't work properly for CompressedData.
	if (dataImages.size() > 0 && dataImages[0].size > 0)
		return (void *) &(dataImages[0].data[0]);
	else
		return 0;
}

int CompressedData::getMipmapCount() const
{
	return dataImages.size();
}

int CompressedData::getSize(int miplevel) const
{
	checkMipmapLevelExists(miplevel);

	return dataImages[miplevel].size;
}

void *CompressedData::getData(int miplevel) const
{
	checkMipmapLevelExists(miplevel);

	return (void *) &dataImages[miplevel].data[0];
}

int CompressedData::getWidth(int miplevel) const
{
	checkMipmapLevelExists(miplevel);

	return dataImages[miplevel].width;
}

int CompressedData::getHeight(int miplevel) const
{
	checkMipmapLevelExists(miplevel);

	return dataImages[miplevel].height;
}

CompressedData::TextureType CompressedData::getType() const
{
	return type;
}

void CompressedData::checkMipmapLevelExists(int miplevel) const
{
	if (miplevel < 0 || miplevel >= dataImages.size())
		throw love::Exception("Mipmap level %d does not exist", miplevel);
}

bool CompressedData::getConstant(const char *in, CompressedData::TextureType &out)
{
	return types.find(in, out);
}

bool CompressedData::getConstant(CompressedData::TextureType in, const char *&out)
{
	return types.find(in, out);
}

StringMap<CompressedData::TextureType, CompressedData::TYPE_MAX_ENUM>::Entry CompressedData::typeEntries[] =
{
	{"unknown", CompressedData::TYPE_UNKNOWN},
	{"dxt1", CompressedData::TYPE_DXT1},
	{"dxt3", CompressedData::TYPE_DXT3},
	{"dxt5", CompressedData::TYPE_DXT5},
	{"bc5", CompressedData::TYPE_BC5},
	{"bc5s", CompressedData::TYPE_BC5s},
	{"bc7", CompressedData::TYPE_BC7},
	{"bc7srgb", CompressedData::TYPE_BC7srgb},
};

StringMap<CompressedData::TextureType, CompressedData::TYPE_MAX_ENUM> CompressedData::types(CompressedData::typeEntries, sizeof(CompressedData::typeEntries));

} // image
} // love
