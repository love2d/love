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

#include "StreamBuffer.h"
#include "Graphics.h"


namespace love
{
namespace graphics
{
namespace vulkan
{

static VkBufferUsageFlags getUsageFlags(BufferUsage mode)
{
	switch (mode)
	{
	case BUFFERUSAGE_VERTEX: return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	case BUFFERUSAGE_INDEX: return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	case BUFFERUSAGE_UNIFORM: return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	default:
		throw love::Exception("unsupported BufferUsage mode");
	}
}

StreamBuffer::StreamBuffer(graphics::Graphics *gfx, BufferUsage mode, size_t size)
	: love::graphics::StreamBuffer(mode, size)
	, vgfx(dynamic_cast<Graphics*>(gfx))
{
	loadVolatile();
}

bool StreamBuffer::loadVolatile()
{
	allocator = vgfx->getVmaAllocator();

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = getSize() * MAX_FRAMES_IN_FLIGHT; // TODO: Is this sufficient or should it be +1?
	bufferInfo.usage = getUsageFlags(mode);
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocCreateInfo = {};
	allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

	if (vmaCreateBuffer(allocator, &bufferInfo, &allocCreateInfo, &buffer, &allocation, &allocInfo) != VK_SUCCESS)
		throw love::Exception("Cannot create stream buffer: out of graphics memory.");

	VkMemoryPropertyFlags properties;
	vmaGetAllocationMemoryProperties(allocator, allocation, &properties);
	if (properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
		coherent = true;
	else
		coherent = false;

	return true;
}

void StreamBuffer::unloadVolatile()
{
	if (buffer == VK_NULL_HANDLE)
		return;

	vgfx->queueCleanUp([allocator=allocator, buffer=buffer, allocation=allocation](){
		vmaDestroyBuffer(allocator, buffer, allocation);
	});
	buffer = VK_NULL_HANDLE;
}

StreamBuffer::~StreamBuffer()
{
	unloadVolatile();
}

ptrdiff_t StreamBuffer::getHandle() const
{
	return (ptrdiff_t) buffer;
}

size_t StreamBuffer::getGPUReadOffset() const
{
	return (frameIndex * bufferSize) + frameGPUReadOffset;
}

love::graphics::StreamBuffer::MapInfo StreamBuffer::map(size_t /*minsize*/)
{
	// TODO: do we also need to wait until a fence is complete, here?

	MapInfo info;
	info.size = bufferSize - frameGPUReadOffset;
	info.data = (uint8*)allocInfo.pMappedData + (frameIndex * bufferSize) + frameGPUReadOffset;
	return info;
}

size_t StreamBuffer::unmap(size_t usedSize)
{
	size_t offset = (frameIndex * bufferSize) + frameGPUReadOffset;

	if (!coherent)
		vmaFlushAllocation(allocator, allocation, offset, usedSize);

	return offset;
}

void StreamBuffer::markUsed(size_t usedSize)
{
	frameGPUReadOffset += usedSize;
}

void StreamBuffer::nextFrame()
{
	frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
	frameGPUReadOffset = 0;
}

} // vulkan
} // graphics
} // love
