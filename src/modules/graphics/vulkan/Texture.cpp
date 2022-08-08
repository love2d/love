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
	: love::graphics::Texture(gfx, settings, data), gfx(gfx), data(data) {
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

	layerCount = 1;
	if (texType == TEXTURE_VOLUME)
		layerCount = getDepth();
	else if (texType == TEXTURE_2D_ARRAY)
		layerCount = getLayerCount();
	else if (texType == TEXTURE_CUBE)
		layerCount = 6;

	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = Vulkan::getImageType(getTextureType());
	imageInfo.extent.width = static_cast<uint32_t>(width);
	imageInfo.extent.height = static_cast<uint32_t>(height);
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = layerCount;
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

	// fixme: we should use VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL as the default image layout instead of VK_IMAGE_LAYOUT_GENERAL.
	Vulkan::cmdTransitionImageLayout(commandBuffer, textureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, 0, 1, 0, layerCount);

	if (data) {
		for (int slice = 0; slice < layerCount; slice++) {
			auto sliceData = data->get(slice, 0);
			auto size = sliceData->getSize();
			auto dataPtr = sliceData->getData();
			Rect rect{};
			rect.x = 0;
			rect.y = 0;
			rect.w = sliceData->getWidth();
			rect.h = sliceData->getHeight();

			uploadByteData(format, dataPtr, size, 0, slice, rect);
		}
	} else {
		if (isRenderTarget()) {
			clear(false);
		}
		else {
			clear(true);
		}
	}
	createTextureImageView();
	textureSampler = vgfx->getCachedSampler(samplerState);

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

void Texture::clear(bool white) {
	auto commandBuffer = vgfx->beginSingleTimeCommands();

	auto clearColor = getClearValue(white);

	VkImageSubresourceRange range{};
	range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	range.layerCount = layerCount;
	range.levelCount = 1;

	vkCmdClearColorImage(commandBuffer, textureImage, VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &range);

	vgfx->endSingleTimeCommands(commandBuffer);
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

	Vulkan::cmdTransitionImageLayout(commandBuffer, textureImage, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, level, 1, slice, 1);

	vkCmdCopyBufferToImage(
		commandBuffer,
		stagingBuffer,
		textureImage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	Vulkan::cmdTransitionImageLayout(commandBuffer, textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, level, 1, slice, 1);

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

	Vulkan::cmdTransitionImageLayout(commandBuffer, textureImage, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	vkCmdCopyBufferToImage(commandBuffer, (VkBuffer)source->getHandle(), textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	Vulkan::cmdTransitionImageLayout(commandBuffer, textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
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

	Vulkan::cmdTransitionImageLayout(commandBuffer, textureImage, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	vkCmdCopyImageToBuffer(commandBuffer, textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, (VkBuffer) dest->getHandle(), 1, &region);

	Vulkan::cmdTransitionImageLayout(commandBuffer, textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
}

} // vulkan
} // graphics
} // love
