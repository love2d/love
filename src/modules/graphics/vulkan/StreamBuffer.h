#ifndef LOVE_GRAPHICS_VULKAN_STREAMBUFFER_H
#define LOVE_GRAPHICS_VULKAN_STREAMBUFFER_H

#include "modules/graphics/StreamBuffer.h"
#include "vulkan/vulkan.h"


namespace love {
	namespace graphics {
		namespace vulkan {
			class StreamBuffer : public love::graphics::StreamBuffer {
			public:
				StreamBuffer(VkDevice device, VkPhysicalDevice physicalDevice, BufferUsage mode, size_t size);
				virtual ~StreamBuffer();

				MapInfo map(size_t minsize) override;
				size_t unmap(size_t usedSize) override;
				void markUsed(size_t usedSize) override;

				ptrdiff_t getHandle() const override {
					return (ptrdiff_t) buffer;
				}

			private:
				VkDevice device;
				VkBuffer buffer;
				VkDeviceMemory bufferMemory;
				void* mappedMemory;
			};
		}
	}
}

#endif