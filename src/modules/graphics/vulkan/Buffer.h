#pragma once

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
	Buffer(love::graphics::Graphics *gfx, const Settings& settings, const std::vector<DataDeclaration> &format, const void *data, size_t size, size_t arraylength);
	virtual ~Buffer();

	virtual bool loadVolatile() override;
	virtual void unloadVolatile() override;

	void *map(MapType map, size_t offset, size_t size) override;
	void unmap(size_t usedoffset, size_t usedsize) override;
	bool fill(size_t offset, size_t size, const void *data) override;
	void copyTo(love::graphics::Buffer *dest, size_t sourceoffset, size_t destoffset, size_t size) override;
	ptrdiff_t getHandle() const override;
	ptrdiff_t getTexelBufferHandle() const override;

private:
	// todo use a staging buffer for improved performance
	VkBuffer buffer = VK_NULL_HANDLE;
	Graphics *vgfx = nullptr;
	VmaAllocator allocator;
	VmaAllocation allocation;
	VmaAllocationInfo allocInfo;
	BufferUsageFlags usageFlags;
};

} // vulkan
} // graphics
} // love
