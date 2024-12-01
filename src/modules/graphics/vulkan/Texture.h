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
	Texture(love::graphics::Graphics *gfx, love::graphics::Texture *base, const Texture::ViewSettings &viewsettings);
	virtual ~Texture();

	bool loadVolatile() override;
	void unloadVolatile() override;
	 
	void setSamplerState(const SamplerState &s) override;

	VkImageLayout getImageLayout() const;
	VkImageLayout getMSAAImageLayout() const;

	void copyFromBuffer(graphics::Buffer *source, size_t sourceoffset, int sourcewidth, size_t size, int slice, int mipmap, const Rect &rect) override;
	void copyToBuffer(graphics::Buffer *dest, int slice, int mipmap, const Rect &rect, size_t destoffset, int destwidth, size_t size) override;

	ptrdiff_t getRenderTargetHandle() const override;
	ptrdiff_t getSamplerHandle() const override;

	VkImageView getRenderTargetView(int mip, int layer);
	VkImageView getMSAARenderTargetView(int mip, int layer);
	VkSampleCountFlagBits getMsaaSamples() const;

	void uploadByteData(const void *data, size_t size, int level, int slice, const Rect &r) override;

	void generateMipmapsInternal() override;

	int getMSAA() const override;
	ptrdiff_t getHandle() const override;

	static VkClearColorValue getClearColor(love::graphics::Texture *texture, const ColorD &color);

private:

	struct VulkanImageData
	{
		VkImage image = VK_NULL_HANDLE;
		VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
		VmaAllocation allocation = VK_NULL_HANDLE;
		VkImageView imageView = VK_NULL_HANDLE;
		std::vector<std::vector<VkImageView>> renderTargetImageViews;
	};

	void createTextureImageView();
	void createRenderTargetImageViews(VulkanImageData &data);
	void clear(const VulkanImageData &data);

	Graphics *vgfx = nullptr;
	VkDevice device = VK_NULL_HANDLE;
	VkImageAspectFlags imageAspect;
	VmaAllocator allocator = VK_NULL_HANDLE;
	VulkanImageData imageData;
	VulkanImageData msaaImageData;
	VkSampler textureSampler = VK_NULL_HANDLE;
	Slices slices;
	int layerCount = 0;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
};

} // vulkan
} // graphics
} // love
