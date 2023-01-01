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
	PIXELFORMAT_R8,
	PIXELFORMAT_RG8,
	PIXELFORMAT_RGBA8,
	PIXELFORMAT_sRGBA8,
	PIXELFORMAT_R16,
	PIXELFORMAT_RG16,
	PIXELFORMAT_RGBA16,
	PIXELFORMAT_R16F,
	PIXELFORMAT_RG16F,
	PIXELFORMAT_RGBA16F,
	PIXELFORMAT_R32F,
	PIXELFORMAT_RG32F,
	PIXELFORMAT_RGBA32F,

	PIXELFORMAT_LA8, // Same as RG8, but accessed as (L, L, L, A)

	// packed formats
	PIXELFORMAT_RGBA4,    // LSB->MSB: [a, b, g, r]
	PIXELFORMAT_RGB5A1,   // LSB->MSB: [a, b, g, r]
	PIXELFORMAT_RGB565,   // LSB->MSB: [b, g, r]
	PIXELFORMAT_RGB10A2,  // LSB->MSB: [r, g, b, a]
	PIXELFORMAT_RG11B10F, // LSB->MSB: [r, g, b]

	// depth/stencil formats
	PIXELFORMAT_STENCIL8,
	PIXELFORMAT_DEPTH16,
	PIXELFORMAT_DEPTH24,
	PIXELFORMAT_DEPTH32F,
	PIXELFORMAT_DEPTH24_STENCIL8,
	PIXELFORMAT_DEPTH32F_STENCIL8,

	// compressed formats
	PIXELFORMAT_DXT1,
	PIXELFORMAT_DXT3,
	PIXELFORMAT_DXT5,
	PIXELFORMAT_BC4,
	PIXELFORMAT_BC4s,
	PIXELFORMAT_BC5,
	PIXELFORMAT_BC5s,
	PIXELFORMAT_BC6H,
	PIXELFORMAT_BC6Hs,
	PIXELFORMAT_BC7,
	PIXELFORMAT_PVR1_RGB2,
	PIXELFORMAT_PVR1_RGB4,
	PIXELFORMAT_PVR1_RGBA2,
	PIXELFORMAT_PVR1_RGBA4,
	PIXELFORMAT_ETC1,
	PIXELFORMAT_ETC2_RGB,
	PIXELFORMAT_ETC2_RGBA,
	PIXELFORMAT_ETC2_RGBA1,
	PIXELFORMAT_EAC_R,
	PIXELFORMAT_EAC_Rs,
	PIXELFORMAT_EAC_RG,
	PIXELFORMAT_EAC_RGs,
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
