/**
 * Copyright (c) 2006-2023 LOVE Development Team
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
#include "common/Exception.h"

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
	ePVRTPF_PVRTCI_2bpp_RGB = 0,
	ePVRTPF_PVRTCI_2bpp_RGBA,
	ePVRTPF_PVRTCI_4bpp_RGB,
	ePVRTPF_PVRTCI_4bpp_RGBA,
	ePVRTPF_PVRTCII_2bpp = 4,
	ePVRTPF_PVRTCII_4bpp,
	ePVRTPF_ETC1 = 6,
	ePVRTPF_DXT1 = 7,
	ePVRTPF_DXT2,
	ePVRTPF_DXT3,
	ePVRTPF_DXT4,
	ePVRTPF_DXT5,
	ePVRTPF_BC4,
	ePVRTPF_BC5,
	ePVRTPF_BC6,
	ePVRTPF_BC7,
	ePVRTPF_ETC2_RGB = 22,
	ePVRTPF_ETC2_RGBA,
	ePVRTPF_ETC2_RGBA1,
	ePVRTPF_EAC_R = 25,
	ePVRTPF_EAC_RG,
	ePVRTPF_ASTC_4x4 = 27,
	ePVRTPF_ASTC_5x4,
	ePVRTPF_ASTC_5x5,
	ePVRTPF_ASTC_6x5,
	ePVRTPF_ASTC_6x6,
	ePVRTPF_ASTC_8x5,
	ePVRTPF_ASTC_8x6,
	ePVRTPF_ASTC_8x8,
	ePVRTPF_ASTC_10x5,
	ePVRTPF_ASTC_10x6,
	ePVRTPF_ASTC_10x8,
	ePVRTPF_ASTC_10x10,
	ePVRTPF_ASTC_12x10,
	ePVRTPF_ASTC_12x12,
	ePVRTPF_UNKNOWN_FORMAT = 0x7F
};

enum PVRV3ChannelType
{
	ePVRTCT_UNORM8 = 0,
	ePVRTCT_SNORM8,
	ePVRTCT_UINT8,
	ePVRTCT_SINT8,
	ePVRTCT_UNORM16,
	ePVRTCT_SNORM16,
	ePVRTCT_UINT16,
	ePVRTCT_SINT16,
	ePVRTCT_UNORM32,
	ePVRTCT_SNORM32,
	ePVRTCT_UINT32,
	ePVRTCT_SINT32,
	ePVRTCT_FLOAT
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
			headerArray[i] = swapuint32(headerArray[i]);
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
		header3->pixelFormat = ePVRTPF_UNKNOWN_FORMAT;
		break;
	}
}

static PixelFormat convertFormat(PVRV3PixelFormat format, PVRV3ChannelType channeltype)
{
	bool snorm = false;

	switch (channeltype)
	{
	case ePVRTCT_SNORM8:
	case ePVRTCT_SNORM16:
	case ePVRTCT_SNORM32:
		snorm = true;
		break;
	default:
		break;
	}

	switch (format)
	{
	case ePVRTPF_PVRTCI_2bpp_RGB:
		return PIXELFORMAT_PVR1_RGB2;
	case ePVRTPF_PVRTCI_2bpp_RGBA:
		return PIXELFORMAT_PVR1_RGBA2;
	case ePVRTPF_PVRTCI_4bpp_RGB:
		return PIXELFORMAT_PVR1_RGB4;
	case ePVRTPF_PVRTCI_4bpp_RGBA:
		return PIXELFORMAT_PVR1_RGBA4;
	case ePVRTPF_ETC1:
		return PIXELFORMAT_ETC1;
	case ePVRTPF_DXT1:
		return PIXELFORMAT_DXT1;
	case ePVRTPF_DXT3:
		return PIXELFORMAT_DXT3;
	case ePVRTPF_DXT5:
		return PIXELFORMAT_DXT5;
	case ePVRTPF_BC4:
		return snorm ? PIXELFORMAT_BC4s : PIXELFORMAT_BC4;
	case ePVRTPF_BC5:
		return snorm ? PIXELFORMAT_BC5s : PIXELFORMAT_BC5;
	case ePVRTPF_BC6:
		return snorm ? PIXELFORMAT_BC6Hs : PIXELFORMAT_BC6H;
	case ePVRTPF_BC7:
		return PIXELFORMAT_BC7;
	case ePVRTPF_ETC2_RGB:
		return PIXELFORMAT_ETC2_RGB;
	case ePVRTPF_ETC2_RGBA:
		return PIXELFORMAT_ETC2_RGBA;
	case ePVRTPF_ETC2_RGBA1:
		return PIXELFORMAT_ETC2_RGBA1;
	case ePVRTPF_EAC_R:
		return snorm ? PIXELFORMAT_EAC_Rs : PIXELFORMAT_EAC_R;
	case ePVRTPF_EAC_RG:
		return snorm ? PIXELFORMAT_EAC_RGs : PIXELFORMAT_EAC_RG;
	case ePVRTPF_ASTC_4x4:
		return PIXELFORMAT_ASTC_4x4;
	case ePVRTPF_ASTC_5x4:
		return PIXELFORMAT_ASTC_5x4;
	case ePVRTPF_ASTC_5x5:
		return PIXELFORMAT_ASTC_5x5;
	case ePVRTPF_ASTC_6x5:
		return PIXELFORMAT_ASTC_6x5;
	case ePVRTPF_ASTC_6x6:
		return PIXELFORMAT_ASTC_6x6;
	case ePVRTPF_ASTC_8x5:
		return PIXELFORMAT_ASTC_8x5;
	case ePVRTPF_ASTC_8x6:
		return PIXELFORMAT_ASTC_8x6;
	case ePVRTPF_ASTC_8x8:
		return PIXELFORMAT_ASTC_8x8;
	case ePVRTPF_ASTC_10x5:
		return PIXELFORMAT_ASTC_10x5;
	case ePVRTPF_ASTC_10x6:
		return PIXELFORMAT_ASTC_10x6;
	case ePVRTPF_ASTC_10x8:
		return PIXELFORMAT_ASTC_10x8;
	case ePVRTPF_ASTC_10x10:
		return PIXELFORMAT_ASTC_10x10;
	case ePVRTPF_ASTC_12x10:
		return PIXELFORMAT_ASTC_12x10;
	case ePVRTPF_ASTC_12x12:
		return PIXELFORMAT_ASTC_12x12;
	default:
		return PIXELFORMAT_UNKNOWN;
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
	case ePVRTPF_BC4:
	case ePVRTPF_ETC2_RGB:
	case ePVRTPF_ETC2_RGBA1:
	case ePVRTPF_EAC_R:
		return 4;
	case ePVRTPF_DXT2:
	case ePVRTPF_DXT3:
	case ePVRTPF_DXT4:
	case ePVRTPF_DXT5:
	case ePVRTPF_BC5:
	case ePVRTPF_BC6:
	case ePVRTPF_BC7:
	case ePVRTPF_ETC2_RGBA:
	case ePVRTPF_EAC_RG:
		return 8;
	default:
		return 0;
	}
}

void getFormatMinDimensions(uint64 pixelformat, int &minX, int &minY, int &minZ)
{
	minZ = 1;

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
	case ePVRTPF_BC4:
	case ePVRTPF_BC5:
	case ePVRTPF_BC6:
	case ePVRTPF_BC7:
	case ePVRTPF_ETC1:
	case ePVRTPF_ETC2_RGB:
	case ePVRTPF_ETC2_RGBA:
	case ePVRTPF_ETC2_RGBA1:
	case ePVRTPF_EAC_R:
	case ePVRTPF_EAC_RG:
		minX = minY = 4;
		break;
	case ePVRTPF_ASTC_4x4:
		minX = 4;
		minY = 4;
		break;
	case ePVRTPF_ASTC_5x4:
		minX = 5;
		minY = 4;
		break;
	case ePVRTPF_ASTC_5x5:
		minX = 5;
		minY = 5;
		break;
	case ePVRTPF_ASTC_6x5:
		minX = 6;
		minY = 5;
		break;
	case ePVRTPF_ASTC_6x6:
		minX = 6;
		minY = 6;
		break;
	case ePVRTPF_ASTC_8x5:
		minX = 8;
		minY = 5;
		break;
	case ePVRTPF_ASTC_8x6:
		minX = 8;
		minY = 6;
		break;
	case ePVRTPF_ASTC_8x8:
		minX = 8;
		minY = 8;
		break;
	case ePVRTPF_ASTC_10x5:
		minX = 10;
		minY = 5;
		break;
	case ePVRTPF_ASTC_10x6:
		minX = 10;
		minY = 6;
		break;
	case ePVRTPF_ASTC_10x8:
		minX = 10;
		minY = 8;
		break;
	case ePVRTPF_ASTC_10x10:
		minX = 10;
		minY = 10;
		break;
	case ePVRTPF_ASTC_12x10:
		minX = 12;
		minY = 10;
		break;
	case ePVRTPF_ASTC_12x12:
		minX = 12;
		minY = 12;
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
	int smallestdepth = 1;
	getFormatMinDimensions(header.pixelFormat, smallestwidth, smallestheight, smallestdepth);

	int width = std::max((int) header.width >> miplevel, 1);
	int height = std::max((int) header.height >> miplevel, 1);
	int depth = std::max((int) header.depth >> miplevel, 1);

	// Pad the dimensions.
	width = ((width + smallestwidth - 1) / smallestwidth) * smallestwidth;
	height = ((height + smallestheight - 1) / smallestheight) * smallestheight;
	depth = ((depth + smallestdepth - 1) / smallestdepth) * smallestdepth;

	if (header.pixelFormat >= ePVRTPF_ASTC_4x4 && header.pixelFormat <= ePVRTPF_ASTC_12x12)
		return (width / smallestwidth) * (height / smallestheight) * (depth / smallestdepth) * (128 / 8);
	else
		return getBitsPerPixel(header.pixelFormat) * width * height * depth / 8;
}

} // Anonymous namespace.


bool PVRHandler::canParseCompressed(Data *data)
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

StrongRef<CompressedMemory> PVRHandler::parseCompressed(Data *filedata, std::vector<StrongRef<CompressedSlice>> &images, PixelFormat &format, bool &sRGB)
{
	if (!canParseCompressed(filedata))
		throw love::Exception("Could not decode compressed data (not a PVR file?)");

	PVRTexHeaderV3 header3 = *(PVRTexHeaderV3 *) filedata->getData();

	// If the header isn't the V3 format, assume it's V2 and convert.
	if (header3.version != PVRTEX3_IDENT && header3.version != PVRTEX3_IDENT_REV)
		ConvertPVRHeader(*(PVRTexHeaderV2 *) filedata->getData(), &header3);

	// If the header's endianness doesn't match our own, then we swap everything.
	if (header3.version == PVRTEX3_IDENT_REV)
	{
		header3.version = PVRTEX3_IDENT;
		header3.flags = swapuint32(header3.flags);
		header3.pixelFormat = swapuint64(header3.pixelFormat);
		header3.colorSpace = swapuint32(header3.colorSpace);
		header3.channelType = swapuint32(header3.channelType);
		header3.height = swapuint32(header3.height);
		header3.width = swapuint32(header3.width);
		header3.depth = swapuint32(header3.depth);
		header3.numFaces = swapuint32(header3.numFaces);
		header3.numMipmaps = swapuint32(header3.numMipmaps);
		header3.metaDataSize = swapuint32(header3.metaDataSize);
	}

	if (header3.depth > 1)
		throw love::Exception("Image depths greater than 1 in PVR files are unsupported.");

	PVRV3PixelFormat pixelformat = (PVRV3PixelFormat) header3.pixelFormat;
	PVRV3ChannelType channeltype = (PVRV3ChannelType) header3.channelType;

	PixelFormat cformat = convertFormat(pixelformat, channeltype);

	if (cformat == PIXELFORMAT_UNKNOWN)
		throw love::Exception("Could not parse PVR file: unsupported image format.");

	size_t totalsize = 0;

	// Ignore faces and surfaces except the first ones (for now.)
	for (int i = 0; i < (int) header3.numMipmaps; i++)
		totalsize += getMipLevelSize(header3, i);

	size_t fileoffset = sizeof(PVRTexHeaderV3) + header3.metaDataSize;

	// Make sure the file actually holds this much data...
	if (filedata->getSize() < fileoffset + totalsize)
		throw love::Exception("Could not parse PVR file: invalid size calculation.");

	StrongRef<CompressedMemory> memory;
	memory.set(new CompressedMemory(totalsize), Acquire::NORETAIN);

	size_t curoffset = 0;
	const uint8 *filebytes = (uint8 *) filedata->getData() + fileoffset;

	for (int i = 0; i < (int) header3.numMipmaps; i++)
	{
		size_t mipsize = getMipLevelSize(header3, i);

		if (curoffset + mipsize > totalsize)
			break; // Just in case.

		int width = std::max((int) header3.width >> i, 1);
		int height = std::max((int) header3.height >> i, 1);

		memcpy(memory->data + curoffset, filebytes + curoffset, mipsize);

		auto slice = new CompressedSlice(cformat, width, height, memory, curoffset, mipsize);
		images.push_back(slice);
		slice->release();

		curoffset += mipsize;
	}

	format = cformat;
	sRGB = (header3.colorSpace == 1);

	return memory;
}

} // magpie
} // image
} // love
