/**
 * Copyright (c) 2006-2024 LOVE Development Team
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

#pragma once

#include "stddef.h"

namespace love
{

enum PixelFormat
{
    PIXELFORMAT_UNKNOWN,

	// these are converted to an actual format by love
	PIXELFORMAT_NORMAL,
	PIXELFORMAT_HDR,

	// 1-channel normal formats
	PIXELFORMAT_R8_UNORM,
	PIXELFORMAT_R8_INT,
	PIXELFORMAT_R8_UINT,
	PIXELFORMAT_R16_UNORM,
	PIXELFORMAT_R16_FLOAT,
	PIXELFORMAT_R16_INT,
	PIXELFORMAT_R16_UINT,
	PIXELFORMAT_R32_FLOAT,
	PIXELFORMAT_R32_INT,
	PIXELFORMAT_R32_UINT,

	// 2-channel normal formats
	PIXELFORMAT_RG8_UNORM,
	PIXELFORMAT_RG8_INT,
	PIXELFORMAT_RG8_UINT,
	PIXELFORMAT_LA8_UNORM, // Same as RG8, but accessed as (L, L, L, A)
	PIXELFORMAT_RG16_UNORM,
	PIXELFORMAT_RG16_FLOAT,
	PIXELFORMAT_RG16_INT,
	PIXELFORMAT_RG16_UINT,
	PIXELFORMAT_RG32_FLOAT,
	PIXELFORMAT_RG32_INT,
	PIXELFORMAT_RG32_UINT,

	// 4-channel normal formats
	PIXELFORMAT_RGBA8_UNORM,
	PIXELFORMAT_RGBA8_sRGB,
	PIXELFORMAT_BGRA8_UNORM,
	PIXELFORMAT_BGRA8_sRGB,
	PIXELFORMAT_RGBA8_INT,
	PIXELFORMAT_RGBA8_UINT,
	PIXELFORMAT_RGBA16_UNORM,
	PIXELFORMAT_RGBA16_FLOAT,
	PIXELFORMAT_RGBA16_INT,
	PIXELFORMAT_RGBA16_UINT,
	PIXELFORMAT_RGBA32_FLOAT,
	PIXELFORMAT_RGBA32_INT,
	PIXELFORMAT_RGBA32_UINT,

	// packed formats
	PIXELFORMAT_RGBA4_UNORM,    // LSB->MSB: [a, b, g, r]
	PIXELFORMAT_RGB5A1_UNORM,   // LSB->MSB: [a, b, g, r]
	PIXELFORMAT_RGB565_UNORM,   // LSB->MSB: [b, g, r]
	PIXELFORMAT_RGB10A2_UNORM,  // LSB->MSB: [r, g, b, a]
	PIXELFORMAT_RG11B10_FLOAT,  // LSB->MSB: [r, g, b]

	// depth/stencil formats
	PIXELFORMAT_STENCIL8,
	PIXELFORMAT_DEPTH16_UNORM,
	PIXELFORMAT_DEPTH24_UNORM,
	PIXELFORMAT_DEPTH32_FLOAT,
	PIXELFORMAT_DEPTH24_UNORM_STENCIL8,
	PIXELFORMAT_DEPTH32_FLOAT_STENCIL8,

	// compressed formats
	PIXELFORMAT_DXT1_UNORM,
	PIXELFORMAT_DXT1_sRGB,
	PIXELFORMAT_DXT3_UNORM,
	PIXELFORMAT_DXT3_sRGB,
	PIXELFORMAT_DXT5_UNORM,
	PIXELFORMAT_DXT5_sRGB,
	PIXELFORMAT_BC4_UNORM,
	PIXELFORMAT_BC4_SNORM,
	PIXELFORMAT_BC5_UNORM,
	PIXELFORMAT_BC5_SNORM,
	PIXELFORMAT_BC6H_UFLOAT,
	PIXELFORMAT_BC6H_FLOAT,
	PIXELFORMAT_BC7_UNORM,
	PIXELFORMAT_BC7_sRGB,

	PIXELFORMAT_PVR1_RGB2_UNORM,
	PIXELFORMAT_PVR1_RGB2_sRGB,
	PIXELFORMAT_PVR1_RGB4_UNORM,
	PIXELFORMAT_PVR1_RGB4_sRGB,
	PIXELFORMAT_PVR1_RGBA2_UNORM,
	PIXELFORMAT_PVR1_RGBA2_sRGB,
	PIXELFORMAT_PVR1_RGBA4_UNORM,
	PIXELFORMAT_PVR1_RGBA4_sRGB,

	PIXELFORMAT_ETC1_UNORM,
	PIXELFORMAT_ETC2_RGB_UNORM,
	PIXELFORMAT_ETC2_RGB_sRGB,
	PIXELFORMAT_ETC2_RGBA_UNORM,
	PIXELFORMAT_ETC2_RGBA_sRGB,
	PIXELFORMAT_ETC2_RGBA1_UNORM,
	PIXELFORMAT_ETC2_RGBA1_sRGB,
	PIXELFORMAT_EAC_R_UNORM,
	PIXELFORMAT_EAC_R_SNORM,
	PIXELFORMAT_EAC_RG_UNORM,
	PIXELFORMAT_EAC_RG_SNORM,

	PIXELFORMAT_ASTC_4x4_UNORM,
	PIXELFORMAT_ASTC_5x4_UNORM,
	PIXELFORMAT_ASTC_5x5_UNORM,
	PIXELFORMAT_ASTC_6x5_UNORM,
	PIXELFORMAT_ASTC_6x6_UNORM,
	PIXELFORMAT_ASTC_8x5_UNORM,
	PIXELFORMAT_ASTC_8x6_UNORM,
	PIXELFORMAT_ASTC_8x8_UNORM,
	PIXELFORMAT_ASTC_10x5_UNORM,
	PIXELFORMAT_ASTC_10x6_UNORM,
	PIXELFORMAT_ASTC_10x8_UNORM,
	PIXELFORMAT_ASTC_10x10_UNORM,
	PIXELFORMAT_ASTC_12x10_UNORM,
	PIXELFORMAT_ASTC_12x12_UNORM,
	PIXELFORMAT_ASTC_4x4_sRGB,
	PIXELFORMAT_ASTC_5x4_sRGB,
	PIXELFORMAT_ASTC_5x5_sRGB,
	PIXELFORMAT_ASTC_6x5_sRGB,
	PIXELFORMAT_ASTC_6x6_sRGB,
	PIXELFORMAT_ASTC_8x5_sRGB,
	PIXELFORMAT_ASTC_8x6_sRGB,
	PIXELFORMAT_ASTC_8x8_sRGB,
	PIXELFORMAT_ASTC_10x5_sRGB,
	PIXELFORMAT_ASTC_10x6_sRGB,
	PIXELFORMAT_ASTC_10x8_sRGB,
	PIXELFORMAT_ASTC_10x10_sRGB,
	PIXELFORMAT_ASTC_12x10_sRGB,
	PIXELFORMAT_ASTC_12x12_sRGB,

	PIXELFORMAT_MAX_ENUM
};

enum PixelFormatType
{
	PIXELFORMATTYPE_UNORM,
	PIXELFORMATTYPE_SNORM,
	PIXELFORMATTYPE_UFLOAT,
	PIXELFORMATTYPE_SFLOAT,
	PIXELFORMATTYPE_UINT,
	PIXELFORMATTYPE_SINT,
};

struct PixelFormatInfo
{
	int components;
	size_t blockWidth;
	size_t blockHeight;
	size_t blockSize;
	bool color;
	bool depth;
	bool stencil;
	bool compressed;
	bool sRGB;
	PixelFormatType dataType;
};

bool getConstant(PixelFormat in, const char *&out);
bool getConstant(const char *in, PixelFormat &out);

const PixelFormatInfo &getPixelFormatInfo(PixelFormat format);

/**
 * Gets the name of the specified pixel format.
 **/
const char *getPixelFormatName(PixelFormat format);

/**
 * Gets whether the specified pixel format is a compressed type.
 **/
bool isPixelFormatCompressed(PixelFormat format);

/**
 * Gets whether the specified pixel format is a color type.
 **/
bool isPixelFormatColor(PixelFormat format);

/**
 * Gets whether the specified pixel format is a depth or stencil type.
 **/
bool isPixelFormatDepthStencil(PixelFormat format);

/**
 * Gets whether the specified pixel format is a depth type.
 **/
bool isPixelFormatDepth(PixelFormat format);

/**
 * Gets whether the specified pixel format is a stencil type.
 **/
bool isPixelFormatStencil(PixelFormat format);

/**
 * Gets whether the specified color pixel format is sRGB-encoded.
 **/
bool isPixelFormatSRGB(PixelFormat format);

/**
 * Gets whether the specified pixel format is a signed or unsigned integer type.
 **/
bool isPixelFormatInteger(PixelFormat format);

/**
 * Gets the sRGB version of a linear pixel format, if applicable.
 **/
PixelFormat getSRGBPixelFormat(PixelFormat format);

/**
 * Gets the linear version of a sRGB pixel format, if applicable.
 **/
PixelFormat getLinearPixelFormat(PixelFormat format);

/**
 * Gets the block size in bytes of the specified pixel format.
 * This is the size in bytes of a pixel for uncompressed formats, but *not*
 * for compressed formats!
 **/
size_t getPixelFormatBlockSize(PixelFormat format);

/**
 * Gets the size in bytes of a row of an uncompressed pixel format.
 **/
size_t getPixelFormatUncompressedRowSize(PixelFormat format, int width);

/**
 * Gets the size in bytes of a row of a compressed pixel format. This is the
 * number of blocks used by the given width, multiplied by the block size. The
 * number of rows of blocks for a given height can be computed by
 * getPixelFormatCompressedBlockRowCount.
 **/
size_t getPixelFormatCompressedBlockRowSize(PixelFormat format, int width);

/**
 * Gets the number of rows of blocks the given compressed pixel format will use,
 * for the given height in pixels.
 **/
size_t getPixelFormatCompressedBlockRowCount(PixelFormat format, int height);

/**
 * Gets the size in bytes of a slice (width x height 2D plane) which uses the
 * given pixel format.
 **/
size_t getPixelFormatSliceSize(PixelFormat format, int width, int height);

/**
 * Gets the number of color components in the given pixel format.
 **/
int getPixelFormatColorComponents(PixelFormat format);

} // love
