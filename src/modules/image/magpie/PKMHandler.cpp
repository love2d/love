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

// LOVE
#include "PKMHandler.h"
#include "common/int.h"

namespace love
{
namespace image
{
namespace magpie
{

namespace
{

// Big endian to host (and vice versa.)
inline uint16 swap16big(uint16 x)
{
#ifdef LOVE_BIG_ENDIAN
	return x;
#else
	return swapuint16(x);
#endif
}

static const uint8 pkmIdentifier[] = {'P','K','M',' '};

struct PKMHeader
{
	uint8 identifier[4];
	uint8 version[2];
	uint16 textureFormatBig;
	uint16 extendedWidthBig;
	uint16 extendedHeightBig;
	uint16 widthBig;
	uint16 heightBig;
};

enum PKMTextureFormat
{
	ETC1_RGB_NO_MIPMAPS = 0,
	ETC2PACKAGE_RGB_NO_MIPMAPS,
	ETC2PACKAGE_RGBA_NO_MIPMAPS_OLD,
	ETC2PACKAGE_RGBA_NO_MIPMAPS,
	ETC2PACKAGE_RGBA1_NO_MIPMAPS,
	ETC2PACKAGE_R_NO_MIPMAPS,
	ETC2PACKAGE_RG_NO_MIPMAPS,
	ETC2PACKAGE_R_SIGNED_NO_MIPMAPS,
	ETC2PACKAGE_RG_SIGNED_NO_MIPMAPS
};

static CompressedImageData::Format convertFormat(uint16 texformat)
{
	switch (texformat)
	{
	case ETC1_RGB_NO_MIPMAPS:
		return CompressedImageData::FORMAT_ETC1;
	case ETC2PACKAGE_RGB_NO_MIPMAPS:
		return CompressedImageData::FORMAT_ETC2_RGB;
	case ETC2PACKAGE_RGBA_NO_MIPMAPS_OLD:
	case ETC2PACKAGE_RGBA_NO_MIPMAPS:
		return CompressedImageData::FORMAT_ETC2_RGBA;
	case ETC2PACKAGE_RGBA1_NO_MIPMAPS:
		return CompressedImageData::FORMAT_ETC2_RGBA1;
	case ETC2PACKAGE_R_NO_MIPMAPS:
		return CompressedImageData::FORMAT_EAC_R;
	case ETC2PACKAGE_RG_NO_MIPMAPS:
		return CompressedImageData::FORMAT_EAC_RG;
	case ETC2PACKAGE_R_SIGNED_NO_MIPMAPS:
		return CompressedImageData::FORMAT_EAC_Rs;
	case ETC2PACKAGE_RG_SIGNED_NO_MIPMAPS:
		return CompressedImageData::FORMAT_EAC_RGs;
	default:
		return CompressedImageData::FORMAT_UNKNOWN;
	}
}

} // Anonymous namespace.

bool PKMHandler::canParse(const filesystem::FileData *data)
{
	if (data->getSize() <= sizeof(PKMHeader))
		return false;

	const PKMHeader *header = (const PKMHeader *) data->getData();

	if (memcmp(header->identifier, pkmIdentifier, 4) != 0)
		return false;

	// At the time of this writing, only v1.0 and v2.0 exist.
	if ((header->version[0] != '2' && header->version[0] != '1') || header->version[1] != '0')
		return false;

	return true;
}

uint8 *PKMHandler::parse(filesystem::FileData *filedata, std::vector<CompressedImageData::SubImage> &images, size_t &dataSize, CompressedImageData::Format &format, bool &sRGB)
{
	if (!canParse(filedata))
		throw love::Exception("Could not decode compressed data (not a PKM file?)");

	PKMHeader header = *(const PKMHeader *) filedata->getData();

	header.textureFormatBig = swap16big(header.textureFormatBig);
	header.extendedWidthBig = swap16big(header.extendedWidthBig);
	header.extendedHeightBig = swap16big(header.extendedHeightBig);
	header.widthBig = swap16big(header.widthBig);
	header.heightBig = swap16big(header.heightBig);

	CompressedImageData::Format cformat = convertFormat(header.textureFormatBig);

	if (cformat == CompressedImageData::FORMAT_UNKNOWN)
		throw love::Exception("Could not parse PKM file: unsupported texture format.");

	// The rest of the file after the header is all texture data.
	size_t totalsize = filedata->getSize() - sizeof(PKMHeader);
	uint8 *data = nullptr;

	try
	{
		data = new uint8[totalsize];
	}
	catch (std::bad_alloc &)
	{
		throw love::Exception("Out of memory.");
	}

	// PKM files only store a single mipmap level.
	memcpy(data, (uint8 *) filedata->getData() + sizeof(PKMHeader), totalsize);

	CompressedImageData::SubImage mip;

	// TODO: verify whether glCompressedTexImage works properly with the unpadded
	// width and height values (extended == padded.)
	mip.width = header.widthBig;
	mip.height = header.heightBig;

	mip.size = totalsize;
	mip.data = data;

	images.push_back(mip);

	dataSize = totalsize;
	format = cformat;
	sRGB = false;

	return data;
}

} // magpie
} // image
} // love

