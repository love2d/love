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

#include "CompressedData.h"

namespace love
{
namespace image
{

CompressedData::CompressedData()
	: format(FORMAT_UNKNOWN)
	, sRGB(false)
	, data(nullptr)
	, dataSize(0)
{
}

CompressedData::~CompressedData()
{
}

size_t CompressedData::getSize() const
{
	return dataSize;
}

void *CompressedData::getData() const
{
	return data;
}

int CompressedData::getMipmapCount() const
{
	return (int) dataImages.size();
}

size_t CompressedData::getSize(int miplevel) const
{
	checkMipmapLevelExists(miplevel);

	return dataImages[miplevel].size;
}

void *CompressedData::getData(int miplevel) const
{
	checkMipmapLevelExists(miplevel);

	return &dataImages[miplevel].data[0];
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

CompressedData::Format CompressedData::getFormat() const
{
	return format;
}

bool CompressedData::isSRGB() const
{
	return sRGB;
}

void CompressedData::checkMipmapLevelExists(int miplevel) const
{
	if (miplevel < 0 || miplevel >= (int) dataImages.size())
		throw love::Exception("Mipmap level %d does not exist", miplevel + 1);
}

bool CompressedData::getConstant(const char *in, CompressedData::Format &out)
{
	return formats.find(in, out);
}

bool CompressedData::getConstant(CompressedData::Format in, const char *&out)
{
	return formats.find(in, out);
}

StringMap<CompressedData::Format, CompressedData::FORMAT_MAX_ENUM>::Entry CompressedData::formatEntries[] =
{
	{"unknown", CompressedData::FORMAT_UNKNOWN},
	{"DXT1", CompressedData::FORMAT_DXT1},
	{"DXT3", CompressedData::FORMAT_DXT3},
	{"DXT5", CompressedData::FORMAT_DXT5},
	{"BC4", CompressedData::FORMAT_BC4},
	{"BC4s", CompressedData::FORMAT_BC4s},
	{"BC5", CompressedData::FORMAT_BC5},
	{"BC5s", CompressedData::FORMAT_BC5s},
	{"BC6h", CompressedData::FORMAT_BC6H},
	{"BC6hs", CompressedData::FORMAT_BC6Hs},
	{"BC7", CompressedData::FORMAT_BC7},
	{"ETC1", CompressedData::FORMAT_ETC1},
	{"ETC2rgb", CompressedData::FORMAT_ETC2_RGB},
	{"ETC2rgba", CompressedData::FORMAT_ETC2_RGBA},
	{"ETC2rgba1", CompressedData::FORMAT_ETC2_RGBA1},
	{"EACr", CompressedData::FORMAT_EAC_R},
	{"EACrs", CompressedData::FORMAT_EAC_Rs},
	{"EACrg", CompressedData::FORMAT_EAC_RG},
	{"EACrgs", CompressedData::FORMAT_EAC_RGs},
	{"PVR1rgb2", CompressedData::FORMAT_PVR1_RGB2},
	{"PVR1rgb4", CompressedData::FORMAT_PVR1_RGB4},
	{"PVR1rgba2", CompressedData::FORMAT_PVR1_RGBA2},
	{"PVR1rgba4", CompressedData::FORMAT_PVR1_RGBA4},
};

StringMap<CompressedData::Format, CompressedData::FORMAT_MAX_ENUM> CompressedData::formats(CompressedData::formatEntries, sizeof(CompressedData::formatEntries));

} // image
} // love
