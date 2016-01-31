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

CompressedImageData::CompressedImageData()
	: format(FORMAT_UNKNOWN)
	, sRGB(false)
	, data(nullptr)
	, dataSize(0)
{
}

CompressedImageData::~CompressedImageData()
{
}

size_t CompressedImageData::getSize() const
{
	return dataSize;
}

void *CompressedImageData::getData() const
{
	return data;
}

int CompressedImageData::getMipmapCount() const
{
	return (int) dataImages.size();
}

size_t CompressedImageData::getSize(int miplevel) const
{
	checkMipmapLevelExists(miplevel);

	return dataImages[miplevel].size;
}

void *CompressedImageData::getData(int miplevel) const
{
	checkMipmapLevelExists(miplevel);

	return &dataImages[miplevel].data[0];
}

int CompressedImageData::getWidth(int miplevel) const
{
	checkMipmapLevelExists(miplevel);

	return dataImages[miplevel].width;
}

int CompressedImageData::getHeight(int miplevel) const
{
	checkMipmapLevelExists(miplevel);

	return dataImages[miplevel].height;
}

CompressedImageData::Format CompressedImageData::getFormat() const
{
	return format;
}

bool CompressedImageData::isSRGB() const
{
	return sRGB;
}

void CompressedImageData::checkMipmapLevelExists(int miplevel) const
{
	if (miplevel < 0 || miplevel >= (int) dataImages.size())
		throw love::Exception("Mipmap level %d does not exist", miplevel + 1);
}

bool CompressedImageData::getConstant(const char *in, CompressedImageData::Format &out)
{
	return formats.find(in, out);
}

bool CompressedImageData::getConstant(CompressedImageData::Format in, const char *&out)
{
	return formats.find(in, out);
}

StringMap<CompressedImageData::Format, CompressedImageData::FORMAT_MAX_ENUM>::Entry CompressedImageData::formatEntries[] =
{
	{"unknown", FORMAT_UNKNOWN},
	{"DXT1", FORMAT_DXT1},
	{"DXT3", FORMAT_DXT3},
	{"DXT5", FORMAT_DXT5},
	{"BC4", FORMAT_BC4},
	{"BC4s", FORMAT_BC4s},
	{"BC5", FORMAT_BC5},
	{"BC5s", FORMAT_BC5s},
	{"BC6h", FORMAT_BC6H},
	{"BC6hs", FORMAT_BC6Hs},
	{"BC7", FORMAT_BC7},
	{"PVR1rgb2", FORMAT_PVR1_RGB2},
	{"PVR1rgb4", FORMAT_PVR1_RGB4},
	{"PVR1rgba2", FORMAT_PVR1_RGBA2},
	{"PVR1rgba4", FORMAT_PVR1_RGBA4},
	{"ETC1", FORMAT_ETC1},
	{"ETC2rgb", FORMAT_ETC2_RGB},
	{"ETC2rgba", FORMAT_ETC2_RGBA},
	{"ETC2rgba1", FORMAT_ETC2_RGBA1},
	{"EACr", FORMAT_EAC_R},
	{"EACrs", FORMAT_EAC_Rs},
	{"EACrg", FORMAT_EAC_RG},
	{"EACrgs", FORMAT_EAC_RGs},
	{"ASTC4x4", FORMAT_ASTC_4x4},
	{"ASTC5x4", FORMAT_ASTC_5x4},
	{"ASTC5x5", FORMAT_ASTC_5x5},
	{"ASTC6x5", FORMAT_ASTC_6x5},
	{"ASTC6x6", FORMAT_ASTC_6x6},
	{"ASTC8x5", FORMAT_ASTC_8x5},
	{"ASTC8x6", FORMAT_ASTC_8x6},
	{"ASTC8x8", FORMAT_ASTC_8x8},
	{"ASTC10x5", FORMAT_ASTC_10x5},
	{"ASTC10x6", FORMAT_ASTC_10x6},
	{"ASTC10x8", FORMAT_ASTC_10x8},
	{"ASTC10x10", FORMAT_ASTC_10x10},
	{"ASTC12x10", FORMAT_ASTC_12x10},
	{"ASTC12x12", FORMAT_ASTC_12x12},
};

StringMap<CompressedImageData::Format, CompressedImageData::FORMAT_MAX_ENUM> CompressedImageData::formats(CompressedImageData::formatEntries, sizeof(CompressedImageData::formatEntries));

} // image
} // love
