/**
 * Copyright (c) 2006-2020 LOVE Development Team
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

#include "Metal.h"

namespace love
{
namespace graphics
{
namespace metal
{

MTLTextureType Metal::getTextureType(TextureType type, int msaa)
{
	switch (type)
	{
		case TEXTURE_2D: return msaa > 1 ? MTLTextureType2DMultisample : MTLTextureType2D;
		case TEXTURE_VOLUME: return MTLTextureType3D;
		case TEXTURE_2D_ARRAY: return MTLTextureType2DArray;
		case TEXTURE_CUBE: return MTLTextureTypeCube;
		case TEXTURE_MAX_ENUM: return MTLTextureType2D;
	}
	return MTLTextureType2D;
}

MTLPixelFormat Metal::convertPixelFormat(PixelFormat format, bool &isSRGB)
{
	MTLPixelFormat mtlformat = MTLPixelFormatRGBA8Unorm;

	if (format == PIXELFORMAT_RGBA8_UNORM && isSRGB)
		format = PIXELFORMAT_sRGBA8_UNORM;

	if (!isPixelFormatCompressed(format) && format != PIXELFORMAT_sRGBA8_UNORM)
		isSRGB = false;

	switch (format)
	{
	case PIXELFORMAT_R8_UNORM:
		mtlformat = MTLPixelFormatR8Unorm;
		break;
	case PIXELFORMAT_RG8_UNORM:
		mtlformat = MTLPixelFormatRG8Unorm;
		break;
	case PIXELFORMAT_RGBA8_UNORM:
		mtlformat = MTLPixelFormatRGBA8Unorm;
		break;
	case PIXELFORMAT_sRGBA8_UNORM:
		mtlformat = MTLPixelFormatRGBA8Unorm_sRGB;
		break;
	case PIXELFORMAT_R16_UNORM:
		mtlformat = MTLPixelFormatR16Unorm;
		break;
	case PIXELFORMAT_RG16_UNORM:
		mtlformat = MTLPixelFormatRG16Unorm;
		break;
	case PIXELFORMAT_RGBA16_UNORM:
		mtlformat = MTLPixelFormatRGBA16Unorm;
		break;
	case PIXELFORMAT_R16_FLOAT:
		mtlformat = MTLPixelFormatR16Float;
		break;
	case PIXELFORMAT_RG16_FLOAT:
		mtlformat = MTLPixelFormatRG16Float;
		break;
	case PIXELFORMAT_RGBA16_FLOAT:
		mtlformat = MTLPixelFormatRGBA16Float;
		break;
	case PIXELFORMAT_R32_FLOAT:
		mtlformat = MTLPixelFormatR32Float;
		break;
	case PIXELFORMAT_RG32_FLOAT:
		mtlformat = MTLPixelFormatRG32Float;
		break;
	case PIXELFORMAT_RGBA32_FLOAT:
		mtlformat = MTLPixelFormatRGBA32Float;
		break;

	case PIXELFORMAT_LA8_UNORM:
		mtlformat = MTLPixelFormatRGBA8Unorm;
		break;

	case PIXELFORMAT_RGBA4_UNORM:
#ifdef LOVE_IOS
		mtlformat = MTLPixelFormatABGR4Unorm;
#else
		mtlformat = MTLPixelFormatRGBA8Unorm;
#endif
		break;
	case PIXELFORMAT_RGB5A1_UNORM:
#ifdef LOVE_IOS
		mtlformat = MTLPixelFormatA1BGR5Unorm;
#else
		mtlformat = MTLPixelFormatRGBA8Unorm;
#endif
		break;
	case PIXELFORMAT_RGB565_UNORM:
#ifdef LOVE_IOS
		mtlformat = MTLPixelFormatB5G6R5Unorm;
#else
		mtlformat = MTLPixelFormatRGBA8Unorm;
#endif
		break;
	case PIXELFORMAT_RGB10A2_UNORM:
		mtlformat = MTLPixelFormatRGB10A2Unorm;
		break;
	case PIXELFORMAT_RG11B10_FLOAT:
		mtlformat = MTLPixelFormatRG11B10Float;
		break;

	case PIXELFORMAT_STENCIL8:
		mtlformat = MTLPixelFormatStencil8;
		break;
	case PIXELFORMAT_DEPTH16_UNORM:
#ifdef LOVE_IOS
		mtlformat = MTLPixelFormatDepth32Float;
#else
		mtlformat = MTLPixelFormatDepth16Unorm;
#endif
		break;
	case PIXELFORMAT_DEPTH24_UNORM:
		mtlformat = MTLPixelFormatDepth32Float;
		break;
	case PIXELFORMAT_DEPTH32_FLOAT:
		mtlformat = MTLPixelFormatDepth32Float;
		break;
	case PIXELFORMAT_DEPTH24_UNORM_STENCIL8:
		mtlformat = MTLPixelFormatDepth24Unorm_Stencil8;
		break;
	case PIXELFORMAT_DEPTH32_FLOAT_STENCIL8:
		mtlformat = MTLPixelFormatDepth32Float_Stencil8;
		break;

	case PIXELFORMAT_DXT1_UNORM:
#ifndef LOVE_IOS
		mtlformat = isSRGB ? MTLPixelFormatBC1_RGBA_sRGB : MTLPixelFormatBC1_RGBA;
#endif
		break;
	case PIXELFORMAT_DXT3_UNORM:
#ifndef LOVE_IOS
		mtlformat = isSRGB ? MTLPixelFormatBC2_RGBA_sRGB : MTLPixelFormatBC2_RGBA;
#endif
		break;
	case PIXELFORMAT_DXT5_UNORM:
#ifndef LOVE_IOS
		mtlformat = isSRGB ? MTLPixelFormatBC3_RGBA_sRGB : MTLPixelFormatBC3_RGBA;
#endif
		break;
	case PIXELFORMAT_BC4_UNORM:
#ifndef LOVE_IOS
		isSRGB = false;
		mtlformat = MTLPixelFormatBC4_RUnorm;
#endif
		break;
	case PIXELFORMAT_BC4_SNORM:
#ifndef LOVE_IOS
		isSRGB = false;
		mtlformat = MTLPixelFormatBC4_RSnorm;
#endif
		break;
	case PIXELFORMAT_BC5_UNORM:
#ifndef LOVE_IOS
		isSRGB = false;
		mtlformat = MTLPixelFormatBC5_RGUnorm;
#endif
		break;
	case PIXELFORMAT_BC5_SNORM:
#ifndef LOVE_IOS
		isSRGB = false;
		mtlformat = MTLPixelFormatBC5_RGSnorm;
#endif
		break;
	case PIXELFORMAT_BC6H_UFLOAT:
#ifndef LOVE_IOS
		isSRGB = false;
		mtlformat = MTLPixelFormatBC6H_RGBUfloat;
#endif
		break;
	case PIXELFORMAT_BC6H_FLOAT:
#ifndef LOVE_IOS
		isSRGB = false;
		mtlformat = MTLPixelFormatBC6H_RGBFloat;
#endif
		break;
	case PIXELFORMAT_BC7_UNORM:
#ifndef LOVE_IOS
		mtlformat = isSRGB ? MTLPixelFormatBC7_RGBAUnorm_sRGB : MTLPixelFormatBC7_RGBAUnorm;
#endif
		break;

	case PIXELFORMAT_PVR1_RGB2_UNORM:
#ifdef LOVE_IOS
		mtlformat = isSRGB ? MTLPixelFormatPVRTC_RGB_2BPP_sRGB : MTLPixelFormatPVRTC_RGB_2BPP;
#endif
		break;
	case PIXELFORMAT_PVR1_RGB4_UNORM:
#ifdef LOVE_IOS
		mtlformat = isSRGB ? MTLPixelFormatPVRTC_RGB_4BPP_sRGB : MTLPixelFormatPVRTC_RGB_4BPP;
#endif
		break;
	case PIXELFORMAT_PVR1_RGBA2_UNORM:
#ifdef LOVE_IOS
		mtlformat = isSRGB ? MTLPixelFormatPVRTC_RGB_2BPP_sRGB : MTLPixelFormatPVRTC_RGBA_2BPP;
#endif
		break;
	case PIXELFORMAT_PVR1_RGBA4_UNORM:
#ifdef LOVE_IOS
		mtlformat = isSRGB ? MTLPixelFormatPVRTC_RGB_4BPP_sRGB : MTLPixelFormatPVRTC_RGBA_4BPP;
#endif
		break;
	case PIXELFORMAT_ETC1_UNORM:
#ifdef LOVE_IOS
		mtlformat = isSRGB ? MTLPixelFormatETC2_RGB8_sRGB : MTLPixelFormatETC2_RGB8;
#endif
		break;
	case PIXELFORMAT_ETC2_RGB_UNORM:
#ifdef LOVE_IOS
		mtlformat = isSRGB ? MTLPixelFormatETC2_RGB8_sRGB : MTLPixelFormatETC2_RGB8;
#endif
		break;
	case PIXELFORMAT_ETC2_RGBA_UNORM:
#ifdef LOVE_IOS
		mtlformat = isSRGB ? MTLPixelFormatEAC_RGBA8_sRGB : MTLPixelFormatEAC_RGBA8;
#endif
		break;
	case PIXELFORMAT_ETC2_RGBA1_UNORM:
#ifdef LOVE_IOS
		mtlformat = isSRGB ? MTLPixelFormatETC2_RGBA1_sRGB : MTLPixelFormatETC2_RGBA1;
#endif
		break;
	case PIXELFORMAT_EAC_R_UNORM:
		break;
	case PIXELFORMAT_EAC_R_SNORM:
		break;
	case PIXELFORMAT_EAC_RG_UNORM:
		break;
	case PIXELFORMAT_EAC_RG_SNORM:
		break;
	case PIXELFORMAT_ASTC_4x4:
		break;
	case PIXELFORMAT_ASTC_5x4:
		break;
	case PIXELFORMAT_ASTC_5x5:
		break;
	case PIXELFORMAT_ASTC_6x5:
		break;
	case PIXELFORMAT_ASTC_6x6:
		break;
	case PIXELFORMAT_ASTC_8x5:
		break;
	case PIXELFORMAT_ASTC_8x6:
		break;
	case PIXELFORMAT_ASTC_8x8:
		break;
	case PIXELFORMAT_ASTC_10x5:
		break;
	case PIXELFORMAT_ASTC_10x6:
		break;
	case PIXELFORMAT_ASTC_10x8:
		break;
	case PIXELFORMAT_ASTC_10x10:
		break;
	case PIXELFORMAT_ASTC_12x10:
		break;
	case PIXELFORMAT_ASTC_12x12:
		break;

	case PIXELFORMAT_UNKNOWN:
	case PIXELFORMAT_NORMAL:
	case PIXELFORMAT_HDR:
	case PIXELFORMAT_MAX_ENUM:
		break;
	}

	return mtlformat;
}

Metal metal;

} // metal
} // graphics
} // love
