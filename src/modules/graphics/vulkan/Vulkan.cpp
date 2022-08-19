#include "Vulkan.h"

#include <sstream>


namespace love {
namespace graphics {
namespace vulkan {
static uint32_t numShaderSwitches;
static int vsync = 1;

void Vulkan::shaderSwitch() {
	numShaderSwitches++;
}

uint32_t Vulkan::getNumShaderSwitches() {
	return numShaderSwitches;
}

void Vulkan::resetShaderSwitches() {
	numShaderSwitches = 0;
}

void Vulkan::setVsync(int value) {
	vsync = value;
}

int Vulkan::getVsync() {
	return vsync;
}

VkFormat Vulkan::getVulkanVertexFormat(DataFormat format) {
	switch (format) {
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

TextureFormat Vulkan::getTextureFormat(PixelFormat format) {
	TextureFormat textureFormat{};

	switch (format) {
		case PIXELFORMAT_UNKNOWN:
			throw love::Exception("unknown pixel format");
		case PIXELFORMAT_NORMAL:
			textureFormat.internalFormat = VK_FORMAT_R8G8B8A8_SRGB;
			textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_FLOAT;
			break;
		case PIXELFORMAT_HDR:
			throw love::Exception("unimplemented pixel format: hdr");
		case PIXELFORMAT_R8_UNORM:
			textureFormat.internalFormat = VK_FORMAT_R8_UNORM;
			textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_FLOAT;
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
			textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_FLOAT;
			break;
		case PIXELFORMAT_R16_FLOAT:
			textureFormat.internalFormat = VK_FORMAT_R16_SFLOAT;
			textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_FLOAT;
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
			textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_FLOAT;
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
			textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_FLOAT;
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
			textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_FLOAT;
			break;
		case PIXELFORMAT_RG16_UNORM:
			textureFormat.internalFormat = VK_FORMAT_R16G16_UNORM;
			textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_FLOAT;
			break;
		case PIXELFORMAT_RG16_FLOAT:
			textureFormat.internalFormat = VK_FORMAT_R16G16_SFLOAT;
			textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_FLOAT;
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
			textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_FLOAT;
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
			textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_FLOAT;
			break;
		case PIXELFORMAT_RGBA8_UNORM_sRGB:
			textureFormat.internalFormat = VK_FORMAT_R8G8B8A8_SRGB;
			textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_FLOAT;
			break;
		case PIXELFORMAT_BGRA8_UNORM:
			textureFormat.internalFormat = VK_FORMAT_B8G8R8A8_UNORM;
			textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_FLOAT;
			break;
		case PIXELFORMAT_BGRA8_UNORM_sRGB:
			textureFormat.internalFormat = VK_FORMAT_B8G8R8A8_SRGB;
			textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_FLOAT;
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
			textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_FLOAT;
			break;
		case PIXELFORMAT_RGBA16_FLOAT:
			textureFormat.internalFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
			textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_FLOAT;
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
			textureFormat.internalFormatRepresentation = FORMATREPRESENTATION_FLOAT;
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
		case PIXELFORMAT_RGB5A1_UNORM:   // LSB->MSB: [a: b: g: r]
		case PIXELFORMAT_RGB565_UNORM:   // LSB->MSB: [b: g: r]
		case PIXELFORMAT_RGB10A2_UNORM:  // LSB->MSB: [r: g: b: a]
		case PIXELFORMAT_RG11B10_FLOAT:  // LSB->MSB: [r: g: b]
		case PIXELFORMAT_STENCIL8:
		case PIXELFORMAT_DEPTH16_UNORM:
		case PIXELFORMAT_DEPTH24_UNORM:
		case PIXELFORMAT_DEPTH32_FLOAT:
		case PIXELFORMAT_DEPTH24_UNORM_STENCIL8:
		case PIXELFORMAT_DEPTH32_FLOAT_STENCIL8:
		case PIXELFORMAT_DXT1_UNORM:
		case PIXELFORMAT_DXT3_UNORM:
		case PIXELFORMAT_DXT5_UNORM:
		case PIXELFORMAT_BC4_UNORM:
		case PIXELFORMAT_BC4_SNORM:
		case PIXELFORMAT_BC5_UNORM:
		case PIXELFORMAT_BC5_SNORM:
		case PIXELFORMAT_BC6H_UFLOAT:
		case PIXELFORMAT_BC6H_FLOAT:
		case PIXELFORMAT_BC7_UNORM:
		case PIXELFORMAT_PVR1_RGB2_UNORM:
		case PIXELFORMAT_PVR1_RGB4_UNORM:
		case PIXELFORMAT_PVR1_RGBA2_UNORM:
		case PIXELFORMAT_PVR1_RGBA4_UNORM:
		case PIXELFORMAT_ETC1_UNORM:
		case PIXELFORMAT_ETC2_RGB_UNORM:
		case PIXELFORMAT_ETC2_RGBA_UNORM:
		case PIXELFORMAT_ETC2_RGBA1_UNORM:
		case PIXELFORMAT_EAC_R_UNORM:
		case PIXELFORMAT_EAC_R_SNORM:
		case PIXELFORMAT_EAC_RG_UNORM:
		case PIXELFORMAT_EAC_RG_SNORM:
		case PIXELFORMAT_ASTC_4x4:
		case PIXELFORMAT_ASTC_5x4:
		case PIXELFORMAT_ASTC_5x5:
		case PIXELFORMAT_ASTC_6x5:
		case PIXELFORMAT_ASTC_6x6:
		case PIXELFORMAT_ASTC_8x5:
		case PIXELFORMAT_ASTC_8x6:
		case PIXELFORMAT_ASTC_8x8:
		case PIXELFORMAT_ASTC_10x5:
		case PIXELFORMAT_ASTC_10x6:
		case PIXELFORMAT_ASTC_10x8:
		case PIXELFORMAT_ASTC_10x10:
		case PIXELFORMAT_ASTC_12x10:
		case PIXELFORMAT_ASTC_12x12:
			throw love::Exception("unimplemented pixel format");
		default:
			throw love::Exception("unknown pixel format");
	}

	return textureFormat;
}

// values taken from https://pcisig.com/membership/member-companies
// as specified at https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkPhysicalDeviceProperties.html
std::string Vulkan::getVendorName(uint32_t vendorId) {
	switch (vendorId) {
	case 4130:
		return "AMD";
	case 4318:
		return "Nvidia";
	case 8086:
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
	default:
		return "unknown";
	}
}

std::string Vulkan::getVulkanApiVersion(uint32_t version) {
	std::stringstream ss;

	ss << VK_API_VERSION_MAJOR(version) 
		<< "." << VK_API_VERSION_MINOR(version) 
		<< "." << VK_API_VERSION_PATCH(version);

	return ss.str();
}

VkPrimitiveTopology Vulkan::getPrimitiveTypeTopology(graphics::PrimitiveType primitiveType) {
	switch (primitiveType) {
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

VkBlendFactor Vulkan::getBlendFactor(BlendFactor blendFactor) {
	switch (blendFactor) {
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

VkBlendOp Vulkan::getBlendOp(BlendOperation op) {
	switch (op) {
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

VkBool32 Vulkan::getBool(bool b) {
	if (b) {
		return VK_TRUE;
	} else {
		return VK_FALSE;
	}
}

VkColorComponentFlags Vulkan::getColorMask(ColorChannelMask mask) {
	VkColorComponentFlags flags = 0;

	if (mask.r) {
		flags |= VK_COLOR_COMPONENT_R_BIT;
	}
	if (mask.g) {
		flags |= VK_COLOR_COMPONENT_G_BIT;
	}
	if (mask.b) {
		flags |= VK_COLOR_COMPONENT_B_BIT;
	}
	if (mask.a) {
		flags |= VK_COLOR_COMPONENT_A_BIT;
	}

	return flags;
}

VkFrontFace Vulkan::getFrontFace(Winding winding) {
	switch (winding) {
	case WINDING_CW:
		return VK_FRONT_FACE_CLOCKWISE;
	case WINDING_CCW:
		return VK_FRONT_FACE_COUNTER_CLOCKWISE;
	default:
		throw love::Exception("unknown winding");
	}
}

VkCullModeFlags Vulkan::getCullMode(CullMode cullmode) {
	switch (cullmode) {
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

VkImageType Vulkan::getImageType(TextureType textureType) {
	switch (textureType) {
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

VkImageViewType Vulkan::getImageViewType(TextureType textureType) {
	switch (textureType) {
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

VkPolygonMode Vulkan::getPolygonMode(bool wireframe) {
	if (wireframe) {
		return VK_POLYGON_MODE_LINE;
	} else {
		return VK_POLYGON_MODE_FILL;
	}
}

VkFilter Vulkan::getFilter(SamplerState::FilterMode mode) {
	switch (mode) {
	case SamplerState::FILTER_LINEAR:
		return VK_FILTER_LINEAR;
	case SamplerState::FILTER_NEAREST:
		return VK_FILTER_NEAREST;
	default:
		throw love::Exception("unkonwn filter mode");
	}
}

VkSamplerAddressMode Vulkan::getWrapMode(SamplerState::WrapMode mode) {
	switch (mode) {
		//fixme: not accounting for different clamps (how does that work in vulkan?)
	case SamplerState::WRAP_CLAMP:
	case SamplerState::WRAP_CLAMP_ZERO:
	case SamplerState::WRAP_CLAMP_ONE:
		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	case SamplerState::WRAP_REPEAT:
		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	case SamplerState::WRAP_MIRRORED_REPEAT:
		return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	default:
		throw love::Exception("unknown wrap mode");
	}
}

VkCompareOp Vulkan::getCompareOp(CompareMode mode) {
	switch (mode) {
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

VkSamplerMipmapMode Vulkan::getMipMapMode(SamplerState::MipmapFilterMode mode) {
	switch (mode) {
	case SamplerState::MIPMAP_FILTER_NEAREST:
		return VK_SAMPLER_MIPMAP_MODE_NEAREST;
	case SamplerState::MIPMAP_FILTER_NONE:
	case SamplerState::MIPMAP_FILTER_LINEAR:
	default:
		return VK_SAMPLER_MIPMAP_MODE_LINEAR;
	}
}


VkDescriptorType Vulkan::getDescriptorType(graphics::Shader::UniformType type) {
	switch (type) {
	case graphics::Shader::UniformType::UNIFORM_FLOAT:
	case graphics::Shader::UniformType::UNIFORM_MATRIX:
	case graphics::Shader::UniformType::UNIFORM_INT:
	case graphics::Shader::UniformType::UNIFORM_UINT:
	case graphics::Shader::UniformType::UNIFORM_BOOL:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	case graphics::Shader::UniformType::UNIFORM_SAMPLER:
		return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	case graphics::Shader::UniformType::UNIFORM_STORAGETEXTURE:
		return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	case graphics::Shader::UniformType::UNIFORM_TEXELBUFFER:
		return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
	case graphics::Shader::UniformType::UNIFORM_STORAGEBUFFER:
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	default:
		throw love::Exception("unkonwn uniform type");
	}
}

void Vulkan::cmdTransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout,
	uint32_t baseLevel, uint32_t levelCount, uint32_t baseLayer, uint32_t layerCount) {
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = baseLevel;
	barrier.subresourceRange.levelCount = levelCount;
	barrier.subresourceRange.baseArrayLayer = baseLayer;
	barrier.subresourceRange.layerCount = layerCount;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = 0;

		sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
}
} // vulkan
} // graphics
} // love
