#ifndef LOVE_GRAPHICS_VULKAN_VULKAN_H
#define LOVE_GRAPHICS_VULKAN_VULKAN_H

#include "Vulkan.h"
#include "Graphics.h"

namespace love {
	namespace graphics {
		namespace vulkan {
			class Vulkan {
			public:
				static VkFormat getVulkanVertexFormat(DataFormat format);
			};
		}
	}
}

#endif
