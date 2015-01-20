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

// LOVE
#include "PVRHandler.h"
#include "common/int.h"

// C++
#include <algorithm>

namespace love
{
namespace image
{
namespace magpie
{

namespace
{

// 'P' 'V' 'R' 3
static const uint32 PVRTEX3_IDENT = 0x03525650;
static const uint32 PVRTEX3_IDENT_REV = 0x50565203;

#pragma pack(push, 4)
struct PVRTexHeaderV3
{
	uint32 version;      /// Version of the file header, used to identify it.
	uint32 flags;        /// Various format flags.
	uint64 pixelFormat;  /// The pixel format, 8cc value storing the 4 channel identifiers and their respective sizes.
	uint32 colorSpace;   /// The Color Space of the texture, currently either linear RGB or sRGB.
	uint32 channelType;  /// Variable type that the channel is stored in. Supports signed/unsigned int/short/byte or float for now.
	uint32 height;       /// Height of the texture.
	uint32 width;        /// Width of the texture.
	uint32 depth;        /// Depth of the texture. (Z-slices)
	uint32 numSurfaces;  /// Number of members in a Texture Array.
	uint32 numFaces;     /// Number of faces in a Cube Map. Maybe be a value other than 6.
	uint32 numMipmaps;   /// Number of MIP Maps in the texture - NB: Includes top level.
	uint32 metaDataSize; /// Size of the accompanying meta data.
};
#pragma pack(pop)

enum PVRV3PixelFormat
{
	ePVRTPF_PVRTCI_2bpp_RGB = 0x00,
	ePVRTPF_PVRTCI_2bpp_RGBA,
	ePVRTPF_PVRTCI_4bpp_RGB,
	ePVRTPF_PVRTCI_4bpp_RGBA,
	ePVRTPF_PVRTCII_2bpp,
	ePVRTPF_PVRTCII_4bpp,
	ePVRTPF_ETC1 = 0x06,
	ePVRTPF_DXT1,
	ePVRTPF_DXT2,
	ePVRTPF_DXT3,
	ePVRTPF_DXT4,
	ePVRTPF_DXT5,
	ePVRTF_UNKNOWN_FORMAT = 0x7F
};

// 'P' 'V' 'R' '!'
static const uint32 PVRTEX2_IDENT = 0x21525650;
static const uint32 PVRTEX2_IDENT_REV = 0x50565221;

struct PVRTexHeaderV2
{
	uint32 headerSize;
	uint32 height;
	uint32 width;
	uint32 numMipmaps;
	uint32 flags;
	uint32 dataSize;
	uint32 bpp;
	uint32 bitmaskRed;
	uint32 bitmaskGreen;
	uint32 bitmaskBlue;
	uint32 bitmaskAlpha;
	uint32 pvrTag;
	uint32 numSurfaces;
};

// The legacy V2 pixel types we support.
enum PVRPixelTypeV2
{
	PixelTypePVRTC2 = 0x18,
	PixelTypePVRTC4,
	PixelTypePVRTCII2 = 0x1C,
	PixelTypePVRTCII4,
	PixelTypeDXT1 = 0x20,
	PixelTypeDXT3 = 0x22,
	PixelTypeDXT5 = 0x24,
	PixelTypeETC1 = 0x36
};

// Convert a V2 header to V3.
void ConvertPVRHeader(PVRTexHeaderV2 header2, PVRTexHeaderV3 *header3)
{
	// If the header's endianness doesn't match our own, we swap everything.
	if (header2.pvrTag == PVRTEX2_IDENT_REV)
	{
		// All of the struct's members are uint32 values, so we can do this.
		uint32 *headerArray = (uint32 *) &header2;
		for (size_t i = 0; i < sizeof(PVRTexHeaderV2) / sizeof(uint32); i++)
			headerArray[i] = swap32(headerArray[i]);
	}

	memset(header3, 0, sizeof(PVRTexHeaderV3));

	header3->version = PVRTEX3_IDENT;
	header3->height = header2.height;
	header3->width = header2.width;
	header3->depth = 1;
	header3->numSurfaces = header2.numSurfaces;
	header3->numFaces = 1;
	header3->numMipmaps = header2.numMipmaps;
	header3->metaDataSize = 0;

	switch ((PVRPixelTypeV2) (header2.flags & 0xFF))
	{
	case PixelTypePVRTC2:
		header3->pixelFormat = ePVRTPF_PVRTCI_2bpp_RGBA;
		break;
	case PixelTypePVRTC4:
		header3->pixelFormat = ePVRTPF_PVRTCI_4bpp_RGBA;
		break;
	case PixelTypePVRTCII2:
		header3->pixelFormat = ePVRTPF_PVRTCII_2bpp;
		break;
	case PixelTypePVRTCII4:
		header3->pixelFormat = ePVRTPF_PVRTCII_4bpp;
		break;
	case PixelTypeDXT1:
		header3->pixelFormat = ePVRTPF_DXT1;
		break;
	case PixelTypeDXT3:
		header3->pixelFormat = ePVRTPF_DXT3;
		break;
	case PixelTypeDXT5:
		header3->pixelFormat = ePVRTPF_DXT5;
		break;
	case PixelTypeETC1:
		header3->pixelFormat = ePVRTPF_ETC1;
		break;
	default:
		header3->pixelFormat = ePVRTF_UNKNOWN_FORMAT;
		break;
	}
}

CompressedData::Format convertFormat(PVRV3PixelFormat format)
{
	switch (format)
	{
	case ePVRTPF_PVRTCI_2bpp_RGB:
		return CompressedData::FORMAT_PVR1_RGB2;
	case ePVRTPF_PVRTCI_2bpp_RGBA:
		return CompressedData::FORMAT_PVR1_RGBA2;
	case ePVRTPF_PVRTCI_4bpp_RGB:
		return CompressedData::FORMAT_PVR1_RGB4;
	case ePVRTPF_PVRTCI_4bpp_RGBA:
		return CompressedData::FORMAT_PVR1_RGBA4;
	case ePVRTPF_ETC1:
		return CompressedData::FORMAT_ETC1;
	case ePVRTPF_DXT1:
		return CompressedData::FORMAT_DXT1;
	case ePVRTPF_DXT3:
		return CompressedData::FORMAT_DXT3;
	case ePVRTPF_DXT5:
		return CompressedData::FORMAT_DXT5;
	default:
		return CompressedData::FORMAT_UNKNOWN;
	}
}

int getBitsPerPixel(uint64 pixelformat)
{
	// Uncompressed formats have their bits per pixel stored in the high bits.
	if ((pixelformat & 0xFFFFFFFF) != pixelformat)
	{
		const uint8 *charformat = (const uint8 *) &pixelformat;
		return charformat[4] + charformat[5] + charformat[6] + charformat[7];
	}

	switch (pixelformat)
	{
	case ePVRTPF_PVRTCI_2bpp_RGB:
	case ePVRTPF_PVRTCI_2bpp_RGBA:
	case ePVRTPF_PVRTCII_2bpp:
		return 2;
	case ePVRTPF_PVRTCI_4bpp_RGB:
	case ePVRTPF_PVRTCI_4bpp_RGBA:
	case ePVRTPF_PVRTCII_4bpp:
	case ePVRTPF_ETC1:
	case ePVRTPF_DXT1:
		return 4;
	case ePVRTPF_DXT2:
	case ePVRTPF_DXT3:
	case ePVRTPF_DXT4:
	case ePVRTPF_DXT5:
		return 8;
	default:
		return 0;
	}
}

void getFormatMinDimensions(uint64 pixelformat, int &minX, int &minY)
{
	switch (pixelformat)
	{
	case ePVRTPF_PVRTCI_2bpp_RGB:
	case ePVRTPF_PVRTCI_2bpp_RGBA:
		minX = 16;
		minY = 8;
		break;
	case ePVRTPF_PVRTCI_4bpp_RGB:
	case ePVRTPF_PVRTCI_4bpp_RGBA:
		minX = minY = 8;
		break;
	case ePVRTPF_PVRTCII_2bpp:
		minX = 8;
		minY = 4;
		break;
	case ePVRTPF_PVRTCII_4bpp:
		minX = minY = 4;
		break;
	case ePVRTPF_DXT1:
	case ePVRTPF_DXT2:
	case ePVRTPF_DXT3:
	case ePVRTPF_DXT4:
	case ePVRTPF_DXT5:
	case ePVRTPF_ETC1:
		minX = minY = 4;
		break;
	default: // We don't handle all possible formats, but that's fine.
		minX = minY = 1;
		break;
	}
}

size_t getMipLevelSize(const PVRTexHeaderV3 &header, int miplevel)
{
	int smallestwidth = 1;
	int smallestheight = 1;
	getFormatMinDimensions(header.pixelFormat, smallestwidth, smallestheight);

	int width = std::max((int) header.width >> miplevel, 1);
	int height = std::max((int) header.height >> miplevel, 1);
	int depth = std::max((int) header.depth >> miplevel, 1);

	// Pad the dimensions.
	width += (-width) % smallestwidth;
	height += (-height) % smallestheight;

	return getBitsPerPixel(header.pixelFormat) * width * height * depth / 8;
}

} // Anonymous namespace.


bool PVRHandler::canParse(const filesystem::FileData *data)
{
	if (data->getSize() < sizeof(PVRTexHeaderV2) || data->getSize() < sizeof(PVRTexHeaderV3))
		return false;

	PVRTexHeaderV3 *header3 = (PVRTexHeaderV3 *) data->getData();

	// Magic number (FourCC identifier.)
	if (header3->version == PVRTEX3_IDENT || header3->version == PVRTEX3_IDENT_REV)
		return true;

	// Maybe it has a V2 header.
	PVRTexHeaderV2 *header2 = (PVRTexHeaderV2 *) data->getData();

	// FourCC identifier.
	if (header2->pvrTag == PVRTEX2_IDENT || header2->pvrTag == PVRTEX2_IDENT_REV)
		return true;

	return false;
}

uint8 *PVRHandler::parse(filesystem::FileData *filedata, std::vector<CompressedData::SubImage> &images, size_t &dataSize, CompressedData::Format &format)
{
	if (!canParse(filedata))
		throw love::Exception("Could not decode compressed data (not a PVR file?)");

	PVRTexHeaderV3 header3 = *(PVRTexHeaderV3 *) filedata->getData();

	// If the header isn't the V3 format, assume it's V2 and convert.
	if (header3.version != PVRTEX3_IDENT && header3.version != PVRTEX3_IDENT_REV)
		ConvertPVRHeader(*(PVRTexHeaderV2 *) filedata->getData(), &header3);

	// If the header's endianness doesn't match our own, then we swap everything.
	if (header3.version == PVRTEX3_IDENT_REV)
	{
		header3.version = PVRTEX3_IDENT;
		header3.flags = swap32(header3.flags);
		header3.pixelFormat = swap64(header3.pixelFormat);
		header3.colorSpace = swap32(header3.colorSpace);
		header3.channelType = swap32(header3.channelType);
		header3.height = swap32(header3.height);
		header3.width = swap32(header3.width);
		header3.depth = swap32(header3.depth);
		header3.numFaces = swap32(header3.numFaces);
		header3.numMipmaps = swap32(header3.numMipmaps);
		header3.metaDataSize = swap32(header3.metaDataSize);
	}

	if (header3.depth > 1)
		throw love::Exception("Image depths greater than 1 in PVR files are unsupported.");

	CompressedData::Format cformat = convertFormat((PVRV3PixelFormat) header3.pixelFormat);

	if (cformat == CompressedData::FORMAT_UNKNOWN)
		throw love::Exception("Could not parse PVR file: unsupported image format.");

	size_t totalsize = 0;
	uint8 *data = nullptr;

	// Ignore faces and surfaces except the first ones (for now.)
	for (int i = 0; i < (int) header3.numMipmaps; i++)
		totalsize += getMipLevelSize(header3, i);

	size_t fileoffset = sizeof(PVRTexHeaderV3) + header3.metaDataSize;

	// Make sure the file actually holds this much data...
	if (filedata->getSize() < fileoffset + totalsize)
		throw love::Exception("Could not parse PVR file: invalid size calculation.");

	try
	{
		data = new uint8[totalsize];
	}
	catch (std::bad_alloc &)
	{
		throw love::Exception("Out of memory.");
	}

	size_t curoffset = 0;
	const uint8 *filebytes = (uint8 *) filedata->getData() + fileoffset;

	for (int i = 0; i < (int) header3.numMipmaps; i++)
	{
		size_t mipsize = getMipLevelSize(header3, i);

		if (curoffset + mipsize > totalsize)
			break; // Just in case.

		CompressedData::SubImage mip;
		mip.width = std::max((int) header3.width >> i, 1);
		mip.height = std::max((int) header3.height >> i, 1);
		mip.size = mipsize;

		memcpy(data + curoffset, filebytes + curoffset, mipsize);
		mip.data = data + curoffset;

		curoffset += mipsize;

		images.push_back(mip);
	}

	dataSize = totalsize;
	format = cformat;

	return data;
}

} // magpie
} // image
} // love
