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

#pragma once

#include "common/Range.h"

#include "graphics/Buffer.h"
#include "graphics/Volatile.h"

#include "VulkanWrapper.h"


namespace love
{
namespace graphics
{
namespace vulkan
{

class Graphics;

class Buffer final
	: public love::graphics::Buffer
	, public Volatile
{
public:
	Buffer(love::graphics::Graphics *gfx, const Settings &settings, const std::vector<DataDeclaration> &format, const void *data, size_t size, size_t arraylength);
	virtual ~Buffer();

	virtual bool loadVolatile() override;
	virtual void unloadVolatile() override;

	void *map(MapType map, size_t offset, size_t size) override;
	void unmap(size_t usedoffset, size_t usedsize) override;
	bool fill(size_t offset, size_t size, const void *data) override;
	void copyTo(love::graphics::Buffer *dest, size_t sourceoffset, size_t destoffset, size_t size) override;
	ptrdiff_t getHandle() const override;
	ptrdiff_t getTexelBufferHandle() const override;

	void postGPUWriteBarrier(VkCommandBuffer cmd);

	VkAccessFlags getBarrierDstAccessFlags() const { return barrierDstAccessFlags; }
	VkPipelineStageFlags getBarrierDstStageFlags() const { return barrierDstStageFlags; }

private:

	void clearInternal(size_t offset, size_t size) override;

	bool zeroInitialize;
	const void *initialData;
	VkBuffer buffer = VK_NULL_HANDLE;
	VkBuffer stagingBuffer = VK_NULL_HANDLE;
	VkBufferView bufferView = VK_NULL_HANDLE;
	Graphics *vgfx = nullptr;
	VmaAllocator allocator;
	VmaAllocation allocation;
	VmaAllocation stagingAllocation;
	VmaAllocationInfo allocInfo;
	VmaAllocationInfo stagingAllocInfo;
	BufferUsageFlags usageFlags;
	Range mappedRange;
	bool coherent;

	VkAccessFlags barrierDstAccessFlags = 0;
	VkPipelineStageFlags barrierDstStageFlags = 0;
};

} // vulkan
} // graphics
} // love
