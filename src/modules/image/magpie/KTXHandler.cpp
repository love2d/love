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
#include "KTXHandler.h"
#include "common/int.h"

// C
#include <string.h>

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

#define KTX_IDENTIFIER_REF {0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A}
#define KTX_ENDIAN_REF     (0x04030201)
#define KTX_ENDIAN_REF_REV (0x01020304)
#define KTX_HEADER_SIZE    (64)

struct KTXHeader
{
	uint8  identifier[12];
	uint32 endianness;
	uint32 glType;
	uint32 glTypeSize;
	uint32 glFormat;
	uint32 glInternalFormat;
	uint32 glBaseInternalFormat;
	uint32 pixelWidth;
	uint32 pixelHeight;
	uint32 pixelDepth;
	uint32 numberOfArrayElements;
	uint32 numberOfFaces;
	uint32 numberOfMipmapLevels;
	uint32 bytesOfKeyValueData;
};

static_assert(sizeof(KTXHeader) == KTX_HEADER_SIZE, "Real size of KTX header doesn't match struct size!");

enum KTXGLInternalFormat
{
	KTX_GL_ETC1_RGB8_OES = 0x8D64,

	// ETC2 and EAC.
	KTX_GL_COMPRESSED_R11_EAC                        = 0x9270,
	KTX_GL_COMPRESSED_SIGNED_R11_EAC                 = 0x9271,
	KTX_GL_COMPRESSED_RG11_EAC                       = 0x9272,
	KTX_GL_COMPRESSED_SIGNED_RG11_EAC                = 0x9273,
	KTX_GL_COMPRESSED_RGB8_ETC2                      = 0x9274,
	KTX_GL_COMPRESSED_SRGB8_ETC2                     = 0x9275,
	KTX_GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2  = 0x9276,
	KTX_GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 = 0x9277,
	KTX_GL_COMPRESSED_RGBA8_ETC2_EAC                 = 0x9278,
	KTX_GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC          = 0x9279,

	// PVRTC1.
	KTX_GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG  = 0x8C00,
	KTX_GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG  = 0x8C01,
	KTX_GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG = 0x8C02,
	KTX_GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG = 0x8C03,

	// DXT1, DXT3, and DXT5.
	KTX_GL_COMPRESSED_RGB_S3TC_DXT1_EXT        = 0x83F0,
	KTX_GL_COMPRESSED_RGBA_S3TC_DXT3_EXT       = 0x83F2,
	KTX_GL_COMPRESSED_RGBA_S3TC_DXT5_EXT       = 0x83F3,
	KTX_GL_COMPRESSED_SRGB_S3TC_DXT1_EXT       = 0x8C4C,
	KTX_GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT = 0x8C4E,
	KTX_GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT = 0x8C4F,

	// BC4 and BC5.
	KTX_GL_COMPRESSED_RED_RGTC1        = 0x8DBB,
	KTX_GL_COMPRESSED_SIGNED_RED_RGTC1 = 0x8DBC,
	KTX_GL_COMPRESSED_RG_RGTC2         = 0x8DBD,
	KTX_GL_COMPRESSED_SIGNED_RG_RGTC2  = 0x8DBE,

	// BC6 and BC7.
	KTX_GL_COMPRESSED_RGBA_BPTC_UNORM         = 0x8E8C,
	KTX_GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM   = 0x8E8D,
	KTX_GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT   = 0x8E8E,
	KTX_GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT = 0x8E8F,

	// ASTC.
	KTX_GL_COMPRESSED_RGBA_ASTC_4x4_KHR           = 0x93B0,
	KTX_GL_COMPRESSED_RGBA_ASTC_5x4_KHR           = 0x93B1,
	KTX_GL_COMPRESSED_RGBA_ASTC_5x5_KHR           = 0x93B2,
	KTX_GL_COMPRESSED_RGBA_ASTC_6x5_KHR           = 0x93B3,
	KTX_GL_COMPRESSED_RGBA_ASTC_6x6_KHR           = 0x93B4,
	KTX_GL_COMPRESSED_RGBA_ASTC_8x5_KHR           = 0x93B5,
	KTX_GL_COMPRESSED_RGBA_ASTC_8x6_KHR           = 0x93B6,
	KTX_GL_COMPRESSED_RGBA_ASTC_8x8_KHR           = 0x93B7,
	KTX_GL_COMPRESSED_RGBA_ASTC_10x5_KHR          = 0x93B8,
	KTX_GL_COMPRESSED_RGBA_ASTC_10x6_KHR          = 0x93B9,
	KTX_GL_COMPRESSED_RGBA_ASTC_10x8_KHR          = 0x93BA,
	KTX_GL_COMPRESSED_RGBA_ASTC_10x10_KHR         = 0x93BB,
	KTX_GL_COMPRESSED_RGBA_ASTC_12x10_KHR         = 0x93BC,
	KTX_GL_COMPRESSED_RGBA_ASTC_12x12_KHR         = 0x93BD,
	KTX_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR   = 0x93D0,
	KTX_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR   = 0x93D1,
	KTX_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR   = 0x93D2,
	KTX_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR   = 0x93D3,
	KTX_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR   = 0x93D4,
	KTX_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR   = 0x93D5,
	KTX_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR   = 0x93D6,
	KTX_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR   = 0x93D7,
	KTX_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR  = 0x93D8,
	KTX_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR  = 0x93D9,
	KTX_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR  = 0x93DA,
	KTX_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR = 0x93DB,
	KTX_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR = 0x93DC,
	KTX_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR = 0x93DD
};

CompressedImageData::Format convertFormat(uint32 glformat, bool &sRGB)
{
	sRGB = false;

	// hnnngg ASTC...

	switch (glformat)
	{
	case KTX_GL_ETC1_RGB8_OES:
		return CompressedImageData::FORMAT_ETC1;

	// EAC and ETC2.
	case KTX_GL_COMPRESSED_R11_EAC:
		return CompressedImageData::FORMAT_EAC_R;
	case KTX_GL_COMPRESSED_SIGNED_R11_EAC:
		return CompressedImageData::FORMAT_EAC_Rs;
	case KTX_GL_COMPRESSED_RG11_EAC:
		return CompressedImageData::FORMAT_EAC_RG;
	case KTX_GL_COMPRESSED_SIGNED_RG11_EAC:
		return CompressedImageData::FORMAT_EAC_RGs;
	case KTX_GL_COMPRESSED_RGB8_ETC2:
		return CompressedImageData::FORMAT_ETC2_RGB;
	case KTX_GL_COMPRESSED_SRGB8_ETC2:
		sRGB = true;
		return CompressedImageData::FORMAT_ETC2_RGB;
	case KTX_GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
		return CompressedImageData::FORMAT_ETC2_RGBA1;
	case KTX_GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
		sRGB = true;
		return CompressedImageData::FORMAT_ETC2_RGBA1;
	case KTX_GL_COMPRESSED_RGBA8_ETC2_EAC:
		return CompressedImageData::FORMAT_ETC2_RGBA;
	case KTX_GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
		sRGB = true;
		return CompressedImageData::FORMAT_ETC2_RGBA;

	// PVRTC.
	case KTX_GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG:
		return CompressedImageData::FORMAT_PVR1_RGB4;
	case KTX_GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG:
		return CompressedImageData::FORMAT_PVR1_RGB2;
	case KTX_GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:
		return CompressedImageData::FORMAT_PVR1_RGBA4;
	case KTX_GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG:
		return CompressedImageData::FORMAT_PVR1_RGBA2;

	// DXT.
	case KTX_GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
		sRGB = true;
	case KTX_GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
		return CompressedImageData::FORMAT_DXT1;
	case KTX_GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
		sRGB = true;
	case KTX_GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
		return CompressedImageData::FORMAT_DXT3;
	case KTX_GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
		sRGB = true;
	case KTX_GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
		return CompressedImageData::FORMAT_DXT5;

	// BC4 and BC5.
	case KTX_GL_COMPRESSED_RED_RGTC1:
		return CompressedImageData::FORMAT_BC4;
	case KTX_GL_COMPRESSED_SIGNED_RED_RGTC1:
		return CompressedImageData::FORMAT_BC4s;
	case KTX_GL_COMPRESSED_RG_RGTC2:
		return CompressedImageData::FORMAT_BC5;
	case KTX_GL_COMPRESSED_SIGNED_RG_RGTC2:
		return CompressedImageData::FORMAT_BC5s;

	// BC6 and BC7.
	case KTX_GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
		sRGB = true;
	case KTX_GL_COMPRESSED_RGBA_BPTC_UNORM:
		return CompressedImageData::FORMAT_BC7;
	case KTX_GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
		return CompressedImageData::FORMAT_BC6Hs;
	case KTX_GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
		return CompressedImageData::FORMAT_BC6H;

	// ASTC.
	case KTX_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:
		sRGB = true;
	case KTX_GL_COMPRESSED_RGBA_ASTC_4x4_KHR:
		return CompressedImageData::FORMAT_ASTC_4x4;
	case KTX_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR:
		sRGB = true;
	case KTX_GL_COMPRESSED_RGBA_ASTC_5x4_KHR:
		return CompressedImageData::FORMAT_ASTC_5x4;
	case KTX_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR:
		sRGB = true;
	case KTX_GL_COMPRESSED_RGBA_ASTC_5x5_KHR:
		return CompressedImageData::FORMAT_ASTC_5x5;
	case KTX_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR:
		sRGB = true;
	case KTX_GL_COMPRESSED_RGBA_ASTC_6x5_KHR:
		return CompressedImageData::FORMAT_ASTC_6x5;
	case KTX_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR:
		sRGB = true;
	case KTX_GL_COMPRESSED_RGBA_ASTC_6x6_KHR:
		return CompressedImageData::FORMAT_ASTC_6x6;
	case KTX_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR:
		sRGB = true;
	case KTX_GL_COMPRESSED_RGBA_ASTC_8x5_KHR:
		return CompressedImageData::FORMAT_ASTC_8x5;
	case KTX_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR:
		sRGB = true;
	case KTX_GL_COMPRESSED_RGBA_ASTC_8x6_KHR:
		return CompressedImageData::FORMAT_ASTC_8x6;
	case KTX_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR:
		sRGB = true;
	case KTX_GL_COMPRESSED_RGBA_ASTC_8x8_KHR:
		return CompressedImageData::FORMAT_ASTC_8x8;
	case KTX_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR:
		sRGB = true;
	case KTX_GL_COMPRESSED_RGBA_ASTC_10x5_KHR:
		return CompressedImageData::FORMAT_ASTC_10x5;
	case KTX_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR:
		sRGB = true;
	case KTX_GL_COMPRESSED_RGBA_ASTC_10x6_KHR:
		return CompressedImageData::FORMAT_ASTC_10x6;
	case KTX_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR:
		sRGB = true;
	case KTX_GL_COMPRESSED_RGBA_ASTC_10x8_KHR:
		return CompressedImageData::FORMAT_ASTC_10x8;
	case KTX_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR:
		sRGB = true;
	case KTX_GL_COMPRESSED_RGBA_ASTC_10x10_KHR:
		return CompressedImageData::FORMAT_ASTC_10x10;
	case KTX_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR:
		sRGB = true;
	case KTX_GL_COMPRESSED_RGBA_ASTC_12x10_KHR:
		return CompressedImageData::FORMAT_ASTC_12x10;
	case KTX_GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR:
		sRGB = true;
	case KTX_GL_COMPRESSED_RGBA_ASTC_12x12_KHR:
		return CompressedImageData::FORMAT_ASTC_12x12;
	default:
		return CompressedImageData::FORMAT_UNKNOWN;
	}
}

} // Anonymous namespace.

bool KTXHandler::canParse(const filesystem::FileData *data)
{
	if (data->getSize() < sizeof(KTXHeader))
		return false;

	KTXHeader *header = (KTXHeader *) data->getData();
	uint8 ktxidentifier[12] = KTX_IDENTIFIER_REF;

	if (memcmp(header->identifier, ktxidentifier, 12) != 0)
		return false;

	if (header->endianness != KTX_ENDIAN_REF && header->endianness != KTX_ENDIAN_REF_REV)
		return false;

	return true;
}

uint8 *KTXHandler::parse(filesystem::FileData *filedata, std::vector<CompressedImageData::SubImage> &images, size_t &dataSize, CompressedImageData::Format &format, bool &sRGB)
{
	if (!canParse(filedata))
		throw love::Exception("Could not decode compressed data (not a KTX file?)");

	KTXHeader header = *(KTXHeader *) filedata->getData();

	if (header.endianness == KTX_ENDIAN_REF_REV)
	{
		uint32 *headerArray = (uint32 *) &header.glType;
		for (int i = 0; i < 12; i++)
			headerArray[i] = swapuint32(headerArray[i]);
	}

	header.numberOfMipmapLevels = std::max(header.numberOfMipmapLevels, 1u);

	bool isSRGB = false;
	CompressedImageData::Format cformat = convertFormat(header.glInternalFormat, isSRGB);

	if (cformat == CompressedImageData::FORMAT_UNKNOWN)
		throw love::Exception("Unsupported image format in KTX file.");

	if (header.numberOfArrayElements > 0)
		throw love::Exception("Texture arrays in KTX files are not supported.");

	if (header.pixelDepth > 1)
		throw love::Exception("3D textures in KTX files are not supported.");

	if (header.numberOfFaces > 1)
		throw love::Exception("Cubemap textures in KTX files are not supported.");

	size_t fileoffset = sizeof(KTXHeader) + header.bytesOfKeyValueData;
	const uint8 *filebytes = (uint8 *) filedata->getData();
	size_t totalsize = 0;

	// Calculate the total size needed to hold the data in memory.
	for (int i = 0; i < (int) header.numberOfMipmapLevels; i++)
	{
		if (fileoffset + sizeof(uint32) > filedata->getSize())
			throw love::Exception("Could not parse KTX file: unexpected EOF.");

		uint32 mipsize = *(uint32 *) (filebytes + fileoffset);

		if (header.endianness == KTX_ENDIAN_REF_REV)
			mipsize = swapuint32(mipsize);

		fileoffset += sizeof(uint32);

		// All mipsize fields are at a file offset that's a multiple of 4, so
		// there might be some padding after the actual data in this mip level.
		uint32 mipsizepadded = (mipsize + 3) & ~uint32(3);

		totalsize += mipsizepadded;
		fileoffset += mipsizepadded;
	}

	uint8 *data = nullptr;
	try
	{
		data = new uint8[totalsize];
	}
	catch (std::bad_alloc &)
	{
		throw love::Exception("Out of memory.");
	}

	// Reset the file offset to the start of the file's image data.
	fileoffset = sizeof(KTXHeader) + header.bytesOfKeyValueData;
	size_t dataoffset = 0;

	// Copy each mipmap level of the image from the file to our block of memory.
	for (int i = 0; i < (int) header.numberOfMipmapLevels; i++)
	{
		uint32 mipsize = *(uint32 *) (filebytes + fileoffset);

		if (header.endianness == KTX_ENDIAN_REF_REV)
			mipsize = swapuint32(mipsize);

		fileoffset += sizeof(uint32);

		uint32 mipsizepadded = (mipsize + 3) & ~uint32(3);

		CompressedImageData::SubImage mip;
		mip.width = (int) std::max(header.pixelWidth >> i, 1u);
		mip.height = (int) std::max(header.pixelHeight >> i, 1u);
		mip.size = mipsize;

		memcpy(data + dataoffset, filebytes + fileoffset, mipsize);
		mip.data = data + dataoffset;

		fileoffset += mipsizepadded;
		dataoffset += mipsizepadded;

		images.push_back(mip);
	}

	dataSize = totalsize;
	format = cformat;
	sRGB = isSRGB;

	return data;
}

} // magpie
} // image
} // love
