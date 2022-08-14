#include "Texture.h"
#include "Graphics.h"
#include "Vulkan.h"

#include <limits>

// make vulkan::Graphics functions available
#define vgfx ((Graphics*)gfx)

namespace love {
namespace graphics {
namespace vulkan {
Texture::Texture(love::graphics::Graphics* gfx, const Settings& settings, const Slices* data)
	: love::graphics::Texture(gfx, settings, data), gfx(gfx), slices(settings.type) {
	if (data) {
		slices = *data;
	}

	loadVolatile();
}

bool Texture::loadVolatile() {
	allocator = vgfx->getVmaAllocator();
	device = vgfx->getDevice();

	auto vulkanFormat = Vulkan::getTextureFormat(format);

	// fixme: can we cut down these flags?
	VkImageUsageFlags usageFlags = 
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | 
		VK_IMAGE_USAGE_SAMPLED_BIT | 
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	VkImageCreateFlags createFlags = 0;

	layerCount = 1;
	if (texType == TEXTURE_VOLUME) {
		layerCount = getDepth();
	}
	else if (texType == TEXTURE_2D_ARRAY) {
		layerCount = getLayerCount();
	}
	else if (texType == TEXTURE_CUBE) {
		layerCount = 6;
		createFlags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	}

	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.flags = createFlags;
	imageInfo.imageType = Vulkan::getImageType(getTextureType());
	imageInfo.extent.width = static_cast<uint32_t>(width);
	imageInfo.extent.height = static_cast<uint32_t>(height);
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = static_cast<uint32_t>(getMipmapCount());
	imageInfo.arrayLayers = static_cast<uint32_t>(layerCount);
	imageInfo.format = vulkanFormat.internalFormat;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usageFlags;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

	VmaAllocationCreateInfo imageAllocationCreateInfo{};

	if (vmaCreateImage(allocator, &imageInfo, &imageAllocationCreateInfo, &textureImage, &textureImageAllocation, nullptr) != VK_SUCCESS) {
		throw love::Exception("failed to create image");
	}

	auto commandBuffer = vgfx->getDataTransferCommandBuffer();

	// fixme: we probably should select a different default layout when the texture is not readable, instead of VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	Vulkan::cmdTransitionImageLayout(commandBuffer, textureImage, 
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
		0, static_cast<uint32_t>(getMipmapCount()), 
		0, static_cast<uint32_t>(layerCount));

	bool hasdata = slices.get(0, 0) != nullptr;

	if (hasdata) {
		for (int mip = 0; mip < layerCount; mip++) {
			// fixme: deal with compressed images.

			int sliceCount;
			if (texType == TEXTURE_CUBE) {
				sliceCount = 6;
			} else {
				sliceCount = slices.getSliceCount();
			}
			for (int slice = 0; slice < sliceCount; slice++) {
				auto* id = slices.get(slice, mip);
				if (id != nullptr) {
					uploadImageData(id, mip, slice, 0, 0);
				}
			}
		}
	} else {
		clear();
	}
	createTextureImageView();
	textureSampler = vgfx->getCachedSampler(samplerState);

	if (slices.getMipmapCount() <= 1 && getMipmapsMode() != MIPMAPS_NONE) {
		generateMipmaps();
	}

	return true;
}

void Texture::unloadVolatile() {
	if (textureImage == VK_NULL_HANDLE)
		return;

	vgfx->queueCleanUp([
		device = device, 
		textureImageView = textureImageView, 
		allocator = allocator, 
		textureImage = textureImage, 
		textureImageAllocation = textureImageAllocation] () {
		vkDestroyImageView(device, textureImageView, nullptr);
		vmaDestroyImage(allocator, textureImage, textureImageAllocation);
	});

	textureImage = VK_NULL_HANDLE;
}

Texture::~Texture() {
	unloadVolatile();
}

void Texture::setSamplerState(const SamplerState &s) {
	love::graphics::Texture::setSamplerState(s);
	textureSampler = vgfx->getCachedSampler(s);
}

void Texture::createTextureImageView() {
	auto vulkanFormat = Vulkan::getTextureFormat(format);

	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = textureImage;
	viewInfo.viewType = Vulkan::getImageViewType(getTextureType());
	viewInfo.format = vulkanFormat.internalFormat;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = layerCount;
	viewInfo.components.r = vulkanFormat.swizzleR;
	viewInfo.components.g = vulkanFormat.swizzleG;
	viewInfo.components.b = vulkanFormat.swizzleB;
	viewInfo.components.a = vulkanFormat.swizzleA;

	if (vkCreateImageView(device, &viewInfo, nullptr, &textureImageView) != VK_SUCCESS) {
		throw love::Exception("could not create texture image view");
	}
}

void Texture::clear() {
	auto commandBuffer = vgfx->getDataTransferCommandBuffer();

	auto clearColor = getClearValue(false);

	VkImageSubresourceRange range{};
	range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	range.layerCount = static_cast<uint32_t>(layerCount);
	range.levelCount = static_cast<uint32_t>(getMipmapCount());

	Vulkan::cmdTransitionImageLayout(commandBuffer, textureImage, 
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
		0, range.levelCount, 0, range.layerCount);

	vkCmdClearColorImage(commandBuffer, textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &range);

	Vulkan::cmdTransitionImageLayout(commandBuffer, textureImage, 
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
		0, range.levelCount, 0, range.layerCount);
}

VkClearColorValue Texture::getClearValue(bool white) {
	auto vulkanFormat = Vulkan::getTextureFormat(format);

	VkClearColorValue clearColor{};
	if (white) {
		switch (vulkanFormat.internalFormatRepresentation) {
		case FORMATREPRESENTATION_FLOAT:
			clearColor.float32[0] = 1.0f;
			clearColor.float32[1] = 1.0f;
			clearColor.float32[2] = 1.0f;
			clearColor.float32[3] = 1.0f;
			break;
		case FORMATREPRESENTATION_SINT:
			clearColor.int32[0] = std::numeric_limits<int32_t>::max();
			clearColor.int32[1] = std::numeric_limits<int32_t>::max();
			clearColor.int32[2] = std::numeric_limits<int32_t>::max();
			clearColor.int32[3] = std::numeric_limits<int32_t>::max();
			break;
		case FORMATREPRESENTATION_UINT:
			clearColor.uint32[0] = std::numeric_limits<uint32_t>::max();
			clearColor.uint32[1] = std::numeric_limits<uint32_t>::max();
			clearColor.uint32[2] = std::numeric_limits<uint32_t>::max();
			clearColor.uint32[3] = std::numeric_limits<uint32_t>::max();
			break;
		}
	}
	else {
		switch (vulkanFormat.internalFormatRepresentation) {
		case FORMATREPRESENTATION_FLOAT:
			clearColor.float32[0] = 0.0f;
			clearColor.float32[1] = 0.0f;
			clearColor.float32[2] = 0.0f;
			clearColor.float32[3] = 0.0f;
			break;
		case FORMATREPRESENTATION_SINT:
			clearColor.int32[0] = 0;
			clearColor.int32[1] = 0;
			clearColor.int32[2] = 0;
			clearColor.int32[3] = 0;
			break;
		case FORMATREPRESENTATION_UINT:
			clearColor.uint32[0] = 0;
			clearColor.uint32[1] = 0;
			clearColor.uint32[2] = 0;
			clearColor.uint32[3] = 0;
			break;
		}
	}
	return clearColor;
}

void Texture::generateMipmapsInternal() {
	auto commandBuffer = vgfx->getDataTransferCommandBuffer();

	Vulkan::cmdTransitionImageLayout(commandBuffer, textureImage, 
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
		0, static_cast<uint32_t>(getMipmapCount()), 0, static_cast<uint32_t>(layerCount));

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = textureImage;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = static_cast<uint32_t>(layerCount);
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1u;

	uint32_t mipLevels = static_cast<uint32_t>(getMipmapCount());

	for (uint32_t i = 1; i < mipLevels; i++) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { getWidth(i - 1), getHeight(i - 1), 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = static_cast<uint32_t>(layerCount);

		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { getWidth(i), getHeight(i), 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = static_cast<uint32_t>(layerCount);

		vkCmdBlitImage(commandBuffer, 
			textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);
}

void Texture::uploadByteData(PixelFormat pixelformat, const void* data, size_t size, int level, int slice, const Rect& r) {
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
	vmaCreateBuffer(allocator, &bufferCreateInfo, &allocCreateInfo, &stagingBuffer, &vmaAllocation, &allocInfo);

	memcpy(allocInfo.pMappedData, data, size);

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = level;
	region.imageSubresource.baseArrayLayer = slice;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { r.x, r.y, 0 };
	region.imageExtent = {
		static_cast<uint32_t>(r.w),
		static_cast<uint32_t>(r.h), 1
	};

	auto commandBuffer = vgfx->getDataTransferCommandBuffer();

	Vulkan::cmdTransitionImageLayout(commandBuffer, textureImage, 
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
		level, 1, slice, 1);

	vkCmdCopyBufferToImage(
		commandBuffer,
		stagingBuffer,
		textureImage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	Vulkan::cmdTransitionImageLayout(commandBuffer, textureImage, 
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
		level, 1, slice, 1);

	vgfx->queueCleanUp([allocator = allocator, stagingBuffer, vmaAllocation]() {
		vmaDestroyBuffer(allocator, stagingBuffer, vmaAllocation);
	});
}

void Texture::copyFromBuffer(graphics::Buffer* source, size_t sourceoffset, int sourcewidth, size_t size, int slice, int mipmap, const Rect& rect) {
	auto commandBuffer = vgfx->getDataTransferCommandBuffer();

	VkImageSubresourceLayers layers{};
	layers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	layers.mipLevel = mipmap;
	layers.baseArrayLayer = slice;
	layers.layerCount = 1;

	VkBufferImageCopy region{};
	region.bufferOffset = sourceoffset;
	region.bufferRowLength = sourcewidth;
	region.bufferImageHeight = 1;
	region.imageSubresource = layers;
	region.imageExtent.width = static_cast<uint32_t>(rect.w);
	region.imageExtent.height = static_cast<uint32_t>(rect.h);

	Vulkan::cmdTransitionImageLayout(commandBuffer, textureImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	vkCmdCopyBufferToImage(commandBuffer, (VkBuffer)source->getHandle(), textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	Vulkan::cmdTransitionImageLayout(commandBuffer, textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void Texture::copyToBuffer(graphics::Buffer* dest, int slice, int mipmap, const Rect& rect, size_t destoffset, int destwidth, size_t size) {
	auto commandBuffer = vgfx->getDataTransferCommandBuffer();

	VkImageSubresourceLayers layers{};
	layers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	layers.mipLevel = mipmap;
	layers.baseArrayLayer = slice;
	layers.layerCount = 1;

	VkBufferImageCopy region{};
	region.bufferOffset = destoffset;
	region.bufferRowLength = destwidth;
	region.bufferImageHeight = 1;
	region.imageSubresource = layers;
	region.imageExtent.width = static_cast<uint32_t>(rect.w);
	region.imageExtent.height = static_cast<uint32_t>(rect.h);

	Vulkan::cmdTransitionImageLayout(commandBuffer, textureImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	vkCmdCopyImageToBuffer(commandBuffer, textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, (VkBuffer) dest->getHandle(), 1, &region);

	Vulkan::cmdTransitionImageLayout(commandBuffer, textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

} // vulkan
} // graphics
} // love
