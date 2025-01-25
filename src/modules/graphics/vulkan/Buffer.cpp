/**
 * Copyright (c) 2006-2024 LOVE Development Team
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

#include "Buffer.h"
#include "Graphics.h"

namespace love
{
namespace graphics
{
namespace vulkan
{

static VkBufferUsageFlags getUsageBit(BufferUsage mode)
{
	switch (mode)
	{
	case BUFFERUSAGE_VERTEX: return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	case BUFFERUSAGE_INDEX: return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	case BUFFERUSAGE_UNIFORM: return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	case BUFFERUSAGE_TEXEL: return VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
	case BUFFERUSAGE_SHADER_STORAGE: return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	case BUFFERUSAGE_INDIRECT_ARGUMENTS: return VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
	default:
		throw love::Exception("Unsupported BufferUsage mode: %d", mode);
	}
}

static VkBufferUsageFlags getVulkanUsageFlags(BufferUsageFlags flags)
{
	VkBufferUsageFlags vkFlags = 0;
	for (int i = 0; i < BUFFERUSAGE_MAX_ENUM; i++)
	{
		BufferUsageFlags flag = static_cast<BufferUsageFlags>(1u << i);
		if (flags & flag)
			vkFlags |= getUsageBit((BufferUsage)i);
	}
	return vkFlags;
}

Buffer::Buffer(love::graphics::Graphics *gfx, const Settings &settings, const std::vector<DataDeclaration> &format, const void *data, size_t size, size_t arraylength)
	: love::graphics::Buffer(gfx, settings, format, size, arraylength)
	, zeroInitialize(settings.zeroInitialize)
	, initialData(data)
	, vgfx(dynamic_cast<Graphics*>(gfx))
	, usageFlags(settings.usageFlags)
{
	// All buffers can be copied to and from.
	barrierDstAccessFlags = VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
	barrierDstStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;

	if (usageFlags & BUFFERUSAGEFLAG_VERTEX)
	{
		barrierDstAccessFlags |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
		barrierDstStageFlags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
	}
	if (usageFlags & BUFFERUSAGEFLAG_INDEX)
	{
		barrierDstAccessFlags |= VK_ACCESS_INDEX_READ_BIT;
		barrierDstStageFlags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
	}
	if (usageFlags & BUFFERUSAGEFLAG_TEXEL)
	{
		barrierDstAccessFlags |= VK_ACCESS_SHADER_READ_BIT;
		barrierDstStageFlags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	}
	if (usageFlags & BUFFERUSAGEFLAG_SHADER_STORAGE)
	{
		barrierDstAccessFlags |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
		barrierDstStageFlags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	}
	if (usageFlags & BUFFERUSAGEFLAG_INDIRECT_ARGUMENTS)
	{
		barrierDstAccessFlags |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
		barrierDstStageFlags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
	}

	loadVolatile();
}

bool Buffer::loadVolatile()
{
	allocator = vgfx->getVmaAllocator();

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = getSize();
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | getVulkanUsageFlags(usageFlags);

	VmaAllocationCreateInfo allocCreateInfo{};
	allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
	if (dataUsage == BUFFERDATAUSAGE_READBACK)
		allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

	auto result = vmaCreateBuffer(allocator, &bufferInfo, &allocCreateInfo, &buffer, &allocation, &allocInfo);
	if (result != VK_SUCCESS)
		throw love::Exception("Failed to create Vulkan buffer: %s", Vulkan::getErrorString(result));

	if (zeroInitialize)
	{
		auto cmd = vgfx->getCommandBufferForDataTransfer();
		vkCmdFillBuffer(cmd, buffer, 0, VK_WHOLE_SIZE, 0);
		postGPUWriteBarrier(cmd);
	}

	if (initialData)
		fill(0, size, initialData);

	if (usageFlags & BUFFERUSAGEFLAG_TEXEL)
	{
		VkBufferViewCreateInfo bufferViewInfo{};
		bufferViewInfo.buffer = buffer;
		bufferViewInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
		bufferViewInfo.format = Vulkan::getVulkanVertexFormat(getDataMember(0).decl.format);
		bufferViewInfo.range = VK_WHOLE_SIZE;

		result = vkCreateBufferView(vgfx->getDevice(), &bufferViewInfo, nullptr, &bufferView);
		if (result != VK_SUCCESS)
			throw love::Exception("Failed to create Vulkan texel buffer view: %s", Vulkan::getErrorString(result));
	}

	VkMemoryPropertyFlags memoryProperties;
	vmaGetAllocationMemoryProperties(allocator, allocation, &memoryProperties);
	if (memoryProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
		coherent = true;
	else
		coherent = false;

	if (!debugName.empty() && vgfx->getEnabledOptionalInstanceExtensions().debugInfo)
	{
		auto device = vgfx->getDevice();

		VkDebugUtilsObjectNameInfoEXT nameInfo{};
		nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		nameInfo.objectType = VK_OBJECT_TYPE_BUFFER;
		nameInfo.objectHandle = (uint64_t)buffer;
		nameInfo.pObjectName = debugName.c_str();
		vkSetDebugUtilsObjectNameEXT(device, &nameInfo);
	}

	return true;
}

void Buffer::unloadVolatile()
{
	if (buffer == VK_NULL_HANDLE)
		return;

	auto device = vgfx->getDevice();

	vgfx->queueCleanUp(
		[device=device, allocator=allocator, buffer=buffer, allocation=allocation, bufferView=bufferView](){
		vkDeviceWaitIdle(device);
		vmaDestroyBuffer(allocator, buffer, allocation);
		if (bufferView)
			vkDestroyBufferView(device, bufferView, nullptr);
	});

	buffer = VK_NULL_HANDLE;
	bufferView = VK_NULL_HANDLE;
}

Buffer::~Buffer()
{
	unloadVolatile();
}

ptrdiff_t Buffer::getHandle() const
{
	return (ptrdiff_t) buffer;
}

ptrdiff_t Buffer::getTexelBufferHandle() const
{
	return (ptrdiff_t) bufferView;
}

void *Buffer::map(MapType map, size_t offset, size_t size)
{
	if (size == 0)
		return nullptr;

	if (map == MAP_WRITE_INVALIDATE && (isImmutable() || dataUsage == BUFFERDATAUSAGE_READBACK))
		return nullptr;

	if (map == MAP_READ_ONLY && dataUsage != BUFFERDATAUSAGE_READBACK)
		return  nullptr;

	mappedRange = Range(offset, size);

	if (!Range(0, getSize()).contains(mappedRange))
		return nullptr;

	if (dataUsage == BUFFERDATAUSAGE_READBACK)
	{
		if (!coherent)
			vmaInvalidateAllocation(allocator, allocation, offset, size);

		char *data = (char*)allocInfo.pMappedData;
		return (void*) (data + offset);
	}
	else
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VkResult result = vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &stagingBuffer, &stagingAllocation, &stagingAllocInfo);
		if (result != VK_SUCCESS)
			throw love::Exception("Failed to create Vulkan staging buffer: %s", Vulkan::getErrorString(result));

		return stagingAllocInfo.pMappedData;
	}
}

bool Buffer::fill(size_t offset, size_t size, const void *data)
{
	if (size == 0 || isImmutable() || dataUsage == BUFFERDATAUSAGE_READBACK)
		return false;

	if (!Range(0, getSize()).contains(Range(offset, size)))
		return false;

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

	VkBuffer fillBuffer;
	VmaAllocation fillAllocation;
	VmaAllocationInfo fillAllocInfo;

	VkResult result = vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &fillBuffer, &fillAllocation, &fillAllocInfo);
	if (result != VK_SUCCESS)
		throw love::Exception("Failed to create Vulkan fill buffer: %s", Vulkan::getErrorString(result));

	memcpy(fillAllocInfo.pMappedData, data, size);

	VkMemoryPropertyFlags memoryProperties;
	vmaGetAllocationMemoryProperties(allocator, fillAllocation, &memoryProperties);
	if (~memoryProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
		vmaFlushAllocation(allocator, fillAllocation, 0, size);

	VkBufferCopy bufferCopy{};
	bufferCopy.srcOffset = 0;
	bufferCopy.dstOffset = offset;
	bufferCopy.size = size;

	auto cmd = vgfx->getCommandBufferForDataTransfer();
	vkCmdCopyBuffer(cmd, fillBuffer, buffer, 1, &bufferCopy);

	postGPUWriteBarrier(cmd);

	vgfx->queueCleanUp([allocator = allocator, fillBuffer = fillBuffer, fillAllocation = fillAllocation]() {
		vmaDestroyBuffer(allocator, fillBuffer, fillAllocation);
	});

	return true;
}

void Buffer::unmap(size_t usedoffset, size_t usedsize)
{
	if (dataUsage != BUFFERDATAUSAGE_READBACK)
	{
		VkBufferCopy bufferCopy{};
		bufferCopy.srcOffset = usedoffset - mappedRange.getOffset();
		bufferCopy.dstOffset = usedoffset;
		bufferCopy.size = usedsize;

		VkMemoryPropertyFlags memoryProperties;
		vmaGetAllocationMemoryProperties(allocator, stagingAllocation, &memoryProperties);
		if (~memoryProperties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
			vmaFlushAllocation(allocator, stagingAllocation, bufferCopy.srcOffset, usedsize);

		auto cmd = vgfx->getCommandBufferForDataTransfer();
		vkCmdCopyBuffer(cmd, stagingBuffer, buffer, 1, &bufferCopy);

		postGPUWriteBarrier(cmd);

		vgfx->queueCleanUp([allocator = allocator, stagingBuffer = stagingBuffer, stagingAllocation = stagingAllocation]() {
			vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
		});
	}
}

void Buffer::clearInternal(size_t offset, size_t size)
{
	auto cmd = vgfx->getCommandBufferForDataTransfer();
	vkCmdFillBuffer(cmd, buffer, offset, size, 0);
	postGPUWriteBarrier(cmd);
}

void Buffer::copyTo(love::graphics::Buffer *dest, size_t sourceoffset, size_t destoffset, size_t size)
{
	auto commandBuffer = vgfx->getCommandBufferForDataTransfer();

	VkBufferCopy bufferCopy{};
	bufferCopy.srcOffset = sourceoffset;
	bufferCopy.dstOffset = destoffset;
	bufferCopy.size = size;

	vkCmdCopyBuffer(commandBuffer, buffer, (VkBuffer) dest->getHandle(), 1, &bufferCopy);

	((Buffer *)dest)->postGPUWriteBarrier(commandBuffer);
}

void Buffer::postGPUWriteBarrier(VkCommandBuffer cmd)
{
	VkMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = barrierDstAccessFlags;

	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, barrierDstStageFlags, 0, 1, &barrier, 0, nullptr, 0, nullptr);
}

} // vulkan
} // graphics
} // love
