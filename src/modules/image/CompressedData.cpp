/**
 * Copyright (c) 2006-2014 LOVE Development Team
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
	, data(0)
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
	return dataImages.size();
}

size_t CompressedData::getSize(int miplevel) const
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

CompressedData::Format CompressedData::getFormat() const
{
	return format;
}

void CompressedData::checkMipmapLevelExists(int miplevel) const
{
	if (miplevel < 0 || miplevel >= (int) dataImages.size())
		throw love::Exception("Mipmap level %d does not exist", miplevel);
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
	{"dxt1", CompressedData::FORMAT_DXT1},
	{"dxt3", CompressedData::FORMAT_DXT3},
	{"dxt5", CompressedData::FORMAT_DXT5},
	{"bc4", CompressedData::FORMAT_BC4},
	{"bc4s", CompressedData::FORMAT_BC4s},
	{"bc5", CompressedData::FORMAT_BC5},
	{"bc5s", CompressedData::FORMAT_BC5s},
};

StringMap<CompressedData::Format, CompressedData::FORMAT_MAX_ENUM> CompressedData::formats(CompressedData::formatEntries, sizeof(CompressedData::formatEntries));

} // image
} // love
