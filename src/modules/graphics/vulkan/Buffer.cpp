#include "Buffer.h"
#include "Graphics.h"

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

			Buffer::Buffer(love::graphics::Graphics* gfx, const Settings& settings, const std::vector<DataDeclaration>& format, const void* data, size_t size, size_t arraylength) 
				: love::graphics::Buffer(gfx, settings, format, size, arrayLength) {
				auto vgfx = (Graphics*)gfx;
				device = vgfx->getDevice();
				auto physicalDevice = vgfx->getPhysicalDevice();

				VkBufferCreateInfo bufferInfo{};
				bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				bufferInfo.size = getSize();
				bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;	// todo: only vertex buffers are allowed for now
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

				vkMapMemory(device, bufferMemory, 0, getSize(), 0, &mappedMemory);
				memcpy(mappedMemory, data, size);
				vkUnmapMemory(device, bufferMemory);
			}

			Buffer::~Buffer() {
				vkDestroyBuffer(device, buffer, nullptr);
				vkFreeMemory(device, bufferMemory, nullptr);
			}

			void* Buffer::map(MapType map, size_t offset, size_t size) {
				vkMapMemory(device, bufferMemory, offset, size, 0, &mappedMemory);
				return mappedMemory;
			}

			bool Buffer::fill(size_t offset, size_t size, const void *data) {
				memcpy(mappedMemory, data, size);
				return true;
			}

			void Buffer::unmap(size_t usedoffset, size_t usedsize) {
				vkUnmapMemory(device, bufferMemory);
			}

			void Buffer::copyTo(love::graphics::Buffer* dest, size_t sourceoffset, size_t destoffset, size_t size) {
				throw love::Exception("not implemented yet");
			}
		}
	}
}