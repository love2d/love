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

#include "graphics/Volatile.h"
#include "graphics/StreamBuffer.h"
#include "graphics/Graphics.h"

#include "VulkanWrapper.h"

namespace love
{
namespace graphics
{
namespace vulkan
{

class Graphics;

class StreamBuffer final
	: public love::graphics::StreamBuffer
	, public graphics::Volatile
{
public:
	StreamBuffer(graphics::Graphics *gfx, BufferUsage mode, size_t size);
	virtual ~StreamBuffer();

	virtual bool loadVolatile() override;

	virtual void unloadVolatile() override;

	size_t getGPUReadOffset() const override;
	MapInfo map(size_t minsize) override;
	size_t unmap(size_t usedSize) override;
	void markUsed(size_t usedSize) override;

	void nextFrame() override;

	ptrdiff_t getHandle() const override;

private:
	Graphics *vgfx = nullptr;
	VmaAllocator allocator;
	VmaAllocation allocation;
	VmaAllocationInfo allocInfo;
	VkBuffer buffer = VK_NULL_HANDLE;
	int frameIndex = 0;
	bool coherent;

};

} // vulkan
} // graphics
} // love
