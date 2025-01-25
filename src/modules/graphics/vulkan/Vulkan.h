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

#include "graphics/Graphics.h"
#include "VulkanWrapper.h"

namespace love
{
namespace graphics
{
namespace vulkan 
{

enum InternalFormatRepresentation
{
	FORMATREPRESENTATION_FLOAT,
	FORMATREPRESENTATION_UINT,
	FORMATREPRESENTATION_SINT,
	FORMATREPRESENTATION_MAX_ENUM
};

struct TextureFormat
{
	InternalFormatRepresentation internalFormatRepresentation = FORMATREPRESENTATION_FLOAT;
	VkFormat internalFormat = VK_FORMAT_UNDEFINED;

	VkComponentSwizzle swizzleR = VK_COMPONENT_SWIZZLE_IDENTITY;
	VkComponentSwizzle swizzleG = VK_COMPONENT_SWIZZLE_IDENTITY;
	VkComponentSwizzle swizzleB = VK_COMPONENT_SWIZZLE_IDENTITY;
	VkComponentSwizzle swizzleA = VK_COMPONENT_SWIZZLE_IDENTITY;
};

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

class Vulkan
{
public:
	static void shaderSwitch();
	static uint32_t getNumShaderSwitches();
	static void resetShaderSwitches();

	static const char *getErrorString(VkResult result);
	static VkFormat getVulkanVertexFormat(DataFormat format);
	static TextureFormat getTextureFormat(PixelFormat format);
	static std::string getVendorName(uint32_t vendorId);
	static std::string getVulkanApiVersion(uint32_t apiVersion);
	static VkPrimitiveTopology getPrimitiveTypeTopology(graphics::PrimitiveType);
	static VkBlendFactor getBlendFactor(BlendFactor);
	static VkBlendOp getBlendOp(BlendOperation);
	static VkBool32 getBool(bool);
	static VkColorComponentFlags getColorMask(ColorChannelMask);
	static VkFrontFace getFrontFace(Winding);
	static VkCullModeFlags getCullMode(CullMode);
	static VkImageType getImageType(TextureType);
	static VkImageViewType getImageViewType(TextureType);
	static VkPolygonMode getPolygonMode(bool wireframe);
	static VkFilter getFilter(SamplerState::FilterMode);
	static VkSamplerAddressMode getWrapMode(SamplerState::WrapMode);
	static VkCompareOp getCompareOp(CompareMode);
	static VkSamplerMipmapMode getMipMapMode(SamplerState::MipmapFilterMode);
	static VkDescriptorType getDescriptorType(graphics::Shader::UniformType);
	static VkStencilOp getStencilOp(StencilAction);
	static VkIndexType getVulkanIndexBufferType(IndexDataType type);

	static void addImageLayoutTransitionOptions(
		bool previous, bool renderTarget, bool depthStencil, VkImageLayout layout, VkAccessFlags& accessMask, VkPipelineStageFlags& stageFlags);

	static void cmdTransitionImageLayout(
		VkCommandBuffer, VkImage, PixelFormat format, bool renderTarget, VkImageLayout oldLayout, VkImageLayout newLayout,
		uint32_t baseLevel = 0, uint32_t levelCount = VK_REMAINING_MIP_LEVELS, uint32_t baseLayer = 0, uint32_t layerCount = VK_REMAINING_ARRAY_LAYERS);
};

} // vulkan
} // graphics
} // love
