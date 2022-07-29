#include "StreamBuffer.h"
#include "vulkan/vulkan.h"
#include "Graphics.h"


namespace love {
	namespace graphics {
		namespace vulkan {
			static VkBufferUsageFlags getUsageFlags(BufferUsage mode) {
				switch (mode) {
				case BUFFERUSAGE_VERTEX: return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
				case BUFFERUSAGE_INDEX: return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
				case BUFFERUSAGE_UNIFORM: return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
				default:
					throw love::Exception("unsupported BufferUsage mode");
				}
			}

			StreamBuffer::StreamBuffer(graphics::Graphics* gfx, BufferUsage mode, size_t size)
				:	love::graphics::StreamBuffer(mode, size), gfx(gfx) {
				loadVolatile();
			}

			bool StreamBuffer::loadVolatile() {
				Graphics* vgfx = (Graphics*)gfx;
				allocator = vgfx->getVmaAllocator();

				VkBufferCreateInfo bufferInfo{};
				bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				bufferInfo.size = getSize();
				bufferInfo.usage = getUsageFlags(mode);
				bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

				VmaAllocationCreateInfo allocCreateInfo = {};
				allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
				allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;	// always mapped

				vmaCreateBuffer(allocator, &bufferInfo, &allocCreateInfo, &buffer, &allocation, &allocInfo);

				usedGPUMemory = 0;

				return true;
			}

			void StreamBuffer::unloadVolatile() {
				if (buffer == VK_NULL_HANDLE)
					return;

				auto vgfx = (Graphics*)gfx;
				vgfx->queueCleanUp([allocator=allocator, buffer=buffer, allocation=allocation](){
					vmaDestroyBuffer(allocator, buffer, allocation);
				});
				buffer = VK_NULL_HANDLE;
			}

			StreamBuffer::~StreamBuffer() {
				unloadVolatile();
			}

			love::graphics::StreamBuffer::MapInfo StreamBuffer::map(size_t minsize) {
				(void)minsize;
				return love::graphics::StreamBuffer::MapInfo((uint8*) allocInfo.pMappedData + usedGPUMemory, getSize());
			}

			size_t StreamBuffer::unmap(size_t usedSize) {
				return usedGPUMemory;
			}

			void StreamBuffer::markUsed(size_t usedSize) {
				usedGPUMemory += usedSize;
			}

			void StreamBuffer::nextFrame() {
				usedGPUMemory = 0;
			}
		}
	}
}
