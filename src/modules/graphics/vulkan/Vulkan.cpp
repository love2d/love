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

#include "Vulkan.h"

#include <sstream>


namespace love
{
namespace graphics
{
namespace vulkan
{

static uint32_t numShaderSwitches;

void Vulkan::shaderSwitch()
{
	numShaderSwitches++;
}

uint32_t Vulkan::getNumShaderSwitches()
{
	return numShaderSwitches;
}

void Vulkan::resetShaderSwitches()
{
	numShaderSwitches = 0;
}

VkFormat Vulkan::getVulkanVertexFormat(DataFormat format)
{
	switch (format)
	{
	case DATAFORMAT_FLOAT:
		return VK_FORMAT_R32_SFLOAT;
	case DATAFORMAT_FLOAT_VEC2:
		return VK_FORMAT_R32G32_SFLOAT;
	case DATAFORMAT_FLOAT_VEC3:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case DATAFORMAT_FLOAT_VEC4:
		return VK_FORMAT_R32G32B32A32_SFLOAT;

	case DATAFORMAT_FLOAT_MAT2X2:
	case DATAFORMAT_FLOAT_MAT2X3:
	case DATAFORMAT_FLOAT_MAT2X4:
	case DATAFORMAT_FLOAT_MAT3X2:
	case DATAFORMAT_FLOAT_MAT3X3:
	case DATAFORMAT_FLOAT_MAT3X4:
	case DATAFORMAT_FLOAT_MAT4X2:
	case DATAFORMAT_FLOAT_MAT4X3:
	case DATAFORMAT_FLOAT_MAT4X4:
		throw love::Exception("unimplemented data format (matnxm)");

	case DATAFORMAT_INT32:
		return VK_FORMAT_R32_SINT;
	case DATAFORMAT_INT32_VEC2:
		return VK_FORMAT_R32G32_SINT;
	case DATAFORMAT_INT32_VEC3:
		return VK_FORMAT_R32G32B32_SINT;
	case DATAFORMAT_INT32_VEC4:
		return VK_FORMAT_R32G32B32A32_SINT;

	case DATAFORMAT_UINT32:
		return VK_FORMAT_R32_UINT;
	case DATAFORMAT_UINT32_VEC2:
		return VK_FORMAT_R32G32_UINT;
	case DATAFORMAT_UINT32_VEC3:
		return VK_FORMAT_R32G32B32_UINT;
	case DATAFORMAT_UINT32_VEC4:
		return VK_FORMAT_R32G32B32A32_UINT;

	case DATAFORMAT_SNORM8_VEC4:
		return VK_FORMAT_R8G8B8A8_SNORM;
	case DATAFORMAT_UNORM8_VEC4:
		return VK_FORMAT_R8G8B8A8_UNORM;
	case DATAFORMAT_INT8_VEC4:
		return VK_FORMAT_R8G8B8A8_SINT;
	case DATAFORMAT_UINT8_VEC4:
		return VK_FORMAT_R8G8B8A8_UINT;

	case DATAFORMAT_SNORM16_VEC2:
		return VK_FORMAT_R16G16_SNORM;
	case DATAFORMAT_SNORM16_VEC4:
		return VK_FORMAT_R16G16B16A16_SNORM;
	case DATAFORMAT_UNORM16_VEC2:
		return VK_FORMAT_R16G16_UNORM;
	case DATAFORMAT_UNORM16_VEC4:
		return VK_FORMAT_R16G16B16A16_UNORM;

	case DATAFORMAT_INT16_VEC2:
		return VK_FORMAT_R16G16_SINT;
	case DATAFORMAT_INT16_VEC4:
		return VK_FORMAT_R16G16B16A16_SINT;

	case DATAFORMAT_UINT16:
		return VK_FORMAT_R16_UINT;
	case DATAFORMAT_UINT16_VEC2:
		return VK_FORMAT_R16G16_UINT;
	case DATAFORMAT_UINT16_VEC4:
		return VK_FORMAT_R16G16B16A16_UINT;

	case DATAFORMAT_BOOL:
	case DATAFORMAT_BOOL_VEC2:
	case DATAFORMAT_BOOL_VEC3:
	case DATAFORMAT_BOOL_VEC4:
		throw love::Exception("unimplemented data format (bool)");

	default:
		throw love::Exception("unknown data format");
	}
}

TextureFormat Vulkan::getTextureFormat(PixelFormat format)
{
	TextureFormat textureFormat{};

	switch (format)
	{
	case PIXELFORMAT_UNKNOWN:
		throw love::Exception("unknown pixel format");
	case PIXELFORMAT_NORMAL:
		textureFormat.internalFormat = VK_FORMAT_R8G8B8A8_SRGB;
		break;
	case PIXELFORMAT_HDR:
		throw love::Exception("unimplemented pixel format: hdr");
	case PIXELFORMAT_R8_UNORM:
		textureFormat.internalFormat = VK_FORMAT_R8_UNORM;
		break;
	case PIXELFORMAT_R8_INT:
		textureFormat.internalFormat = VK_FORMAT_R8_SINT;
		textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_SINT;
		break;
	case PIXELFORMAT_R8_UINT:
		textureFormat.internalFormat = VK_FORMAT_R8_UINT;
		textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_UINT;
		break;
	case PIXELFORMAT_R16_UNORM:
		textureFormat.internalFormat = VK_FORMAT_R16_UNORM;
		break;
	case PIXELFORMAT_R16_FLOAT:
		textureFormat.internalFormat = VK_FORMAT_R16_SFLOAT;
		break;
	case PIXELFORMAT_R16_INT:
		textureFormat.internalFormat = VK_FORMAT_R16_SINT;
		textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_SINT;
		break;
	case PIXELFORMAT_R16_UINT:
		textureFormat.internalFormat = VK_FORMAT_R16_UINT;
		textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_UINT;
		break;
	case PIXELFORMAT_R32_FLOAT:
		textureFormat.internalFormat = VK_FORMAT_R32_SFLOAT;
		break;
	case PIXELFORMAT_R32_INT:
		textureFormat.internalFormat = VK_FORMAT_R32_SINT;
		textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_SINT;
		break;
	case PIXELFORMAT_R32_UINT:
		textureFormat.internalFormat = VK_FORMAT_R32_UINT;
		textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_UINT;
		break;
	case PIXELFORMAT_RG8_UNORM:
		textureFormat.internalFormat = VK_FORMAT_R8G8_UNORM;
		break;
	case PIXELFORMAT_RG8_INT:
		textureFormat.internalFormat = VK_FORMAT_R8G8_SINT;
		textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_SINT;
		break;
	case PIXELFORMAT_RG8_UINT:
		textureFormat.internalFormat = VK_FORMAT_R8G8_UINT;
		textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_UINT;
		break;
	case PIXELFORMAT_LA8_UNORM: // Same as RG8: but accessed as (L: L: L: A)
		textureFormat.internalFormat = VK_FORMAT_R8G8_UNORM;
		textureFormat.swizzleR = VK_COMPONENT_SWIZZLE_R;
		textureFormat.swizzleG = VK_COMPONENT_SWIZZLE_R;
		textureFormat.swizzleB = VK_COMPONENT_SWIZZLE_R;
		textureFormat.swizzleA = VK_COMPONENT_SWIZZLE_G;
		break;
	case PIXELFORMAT_RG16_UNORM:
		textureFormat.internalFormat = VK_FORMAT_R16G16_UNORM;
		break;
	case PIXELFORMAT_RG16_FLOAT:
		textureFormat.internalFormat = VK_FORMAT_R16G16_SFLOAT;
		break;
	case PIXELFORMAT_RG16_INT:
		textureFormat.internalFormat = VK_FORMAT_R16G16_SINT;
		textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_SINT;
		break;
	case PIXELFORMAT_RG16_UINT:
		textureFormat.internalFormat = VK_FORMAT_R16G16_UINT;
		textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_UINT;
		break;
	case PIXELFORMAT_RG32_FLOAT:
		textureFormat.internalFormat = VK_FORMAT_R32G32_SFLOAT;
		break;
	case PIXELFORMAT_RG32_INT:
		textureFormat.internalFormat = VK_FORMAT_R32G32_SINT;
		textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_SINT;
		break;
	case PIXELFORMAT_RG32_UINT:
		textureFormat.internalFormat = VK_FORMAT_R32G32_UINT;
		textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_UINT;
		break;
	case PIXELFORMAT_RGBA8_UNORM:
		textureFormat.internalFormat = VK_FORMAT_R8G8B8A8_UNORM;
		break;
	case PIXELFORMAT_RGBA8_sRGB:
		textureFormat.internalFormat = VK_FORMAT_R8G8B8A8_SRGB;
		break;
	case PIXELFORMAT_BGRA8_UNORM:
		textureFormat.internalFormat = VK_FORMAT_B8G8R8A8_UNORM;
		break;
	case PIXELFORMAT_BGRA8_sRGB:
		textureFormat.internalFormat = VK_FORMAT_B8G8R8A8_SRGB;
		break;
	case PIXELFORMAT_RGBA8_INT:
		textureFormat.internalFormat = VK_FORMAT_R8G8B8A8_SINT;
		textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_SINT;
		break;
	case PIXELFORMAT_RGBA8_UINT:
		textureFormat.internalFormat = VK_FORMAT_R8G8B8A8_UINT;
		textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_UINT;
		break;
	case PIXELFORMAT_RGBA16_UNORM:
		textureFormat.internalFormat = VK_FORMAT_R16G16B16A16_UNORM;
		break;
	case PIXELFORMAT_RGBA16_FLOAT:
		textureFormat.internalFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
		break;
	case PIXELFORMAT_RGBA16_INT:
		textureFormat.internalFormat = VK_FORMAT_R16G16B16A16_SINT;
		textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_SINT;
		break;
	case PIXELFORMAT_RGBA16_UINT:
		textureFormat.internalFormat = VK_FORMAT_R16G16B16A16_UINT;
		textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_UINT;
		break;
	case PIXELFORMAT_RGBA32_FLOAT:
		textureFormat.internalFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
		break;
	case PIXELFORMAT_RGBA32_INT:
		textureFormat.internalFormat = VK_FORMAT_R32G32B32A32_SINT;
		textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_SINT;
		break;
	case PIXELFORMAT_RGBA32_UINT:
		textureFormat.internalFormat = VK_FORMAT_R32G32B32A32_UINT;
		textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_UINT;
		break;
	case PIXELFORMAT_RGBA4_UNORM:    // LSB->MSB: [a: b: g: r]
		textureFormat.internalFormat = VK_FORMAT_R4G4B4A4_UNORM_PACK16;
		break;
	case PIXELFORMAT_RGB5A1_UNORM:   // LSB->MSB: [a: b: g: r]
		textureFormat.internalFormat = VK_FORMAT_R5G5B5A1_UNORM_PACK16;
		break;
	case PIXELFORMAT_RGB565_UNORM:   // LSB->MSB: [b: g: r]
		textureFormat.internalFormat = VK_FORMAT_R5G6B5_UNORM_PACK16;
		break;
	case PIXELFORMAT_RGB10A2_UNORM:  // LSB->MSB: [r: g: b: a]
		textureFormat.internalFormat = VK_FORMAT_A2B10G10R10_UNORM_PACK32;
		break;
	case PIXELFORMAT_RG11B10_FLOAT:  // LSB->MSB: [r: g: b]
		textureFormat.internalFormat = VK_FORMAT_B10G11R11_UFLOAT_PACK32;
		break;
	case PIXELFORMAT_STENCIL8:
		textureFormat.internalFormat = VK_FORMAT_S8_UINT;
		textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_UINT;
		break;
	case PIXELFORMAT_DEPTH16_UNORM:
		textureFormat.internalFormat = VK_FORMAT_D16_UNORM;
		textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_UINT;
		break;
	case PIXELFORMAT_DEPTH24_UNORM:
		textureFormat.internalFormat = VK_FORMAT_X8_D24_UNORM_PACK32;
		textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_UINT;
		break;
	case PIXELFORMAT_DEPTH24_UNORM_STENCIL8:
		textureFormat.internalFormat = VK_FORMAT_D24_UNORM_S8_UINT;
		textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_UINT;
		break;
	case PIXELFORMAT_DEPTH32_FLOAT:
		textureFormat.internalFormat = VK_FORMAT_D32_SFLOAT;
		break;
	case PIXELFORMAT_DEPTH32_FLOAT_STENCIL8:
		textureFormat.internalFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
		break;
	case PIXELFORMAT_DXT1_UNORM:
		textureFormat.internalFormat = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
		break;
	case PIXELFORMAT_DXT1_sRGB:
		textureFormat.internalFormat = VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
		break;
	case PIXELFORMAT_DXT3_UNORM:
		textureFormat.internalFormat = VK_FORMAT_BC2_UNORM_BLOCK;
		break;
	case PIXELFORMAT_DXT3_sRGB:
		textureFormat.internalFormat = VK_FORMAT_BC2_SRGB_BLOCK;
		break;
	case PIXELFORMAT_DXT5_UNORM:
		textureFormat.internalFormat = VK_FORMAT_BC3_UNORM_BLOCK;
		break;
	case PIXELFORMAT_DXT5_sRGB:
		textureFormat.internalFormat = VK_FORMAT_BC3_SRGB_BLOCK;
		break;
	case PIXELFORMAT_BC4_UNORM:
		textureFormat.internalFormat = VK_FORMAT_BC4_UNORM_BLOCK;
		break;
	case PIXELFORMAT_BC4_SNORM:
		textureFormat.internalFormat = VK_FORMAT_BC4_SNORM_BLOCK;
		break;
	case PIXELFORMAT_BC5_UNORM:
		textureFormat.internalFormat = VK_FORMAT_BC5_UNORM_BLOCK;
		break;
	case PIXELFORMAT_BC5_SNORM:
		textureFormat.internalFormat = VK_FORMAT_BC5_SNORM_BLOCK;
		break;
	case PIXELFORMAT_BC6H_UFLOAT:
		textureFormat.internalFormat = VK_FORMAT_BC6H_UFLOAT_BLOCK;
		break;
	case PIXELFORMAT_BC6H_FLOAT:
		textureFormat.internalFormat = VK_FORMAT_BC6H_SFLOAT_BLOCK;
		break;
	case PIXELFORMAT_BC7_UNORM:
		textureFormat.internalFormat = VK_FORMAT_BC7_UNORM_BLOCK;
		break;
	case PIXELFORMAT_BC7_sRGB:
		textureFormat.internalFormat = VK_FORMAT_BC7_SRGB_BLOCK;
		break;
	case PIXELFORMAT_PVR1_RGB2_UNORM:
		textureFormat.internalFormat = VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG;
		break;
	case PIXELFORMAT_PVR1_RGB2_sRGB:
		textureFormat.internalFormat = VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG;
		break;
	case PIXELFORMAT_PVR1_RGB4_UNORM:
		textureFormat.internalFormat = VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG;
		break;
	case PIXELFORMAT_PVR1_RGB4_sRGB:
		textureFormat.internalFormat = VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG;
		break;
	case PIXELFORMAT_PVR1_RGBA2_UNORM:
		textureFormat.internalFormat = VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG;
		break;
	case PIXELFORMAT_PVR1_RGBA2_sRGB:
		textureFormat.internalFormat = VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG;
		break;
	case PIXELFORMAT_PVR1_RGBA4_UNORM:
		textureFormat.internalFormat = VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG;
		break;
	case PIXELFORMAT_PVR1_RGBA4_sRGB:
		textureFormat.internalFormat = VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG;
		break;
	case PIXELFORMAT_ETC1_UNORM:
		textureFormat.internalFormat = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
		break;
	case PIXELFORMAT_ETC2_RGB_UNORM:
		textureFormat.internalFormat = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
		break;
	case PIXELFORMAT_ETC2_RGB_sRGB:
		textureFormat.internalFormat = VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
		break;
	case PIXELFORMAT_ETC2_RGBA_UNORM:
		textureFormat.internalFormat = VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
		break;
	case PIXELFORMAT_ETC2_RGBA_sRGB:
		textureFormat.internalFormat = VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
		break;
	case PIXELFORMAT_ETC2_RGBA1_UNORM:
		textureFormat.internalFormat = VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
		break;
	case PIXELFORMAT_ETC2_RGBA1_sRGB:
		textureFormat.internalFormat = VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;
		break;
	case PIXELFORMAT_EAC_R_UNORM:
		textureFormat.internalFormat = VK_FORMAT_EAC_R11_UNORM_BLOCK;
		break;
	case PIXELFORMAT_EAC_R_SNORM:
		textureFormat.internalFormat = VK_FORMAT_EAC_R11_SNORM_BLOCK;
		break;
	case PIXELFORMAT_EAC_RG_UNORM:
		textureFormat.internalFormat = VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
		break;
	case PIXELFORMAT_EAC_RG_SNORM:
		textureFormat.internalFormat = VK_FORMAT_EAC_R11G11_SNORM_BLOCK;
		break;
	case PIXELFORMAT_ASTC_4x4_UNORM:
		textureFormat.internalFormat = VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
		break;
	case PIXELFORMAT_ASTC_5x4_UNORM:
		textureFormat.internalFormat = VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
		break;
	case PIXELFORMAT_ASTC_5x5_UNORM:
		textureFormat.internalFormat = VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
		break;
	case PIXELFORMAT_ASTC_6x5_UNORM:
		textureFormat.internalFormat = VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
		break;
	case PIXELFORMAT_ASTC_6x6_UNORM:
		textureFormat.internalFormat = VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
		break;
	case PIXELFORMAT_ASTC_8x5_UNORM:
		textureFormat.internalFormat = VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
		break;
	case PIXELFORMAT_ASTC_8x6_UNORM:
		textureFormat.internalFormat = VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
		break;
	case PIXELFORMAT_ASTC_8x8_UNORM:
		textureFormat.internalFormat = VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
		break;
	case PIXELFORMAT_ASTC_10x5_UNORM:
		textureFormat.internalFormat = VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
		break;
	case PIXELFORMAT_ASTC_10x6_UNORM:
		textureFormat.internalFormat = VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
		break;
	case PIXELFORMAT_ASTC_10x8_UNORM:
		textureFormat.internalFormat = VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
		break;
	case PIXELFORMAT_ASTC_10x10_UNORM:
		textureFormat.internalFormat = VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
		break;
	case PIXELFORMAT_ASTC_12x10_UNORM:
		textureFormat.internalFormat = VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
		break;
	case PIXELFORMAT_ASTC_12x12_UNORM:
		textureFormat.internalFormat = VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
		break;
	case PIXELFORMAT_ASTC_4x4_sRGB:
		textureFormat.internalFormat = VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
		break;
	case PIXELFORMAT_ASTC_5x4_sRGB:
		textureFormat.internalFormat = VK_FORMAT_ASTC_5x4_SRGB_BLOCK;
		break;
	case PIXELFORMAT_ASTC_5x5_sRGB:
		textureFormat.internalFormat = VK_FORMAT_ASTC_5x5_SRGB_BLOCK;
		break;
	case PIXELFORMAT_ASTC_6x5_sRGB:
		textureFormat.internalFormat = VK_FORMAT_ASTC_6x5_SRGB_BLOCK;
		break;
	case PIXELFORMAT_ASTC_6x6_sRGB:
		textureFormat.internalFormat = VK_FORMAT_ASTC_6x6_SRGB_BLOCK;
		break;
	case PIXELFORMAT_ASTC_8x5_sRGB:
		textureFormat.internalFormat = VK_FORMAT_ASTC_8x5_SRGB_BLOCK;
		break;
	case PIXELFORMAT_ASTC_8x6_sRGB:
		textureFormat.internalFormat = VK_FORMAT_ASTC_8x6_SRGB_BLOCK;
		break;
	case PIXELFORMAT_ASTC_8x8_sRGB:
		textureFormat.internalFormat = VK_FORMAT_ASTC_8x8_SRGB_BLOCK;
		break;
	case PIXELFORMAT_ASTC_10x5_sRGB:
		textureFormat.internalFormat = VK_FORMAT_ASTC_10x5_SRGB_BLOCK;
		break;
	case PIXELFORMAT_ASTC_10x6_sRGB:
		textureFormat.internalFormat = VK_FORMAT_ASTC_10x6_SRGB_BLOCK;
		break;
	case PIXELFORMAT_ASTC_10x8_sRGB:
		textureFormat.internalFormat = VK_FORMAT_ASTC_10x8_SRGB_BLOCK;
		break;
	case PIXELFORMAT_ASTC_10x10_sRGB:
		textureFormat.internalFormat = VK_FORMAT_ASTC_10x10_SRGB_BLOCK;
		break;
	case PIXELFORMAT_ASTC_12x10_sRGB:
		textureFormat.internalFormat = VK_FORMAT_ASTC_12x10_SRGB_BLOCK;
		break;
	case PIXELFORMAT_ASTC_12x12_sRGB:
		textureFormat.internalFormat = VK_FORMAT_ASTC_12x12_SRGB_BLOCK;
		break;
	default:
		throw love::Exception("unknown pixel format");
	}

	return textureFormat;
}

// values taken from https://pcisig.com/membership/member-companies and https://vulkan.gpuinfo.org/displaycoreproperty.php?name=vendorid&platform=all
// as specified at https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkPhysicalDeviceProperties.html
std::string Vulkan::getVendorName(uint32_t vendorId)
{
	switch (vendorId)
	{
	case 4130:
	case 4098:
		return "AMD";
	case 4318:
		return "Nvidia";
	case 32902:
		return "Intel";
	case 4203:
		return "Apple";
	case 5140:
		return "Microsoft";
	case 5045:
		return "ARM";
	case 20803:
		return "Qualcomm";
	case 5348:
		return "Broadcom";
	case 65541:
		return "Mesa";
	default:
		return "unknown";
	}
}

std::string Vulkan::getVulkanApiVersion(uint32_t version)
{
	std::stringstream ss;

	ss << VK_API_VERSION_MAJOR(version) 
		<< "." << VK_API_VERSION_MINOR(version) 
		<< "." << VK_API_VERSION_PATCH(version);

	return ss.str();
}

VkPrimitiveTopology Vulkan::getPrimitiveTypeTopology(graphics::PrimitiveType primitiveType)
{
	switch (primitiveType)
	{
	case PRIMITIVE_POINTS:
		return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
	case PRIMITIVE_TRIANGLES:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	case PRIMITIVE_TRIANGLE_FAN:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
	case PRIMITIVE_TRIANGLE_STRIP:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	default:
		throw love::Exception("unknown primitive type");
	}
}

VkBlendFactor Vulkan::getBlendFactor(BlendFactor blendFactor)
{
	switch (blendFactor)
	{
	case BLENDFACTOR_ZERO:
		return VK_BLEND_FACTOR_ZERO;
	case BLENDFACTOR_ONE:
		return VK_BLEND_FACTOR_ONE;
	case BLENDFACTOR_SRC_COLOR:
		return VK_BLEND_FACTOR_SRC_COLOR;
	case BLENDFACTOR_ONE_MINUS_SRC_COLOR:
		return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
	case BLENDFACTOR_SRC_ALPHA:
		return VK_BLEND_FACTOR_SRC_ALPHA;
	case BLENDFACTOR_ONE_MINUS_SRC_ALPHA:
		return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	case BLENDFACTOR_DST_COLOR:
		return VK_BLEND_FACTOR_DST_COLOR;
	case BLENDFACTOR_ONE_MINUS_DST_COLOR:
		return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
	case BLENDFACTOR_DST_ALPHA:
		return VK_BLEND_FACTOR_DST_ALPHA;
	case BLENDFACTOR_ONE_MINUS_DST_ALPHA:
		return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
	case BLENDFACTOR_SRC_ALPHA_SATURATED:
		return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
	default:
		throw love::Exception("unknown blend factor");
	}
}

VkBlendOp Vulkan::getBlendOp(BlendOperation op)
{
	switch (op)
	{
	case BLENDOP_ADD:
		return VK_BLEND_OP_ADD;
	case BLENDOP_MAX:
		return VK_BLEND_OP_MAX;
	case BLENDOP_MIN:
		return VK_BLEND_OP_MIN;
	case BLENDOP_SUBTRACT:
		return VK_BLEND_OP_SUBTRACT;
	case BLENDOP_REVERSE_SUBTRACT:
		return VK_BLEND_OP_REVERSE_SUBTRACT;
	default:
		throw love::Exception("unknown blend operation");
	}
}

VkBool32 Vulkan::getBool(bool b)
{
	if (b)
		return VK_TRUE;
	else
		return VK_FALSE;
}

VkColorComponentFlags Vulkan::getColorMask(ColorChannelMask mask)
{
	VkColorComponentFlags flags = 0;

	if (mask.r)
		flags |= VK_COLOR_COMPONENT_R_BIT;
	if (mask.g)
		flags |= VK_COLOR_COMPONENT_G_BIT;
	if (mask.b)
		flags |= VK_COLOR_COMPONENT_B_BIT;
	if (mask.a)
		flags |= VK_COLOR_COMPONENT_A_BIT;

	return flags;
}

VkFrontFace Vulkan::getFrontFace(Winding winding)
{
	switch (winding)
	{
	case WINDING_CW:
		return VK_FRONT_FACE_CLOCKWISE;
	case WINDING_CCW:
		return VK_FRONT_FACE_COUNTER_CLOCKWISE;
	default:
		throw love::Exception("unknown winding");
	}
}

VkCullModeFlags Vulkan::getCullMode(CullMode cullmode)
{
	switch (cullmode)
	{
	case CULL_BACK:
		return VK_CULL_MODE_BACK_BIT;
	case CULL_FRONT:
		return VK_CULL_MODE_FRONT_BIT;
	case CULL_NONE:
		return VK_CULL_MODE_NONE;
	default:
		throw love::Exception("unknown cull mode");
	}
}

VkImageType Vulkan::getImageType(TextureType textureType)
{
	switch (textureType)
	{
	case TEXTURE_2D:
	case TEXTURE_2D_ARRAY:
	case TEXTURE_CUBE:
		return VK_IMAGE_TYPE_2D;
	case TEXTURE_VOLUME:
		return VK_IMAGE_TYPE_3D;
	default:
		throw love::Exception("unknown texture type");
	}
}

VkImageViewType Vulkan::getImageViewType(TextureType textureType)
{
	switch (textureType)
	{
	case TEXTURE_2D:
		return VK_IMAGE_VIEW_TYPE_2D;
	case TEXTURE_2D_ARRAY:
		return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	case TEXTURE_CUBE:
		return VK_IMAGE_VIEW_TYPE_CUBE;
	case TEXTURE_VOLUME:
		return VK_IMAGE_VIEW_TYPE_3D;
	default:
		throw love::Exception("unknown texture type");
	}
}

VkPolygonMode Vulkan::getPolygonMode(bool wireframe)
{
	if (wireframe)
		return VK_POLYGON_MODE_LINE;
	else
		return VK_POLYGON_MODE_FILL;
}

VkFilter Vulkan::getFilter(SamplerState::FilterMode mode)
{
	switch (mode)
	{
	case SamplerState::FILTER_LINEAR:
		return VK_FILTER_LINEAR;
	case SamplerState::FILTER_NEAREST:
		return VK_FILTER_NEAREST;
	default:
		throw love::Exception("unkonwn filter mode");
	}
}

VkSamplerAddressMode Vulkan::getWrapMode(SamplerState::WrapMode mode)
{
	switch (mode)
	{
	case SamplerState::WRAP_CLAMP:
		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	case SamplerState::WRAP_CLAMP_ZERO:
	case SamplerState::WRAP_CLAMP_ONE:
		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	case SamplerState::WRAP_REPEAT:
		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	case SamplerState::WRAP_MIRRORED_REPEAT:
		return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	default:
		throw love::Exception("unknown wrap mode");
	}
}

VkCompareOp Vulkan::getCompareOp(CompareMode mode)
{
	switch (mode)
	{
	case COMPARE_LESS:
		return VK_COMPARE_OP_LESS;
	case COMPARE_LEQUAL:
		return VK_COMPARE_OP_LESS_OR_EQUAL;
	case COMPARE_EQUAL:
		return VK_COMPARE_OP_EQUAL;
	case COMPARE_GEQUAL:
		return VK_COMPARE_OP_GREATER_OR_EQUAL;
	case COMPARE_GREATER:
		return VK_COMPARE_OP_GREATER;
	case COMPARE_NOTEQUAL:
		return VK_COMPARE_OP_NOT_EQUAL;
	case COMPARE_ALWAYS:
		return VK_COMPARE_OP_ALWAYS;
	case COMPARE_NEVER:
		return VK_COMPARE_OP_NEVER;
	default:
		throw love::Exception("unknown compare mode");
	}
}

VkSamplerMipmapMode Vulkan::getMipMapMode(SamplerState::MipmapFilterMode mode)
{
	switch (mode)
	{
	case SamplerState::MIPMAP_FILTER_NONE:
	case SamplerState::MIPMAP_FILTER_NEAREST:
		return VK_SAMPLER_MIPMAP_MODE_NEAREST;
	case SamplerState::MIPMAP_FILTER_LINEAR:
	default:
		return VK_SAMPLER_MIPMAP_MODE_LINEAR;
	}
}

VkDescriptorType Vulkan::getDescriptorType(graphics::Shader::UniformType type)
{
	switch (type)
	{
	case graphics::Shader::UniformType::UNIFORM_FLOAT:
	case graphics::Shader::UniformType::UNIFORM_MATRIX:
	case graphics::Shader::UniformType::UNIFORM_INT:
	case graphics::Shader::UniformType::UNIFORM_UINT:
	case graphics::Shader::UniformType::UNIFORM_BOOL:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	case graphics::Shader::UniformType::UNIFORM_SAMPLER:
		return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	case graphics::Shader::UniformType::UNIFORM_STORAGETEXTURE:
		return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	case graphics::Shader::UniformType::UNIFORM_TEXELBUFFER:
		return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
	case graphics::Shader::UniformType::UNIFORM_STORAGEBUFFER:
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	default:
		throw love::Exception("unknown uniform type");
	}
}

VkStencilOp Vulkan::getStencilOp(StencilAction action)
{
	switch (action)
	{
	case STENCIL_KEEP:
		return VK_STENCIL_OP_KEEP;
	case STENCIL_ZERO:
		return VK_STENCIL_OP_ZERO;
	case STENCIL_REPLACE:
		return VK_STENCIL_OP_REPLACE;
	case STENCIL_INCREMENT:
		return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
	case STENCIL_DECREMENT:
		return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
	case STENCIL_INCREMENT_WRAP:
		return VK_STENCIL_OP_INCREMENT_AND_WRAP;
	case STENCIL_DECREMENT_WRAP:
		return VK_STENCIL_OP_DECREMENT_AND_WRAP;
	case STENCIL_INVERT:
		return VK_STENCIL_OP_INVERT;
	default:
		throw love::Exception("unknown stencil action");
	}
}

VkIndexType Vulkan::getVulkanIndexBufferType(IndexDataType type)
{
	switch (type)
	{
	case INDEX_UINT16: return VK_INDEX_TYPE_UINT16;
	case INDEX_UINT32: return VK_INDEX_TYPE_UINT32;
	default:
		throw love::Exception("unknown Index Data type");
	}
}

void Vulkan::addImageLayoutTransitionOptions(bool previous, bool renderTarget, bool depthStencil, VkImageLayout layout, VkAccessFlags &accessMask, VkPipelineStageFlags &stageFlags)
{
	switch (layout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		accessMask |= 0;
		if (previous)
			stageFlags |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		else
			stageFlags |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		break;
	case VK_IMAGE_LAYOUT_GENERAL:
		// We use the general image layout for images that are both compute write and readable.
		// todo: can we optimize this?
		accessMask |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT;
		stageFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT;
		if (renderTarget)
		{
			if (depthStencil)
			{
				accessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
				stageFlags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			}
			else
			{
				accessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				stageFlags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			}
		}
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		accessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		stageFlags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		accessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		stageFlags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		accessMask |= VK_ACCESS_SHADER_READ_BIT;
		stageFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		accessMask |= VK_ACCESS_TRANSFER_READ_BIT;
		stageFlags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		accessMask |= VK_ACCESS_TRANSFER_WRITE_BIT;
		stageFlags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
		accessMask |= 0;
		stageFlags |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		break;
	default:
		throw love::Exception("unimplemented image layout");
	}
}

void Vulkan::cmdTransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, PixelFormat format, bool renderTarget, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t baseLevel, uint32_t levelCount, uint32_t baseLayer, uint32_t layerCount)
{
	VkImageMemoryBarrier imageBarrier{};
	imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageBarrier.oldLayout = oldLayout;
	imageBarrier.newLayout = newLayout;
	imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageBarrier.image = image;
	imageBarrier.subresourceRange.baseMipLevel = baseLevel;
	imageBarrier.subresourceRange.levelCount = levelCount;
	imageBarrier.subresourceRange.baseArrayLayer = baseLayer;
	imageBarrier.subresourceRange.layerCount = layerCount;

	const PixelFormatInfo &info = getPixelFormatInfo(format);

	VkPipelineStageFlags sourceStage = 0;
	VkPipelineStageFlags destinationStage = 0;

	addImageLayoutTransitionOptions(true, renderTarget, info.depth || info.stencil, oldLayout, imageBarrier.srcAccessMask, sourceStage);
	addImageLayoutTransitionOptions(false, renderTarget, info.depth || info.stencil, newLayout, imageBarrier.dstAccessMask, destinationStage);

	if (info.color)
		imageBarrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;
	if (info.depth)
		imageBarrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
	if (info.stencil)
		imageBarrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

	if (oldLayout != newLayout)
	{
		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageBarrier
		);
	}
	else
	{
		// No layout transition needed, but we do still need a memory barrier.
		VkMemoryBarrier memoryBarrier{};
		memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		memoryBarrier.srcAccessMask = imageBarrier.srcAccessMask;
		memoryBarrier.dstAccessMask = imageBarrier.dstAccessMask;

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			1, &memoryBarrier,
			0, nullptr,
			0, nullptr
		);
	}
}

} // vulkan
} // graphics
} // love
