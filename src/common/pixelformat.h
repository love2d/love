/**
 * Copyright (c) 2006-2019 LOVE Development Team
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

	// "regular" formats
	PIXELFORMAT_R8_UNORM,
	PIXELFORMAT_RG8_UNORM,
	PIXELFORMAT_RGBA8_UNORM,
	PIXELFORMAT_sRGBA8_UNORM,
	PIXELFORMAT_R16_UNORM,
	PIXELFORMAT_RG16_UNORM,
	PIXELFORMAT_RGBA16_UNORM,
	PIXELFORMAT_R16_FLOAT,
	PIXELFORMAT_RG16_FLOAT,
	PIXELFORMAT_RGBA16_FLOAT,
	PIXELFORMAT_R32_FLOAT,
	PIXELFORMAT_RG32_FLOAT,
	PIXELFORMAT_RGBA32_FLOAT,

	PIXELFORMAT_LA8_UNORM, // Same as RG8, but accessed as (L, L, L, A)

	// packed formats
	PIXELFORMAT_RGBA4_UNORM,    // LSB->MSB: [a, b, g, r]
	PIXELFORMAT_RGB5A1_UNORM,   // LSB->MSB: [a, b, g, r]
	PIXELFORMAT_RGB565_UNORM,   // LSB->MSB: [b, g, r]
	PIXELFORMAT_RGB10A2_UNORM,  // LSB->MSB: [r, g, b, a]
	PIXELFORMAT_RG11B10_FLOAT, // LSB->MSB: [r, g, b]

	// depth/stencil formats
	PIXELFORMAT_STENCIL8,
	PIXELFORMAT_DEPTH16_UNORM,
	PIXELFORMAT_DEPTH24_UNORM,
	PIXELFORMAT_DEPTH32_FLOAT,
	PIXELFORMAT_DEPTH24_UNORM_STENCIL8,
	PIXELFORMAT_DEPTH32_FLOAT_STENCIL8,

	// compressed formats
	PIXELFORMAT_DXT1_UNORM,
	PIXELFORMAT_DXT3_UNORM,
	PIXELFORMAT_DXT5_UNORM,
	PIXELFORMAT_BC4_UNORM,
	PIXELFORMAT_BC4_SNORM,
	PIXELFORMAT_BC5_UNORM,
	PIXELFORMAT_BC5_SNORM,
	PIXELFORMAT_BC6H_UFLOAT,
	PIXELFORMAT_BC6H_FLOAT,
	PIXELFORMAT_BC7_UNORM,
	PIXELFORMAT_PVR1_RGB2_UNORM,
	PIXELFORMAT_PVR1_RGB4_UNORM,
	PIXELFORMAT_PVR1_RGBA2_UNORM,
	PIXELFORMAT_PVR1_RGBA4_UNORM,
	PIXELFORMAT_ETC1_UNORM,
	PIXELFORMAT_ETC2_RGB_UNORM,
	PIXELFORMAT_ETC2_RGBA_UNORM,
	PIXELFORMAT_ETC2_RGBA1_UNORM,
	PIXELFORMAT_EAC_R_UNORM,
	PIXELFORMAT_EAC_R_SNORM,
	PIXELFORMAT_EAC_RG_UNORM,
	PIXELFORMAT_EAC_RG_SNORM,
	PIXELFORMAT_ASTC_4x4,
	PIXELFORMAT_ASTC_5x4,
	PIXELFORMAT_ASTC_5x5,
	PIXELFORMAT_ASTC_6x5,
	PIXELFORMAT_ASTC_6x6,
	PIXELFORMAT_ASTC_8x5,
	PIXELFORMAT_ASTC_8x6,
	PIXELFORMAT_ASTC_8x8,
	PIXELFORMAT_ASTC_10x5,
	PIXELFORMAT_ASTC_10x6,
	PIXELFORMAT_ASTC_10x8,
	PIXELFORMAT_ASTC_10x10,
	PIXELFORMAT_ASTC_12x10,
	PIXELFORMAT_ASTC_12x12,

	PIXELFORMAT_MAX_ENUM
};

bool getConstant(PixelFormat in, const char *&out);
bool getConstant(const char *in, PixelFormat &out);

/**
 * Gets whether the specified pixel format is a compressed type.
 **/
bool isPixelFormatCompressed(PixelFormat format);

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
 * Gets the size in bytes of the specified pixel format.
 * NOTE: Currently returns 0 for compressed formats.
 **/
size_t getPixelFormatSize(PixelFormat format);

/**
 * Gets the number of color components in the given pixel format.
 **/
int getPixelFormatColorComponents(PixelFormat format);

} // love
