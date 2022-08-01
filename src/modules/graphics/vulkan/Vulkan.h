#ifndef LOVE_GRAPHICS_VULKAN_VULKAN_H
#define LOVE_GRAPHICS_VULKAN_VULKAN_H

#include "graphics/Graphics.h"
#include "vulkan/vulkan.h"

namespace love {
namespace graphics {
namespace vulkan {
enum InternalFormatRepresentation {
	FORMATREPRESENTATION_FLOAT,
	FORMATREPRESENTATION_UINT,
	FORMATREPRESENTATION_SINT,
	FORMATREPRESENTATION_MAX_ENUM
};

struct TextureFormat {
	InternalFormatRepresentation internalFormatRepresentation;
	VkFormat internalFormat = VK_FORMAT_UNDEFINED;

	VkComponentSwizzle swizzleR = VK_COMPONENT_SWIZZLE_IDENTITY;
	VkComponentSwizzle swizzleG = VK_COMPONENT_SWIZZLE_IDENTITY;
	VkComponentSwizzle swizzleB = VK_COMPONENT_SWIZZLE_IDENTITY;
	VkComponentSwizzle swizzleA = VK_COMPONENT_SWIZZLE_IDENTITY;
};

class Vulkan {
public:
	static void shaderSwitch();
	static uint32_t getNumShaderSwitches();
	static void resetShaderSwitches();

	static VkFormat getVulkanVertexFormat(DataFormat format);
	static TextureFormat getTextureFormat(PixelFormat);
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

	static void cmdTransitionImageLayout(
		VkCommandBuffer, VkImage, VkImageLayout oldLayout, VkImageLayout newLayout,
		uint32_t baseLevel = 0, uint32_t levelCount = 1, uint32_t baseLayer = 0, uint32_t layerCount = 1);
};
} // vulkan
} // graphics
} // love

#endif
