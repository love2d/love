#include "Vulkan.h"

namespace love {
	namespace graphics {
		namespace vulkan {
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
		}
	}
}
