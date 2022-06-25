#include "graphics/Buffer.h"
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"


namespace love {
	namespace graphics {
		namespace vulkan {
			class Buffer : public love::graphics::Buffer {
			public:
				Buffer(VmaAllocator allocator, love::graphics::Graphics* gfx, const Settings& settings, const std::vector<DataDeclaration>& format, const void* data, size_t size, size_t arraylength);
				virtual ~Buffer();

				void* map(MapType map, size_t offset, size_t size) override;
				void unmap(size_t usedoffset, size_t usedsize) override;
				bool fill(size_t offset, size_t size, const void* data) override;
				void copyTo(love::graphics::Buffer* dest, size_t sourceoffset, size_t destoffset, size_t size) override;
				ptrdiff_t getHandle() const override {
					return (ptrdiff_t) buffer;	// todo ?
				}
				ptrdiff_t getTexelBufferHandle() const override {
					throw love::Exception("unimplemented Buffer::getTexelBufferHandle");
					return (ptrdiff_t) nullptr;	// todo ?
				}

			private:
				// todo use a staging buffer for improved performance
				VkBuffer buffer;
				VmaAllocation allocation;
				VmaAllocationInfo allocInfo;
			};
		}
	}
}
