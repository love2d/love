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
	: type(TYPE_MAX_ENUM)
{
	
}

CompressedData::~CompressedData()
{
	
}

int CompressedData::getSize() const
{
	size_t totalsize = sizeof(MipmapInfo) * dataMipmapInfo.size();

	for (size_t i = 0; i < dataMipmapInfo.size(); i++)
		totalsize += dataMipmapInfo[i].size;

	return totalsize;
}

void *CompressedData::getData() const
{
	// ?
	return (void *) &dataMipmapInfo[0].data[0];
}

int CompressedData::getNumMipmaps() const
{
	return dataMipmapInfo.size();
}

int CompressedData::getSize(int miplevel) const
{
	checkMipmapLevelExists(miplevel);

	return dataMipmapInfo[miplevel].size;
}

void *CompressedData::getData(int miplevel) const
{
	checkMipmapLevelExists(miplevel);

	return (void *) &dataMipmapInfo[miplevel].data[0];
}

int CompressedData::getWidth(int miplevel) const
{
	checkMipmapLevelExists(miplevel);

	return dataMipmapInfo[miplevel].width;
}

int CompressedData::getHeight(int miplevel) const
{
	checkMipmapLevelExists(miplevel);

	return dataMipmapInfo[miplevel].height;
}

CompressedData::TextureType CompressedData::getType() const
{
	return type;
}

void CompressedData::checkMipmapLevelExists(int miplevel) const
{
	if (miplevel < 0 || miplevel >= dataMipmapInfo.size())
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
	{"dxt1", CompressedData::TYPE_DXT1},
	{"dxt3", CompressedData::TYPE_DXT3},
	{"dxt5", CompressedData::TYPE_DXT5},
	{"bc5s", CompressedData::TYPE_BC5s},
	{"bc5", CompressedData::TYPE_BC5},
	{"bc7", CompressedData::TYPE_BC7},
	{"bc7srgb", CompressedData::TYPE_BC7srgb},
};

StringMap<CompressedData::TextureType, CompressedData::TYPE_MAX_ENUM> CompressedData::types(CompressedData::typeEntries, sizeof(CompressedData::typeEntries));

} // image
} // love
