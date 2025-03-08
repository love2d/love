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

#include "Texture.h"
#include "Graphics.h"
#include "Vulkan.h"
#include "Buffer.h"

#include <limits>
#include <array>

namespace love
{
namespace graphics
{
namespace vulkan
{

Texture::Texture(love::graphics::Graphics *gfx, const Settings &settings, const Slices *data)
	: love::graphics::Texture(gfx, settings, data)
	, vgfx(dynamic_cast<Graphics*>(gfx))
	, slices(settings.type)
	, imageAspect(0)
{
	if (data)
		slices = *data;

	loadVolatile();

	// ImageData is referenced by the first loadVolatile call, but we don't
	// hang on to it after that so we can save memory.
	slices.clear();
}

Texture::Texture(love::graphics::Graphics *gfx, love::graphics::Texture *base, const Texture::ViewSettings &viewsettings)
	: love::graphics::Texture(gfx, base, viewsettings)
	, vgfx(dynamic_cast<Graphics*>(gfx))
	, slices(viewsettings.type.get(base->getTextureType()))
	, imageAspect(0)
{
	loadVolatile();
}

bool Texture::loadVolatile()
{
	allocator = vgfx->getVmaAllocator();
	device = vgfx->getDevice();

	bool root = rootView.texture == this;

	if (isPixelFormatDepth(format))
		imageAspect |= VK_IMAGE_ASPECT_DEPTH_BIT;
	if (isPixelFormatStencil(format))
		imageAspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
	if (isPixelFormatColor(format))
		imageAspect |= VK_IMAGE_ASPECT_COLOR_BIT;

	auto vulkanFormat = Vulkan::getTextureFormat(format);

	VkImageUsageFlags usageFlags =
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
		VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	if (readable)
		usageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;

	if (computeWrite)
		usageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;

	if (renderTarget)
	{
		if (isPixelFormatDepthStencil(format))
			usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		else
			usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}

	layerCount = 1;

	if (texType == TEXTURE_2D_ARRAY)
		layerCount = getLayerCount();
	else if (texType == TEXTURE_CUBE)
		layerCount = 6;

	if (root)
	{
		VkImageCreateFlags createFlags = 0;
		std::vector<VkFormat> vkviewformats;

		for (PixelFormat viewformat : viewFormats)
		{
			if (viewformat != format)
			{
				createFlags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
				TextureFormat f = Vulkan::getTextureFormat(viewformat);
				vkviewformats.push_back(f.internalFormat);
			}
		}

		if (texType == TEXTURE_CUBE || (texType == TEXTURE_2D_ARRAY && layerCount >= 6 && pixelWidth == pixelHeight))
			createFlags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		msaaSamples = vgfx->getMsaaCount(requestedMSAA);

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.flags = createFlags;
		imageInfo.imageType = Vulkan::getImageType(getTextureType());
		imageInfo.extent.width = static_cast<uint32_t>(pixelWidth);
		imageInfo.extent.height = static_cast<uint32_t>(pixelHeight);
		imageInfo.extent.depth = static_cast<uint32_t>(depth);
		imageInfo.arrayLayers = static_cast<uint32_t>(layerCount);
		imageInfo.mipLevels = static_cast<uint32_t>(mipmapCount);
		imageInfo.format = vulkanFormat.internalFormat;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usageFlags;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

		VkImageFormatListCreateInfo viewFormatsInfo{};
		viewFormatsInfo.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO;

		if (!vkviewformats.empty() && vgfx->getDeviceApiVersion() >= VK_API_VERSION_1_2)
		{
			viewFormatsInfo.viewFormatCount = (uint32)vkviewformats.size();
			viewFormatsInfo.pViewFormats = vkviewformats.data();

			imageInfo.pNext = &viewFormatsInfo;
		}

		VmaAllocationCreateInfo imageAllocationCreateInfo{};

		// Dedicated allocations are recommended for fullscreen RTs.
		if (renderTarget && pixelWidth >= vgfx->getPixelWidth() && pixelHeight >= vgfx->getPixelHeight())
			imageAllocationCreateInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

		imageAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

		if ((msaaSamples & VK_SAMPLE_COUNT_1_BIT) != 0 || readable)
		{
			VkResult result = vmaCreateImage(allocator, &imageInfo, &imageAllocationCreateInfo, &imageData.image, &imageData.allocation, nullptr);
			if (result != VK_SUCCESS)
				throw love::Exception("Failed to create Vulkan image: %s", Vulkan::getErrorString(result));
		}

		if ((msaaSamples & VK_SAMPLE_COUNT_1_BIT) == 0)
		{
			imageInfo.samples = msaaSamples;
			VkResult result = vmaCreateImage(allocator, &imageInfo, &imageAllocationCreateInfo, &msaaImageData.image, &msaaImageData.allocation, nullptr);
			if (result != VK_SUCCESS)
				throw love::Exception("Failed to create Vulkan MSAA image: %s", Vulkan::getErrorString(result));
		}

		auto commandBuffer = vgfx->getCommandBufferForDataTransfer();

		if (computeWrite)
			imageData.layout = VK_IMAGE_LAYOUT_GENERAL;
		else if (readable)
			imageData.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		else if (renderTarget && isPixelFormatDepthStencil(format))
			imageData.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		else if (renderTarget)
			imageData.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		else // TODO: is there a better layout for this situation?
			imageData.layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

		if (imageData.image != VK_NULL_HANDLE)
		{
			Vulkan::cmdTransitionImageLayout(commandBuffer, imageData.image, format, renderTarget,
				VK_IMAGE_LAYOUT_UNDEFINED, imageData.layout,
				0, VK_REMAINING_MIP_LEVELS,
				0, VK_REMAINING_ARRAY_LAYERS);
		}

		if (msaaImageData.image != VK_NULL_HANDLE)
		{
			if (renderTarget && isPixelFormatDepthStencil(format))
				msaaImageData.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			else if (renderTarget)
				msaaImageData.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			else // TODO: is there a better layout for this situation?
				msaaImageData.layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

			Vulkan::cmdTransitionImageLayout(commandBuffer, msaaImageData.image, format, renderTarget,
				VK_IMAGE_LAYOUT_UNDEFINED, msaaImageData.layout,
				0, VK_REMAINING_MIP_LEVELS,
				0, VK_REMAINING_ARRAY_LAYERS);
		}

		bool hasdata = slices.get(0, 0) != nullptr;

		if (hasdata)
		{
			for (int mip = 0; mip < getMipmapCount(); mip++)
			{
				int sliceCount;
				if (texType == TEXTURE_CUBE)
					sliceCount = 6;
				else
					sliceCount = slices.getSliceCount();

				for (int slice = 0; slice < sliceCount; slice++)
				{
					auto id = slices.get(slice, mip);
					if (id != nullptr)
						uploadImageData(id, mip, slice, 0, 0);
				}
			}
		}
		else
		{
			if (imageData.image != VK_NULL_HANDLE)
				clear(imageData);
			if (msaaImageData.image != VK_NULL_HANDLE)
				clear(msaaImageData);
		}
	}
	else
	{
		Texture *roottex = (Texture *) rootView.texture;
		imageData = roottex->imageData;
		msaaImageData = roottex->msaaImageData;
		imageData.allocation = VK_NULL_HANDLE;
		msaaImageData.allocation = VK_NULL_HANDLE;
		msaaSamples = roottex->msaaSamples;
	}

	createTextureImageView();
	setSamplerState(samplerState);

	if (root && !isPixelFormatDepthStencil(format) && slices.getMipmapCount() <= 1 && getMipmapsMode() != MIPMAPS_NONE)
		generateMipmaps();

	if (renderTarget)
	{
		createRenderTargetImageViews(imageData);
		createRenderTargetImageViews(msaaImageData);
	}

	if (!debugName.empty())
	{
		if (vgfx->getEnabledOptionalInstanceExtensions().debugInfo)
		{
			VkDebugUtilsObjectNameInfoEXT nameInfo{};
			nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
			if (root)
			{
				nameInfo.objectType = VK_OBJECT_TYPE_IMAGE;
				nameInfo.objectHandle = (uint64_t)imageData.image;
			}
			else
			{
				nameInfo.objectType = VK_OBJECT_TYPE_IMAGE_VIEW;
				nameInfo.objectHandle = (uint64_t)imageData.imageView;
			}
			nameInfo.pObjectName = debugName.c_str();
			vkSetDebugUtilsObjectNameEXT(device, &nameInfo);

			if (msaaImageData.image != VK_NULL_HANDLE && root)
			{
				nameInfo.objectType = VK_OBJECT_TYPE_IMAGE;
				nameInfo.objectHandle = (uint64_t)msaaImageData.image;

				std::string msaaName = debugName + " (MSAA buffer)";
				nameInfo.pObjectName = msaaName.c_str();
				vkSetDebugUtilsObjectNameEXT(device, &nameInfo);
			}
		}
	}

	if (root)
		updateGraphicsMemorySize(true);

	return true;
}

void Texture::unloadVolatile()
{
	std::array<VulkanImageData *, 2> datas = {&imageData, &msaaImageData};

	for (const VulkanImageData *data : datas)
	{
		if (data->image == VK_NULL_HANDLE)
			continue;

		vgfx->queueCleanUp([
			device = device,
			allocator = allocator,
			imageData = *data,
			vgfx = vgfx,
			renderTarget = renderTarget,
			format = format] () {
			if (imageData.imageView != VK_NULL_HANDLE)
			{
				if (renderTarget)
					vgfx->cleanupFramebuffers(imageData.imageView, format);
				vkDestroyImageView(device, imageData.imageView, nullptr);
			}
			if (imageData.allocation != VK_NULL_HANDLE)
				vmaDestroyImage(allocator, imageData.image, imageData.allocation);
			for (const auto &views : imageData.renderTargetImageViews)
				for (const auto &view : views)
					vkDestroyImageView(device, view, nullptr);
		});
	}

	imageData = {};
	msaaImageData = {};

	updateGraphicsMemorySize(false);
}

Texture::~Texture()
{
	unloadVolatile();
}

ptrdiff_t Texture::getRenderTargetHandle() const
{
	return (ptrdiff_t)imageData.image;
}

ptrdiff_t Texture::getSamplerHandle() const
{
	return (ptrdiff_t)textureSampler;
}

VkImageView Texture::getRenderTargetView(int mip, int layer)
{
	if (imageData.renderTargetImageViews.empty())
		return imageData.imageView;
	return imageData.renderTargetImageViews.at(mip).at(layer);
}

VkImageView Texture::getMSAARenderTargetView(int mip, int layer)
{
	if (msaaImageData.renderTargetImageViews.empty())
		return msaaImageData.imageView;
	return msaaImageData.renderTargetImageViews.at(mip).at(layer);
}

VkSampleCountFlagBits Texture::getMsaaSamples() const
{
	return msaaSamples;
}

int Texture::getMSAA() const
{
	return static_cast<int>(msaaSamples);
}

ptrdiff_t Texture::getHandle() const
{
	return (ptrdiff_t)imageData.imageView;
}

void Texture::setSamplerState(const SamplerState &s)
{
	samplerState = validateSamplerState(s);
	textureSampler = vgfx->getCachedSampler(samplerState);
}

VkImageLayout Texture::getImageLayout() const
{
	return imageData.layout;
}

VkImageLayout Texture::getMSAAImageLayout() const
{
	return msaaImageData.layout;
}

void Texture::createTextureImageView()
{
	auto vulkanFormat = Vulkan::getTextureFormat(format);
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = imageData.image;
	viewInfo.viewType = Vulkan::getImageViewType(getTextureType());
	viewInfo.format = vulkanFormat.internalFormat;
	viewInfo.subresourceRange.aspectMask = imageAspect;
	// This view is used in descriptor sets, where having both depth and
	// stencil aspects in the same view isn't allowed.
	if (imageAspect & VK_IMAGE_ASPECT_DEPTH_BIT)
		viewInfo.subresourceRange.aspectMask &= ~VK_IMAGE_ASPECT_STENCIL_BIT;
	viewInfo.subresourceRange.baseMipLevel = rootView.startMipmap;
	viewInfo.subresourceRange.levelCount = getMipmapCount();
	viewInfo.subresourceRange.baseArrayLayer = rootView.startLayer;
	viewInfo.subresourceRange.layerCount = layerCount;
	viewInfo.components.r = vulkanFormat.swizzleR;
	viewInfo.components.g = vulkanFormat.swizzleG;
	viewInfo.components.b = vulkanFormat.swizzleB;
	viewInfo.components.a = vulkanFormat.swizzleA;

	imageData.imageView = VK_NULL_HANDLE;
	if (imageData.image != VK_NULL_HANDLE && readable)
	{
		VkResult result = vkCreateImageView(device, &viewInfo, nullptr, &imageData.imageView);
		if (result != VK_SUCCESS)
			throw love::Exception("Could not create Vulkan texture image view: %s", Vulkan::getErrorString(result));
	}
}

void Texture::createRenderTargetImageViews(VulkanImageData &data)
{
	if (data.image == VK_NULL_HANDLE || (getMipmapCount() == 1 && layerCount == 1 && data.imageView != VK_NULL_HANDLE))
	{
		// Reuse the main image view in this situation, if it exists.
		data.renderTargetImageViews.clear();
		return;
	}

	auto vulkanFormat = Vulkan::getTextureFormat(format);

	data.renderTargetImageViews.resize(getMipmapCount());
	for (int mip = 0; mip < getMipmapCount(); mip++)
	{
		data.renderTargetImageViews.at(mip).resize(layerCount);

		for (int slice = 0; slice < layerCount; slice++)
		{
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = data.image;
			viewInfo.viewType = Vulkan::getImageViewType(getTextureType());
			if (viewInfo.viewType == VK_IMAGE_VIEW_TYPE_CUBE)
				viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = vulkanFormat.internalFormat;
			viewInfo.subresourceRange.aspectMask = imageAspect;
			viewInfo.subresourceRange.baseMipLevel = mip + rootView.startMipmap;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = slice + rootView.startLayer;
			viewInfo.subresourceRange.layerCount = 1;
			viewInfo.components.r = vulkanFormat.swizzleR;
			viewInfo.components.g = vulkanFormat.swizzleG;
			viewInfo.components.b = vulkanFormat.swizzleB;
			viewInfo.components.a = vulkanFormat.swizzleA;

			VkResult result = vkCreateImageView(device, &viewInfo, nullptr, &data.renderTargetImageViews.at(mip).at(slice));
			if (result != VK_SUCCESS)
				throw love::Exception("Could not create Vulkan render target image view: %s", Vulkan::getErrorString(result));
		}
	}
}

void Texture::clear(const VulkanImageData &data)
{
	auto commandBuffer = vgfx->getCommandBufferForDataTransfer();

	VkImageSubresourceRange range{};
	range.aspectMask = imageAspect;
	range.baseMipLevel = 0;
	range.levelCount = VK_REMAINING_MIP_LEVELS;
	range.baseArrayLayer = 0;
	range.layerCount = VK_REMAINING_ARRAY_LAYERS;

	VkImageLayout clearLayout = data.layout == VK_IMAGE_LAYOUT_GENERAL ? data.layout : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

	Vulkan::cmdTransitionImageLayout(commandBuffer, data.image, format, renderTarget,
		data.layout, clearLayout,
		0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS);

	if (isPixelFormatDepthStencil(format))
	{
		VkClearDepthStencilValue depthStencilColor{};
		depthStencilColor.depth = 0.0f;
		depthStencilColor.stencil = 0;
		vkCmdClearDepthStencilImage(commandBuffer, data.image, clearLayout, &depthStencilColor, 1, &range);
	}
	else
	{
		auto clearColor = getClearColor(this, ColorD(0, 0, 0, 0));
		vkCmdClearColorImage(commandBuffer, data.image, clearLayout, &clearColor, 1, &range);
	}

	Vulkan::cmdTransitionImageLayout(commandBuffer, data.image, format, renderTarget,
		clearLayout, data.layout,
		0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS);
}

VkClearColorValue Texture::getClearColor(love::graphics::Texture *texture, const ColorD &color)
{
	PixelFormatType formattype = PIXELFORMATTYPE_SFLOAT;
	if (texture != nullptr)
		formattype = getPixelFormatInfo(texture->getPixelFormat()).dataType;

	VkClearColorValue c{};

	switch (formattype)
	{
	case PIXELFORMATTYPE_SINT:
		c.int32[0] = (int32)color.r;
		c.int32[1] = (int32)color.g;
		c.int32[2] = (int32)color.b;
		c.int32[3] = (int32)color.a;
		break;
	case PIXELFORMATTYPE_UINT:
		c.uint32[0] = (uint32)color.r;
		c.uint32[1] = (uint32)color.g;
		c.uint32[2] = (uint32)color.b;
		c.uint32[3] = (uint32)color.a;
		break;
	default:
		{
			Colorf cf((float)color.r, (float)color.g, (float)color.b, (float)color.a);
			gammaCorrectColor(cf);
			c.float32[0] = cf.r;
			c.float32[1] = cf.g;
			c.float32[2] = cf.b;
			c.float32[3] = cf.a;
		}
		break;
	}

	return c;
}

void Texture::generateMipmapsInternal()
{
	auto commandBuffer = vgfx->getCommandBufferForDataTransfer();

	auto dstPipelineStages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT
		| VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT;

	Vulkan::cmdTransitionImageLayout(commandBuffer, imageData.image, format, renderTarget,
		imageData.layout, imageData.layout == VK_IMAGE_LAYOUT_GENERAL ? imageData.layout : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		rootView.startMipmap, static_cast<uint32_t>(getMipmapCount()),
		rootView.startLayer, static_cast<uint32_t>(layerCount));

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = imageData.image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = rootView.startLayer;
	barrier.subresourceRange.layerCount = static_cast<uint32_t>(layerCount);
	barrier.subresourceRange.baseMipLevel = rootView.startMipmap;
	barrier.subresourceRange.levelCount = 1u;

	uint32_t mipLevels = static_cast<uint32_t>(getMipmapCount());

	for (uint32_t i = 1; i < mipLevels; i++)
	{
		barrier.subresourceRange.baseMipLevel = rootView.startMipmap + i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		if (imageData.layout != VK_IMAGE_LAYOUT_GENERAL)
		{
			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);
		}
		else
		{
			VkMemoryBarrier memoryBarrier{};
			memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
			memoryBarrier.srcAccessMask = barrier.srcAccessMask;
			memoryBarrier.dstAccessMask = barrier.dstAccessMask;
			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				1, &memoryBarrier,
				0, nullptr,
				0, nullptr);
		}

		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { getPixelWidth(i - 1), getPixelHeight(i - 1), getDepth(i - 1) };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = rootView.startMipmap + i - 1;
		blit.srcSubresource.baseArrayLayer = rootView.startLayer;
		blit.srcSubresource.layerCount = static_cast<uint32_t>(layerCount);

		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { getPixelWidth(i), getPixelHeight(i), getDepth(i) };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = rootView.startMipmap + i;
		blit.dstSubresource.baseArrayLayer = rootView.startLayer;
		blit.dstSubresource.layerCount = static_cast<uint32_t>(layerCount);

		if (imageData.layout != VK_IMAGE_LAYOUT_GENERAL)
		{
			vkCmdBlitImage(commandBuffer,
				imageData.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				imageData.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);
		}
		else
		{
			vkCmdBlitImage(commandBuffer,
				imageData.image, VK_IMAGE_LAYOUT_GENERAL,
				imageData.image, VK_IMAGE_LAYOUT_GENERAL,
				1, &blit,
				VK_FILTER_LINEAR);
		}

		// This sucks, but we want the whole texture to be VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL because we
		// transition the whole thing back to the original layout after the loop.
		if (i == mipLevels - 1 && imageData.layout != VK_IMAGE_LAYOUT_GENERAL)
		{
			barrier.subresourceRange.baseMipLevel = rootView.startMipmap + i;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);
		}
	}

	Vulkan::cmdTransitionImageLayout(commandBuffer, imageData.image, format, renderTarget,
		imageData.layout == VK_IMAGE_LAYOUT_GENERAL ? imageData.layout : VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, imageData.layout,
		rootView.startMipmap, static_cast<uint32_t>(getMipmapCount()),
		rootView.startLayer, static_cast<uint32_t>(layerCount));
}

void Texture::uploadByteData(const void *data, size_t size, int level, int slice, const Rect &r)
{
	VkBuffer stagingBuffer;
	VmaAllocation vmaAllocation;

	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	VmaAllocationCreateInfo allocCreateInfo = {};
	allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

	VmaAllocationInfo allocInfo;
	VkResult result = vmaCreateBuffer(allocator, &bufferCreateInfo, &allocCreateInfo, &stagingBuffer, &vmaAllocation, &allocInfo);
	if (result != VK_SUCCESS)
		throw love::Exception("Failed to create Vulkan staging buffer for texture data upload: %s", Vulkan::getErrorString(result));

	memcpy(allocInfo.pMappedData, data, size);

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	uint32_t baseLayer = rootView.startLayer;
	if (getTextureType() != TEXTURE_VOLUME)
		baseLayer += slice;

	level += rootView.startMipmap;

	region.imageSubresource.aspectMask = imageAspect;
	region.imageSubresource.mipLevel = level;
	region.imageSubresource.baseArrayLayer = baseLayer;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { r.x, r.y, 0 };
	region.imageExtent = {
		static_cast<uint32_t>(r.w),
		static_cast<uint32_t>(r.h), 1
	};

	if (getTextureType() == TEXTURE_VOLUME)
		region.imageOffset.z = slice;

	auto commandBuffer = vgfx->getCommandBufferForDataTransfer();

	VkImageLayout copyDstLayout = imageData.layout == VK_IMAGE_LAYOUT_GENERAL ? imageData.layout : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

	Vulkan::cmdTransitionImageLayout(commandBuffer, imageData.image, format, renderTarget,
		imageData.layout, copyDstLayout,
		level, 1, baseLayer, 1);

	vkCmdCopyBufferToImage(
		commandBuffer,
		stagingBuffer,
		imageData.image,
		copyDstLayout,
		1,
		&region
	);

	Vulkan::cmdTransitionImageLayout(commandBuffer, imageData.image, format, renderTarget,
		copyDstLayout, imageData.layout,
		level, 1, baseLayer, 1);

	vgfx->queueCleanUp([allocator = allocator, stagingBuffer, vmaAllocation]() {
		vmaDestroyBuffer(allocator, stagingBuffer, vmaAllocation);
	});
}

void Texture::copyFromBuffer(graphics::Buffer *source, size_t sourceoffset, int sourcewidth, size_t size, int slice, int mipmap, const Rect &rect)
{
	auto commandBuffer = vgfx->getCommandBufferForDataTransfer();

	VkImageSubresourceLayers layers{};
	layers.aspectMask = imageAspect;
	layers.mipLevel = mipmap + rootView.startMipmap;
	layers.baseArrayLayer = slice + rootView.startLayer;
	layers.layerCount = 1;

	VkBufferImageCopy region{};
	region.bufferOffset = sourceoffset;
	region.bufferRowLength = sourcewidth;
	region.bufferImageHeight = 1;
	region.imageSubresource = layers;
	region.imageOffset.x = rect.x;
	region.imageOffset.y = rect.y;
	region.imageExtent.width = static_cast<uint32_t>(rect.w);
	region.imageExtent.height = static_cast<uint32_t>(rect.h);

	VkImageLayout copyDstLayout = imageData.layout == VK_IMAGE_LAYOUT_GENERAL ? imageData.layout : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	Vulkan::cmdTransitionImageLayout(commandBuffer, imageData.image, format, renderTarget, imageData.layout, copyDstLayout, layers.mipLevel, 1, layers.baseArrayLayer, 1);

	vkCmdCopyBufferToImage(commandBuffer, (VkBuffer)source->getHandle(), imageData.image, copyDstLayout, 1, &region);

	Vulkan::cmdTransitionImageLayout(commandBuffer, imageData.image, format, renderTarget, copyDstLayout, imageData.layout, layers.mipLevel, 1, layers.baseArrayLayer, 1);
}

void Texture::copyToBuffer(graphics::Buffer *dest, int slice, int mipmap, const Rect &rect, size_t destoffset, int destwidth, size_t size)
{
	auto commandBuffer = vgfx->getCommandBufferForDataTransfer();

	VkImageSubresourceLayers layers{};
	layers.aspectMask = imageAspect;
	layers.mipLevel = mipmap + rootView.startMipmap;
	layers.baseArrayLayer = slice + rootView.startLayer;
	layers.layerCount = 1;

	VkBufferImageCopy region{};
	region.bufferOffset = destoffset;
	region.bufferRowLength = destwidth;
	region.bufferImageHeight = 0;
	region.imageSubresource = layers;
	region.imageOffset.x = rect.x;
	region.imageOffset.y = rect.y;
	region.imageExtent.width = static_cast<uint32_t>(rect.w);
	region.imageExtent.height = static_cast<uint32_t>(rect.h);
	region.imageExtent.depth = 1;

	VkImageLayout copySrcLayout = imageData.layout == VK_IMAGE_LAYOUT_GENERAL ? imageData.layout : VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	Vulkan::cmdTransitionImageLayout(commandBuffer, imageData.image, format, renderTarget, imageData.layout, copySrcLayout, layers.mipLevel, 1, layers.baseArrayLayer, 1);

	vkCmdCopyImageToBuffer(commandBuffer, imageData.image, copySrcLayout, (VkBuffer) dest->getHandle(), 1, &region);

	Vulkan::cmdTransitionImageLayout(commandBuffer, imageData.image, format, renderTarget, copySrcLayout, imageData.layout, layers.mipLevel, 1, layers.baseArrayLayer, 1);

	// TODO: This could be combined with the cmdTransitionImageLayout barrier.
	((Buffer *)dest)->postGPUWriteBarrier(commandBuffer);
}

} // vulkan
} // graphics
} // love
