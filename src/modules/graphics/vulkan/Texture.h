#pragma once

#include "graphics/Texture.h"
#include "graphics/Volatile.h"

#include "VulkanWrapper.h"


namespace love
{
namespace graphics
{
namespace vulkan
{

class Graphics;

class Texture final
	: public graphics::Texture
	, public Volatile
{
public:
	Texture(love::graphics::Graphics *gfx, const Settings &settings, const Slices *data);
	~Texture();

	virtual bool loadVolatile() override;
	virtual void unloadVolatile() override;
	 
	void setSamplerState(const SamplerState &s) override;

	VkImageLayout getImageLayout() const;

	void copyFromBuffer(graphics::Buffer *source, size_t sourceoffset, int sourcewidth, size_t size, int slice, int mipmap, const Rect &rect) override;
	void copyToBuffer(graphics::Buffer *dest, int slice, int mipmap, const Rect &rect, size_t destoffset, int destwidth, size_t size) override;

	ptrdiff_t getRenderTargetHandle() const override;
	ptrdiff_t getSamplerHandle() const override;

	VkImageView getRenderTargetView(int mip, int layer);
	VkSampleCountFlagBits getMsaaSamples() const;

	void uploadByteData(PixelFormat pixelformat, const void *data, size_t size, int level, int slice, const Rect &r) override;

	void generateMipmapsInternal()  override;

	int getMSAA() const override;
	ptrdiff_t getHandle() const override;

private:
	void createTextureImageView();
	void clear();

	VkClearColorValue getClearValue();

	Graphics *vgfx = nullptr;
	VkDevice device = VK_NULL_HANDLE;
	VmaAllocator allocator = VK_NULL_HANDLE;
	VkImage textureImage = VK_NULL_HANDLE;
	VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VmaAllocation textureImageAllocation = VK_NULL_HANDLE;
	VkImageView textureImageView = VK_NULL_HANDLE;
	std::vector<std::vector<VkImageView>> renderTargetImageViews;
	VkSampler textureSampler = VK_NULL_HANDLE;
	Slices slices;
	int layerCount = 0;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
};

} // vulkan
} // graphics
} // love
