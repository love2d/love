#ifndef LOVE_GRAPHICS_VULKAN_STREAMBUFFER_H
#define LOVE_GRAPHICS_VULKAN_STREAMBUFFER_H

#include "graphics/Volatile.h"
#include "modules/graphics/StreamBuffer.h"
#include "vulkan/vulkan.h"
#include "graphics/Graphics.h"

#include "vk_mem_alloc.h"

namespace love {
namespace graphics {
namespace vulkan {
class StreamBuffer : public love::graphics::StreamBuffer, public graphics::Volatile {
public:
	StreamBuffer(graphics::Graphics* gfx, BufferUsage mode, size_t size);
	virtual ~StreamBuffer();

	virtual bool loadVolatile() override;

	virtual void unloadVolatile() override;

	MapInfo map(size_t minsize) override;
	size_t unmap(size_t usedSize) override;
	void markUsed(size_t usedSize) override;

	void nextFrame() override;

	ptrdiff_t getHandle() const override {
		return (ptrdiff_t) buffer;
	}

private:
	graphics::Graphics* gfx;
	VmaAllocator allocator;
	VmaAllocation allocation;
	VmaAllocationInfo allocInfo;
	VkBuffer buffer = VK_NULL_HANDLE;
	size_t usedGPUMemory;

};
} // vulkan
} // graphics
} // love

#endif
