#include "StreamBuffer.h"
#include "vulkan/vulkan.h"


namespace love {
	namespace graphics {
		namespace vulkan {
			static uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFtiler, VkMemoryPropertyFlags properties) {
				VkPhysicalDeviceMemoryProperties memProperties;
				vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

				for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
					if ((typeFtiler & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
						return i;
					}
				}

				throw love::Exception("failed to find suitable memory type");
			}

			static VkBufferUsageFlags getUsageFlags(BufferUsage mode) {
				switch (mode) {
				case BUFFERUSAGE_VERTEX: return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
				case BUFFERUSAGE_INDEX: return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
				default:
					throw love::Exception("unsupported BufferUsage mode");
				}
			}

			StreamBuffer::StreamBuffer(VkDevice device, VkPhysicalDevice physicalDevice, BufferUsage mode, size_t size)
				:	love::graphics::StreamBuffer(mode, size),
					device(device) {
				VkBufferCreateInfo bufferInfo{};
				bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				bufferInfo.size = getSize();
				bufferInfo.usage = getUsageFlags(mode);
				bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

				if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
					throw love::Exception("failed to create buffer");
				}

				VkMemoryRequirements memRequirements;
				vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

				VkMemoryAllocateInfo allocInfo{};
				allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				allocInfo.allocationSize = memRequirements.size;
				allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

				if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
					throw love::Exception("failed to allocate vertex buffer memory");
				}

				vkBindBufferMemory(device, buffer, bufferMemory, 0);
			}

			love::graphics::StreamBuffer::MapInfo StreamBuffer::map(size_t minsize) {
				vkMapMemory(device, bufferMemory, 0, getSize(), 0, &mappedMemory);
				return love::graphics::StreamBuffer::MapInfo((uint8*) mappedMemory, getSize());
			}

			size_t StreamBuffer::unmap(size_t usedSize) {
				vkUnmapMemory(device, bufferMemory);
				return usedSize;
			}

			void StreamBuffer::markUsed(size_t usedSize) {
				(void)usedSize;
			}
		}
	}
}
