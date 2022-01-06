#include "graphics/Buffer.h"
#include <vulkan/vulkan.h>


namespace love {
	namespace graphics {
		namespace vulkan {
			class Buffer : public love::graphics::Buffer {
			public:
				Buffer(love::graphics::Graphics* gfx, const Settings& settings, const std::vector<DataDeclaration>& format, const void* data, size_t size, size_t arraylength);
				virtual ~Buffer();

				void* map(MapType map, size_t offset, size_t size) override;
				void unmap(size_t usedoffset, size_t usedsize) override;
				void fill(size_t offset, size_t size, const void* data) override;
				void copyTo(love::graphics::Buffer* dest, size_t sourceoffset, size_t destoffset, size_t size) override;
				ptrdiff_t getHandle() const override {
					return (ptrdiff_t) buffer;	// todo ?
				}
				ptrdiff_t getTexelBufferHandle() const override {
					return (ptrdiff_t) nullptr;	// todo ?
				}

			private:
				VkDevice device;
				VkPhysicalDevice physicalDevice;

				// todo use a staging buffer for improved performance
				VkBuffer buffer;
				VkDeviceMemory bufferMemory;

				void* mappedMemory;
			};
		}
	}
}
