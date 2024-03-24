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

#include "Metal.h"
#include "common/config.h"

namespace love
{
namespace graphics
{
namespace metal
{

Metal::PixelFormatDesc Metal::convertPixelFormat(id<MTLDevice> device, PixelFormat format)
{
	MTLPixelFormat mtlformat = MTLPixelFormatInvalid;
	PixelFormatDesc desc = {};

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
	case PIXELFORMAT_RGBA8_sRGB:
		mtlformat = MTLPixelFormatRGBA8Unorm_sRGB;
		break;
	case PIXELFORMAT_BGRA8_UNORM:
		mtlformat = MTLPixelFormatBGRA8Unorm;
		break;
	case PIXELFORMAT_BGRA8_sRGB:
		mtlformat = MTLPixelFormatBGRA8Unorm_sRGB;
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

	case PIXELFORMAT_R8_INT:
		mtlformat = MTLPixelFormatR8Sint;
		break;
	case PIXELFORMAT_R8_UINT:
		mtlformat = MTLPixelFormatR8Uint;
		break;
	case PIXELFORMAT_RG8_INT:
		mtlformat = MTLPixelFormatRG8Sint;
		break;
	case PIXELFORMAT_RG8_UINT:
		mtlformat = MTLPixelFormatRG8Uint;
		break;
	case PIXELFORMAT_RGBA8_INT:
		mtlformat = MTLPixelFormatRGBA8Sint;
		break;
	case PIXELFORMAT_RGBA8_UINT:
		mtlformat = MTLPixelFormatRGBA8Uint;
		break;
	case PIXELFORMAT_R16_INT:
		mtlformat = MTLPixelFormatR16Sint;
		break;
	case PIXELFORMAT_R16_UINT:
		mtlformat = MTLPixelFormatR16Uint;
		break;
	case PIXELFORMAT_RG16_INT:
		mtlformat = MTLPixelFormatRG16Sint;
		break;
	case PIXELFORMAT_RG16_UINT:
		mtlformat = MTLPixelFormatRG16Uint;
		break;
	case PIXELFORMAT_RGBA16_INT:
		mtlformat = MTLPixelFormatRGBA16Sint;
		break;
	case PIXELFORMAT_RGBA16_UINT:
		mtlformat = MTLPixelFormatRGBA16Uint;
		break;
	case PIXELFORMAT_R32_INT:
		mtlformat = MTLPixelFormatR32Sint;
		break;
	case PIXELFORMAT_R32_UINT:
		mtlformat = MTLPixelFormatR32Uint;
		break;
	case PIXELFORMAT_RG32_INT:
		mtlformat = MTLPixelFormatRG32Sint;
		break;
	case PIXELFORMAT_RG32_UINT:
		mtlformat = MTLPixelFormatRG32Uint;
		break;
	case PIXELFORMAT_RGBA32_INT:
		mtlformat = MTLPixelFormatRGBA32Sint;
		break;
	case PIXELFORMAT_RGBA32_UINT:
		mtlformat = MTLPixelFormatRGBA32Uint;
		break;

	case PIXELFORMAT_LA8_UNORM:
		// Only supported on some systems.
		if (@available(macOS 10.15, iOS 13, *))
		{
			mtlformat = MTLPixelFormatRG8Unorm;
			desc.swizzled = true;
			desc.swizzle.red = MTLTextureSwizzleRed;
			desc.swizzle.green = MTLTextureSwizzleRed;
			desc.swizzle.blue = MTLTextureSwizzleRed;
			desc.swizzle.alpha = MTLTextureSwizzleGreen;
		}
		break;

	case PIXELFORMAT_RGBA4_UNORM:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatABGR4Unorm;
		break;
	case PIXELFORMAT_RGB5A1_UNORM:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatA1BGR5Unorm;
		break;
	case PIXELFORMAT_RGB565_UNORM:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatB5G6R5Unorm;
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
		if (@available(macOS 10.12, iOS 13.0, *))
			mtlformat = MTLPixelFormatDepth16Unorm;
		else
			mtlformat = MTLPixelFormatDepth32Float;
		break;
	case PIXELFORMAT_DEPTH24_UNORM:
		mtlformat = MTLPixelFormatDepth32Float;
		break;
	case PIXELFORMAT_DEPTH32_FLOAT:
		mtlformat = MTLPixelFormatDepth32Float;
		break;
	case PIXELFORMAT_DEPTH24_UNORM_STENCIL8:
#ifdef LOVE_IOS
		mtlformat = MTLPixelFormatDepth32Float_Stencil8;
#else
		if ([device isDepth24Stencil8PixelFormatSupported])
			mtlformat = MTLPixelFormatDepth24Unorm_Stencil8;
		else
			mtlformat = MTLPixelFormatDepth32Float_Stencil8;
#endif
		break;
	case PIXELFORMAT_DEPTH32_FLOAT_STENCIL8:
		mtlformat = MTLPixelFormatDepth32Float_Stencil8;
		break;

	case PIXELFORMAT_DXT1_UNORM:
#ifndef LOVE_IOS
		mtlformat = MTLPixelFormatBC1_RGBA;
#endif
		break;
	case PIXELFORMAT_DXT1_sRGB:
#ifndef LOVE_IOS
		mtlformat = MTLPixelFormatBC1_RGBA_sRGB;
#endif
		break;
	case PIXELFORMAT_DXT3_UNORM:
#ifndef LOVE_IOS
		mtlformat = MTLPixelFormatBC2_RGBA;
#endif
		break;
	case PIXELFORMAT_DXT3_sRGB:
#ifndef LOVE_IOS
		mtlformat = MTLPixelFormatBC2_RGBA_sRGB;
#endif
		break;
	case PIXELFORMAT_DXT5_UNORM:
#ifndef LOVE_IOS
		mtlformat = MTLPixelFormatBC3_RGBA;
#endif
		break;
	case PIXELFORMAT_DXT5_sRGB:
#ifndef LOVE_IOS
		mtlformat = MTLPixelFormatBC3_RGBA_sRGB;
#endif
		break;
	case PIXELFORMAT_BC4_UNORM:
#ifndef LOVE_IOS
		mtlformat = MTLPixelFormatBC4_RUnorm;
#endif
		break;
	case PIXELFORMAT_BC4_SNORM:
#ifndef LOVE_IOS
		mtlformat = MTLPixelFormatBC4_RSnorm;
#endif
		break;
	case PIXELFORMAT_BC5_UNORM:
#ifndef LOVE_IOS
		mtlformat = MTLPixelFormatBC5_RGUnorm;
#endif
		break;
	case PIXELFORMAT_BC5_SNORM:
#ifndef LOVE_IOS
		mtlformat = MTLPixelFormatBC5_RGSnorm;
#endif
		break;
	case PIXELFORMAT_BC6H_UFLOAT:
#ifndef LOVE_IOS
		mtlformat = MTLPixelFormatBC6H_RGBUfloat;
#endif
		break;
	case PIXELFORMAT_BC6H_FLOAT:
#ifndef LOVE_IOS
		mtlformat = MTLPixelFormatBC6H_RGBFloat;
#endif
		break;
	case PIXELFORMAT_BC7_UNORM:
#ifndef LOVE_IOS
		mtlformat = MTLPixelFormatBC7_RGBAUnorm;
#endif
		break;
	case PIXELFORMAT_BC7_sRGB:
#ifndef LOVE_IOS
		mtlformat = MTLPixelFormatBC7_RGBAUnorm_sRGB;
#endif
		break;

	case PIXELFORMAT_PVR1_RGB2_UNORM:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatPVRTC_RGB_2BPP;
		break;
	case PIXELFORMAT_PVR1_RGB2_sRGB:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatPVRTC_RGB_2BPP_sRGB;
		break;
	case PIXELFORMAT_PVR1_RGB4_UNORM:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatPVRTC_RGB_4BPP;
		break;
	case PIXELFORMAT_PVR1_RGB4_sRGB:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatPVRTC_RGB_4BPP_sRGB;
		break;
	case PIXELFORMAT_PVR1_RGBA2_UNORM:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatPVRTC_RGBA_2BPP;
		break;
	case PIXELFORMAT_PVR1_RGBA2_sRGB:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatPVRTC_RGB_2BPP_sRGB;
		break;
	case PIXELFORMAT_PVR1_RGBA4_UNORM:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatPVRTC_RGBA_4BPP;
		break;
	case PIXELFORMAT_PVR1_RGBA4_sRGB:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatPVRTC_RGB_4BPP_sRGB;
		break;

	case PIXELFORMAT_ETC1_UNORM:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatETC2_RGB8;
		break;
	case PIXELFORMAT_ETC2_RGB_UNORM:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatETC2_RGB8;
		break;
	case PIXELFORMAT_ETC2_RGB_sRGB:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat =  MTLPixelFormatETC2_RGB8_sRGB;
		break;
	case PIXELFORMAT_ETC2_RGBA_UNORM:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatEAC_RGBA8;
		break;
	case PIXELFORMAT_ETC2_RGBA_sRGB:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatEAC_RGBA8_sRGB;
		break;
	case PIXELFORMAT_ETC2_RGBA1_UNORM:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatETC2_RGB8A1;
		break;
	case PIXELFORMAT_ETC2_RGBA1_sRGB:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatETC2_RGB8A1_sRGB;
		break;
	case PIXELFORMAT_EAC_R_UNORM:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatEAC_R11Unorm;
		break;
	case PIXELFORMAT_EAC_R_SNORM:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatEAC_R11Snorm;
		break;
	case PIXELFORMAT_EAC_RG_UNORM:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatEAC_RG11Unorm;
		break;
	case PIXELFORMAT_EAC_RG_SNORM:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatEAC_RG11Snorm;
		break;

	case PIXELFORMAT_ASTC_4x4_UNORM:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatASTC_4x4_LDR;
		break;
	case PIXELFORMAT_ASTC_5x4_UNORM:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatASTC_5x4_LDR;
		break;
	case PIXELFORMAT_ASTC_5x5_UNORM:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatASTC_5x5_LDR;
		break;
	case PIXELFORMAT_ASTC_6x5_UNORM:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatASTC_6x5_LDR;
		break;
	case PIXELFORMAT_ASTC_6x6_UNORM:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatASTC_6x6_LDR;
		break;
	case PIXELFORMAT_ASTC_8x5_UNORM:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatASTC_8x5_LDR;
		break;
	case PIXELFORMAT_ASTC_8x6_UNORM:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatASTC_8x6_LDR;
		break;
	case PIXELFORMAT_ASTC_8x8_UNORM:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatASTC_8x8_LDR;
		break;
	case PIXELFORMAT_ASTC_10x5_UNORM:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatASTC_10x5_LDR;
		break;
	case PIXELFORMAT_ASTC_10x6_UNORM:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatASTC_10x6_LDR;
		break;
	case PIXELFORMAT_ASTC_10x8_UNORM:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatASTC_10x8_LDR;
		break;
	case PIXELFORMAT_ASTC_10x10_UNORM:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatASTC_10x10_LDR;
		break;
	case PIXELFORMAT_ASTC_12x10_UNORM:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatASTC_12x10_LDR;
		break;
	case PIXELFORMAT_ASTC_12x12_UNORM:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatASTC_12x12_LDR;
		break;
	case PIXELFORMAT_ASTC_4x4_sRGB:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatASTC_4x4_sRGB;
		break;
	case PIXELFORMAT_ASTC_5x4_sRGB:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatASTC_5x4_sRGB;
		break;
	case PIXELFORMAT_ASTC_5x5_sRGB:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatASTC_5x5_sRGB;
		break;
	case PIXELFORMAT_ASTC_6x5_sRGB:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatASTC_6x5_sRGB;
		break;
	case PIXELFORMAT_ASTC_6x6_sRGB:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatASTC_6x6_sRGB;
		break;
	case PIXELFORMAT_ASTC_8x5_sRGB:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatASTC_8x5_sRGB;
		break;
	case PIXELFORMAT_ASTC_8x6_sRGB:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatASTC_8x6_sRGB;
		break;
	case PIXELFORMAT_ASTC_8x8_sRGB:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatASTC_8x8_sRGB;
		break;
	case PIXELFORMAT_ASTC_10x5_sRGB:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatASTC_10x5_sRGB;
		break;
	case PIXELFORMAT_ASTC_10x6_sRGB:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatASTC_10x6_sRGB;
		break;
	case PIXELFORMAT_ASTC_10x8_sRGB:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatASTC_10x8_sRGB;
		break;
	case PIXELFORMAT_ASTC_10x10_sRGB:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatASTC_10x10_sRGB;
		break;
	case PIXELFORMAT_ASTC_12x10_sRGB:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatASTC_12x10_sRGB;
		break;
	case PIXELFORMAT_ASTC_12x12_sRGB:
		if (@available(macOS 11.0, iOS 8.0, *))
			mtlformat = MTLPixelFormatASTC_12x12_sRGB;
		break;

	case PIXELFORMAT_UNKNOWN:
	case PIXELFORMAT_NORMAL:
	case PIXELFORMAT_HDR:
	case PIXELFORMAT_MAX_ENUM:
		break;
	}

	desc.format = mtlformat;
	return desc;
}

} // metal
} // graphics
} // love
