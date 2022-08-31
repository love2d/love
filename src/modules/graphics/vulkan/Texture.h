#ifndef LOVE_GRAPHICS_VULKAN_TEXTURE_H
#define LOVE_GRAPHICS_VULKAN_TEXTURE_H

#include "graphics/Texture.h"
#include "graphics/Volatile.h"

#include "VulkanWrapper.h"

#include <iostream>


namespace love {
namespace graphics {
namespace vulkan {
class Texture : public graphics::Texture, public Volatile {
public:
	Texture(love::graphics::Graphics* gfx, const Settings& settings, const Slices* data);
	~Texture();

	virtual bool loadVolatile() override;
	virtual void unloadVolatile() override;
	 
	void setSamplerState(const SamplerState &s) override;

	VkImageLayout getImageLayout() const;

	void copyFromBuffer(graphics::Buffer* source, size_t sourceoffset, int sourcewidth, size_t size, int slice, int mipmap, const Rect& rect) override;
	void copyToBuffer(graphics::Buffer* dest, int slice, int mipmap, const Rect& rect, size_t destoffset, int destwidth, size_t size) override;

	ptrdiff_t getRenderTargetHandle() const override { return (ptrdiff_t)textureImageView; };
	ptrdiff_t getSamplerHandle() const override { return (ptrdiff_t)textureSampler; };

	void uploadByteData(PixelFormat pixelformat, const void* data, size_t size, int level, int slice, const Rect& r) override;

	void generateMipmapsInternal()  override;

	int getMSAA() const override { return 0; };
	ptrdiff_t getHandle() const override { return (ptrdiff_t)textureImage; }

private:
	void createTextureImageView();
	void clear();

	VkClearColorValue getClearValue();

	graphics::Graphics* gfx = nullptr;
	VkDevice device = VK_NULL_HANDLE;
	VmaAllocator allocator = VK_NULL_HANDLE;
	VkImage textureImage = VK_NULL_HANDLE;
	VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VmaAllocation textureImageAllocation = VK_NULL_HANDLE;
	VkImageView textureImageView = VK_NULL_HANDLE;
	VkSampler textureSampler = VK_NULL_HANDLE;
	Slices slices;
	int layerCount = 0;
};
} // vulkan
} // graphics
} // love

#endif
