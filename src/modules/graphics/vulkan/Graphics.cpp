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

#include "common/Exception.h"
#include "common/pixelformat.h"
#include "common/version.h"
#include "common/memory.h"
#include "window/Window.h"
#include "Buffer.h"
#include "Graphics.h"
#include "GraphicsReadback.h"
#include "Shader.h"
#include "Vulkan.h"

#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL_hints.h>

#include <algorithm>
#include <vector>
#include <cstring>
#include <set>
#include <sstream>
#include <array>

#define VOLK_IMPLEMENTATION
#include "libraries/volk/volk.h"

#define VMA_IMPLEMENTATION
#include "libraries/vma/vk_mem_alloc.h"

namespace love
{
namespace graphics
{
namespace vulkan
{

static const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

static const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

constexpr int DEFAULT_VERTEX_BUFFER_BINDING = 0;
constexpr int VERTEX_BUFFER_BINDING_START = 1;

VkDevice Graphics::getDevice() const
{
	return device;
}

VmaAllocator Graphics::getVmaAllocator() const
{
	return vmaAllocator;
}

static void checkOptionalInstanceExtensions(OptionalInstanceExtensions& ext)
{
	uint32_t count;

	vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);

	std::vector<VkExtensionProperties> extensions(count);

	vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());

	for (const auto& extension : extensions)
	{
		if (strcmp(extension.extensionName, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) == 0)
			ext.physicalDeviceProperties2 = true;
		if (strcmp(extension.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
			ext.debugInfo = true;
	}
}

Graphics::Graphics()
	: love::graphics::Graphics("love.graphics.vulkan")
{
	if (!SDL_Vulkan_LoadLibrary(nullptr))
		throw love::Exception("could not find vulkan");

	volkInitializeCustom((PFN_vkGetInstanceProcAddr)SDL_Vulkan_GetVkGetInstanceProcAddr());

	if (isDebugEnabled() && !checkValidationSupport())
	{
		SDL_Vulkan_UnloadLibrary();
		throw love::Exception("validation layers requested, but not available");
	}

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "LOVE";
	appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);	// get this version from somewhere else?
	appInfo.pEngineName = "LOVE Game Framework";
	appInfo.engineVersion = VK_MAKE_API_VERSION(0, VERSION_MAJOR, VERSION_MINOR, VERSION_REV);
	appInfo.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.pNext = nullptr;

	// GetInstanceExtensions works with a null window parameter as long as
	// SDL_Vulkan_LoadLibrary has been called (which we do earlier).
	unsigned int count = 0;
	char const* const* extensions_string = SDL_Vulkan_GetInstanceExtensions(&count);
	if (extensions_string == nullptr)
	{
		SDL_Vulkan_UnloadLibrary();
		throw love::Exception("couldn't retrieve sdl vulkan extensions");
	}

	std::vector<const char*> extensions(extensions_string, extensions_string + count);

	checkOptionalInstanceExtensions(optionalInstanceExtensions);

	if (optionalInstanceExtensions.physicalDeviceProperties2)
		extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
	if (optionalInstanceExtensions.debugInfo)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	if (isDebugEnabled())
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
	{
		SDL_Vulkan_UnloadLibrary();
		throw love::Exception("couldn't create vulkan instance");
	}

	volkLoadInstance(instance);

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	int maxScore = 0;

	if (deviceCount > 0)
	{
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		// This is imperfect because we can't query a swap chain at this point.
		// In theory it could cause a device to have a non-zero score here and
		// no devices to have a non-zero score later in setMode, but hopefully
		// that won't happen in practice...
		for (const auto &device : devices)
			maxScore = std::max(maxScore, rateDeviceSuitability(device, false));
	}

	// Exit here if there are no suitable devices, to let other backends take over.
	if (maxScore == 0)
	{
		vkDestroyInstance(instance, nullptr);
		SDL_Vulkan_UnloadLibrary();
		throw love::Exception("no suitable vulkan physical devices found");
	}
}

Graphics::~Graphics()
{
	defaultVertexBuffer.set(nullptr);
	localUniformBuffer.set(nullptr);

	Volatile::unloadAll();
	cleanup();
	vkDestroyInstance(instance, nullptr);

	SDL_Vulkan_UnloadLibrary();
}

// START OVERRIDEN FUNCTIONS

love::graphics::Texture *Graphics::newTexture(const love::graphics::Texture::Settings &settings, const love::graphics::Texture::Slices *data)
{
	return new Texture(this, settings, data);
}

love::graphics::Texture *Graphics::newTextureView(love::graphics::Texture *base, const Texture::ViewSettings &viewsettings)
{
	return new Texture(this, base, viewsettings);
}

love::graphics::Buffer *Graphics::newBuffer(const love::graphics::Buffer::Settings &settings, const std::vector<love::graphics::Buffer::DataDeclaration> &format, const void *data, size_t size, size_t arraylength)
{
	return new Buffer(this, settings, format, data, size, arraylength);
}

void Graphics::clear(OptionalColorD color, OptionalInt stencil, OptionalDouble depth)
{
	if (!color.hasValue && !stencil.hasValue && !depth.hasValue)
		return;

	std::vector<OptionalColorD> colors;

	if (color.hasValue)
		colors.resize(std::max(1, (int)states.back().renderTargets.colors.size()), color);

	clear(colors, stencil, depth);
}

void Graphics::clear(const std::vector<OptionalColorD> &colors, OptionalInt stencil, OptionalDouble depth)
{
	if (colors.empty() && !stencil.hasValue && !depth.hasValue)
		return;

	flushBatchedDraws();

	const auto &rts = states.back().renderTargets;
	bool rtactive = isRenderTargetActive();
	size_t ncolorbuffers = rtactive ? rts.colors.size() : 1;
	size_t ncolors = std::min(ncolorbuffers, colors.size());

	if (renderPassState.active)
	{
		std::vector<VkClearAttachment> attachments;
		for (size_t i = 0; i < ncolors; i++)
		{
			const OptionalColorD &color = colors[i];
			VkClearAttachment attachment{};
			if (color.hasValue)
			{
				auto texture = i < rts.colors.size() ? rts.colors[i].texture.get() : nullptr;
				attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				attachment.clearValue.color = Texture::getClearColor(texture, color.value);
			}
			attachments.push_back(attachment);
		}

		VkClearAttachment depthStencilAttachment{};

		auto dstexture = rts.depthStencil.texture.get();

		if (stencil.hasValue)
		{
			if ((!rtactive && backbufferHasStencil)
				|| (dstexture && isPixelFormatStencil(dstexture->getPixelFormat())) || (rts.temporaryRTFlags & TEMPORARY_RT_STENCIL) != 0)
			{
				depthStencilAttachment.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
				depthStencilAttachment.clearValue.depthStencil.stencil = static_cast<uint32_t>(stencil.value);
			}
		}
		if (depth.hasValue)
		{
			if ((!rtactive && backbufferHasDepth)
				|| (dstexture && isPixelFormatDepth(dstexture->getPixelFormat())) || (rts.temporaryRTFlags & TEMPORARY_RT_DEPTH) != 0)
			{
				depthStencilAttachment.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
				depthStencilAttachment.clearValue.depthStencil.depth = static_cast<float>(depth.value);
			}
		}

		if (depthStencilAttachment.aspectMask != 0)
			attachments.push_back(depthStencilAttachment);

		VkClearRect rect{};
		rect.layerCount = 1;
		rect.rect.extent.width = static_cast<uint32_t>(renderPassState.width);
		rect.rect.extent.height = static_cast<uint32_t>(renderPassState.height);

		vkCmdClearAttachments(
			commandBuffers[currentFrame],
			static_cast<uint32_t>(attachments.size()), attachments.data(),
			1, &rect);
	}
	else
	{
		for (size_t i = 0; i < ncolors; i++)
		{
			if (colors[i].hasValue)
			{
				renderPassState.renderPassConfiguration.colorAttachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

				auto texture = i < rts.colors.size() ? rts.colors[i].texture.get() : nullptr;
				renderPassState.clearColors[i].color = Texture::getClearColor(texture, colors[i].value);
			}
		}

		if (depth.hasValue)
		{
			renderPassState.renderPassConfiguration.staticData.depthStencilAttachment.depthLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			renderPassState.clearColors[ncolorbuffers].depthStencil.depth = static_cast<float>(depth.value);
		}

		if (stencil.hasValue)
		{
			renderPassState.renderPassConfiguration.staticData.depthStencilAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			renderPassState.clearColors[ncolorbuffers].depthStencil.stencil = static_cast<uint32_t>(stencil.value);
		}

		if (renderPassState.isWindow)
		{
			renderPassState.windowClearRequested = true;
			renderPassState.mainWindowClearColorValue = colors.empty() ? OptionalColorD() : colors[0];
			renderPassState.mainWindowClearDepthValue = depth;
			renderPassState.mainWindowClearStencilValue = stencil;
		}
		else
			startRenderPass();
	}
}

void Graphics::discard(const std::vector<bool> &colorbuffers, bool depthstencil)
{
	if (renderPassState.active)
		endRenderPass();

	auto &renderPassConfiguration = renderPassState.renderPassConfiguration;

	for (size_t i = 0; i < colorbuffers.size(); i++)
	{
		if (colorbuffers[i])
			renderPassConfiguration.colorAttachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	}

	if (depthstencil)
	{
		renderPassConfiguration.staticData.depthStencilAttachment.depthLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		renderPassConfiguration.staticData.depthStencilAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	}

	startRenderPass();
}

void Graphics::submitGpuCommands(SubmitMode submitMode, void *screenshotCallbackData)
{
	flushBatchedDraws();

	if (renderPassState.active)
		endRenderPass();

	VkBuffer screenshotBuffer = VK_NULL_HANDLE;
	VmaAllocation screenshotAllocation = VK_NULL_HANDLE;
	VmaAllocationInfo screenshotAllocationInfo = {};

	if (submitMode == SUBMIT_PRESENT)
	{
		VkImage backbufferImage = fakeBackbuffer != nullptr ? (VkImage)fakeBackbuffer->getRenderTargetHandle() : swapChainImages.at(imageIndex);

		if (pendingScreenshotCallbacks.empty())
		{
			if (fakeBackbuffer == nullptr)
			{
				Vulkan::cmdTransitionImageLayout(
					commandBuffers.at(currentFrame),
					backbufferImage,
					swapChainPixelFormat, true,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
			}
		}
		else
		{
			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = 4ll * swapChainExtent.width * swapChainExtent.height;
			bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VmaAllocationCreateInfo allocCreateInfo{};
			allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
			allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

			auto result = vmaCreateBuffer(
				vmaAllocator,
				&bufferInfo,
				&allocCreateInfo,
				&screenshotBuffer,
				&screenshotAllocation,
				&screenshotAllocationInfo);

			if (result != VK_SUCCESS)
				throw love::Exception("failed to create screenshot readback buffer");

			Vulkan::cmdTransitionImageLayout(
				commandBuffers.at(currentFrame),
				backbufferImage,
				swapChainPixelFormat, true,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

			VkBufferImageCopy region{};
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.layerCount = 1;
			region.imageExtent = {
				swapChainExtent.width,
				swapChainExtent.height,
				1
			};

			vkCmdCopyImageToBuffer(
				commandBuffers.at(currentFrame),
				backbufferImage,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				screenshotBuffer,
				1, &region);

			Vulkan::cmdTransitionImageLayout(
				commandBuffers.at(currentFrame),
				backbufferImage,
				swapChainPixelFormat, true,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				fakeBackbuffer == nullptr ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		}
	}

	endRecordingGraphicsCommands();

	if (!imagesInFlight.empty())
	{
		if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
			vkWaitForFences(device, 1, &imagesInFlight.at(imageIndex), VK_TRUE, UINT64_MAX);
		imagesInFlight[imageIndex] = inFlightFences[currentFrame];
	}

	std::array<VkCommandBuffer, 1> submitCommandbuffers = { commandBuffers.at(currentFrame) };

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores.at(currentFrame) };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };

	if (imageRequested)
	{
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		imageRequested = false;
	}

	submitInfo.commandBufferCount = static_cast<uint32_t>(submitCommandbuffers.size());
	submitInfo.pCommandBuffers = submitCommandbuffers.data();

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores.at(currentFrame) };

	VkFence fence = VK_NULL_HANDLE;

	if (submitMode == SUBMIT_PRESENT)
	{
		if (!swapChainImages.empty())
		{
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;
		}

		vkResetFences(device, 1, &inFlightFences[currentFrame]);
		fence = inFlightFences[currentFrame];
	}

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, fence) != VK_SUCCESS)
		throw love::Exception("failed to submit draw command buffer");
	
	if (submitMode == SUBMIT_NOPRESENT || submitMode == SUBMIT_RESTART || screenshotBuffer != VK_NULL_HANDLE)
	{
		vkQueueWaitIdle(graphicsQueue);

		for (auto &callbacks : readbackCallbacks)
		{
			for (const auto &callback : callbacks)
				callback();
			callbacks.clear();
		}

		if (screenshotBuffer != VK_NULL_HANDLE)
		{
			auto imageModule = Module::getInstance<love::image::Image>(M_IMAGE);

			for (int i = 0; i < (int)pendingScreenshotCallbacks.size(); i++)
			{
				const auto &info = pendingScreenshotCallbacks[i];
				image::ImageData *img = nullptr;

				try
				{
					img = imageModule->newImageData(
						swapChainExtent.width,
						swapChainExtent.height,
						PIXELFORMAT_RGBA8_UNORM,
						screenshotAllocationInfo.pMappedData);
				}
				catch (love::Exception &)
				{
					info.callback(&info, nullptr, nullptr);
					for (int j = i + 1; j < (int)pendingScreenshotCallbacks.size(); j++)
					{
						const auto& ninfo = pendingScreenshotCallbacks[j];
						ninfo.callback(&ninfo, nullptr, nullptr);
					}
					vmaDestroyBuffer(vmaAllocator, screenshotBuffer, screenshotAllocation);
					pendingScreenshotCallbacks.clear();
					throw;
				}

				uint8 *screenshot = (uint8*)img->getData();

				if (swapChainImageFormat == VK_FORMAT_B8G8R8A8_UNORM || swapChainImageFormat == VK_FORMAT_B8G8R8A8_SRGB)
				{
					// Convert from BGRA to RGBA and replace alpha with full opacity.
					for (size_t i = 0; i < img->getSize(); i += 4)
					{
						uint8 r = screenshot[i + 2];
						screenshot[i + 2] = screenshot[i + 0];
						screenshot[i + 0] = r;
						screenshot[i + 3] = 255;
					}
				}
				else
				{
					// Replace alpha with full opacity.
					for (size_t i = 0; i < img->getSize(); i += 4)
						screenshot[i + 3] = 255;
				}

				info.callback(&info, img, screenshotCallbackData);
				img->release();
			}

			vmaDestroyBuffer(vmaAllocator, screenshotBuffer, screenshotAllocation);
			pendingScreenshotCallbacks.clear();
		}

		if (submitMode == SUBMIT_RESTART)
			startRecordingGraphicsCommands();
	}
}

void Graphics::present(void *screenshotCallbackdata)
{
	if (!isActive())
		return;

	if (isRenderTargetActive())
		throw love::Exception("present cannot be called while a render target is active.");

	if (!renderPassState.active && renderPassState.windowClearRequested)
		startRenderPass();

	deprecations.draw(this);

	submitGpuCommands(SUBMIT_PRESENT, screenshotCallbackdata);

	VkResult result = VK_SUCCESS;

	if (!swapChainImages.empty())
	{
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &renderFinishedSemaphores.at(currentFrame);
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapChain;
		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(presentQueue, &presentInfo);
	}
	else
	{
		// Presenting without a real swap chain can happen if the window is minimized.
		// Check every frame to see if a proper one can be created, in this situation.
		VkSurfaceCapabilitiesKHR capabilities = {};
		if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities) == VK_SUCCESS)
		{
			VkExtent2D extent = chooseSwapExtent(capabilities);
			if (extent.width > 0 && extent.height > 0)
				swapChainRecreationRequested = true;
		}
	}

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || swapChainRecreationRequested)
	{
		swapChainRecreationRequested = false;
		recreateSwapChain();
	}
	else if (result != VK_SUCCESS)
		throw love::Exception("failed to present swap chain image");

	for (love::graphics::StreamBuffer *buffer : batchedDrawState.vb)
		buffer->nextFrame();
	batchedDrawState.indexBuffer->nextFrame();

	drawCalls = 0;
	renderTargetSwitchCount = 0;
	drawCallsBatched = 0;

	updatePendingReadbacks();
	updateTemporaryResources();

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	realFrameIndex++;

	beginFrame();
}

void Graphics::backbufferChanged(int width, int height, int pixelwidth, int pixelheight, bool backbufferstencil, bool backbufferdepth, int msaa)
{
	if (swapChain != VK_NULL_HANDLE && (pixelwidth != this->pixelWidth || pixelheight != this->pixelHeight || width != this->width || height != this->height
		|| backbufferstencil != this->backbufferHasStencil || backbufferdepth != this->backbufferHasDepth || msaa != requestedMsaa))
		requestSwapchainRecreation();

	this->width = width;
	this->height = height;
	this->pixelWidth = pixelwidth;
	this->pixelHeight = pixelheight;

	this->backbufferHasStencil = backbufferstencil;
	this->backbufferHasDepth = backbufferdepth;
	this->requestedMsaa = msaa;

	if (!isRenderTargetActive())
		resetProjection();

	if (swapChain != VK_NULL_HANDLE)
		msaaSamples = getMsaaCount(requestedMsaa);

	// Don't wait until the next frame starts to recreate the swapchain - doing so
	// will cause a 1 frame delay in the backbuffer size on resize, and it can cause
	// MSAA state to get out of sync for a frame.
	if (swapChainRecreationRequested)
	{
		swapChainRecreationRequested = false;
		submitGpuCommands(SUBMIT_NOPRESENT);
		recreateSwapChain();
		beginSwapChainFrame();
	}
}

bool Graphics::setMode(void *context, int width, int height, int pixelwidth, int pixelheight, bool backbufferstencil, bool backbufferdepth, int msaa)
{
	// Must be called before the swapchain is created.
	backbufferChanged(width, height, pixelwidth, pixelheight, backbufferstencil, backbufferdepth, msaa);

	cleanUpFunctions.clear();
	cleanUpFunctions.resize(MAX_FRAMES_IN_FLIGHT);

	readbackCallbacks.clear();
	readbackCallbacks.resize(MAX_FRAMES_IN_FLIGHT);

	bool createBaseObjects = physicalDevice == VK_NULL_HANDLE;

	createSurface();

	if (createBaseObjects)
	{
		pickPhysicalDevice();
		createLogicalDevice();
		createPipelineCache();
		initVMA();
		initCapabilities();
	}

	msaaSamples = getMsaaCount(requestedMsaa);

	createSwapChain();
	createImageViews();
	createColorResources();
	createDepthResources();
	transitionColorDepthLayouts = true;

	if (createBaseObjects)
	{
		createCommandPool();
		createCommandBuffers();
		createSyncObjects();
	}

	if (localUniformBuffer == nullptr)
		localUniformBuffer.set(new StreamBuffer(this, BUFFERUSAGE_UNIFORM, 1024 * 512 * 1), Acquire::NORETAIN);

	beginFrame();

	if (createBaseObjects)
	{
		if (batchedDrawState.vb[0] == nullptr)
		{
			// Initial sizes that should be good enough for most cases. It will
			// resize to fit if needed, later.
			batchedDrawState.vb[0] = new StreamBuffer(this, BUFFERUSAGE_VERTEX, 1024 * 1024 * 1);
			batchedDrawState.vb[1] = new StreamBuffer(this, BUFFERUSAGE_VERTEX, 256 * 1024 * 1);
			batchedDrawState.indexBuffer = new StreamBuffer(this, BUFFERUSAGE_INDEX, sizeof(uint16) * LOVE_UINT16_MAX);
		}

		if (defaultVertexBuffer == nullptr)
		{
			struct DefaultData
			{
				float floats[4];
				int ints[4];
				float color[4];
			} data;

			data.floats[0] = 0.0f;
			data.floats[1] = 0.0f;
			data.floats[2] = 0.0f;
			data.floats[3] = 1.0f;

			data.ints[0] = 0;
			data.ints[1] = 0;
			data.ints[2] = 0;
			data.ints[3] = 1;

			data.color[0] = 1.0f;
			data.color[1] = 1.0f;
			data.color[2] = 1.0f;
			data.color[3] = 1.0f;

			std::vector<Buffer::DataDeclaration> format = {
				Buffer::DataDeclaration("Floats", DATAFORMAT_FLOAT_VEC4),
				Buffer::DataDeclaration("Ints", DATAFORMAT_INT32_VEC4),
				Buffer::DataDeclaration("Color", DATAFORMAT_FLOAT_VEC4)
			};

			Buffer::Settings settings(BUFFERUSAGEFLAG_VERTEX, BUFFERDATAUSAGE_STATIC);
			defaultVertexBuffer.set(newBuffer(settings, format, &data, sizeof(DefaultData), 1), Acquire::NORETAIN);

			VkBuffer buffer = (VkBuffer)defaultVertexBuffer->getHandle();
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(commandBuffers.at(currentFrame), DEFAULT_VERTEX_BUFFER_BINDING, 1, &buffer, &offset);
		}

		createDefaultShaders();
		Shader::current = Shader::standardShaders[Shader::StandardShader::STANDARD_DEFAULT];
		createQuadIndexBuffer();
		createFanIndexBuffer();

		currentFrame = 0;
	}

	restoreState(states.back());

	Vulkan::resetShaderSwitches();

	created = true;
	drawCalls = 0;
	drawCallsBatched = 0;

	return true;
}

void Graphics::initCapabilities()
{
	capabilities.features[FEATURE_MULTI_RENDER_TARGET_FORMATS] = true;
	capabilities.features[FEATURE_CLAMP_ZERO] = true;
	capabilities.features[FEATURE_CLAMP_ONE] = true;
	capabilities.features[FEATURE_LIGHTEN] = true;
	capabilities.features[FEATURE_FULL_NPOT] = true;
	capabilities.features[FEATURE_PIXEL_SHADER_HIGHP] = true;
	capabilities.features[FEATURE_SHADER_DERIVATIVES] = true;
	capabilities.features[FEATURE_GLSL3] = true;
	capabilities.features[FEATURE_GLSL4] = true;
	capabilities.features[FEATURE_INSTANCING] = true;
	capabilities.features[FEATURE_TEXEL_BUFFER] = true;
	capabilities.features[FEATURE_COPY_TEXTURE_TO_BUFFER] = true;
	capabilities.features[FEATURE_INDIRECT_DRAW] = true;
	static_assert(FEATURE_MAX_ENUM == 13, "Graphics::initCapabilities must be updated when adding a new graphics feature!");

	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);

	capabilities.limits[LIMIT_POINT_SIZE] = properties.limits.pointSizeRange[1];
	capabilities.limits[LIMIT_TEXTURE_SIZE] = properties.limits.maxImageDimension2D;
	capabilities.limits[LIMIT_TEXTURE_LAYERS] = properties.limits.maxImageArrayLayers;
	capabilities.limits[LIMIT_VOLUME_TEXTURE_SIZE] = properties.limits.maxImageDimension3D;
	capabilities.limits[LIMIT_CUBE_TEXTURE_SIZE] = properties.limits.maxImageDimensionCube;
	capabilities.limits[LIMIT_TEXEL_BUFFER_SIZE] = properties.limits.maxTexelBufferElements;
	capabilities.limits[LIMIT_SHADER_STORAGE_BUFFER_SIZE] = properties.limits.maxStorageBufferRange;
	capabilities.limits[LIMIT_THREADGROUPS_X] = properties.limits.maxComputeWorkGroupCount[0];
	capabilities.limits[LIMIT_THREADGROUPS_Y] = properties.limits.maxComputeWorkGroupCount[1];
	capabilities.limits[LIMIT_THREADGROUPS_Z] = properties.limits.maxComputeWorkGroupCount[2];
	capabilities.limits[LIMIT_RENDER_TARGETS] = properties.limits.maxColorAttachments;
	capabilities.limits[LIMIT_TEXTURE_MSAA] = static_cast<double>(getMsaaCount(64));
	capabilities.limits[LIMIT_ANISOTROPY] = properties.limits.maxSamplerAnisotropy;
	static_assert(LIMIT_MAX_ENUM == 13, "Graphics::initCapabilities must be updated when adding a new system limit!");

	capabilities.textureTypes[TEXTURE_2D] = true;
	capabilities.textureTypes[TEXTURE_2D_ARRAY] = true;
	capabilities.textureTypes[TEXTURE_VOLUME] = true;
	capabilities.textureTypes[TEXTURE_CUBE] = true;
}

void Graphics::getAPIStats(int &shaderswitches) const
{
	shaderswitches = static_cast<int>(Vulkan::getNumShaderSwitches());
}

void Graphics::unSetMode()
{
	if (created)
		submitGpuCommands(SUBMIT_NOPRESENT);

	created = false;

	cleanupSwapChain(true);

	if (surface != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR(instance, surface, nullptr);
		surface = VK_NULL_HANDLE;
	}
}

void Graphics::setActive(bool enable)
{
	flushBatchedDraws();
	active = enable;
}

int Graphics::getRequestedBackbufferMSAA() const
{
	return requestedMsaa;
}

int Graphics::getBackbufferMSAA() const
{
	return static_cast<int>(msaaSamples);
}

void Graphics::setFrontFaceWinding(Winding winding)
{
	const auto& currentState = states.back();

	if (currentState.winding == winding)
		return;

	flushBatchedDraws();

	states.back().winding = winding;

	if (optionalDeviceExtensions.extendedDynamicState)
		vkCmdSetFrontFaceEXT(
			commandBuffers.at(currentFrame),
			Vulkan::getFrontFace(winding));
}

void Graphics::setColorMask(ColorChannelMask mask)
{
	flushBatchedDraws();

	states.back().colorMask = mask;
}

void Graphics::setBlendState(const BlendState &blend)
{
	flushBatchedDraws();

	states.back().blend = blend;
}

void Graphics::setPointSize(float size)
{
	if (size != states.back().pointSize)
		flushBatchedDraws();

	states.back().pointSize = size;
}

bool Graphics::usesGLSLES() const
{
	return false;
}

Graphics::RendererInfo Graphics::getRendererInfo() const
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

	Graphics::RendererInfo info;

	info.name = "Vulkan";
	info.device = deviceProperties.deviceName;
	info.vendor = Vulkan::getVendorName(deviceProperties.vendorID);
	info.version = Vulkan::getVulkanApiVersion(deviceProperties.apiVersion);

	return info;
}

void Graphics::draw(const DrawCommand &cmd)
{
	prepareDraw(cmd.attributesID, *cmd.buffers, cmd.texture, cmd.primitiveType, cmd.cullMode);

	if (cmd.indirectBuffer != nullptr)
	{
		vkCmdDrawIndirect(
			commandBuffers.at(currentFrame),
			(VkBuffer) cmd.indirectBuffer->getHandle(),
			cmd.indirectBufferOffset,
			1,
			0);
	}
	else
	{
		vkCmdDraw(
			commandBuffers.at(currentFrame),
			(uint32) cmd.vertexCount,
			(uint32) cmd.instanceCount,
			(uint32) cmd.vertexStart,
			0);
	}

	drawCalls++;
}

void Graphics::draw(const DrawIndexedCommand &cmd)
{
	prepareDraw(cmd.attributesID, *cmd.buffers, cmd.texture, cmd.primitiveType, cmd.cullMode);

	vkCmdBindIndexBuffer(
		commandBuffers.at(currentFrame),
		(VkBuffer) cmd.indexBuffer->getHandle(),
		(VkDeviceSize) cmd.indexBufferOffset,
		Vulkan::getVulkanIndexBufferType(cmd.indexType));

	if (cmd.indirectBuffer != nullptr)
	{
		vkCmdDrawIndexedIndirect(
			commandBuffers.at(currentFrame),
			(VkBuffer) cmd.indirectBuffer->getHandle(),
			cmd.indirectBufferOffset,
			1,
			0);
	}
	else
	{
		vkCmdDrawIndexed(
			commandBuffers.at(currentFrame),
			(uint32) cmd.indexCount,
			(uint32) cmd.instanceCount,
			0,
			0,
			0);
	}

	drawCalls++;
}

void Graphics::drawQuads(int start, int count, VertexAttributesID attributesID, const BufferBindings &buffers, graphics::Texture *texture)
{
	const int MAX_VERTICES_PER_DRAW = LOVE_UINT16_MAX;
	const int MAX_QUADS_PER_DRAW = MAX_VERTICES_PER_DRAW / 4;

	prepareDraw(attributesID, buffers, texture, PRIMITIVE_TRIANGLES, CULL_NONE);

	vkCmdBindIndexBuffer(
		commandBuffers.at(currentFrame),
		(VkBuffer)quadIndexBuffer->getHandle(),
		0,
		Vulkan::getVulkanIndexBufferType(INDEX_UINT16));

	int baseVertex = start * 4;

	for (int quadindex = 0; quadindex < count; quadindex += MAX_QUADS_PER_DRAW)
	{
		int quadcount = std::min(MAX_QUADS_PER_DRAW, count - quadindex);

		vkCmdDrawIndexed(
			commandBuffers.at(currentFrame),
			static_cast<uint32_t>(quadcount * 6),
			1,
			0,
			baseVertex,
			0);
		baseVertex += quadcount * 4;

		drawCalls++;
	}
}

void Graphics::setColor(Colorf c)
{
	c.r = std::min(std::max(c.r, 0.0f), 1.0f);
	c.g = std::min(std::max(c.g, 0.0f), 1.0f);
	c.b = std::min(std::max(c.b, 0.0f), 1.0f);
	c.a = std::min(std::max(c.a, 0.0f), 1.0f);

	states.back().color = c;
}

void Graphics::applyScissor()
{
	VkRect2D scissor{};

	bool win = renderPassState.isWindow;
	scissor.extent.width = win ? swapChainExtent.width : renderPassState.width;
	scissor.extent.height = win ? swapChainExtent.height : renderPassState.height;

	if (states.back().scissor)
	{
		const Rect &rect = states.back().scissorRect;
		double dpiScale = getCurrentDPIScale();

		int minScissorX = (int)(rect.x * dpiScale);
		int minScissorY = (int)(rect.y * dpiScale);

		int maxScissorX = minScissorX + (int)(rect.w * dpiScale) - 1;
		int maxScissorY = minScissorY + (int)(rect.h * dpiScale) - 1;

		// Avoid negative offsets.
		int minX = std::max(scissor.offset.x, minScissorX);
		int minY = std::max(scissor.offset.y, minScissorY);

		int maxX = std::min(scissor.offset.x + (int)scissor.extent.width - 1, maxScissorX);
		int maxY = std::min(scissor.offset.y + (int)scissor.extent.height - 1, maxScissorY);

		if (maxX >= minX && maxY >= minY)
		{
			scissor.offset.x = minX;
			scissor.offset.y = minY;
			scissor.extent.width = (maxX - minX) + 1;
			scissor.extent.height = (maxY - minY) + 1;
		}
		else
		{
			scissor.extent.width = 0;
			scissor.extent.height = 0;
		}
	}

	vkCmdSetScissor(commandBuffers.at(currentFrame), 0, 1, &scissor);
}

void Graphics::setScissor(const Rect &rect)
{
	flushBatchedDraws();

	states.back().scissor = true;
	states.back().scissorRect = rect;

	if (renderPassState.active)
		applyScissor();
}

void Graphics::setScissor()
{
	flushBatchedDraws();

	states.back().scissor = false;

	if (renderPassState.active)
		applyScissor();
}

void Graphics::setStencilState(const StencilState &s)
{
	validateStencilState(s);

	flushBatchedDraws();

	vkCmdSetStencilWriteMask(commandBuffers.at(currentFrame), VK_STENCIL_FRONT_AND_BACK, s.writeMask);
	
	vkCmdSetStencilCompareMask(commandBuffers.at(currentFrame), VK_STENCIL_FRONT_AND_BACK, s.readMask);
	vkCmdSetStencilReference(commandBuffers.at(currentFrame), VK_STENCIL_FRONT_AND_BACK, s.value);

	if (optionalDeviceExtensions.extendedDynamicState)
		vkCmdSetStencilOpEXT(
			commandBuffers.at(currentFrame),
			VK_STENCIL_FRONT_AND_BACK,
			VK_STENCIL_OP_KEEP, Vulkan::getStencilOp(s.action),
			VK_STENCIL_OP_KEEP, Vulkan::getCompareOp(getReversedCompareMode(s.compare)));

	states.back().stencil = s;
}

void Graphics::setDepthMode(CompareMode compare, bool write)
{
	validateDepthState(write);

	flushBatchedDraws();

	if (optionalDeviceExtensions.extendedDynamicState)
	{
		vkCmdSetDepthCompareOpEXT(
			commandBuffers.at(currentFrame), Vulkan::getCompareOp(compare));

		vkCmdSetDepthWriteEnableEXT(
			commandBuffers.at(currentFrame), Vulkan::getBool(write));
	}

	states.back().depthTest = compare;
	states.back().depthWrite = write;
}

void Graphics::setWireframe(bool enable)
{
	flushBatchedDraws();

	states.back().wireframe = enable;
}

bool Graphics::isPixelFormatSupported(PixelFormat format, uint32 usage)
{
	format = getSizedFormat(format);

	switch (format)
	{
	case PIXELFORMAT_PVR1_RGB2_UNORM:
	case PIXELFORMAT_PVR1_RGB2_sRGB:
	case PIXELFORMAT_PVR1_RGB4_UNORM:
	case PIXELFORMAT_PVR1_RGB4_sRGB:
	case PIXELFORMAT_PVR1_RGBA2_UNORM:
	case PIXELFORMAT_PVR1_RGBA2_sRGB:
	case PIXELFORMAT_PVR1_RGBA4_UNORM:
	case PIXELFORMAT_PVR1_RGBA4_sRGB:
		// Lets not support these in Vulkan - they're deprecated.
		return false;
	default:
		break;
	}

	auto vulkanFormat = Vulkan::getTextureFormat(format);

	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, vulkanFormat.internalFormat, &formatProperties);

	VkFormatFeatureFlags featureFlags = formatProperties.optimalTilingFeatures;
	VkImageUsageFlags usageFlags = 0;

	if (!featureFlags)
		return false;

	if (usage & PIXELFORMATUSAGEFLAGS_SAMPLE)
	{
		usageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
		if (!(featureFlags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
			return false;
	}

	if (usage & PIXELFORMATUSAGEFLAGS_LINEAR)
	{
		if (!(featureFlags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
			return false;
	}

	if (usage & PIXELFORMATUSAGEFLAGS_RENDERTARGET)
	{
		if (isPixelFormatDepth(format) || isPixelFormatDepthStencil(format))
		{
			usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			if (!(featureFlags & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
				return false;
		}
		else
		{
			usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			if (!(featureFlags & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT))
				return false;
		}
	}

	if (usage & PIXELFORMATUSAGEFLAGS_BLEND)
	{
		if (!(featureFlags & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT))
			return false;
	}

	if (usage & PIXELFORMATUSAGEFLAGS_COMPUTEWRITE)
	{
		usageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
		if (!(featureFlags & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT))
			return false;
	}

	if (usage & PIXELFORMATUSAGEFLAGS_MSAA)
	{
		VkImageFormatProperties properties;

		if (vkGetPhysicalDeviceImageFormatProperties(physicalDevice, vulkanFormat.internalFormat, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, usageFlags, 0, &properties) != VK_SUCCESS)
			return false;

		if (static_cast<uint32_t>(properties.sampleCounts) == 1)
			return false;
	}

	return true;
}

Renderer Graphics::getRenderer() const
{
	return RENDERER_VULKAN;
}

graphics::GraphicsReadback *Graphics::newReadbackInternal(ReadbackMethod method, love::graphics::Buffer *buffer, size_t offset, size_t size, data::ByteData *dest, size_t destoffset)
{
	return new GraphicsReadback(this, method, buffer, offset, size, dest, destoffset);
}

graphics::GraphicsReadback *Graphics::newReadbackInternal(ReadbackMethod method, love::graphics::Texture *texture, int slice, int mipmap, const Rect &rect, image::ImageData *dest, int destx, int desty)
{
	return new GraphicsReadback(this, method, texture, slice, mipmap, rect, dest, destx, desty);
}

graphics::ShaderStage *Graphics::newShaderStageInternal(ShaderStageType stage, const std::string &cachekey, const std::string &source, bool gles)
{
	return new ShaderStage(this, stage, source, gles, cachekey);
}

graphics::Shader *Graphics::newShaderInternal(StrongRef<love::graphics::ShaderStage> stages[SHADERSTAGE_MAX_ENUM], const Shader::CompileOptions &options)
{
	return new Shader(stages, options);
}

graphics::StreamBuffer *Graphics::newStreamBuffer(BufferUsage type, size_t size)
{
	return new StreamBuffer(this, type, size);
}

static bool computeDispatchBarrierFlags(Shader *shader, VkAccessFlags &dstAccessFlags, VkPipelineStageFlags &dstStageFlags)
{
	for (const auto &info : shader->getActiveTextureInfo())
	{
		if ((info.access & Shader::ACCESS_WRITE) == 0)
			continue;

		if (info.texture == nullptr)
			return false;

		auto tex = (Texture *) info.texture;

		// All writable images use the GENERAL layout.
		// TODO: this is pretty messy.
		bool depthStencil  = isPixelFormatDepthStencil(tex->getPixelFormat());
		Vulkan::addImageLayoutTransitionOptions(false, tex->isRenderTarget(), depthStencil, VK_IMAGE_LAYOUT_GENERAL, dstAccessFlags, dstStageFlags);
	}

	for (const auto &info : shader->getActiveStorageBufferInfo())
	{
		if ((info.access & Shader::ACCESS_WRITE) == 0)
			continue;

		if (info.buffer == nullptr)
			return false;

		auto b = (Buffer *) info.buffer;
		dstAccessFlags |= b->getBarrierDstAccessFlags();
		dstStageFlags |= b->getBarrierDstStageFlags();
	}

	return true;
}

bool Graphics::dispatch(love::graphics::Shader *shader, int x, int y, int z)
{
	auto computeShader = (Shader *) shader;

	VkMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	VkPipelineStageFlags dstStageMask = 0;
	if (!computeDispatchBarrierFlags(computeShader, barrier.dstAccessMask, dstStageMask))
		return false;

	usedShadersInFrame.insert(computeShader);

	if (renderPassState.active)
		endRenderPass();

	vkCmdBindPipeline(commandBuffers.at(currentFrame), VK_PIPELINE_BIND_POINT_COMPUTE, computeShader->getComputePipeline());

	computeShader->cmdPushDescriptorSets(commandBuffers.at(currentFrame), VK_PIPELINE_BIND_POINT_COMPUTE);

	vkCmdDispatch(commandBuffers.at(currentFrame), (uint32) x, (uint32) y, (uint32) z);

	// Image layout transitions aren't needed, every writable image will be in the GENERAL layout.
	if (barrier.dstAccessMask != 0 || dstStageMask != 0)
		vkCmdPipelineBarrier(commandBuffers.at(currentFrame), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, dstStageMask, 0, 1, &barrier, 0, nullptr, 0, nullptr);

	return true;
}

bool Graphics::dispatch(love::graphics::Shader *shader, love::graphics::Buffer *indirectargs, size_t argsoffset)
{
	auto computeShader = (Shader *) shader;

	VkMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	VkPipelineStageFlags dstStageMask = 0;
	if (!computeDispatchBarrierFlags(computeShader, barrier.dstAccessMask, dstStageMask))
		return false;

	usedShadersInFrame.insert(computeShader);

	if (renderPassState.active)
		endRenderPass();

	vkCmdBindPipeline(commandBuffers.at(currentFrame), VK_PIPELINE_BIND_POINT_COMPUTE, computeShader->getComputePipeline());

	computeShader->cmdPushDescriptorSets(commandBuffers.at(currentFrame), VK_PIPELINE_BIND_POINT_COMPUTE);

	vkCmdDispatchIndirect(commandBuffers.at(currentFrame), (VkBuffer) indirectargs->getHandle(), argsoffset);

	// Image layout transitions aren't needed, every writable image will be in the GENERAL layout.
	if (barrier.dstAccessMask != 0 || dstStageMask != 0)
		vkCmdPipelineBarrier(commandBuffers.at(currentFrame), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, dstStageMask, 0, 1, &barrier, 0, nullptr, 0, nullptr);

	return true;
}

void Graphics::setRenderTargetsInternal(const RenderTargets &rts, int pixelw, int pixelh, bool hasSRGBtexture)
{
	if (renderPassState.active)
		endRenderPass();

	bool isWindow = rts.getFirstTarget().texture == nullptr;
	if (isWindow)
		setDefaultRenderPass();
	else
		setRenderPass(rts, pixelw, pixelh);
}

// END IMPLEMENTATION OVERRIDDEN FUNCTIONS

void Graphics::initDynamicState()
{
	vkCmdSetStencilWriteMask(commandBuffers.at(currentFrame), VK_STENCIL_FRONT_AND_BACK, states.back().stencil.writeMask);
	vkCmdSetStencilCompareMask(commandBuffers.at(currentFrame), VK_STENCIL_FRONT_AND_BACK, states.back().stencil.readMask);
	vkCmdSetStencilReference(commandBuffers.at(currentFrame), VK_STENCIL_FRONT_AND_BACK, states.back().stencil.value);

	if (optionalDeviceExtensions.extendedDynamicState)
	{
		 vkCmdSetStencilOpEXT(
			commandBuffers.at(currentFrame),
			VK_STENCIL_FRONT_AND_BACK,
			VK_STENCIL_OP_KEEP, Vulkan::getStencilOp(states.back().stencil.action),
			VK_STENCIL_OP_KEEP, Vulkan::getCompareOp(getReversedCompareMode(states.back().stencil.compare)));

		vkCmdSetDepthCompareOpEXT(
			commandBuffers.at(currentFrame), Vulkan::getCompareOp(states.back().depthTest));

		vkCmdSetDepthWriteEnableEXT(
			commandBuffers.at(currentFrame), Vulkan::getBool(states.back().depthWrite));

		vkCmdSetFrontFaceEXT(
			commandBuffers.at(currentFrame), Vulkan::getFrontFace(states.back().winding));
	}
}

void Graphics::beginSwapChainFrame()
{
	if (swapChain != VK_NULL_HANDLE)
	{
		while (true)
		{
			VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
			if (result == VK_ERROR_OUT_OF_DATE_KHR)
			{
				recreateSwapChain();
				continue;
			}
			else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
				throw love::Exception("failed to acquire swap chain image");

			break;
		}

		imageRequested = true;
	}
	else
	{
		imageRequested = false;
	}

	startRecordingGraphicsCommands();

	if (!swapChainImages.empty())
	{
		Vulkan::cmdTransitionImageLayout(
			commandBuffers.at(currentFrame),
			swapChainImages[imageIndex],
			swapChainPixelFormat, true,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	}

	if (transitionColorDepthLayouts)
	{
		if (depthImage)
			Vulkan::cmdTransitionImageLayout(
				commandBuffers.at(currentFrame),
				depthImage,
				depthStencilPixelFormat, true,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		if (colorImage)
			Vulkan::cmdTransitionImageLayout(
				commandBuffers.at(currentFrame),
				colorImage,
				swapChainPixelFormat, true,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		transitionColorDepthLayouts = false;
	}
}

void Graphics::beginFrame()
{
	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	for (auto &readbackCallback : readbackCallbacks.at(currentFrame))
		readbackCallback();
	readbackCallbacks.at(currentFrame).clear();

	for (auto &cleanUpFn : cleanUpFunctions.at(currentFrame))
		cleanUpFn();
	cleanUpFunctions.at(currentFrame).clear();

	beginSwapChainFrame();

	Vulkan::resetShaderSwitches();

	for (const auto &shader : usedShadersInFrame)
		shader->newFrame(realFrameIndex);
	usedShadersInFrame.clear();

	localUniformBuffer->nextFrame();
}

void Graphics::startRecordingGraphicsCommands()
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	VkResult result = vkBeginCommandBuffer(commandBuffers.at(currentFrame), &beginInfo);
	if (result != VK_SUCCESS)
		throw love::Exception("Failed to begin recording Vulkan command buffer: %s", Vulkan::getErrorString(result));

	initDynamicState();

	// This must be done after vkBeginCommandBuffer (since newTexture needs an
	// active command buffer for layout transitions), and before setDefaultRenderPass
	// (since that tries to use fakeBackbuffer).
	if (swapChainImages.empty() && fakeBackbuffer == nullptr)
	{
		Texture::Settings settings;
		settings.format = swapChainPixelFormat;
		settings.width = swapChainExtent.width;
		settings.height = swapChainExtent.height;
		settings.renderTarget = true;
		settings.readable.set(false);
		fakeBackbuffer.set((Texture*)newTexture(settings, nullptr), Acquire::NORETAIN);
	}

	setDefaultRenderPass();

	if (defaultVertexBuffer)
	{
		VkBuffer buffer = (VkBuffer)defaultVertexBuffer->getHandle();
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(commandBuffers.at(currentFrame), DEFAULT_VERTEX_BUFFER_BINDING, 1, &buffer, &offset);
	}
}

void Graphics::endRecordingGraphicsCommands()
{
	if (renderPassState.active)
		endRenderPass();

	VkResult result = vkEndCommandBuffer(commandBuffers.at(currentFrame));
	if (result != VK_SUCCESS)
		throw love::Exception("Failed to record Vulkan command buffer: %s", Vulkan::getErrorString(result));
}

VkCommandBuffer Graphics::getCommandBufferForDataTransfer()
{
	if (renderPassState.active)
		endRenderPass();

	return commandBuffers.at(currentFrame);
}

void Graphics::queueCleanUp(std::function<void()> cleanUp)
{
	cleanUpFunctions.at(currentFrame).push_back(cleanUp);
}

void Graphics::addReadbackCallback(std::function<void()> callback)
{
	readbackCallbacks.at(currentFrame).push_back(callback);
}

graphics::Shader::BuiltinUniformData Graphics::getCurrentBuiltinUniformData()
{
	love::graphics::Shader::BuiltinUniformData data;

	data.transformMatrix = getTransform();
	data.projectionMatrix = getDeviceProjection();

	data.scaleParams.x = (float) getCurrentDPIScale();
	data.scaleParams.y = getPointSize();

	// Flip y to convert input y-up [-1, 1] to vulkan's y-down [-1, 1].
	// Convert input z [-1, 1] to vulkan [0, 1].
	uint32 flags = Shader::CLIP_TRANSFORM_FLIP_Y | Shader::CLIP_TRANSFORM_Z_NEG1_1_TO_0_1;
	data.clipSpaceParams = Shader::computeClipSpaceParams(flags);

	const auto &rt = states.back().renderTargets.getFirstTarget();
	if (rt.texture != nullptr)
	{
		data.screenSizeParams.x = rt.texture->getPixelWidth(rt.mipmap);
		data.screenSizeParams.y = rt.texture->getPixelHeight(rt.mipmap);
	}
	else
	{
		data.screenSizeParams.x = getPixelWidth();
		data.screenSizeParams.y = getPixelHeight();
	}

	data.screenSizeParams.z = 1.0f;
	data.screenSizeParams.w = 0.0f;

	data.constantColor = getColor();
	gammaCorrectColor(data.constantColor);

	return data;
}

const OptionalDeviceExtensions &Graphics::getEnabledOptionalDeviceExtensions() const
{
	return optionalDeviceExtensions;
}

const OptionalInstanceExtensions &Graphics::getEnabledOptionalInstanceExtensions() const
{
	return optionalInstanceExtensions;
}

bool Graphics::checkValidationSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char *layerName : validationLayers)
	{
		bool layerFound = false;

		for (const auto &layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
			return false;
	}

	return true;
}

void Graphics::pickPhysicalDevice()
{
	struct DeviceRating
	{
		VkPhysicalDevice device;
		size_t deviceIndex;
		int rating;
	};

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0)
		throw love::Exception("failed to find GPUs with Vulkan support");

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	std::vector<DeviceRating> candidates;

	for (size_t i = 0; i < devices.size(); i++)
	{
		DeviceRating r = {};
		r.device = devices[i];
		r.deviceIndex = i;
		r.rating = rateDeviceSuitability(devices[i], true);
		candidates.push_back(r);
	}

	std::sort(candidates.begin(), candidates.end(), [](const DeviceRating &a, const DeviceRating &b) -> bool
	{
		if (a.rating != b.rating)
			return a.rating > b.rating;
		return a.deviceIndex < b.deviceIndex;
	});

	if (!candidates.empty() && candidates[0].rating > 0)
		physicalDevice = candidates[0].device;
	else
		throw love::Exception("Vulkan: failed to find a suitable GPU.");

	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);
	minUniformBufferOffsetAlignment = properties.limits.minUniformBufferOffsetAlignment;
	deviceApiVersion = properties.apiVersion;

	depthStencilFormat = findDepthFormat();
	switch (depthStencilFormat)
	{
	case VK_FORMAT_D32_SFLOAT_S8_UINT:
		depthStencilPixelFormat = PIXELFORMAT_DEPTH32_FLOAT_STENCIL8;
		break;
	case VK_FORMAT_D24_UNORM_S8_UINT:
		depthStencilPixelFormat = PIXELFORMAT_DEPTH24_UNORM_STENCIL8;
		break;
	default:
		throw love::Exception("Failed to convert vulkan depth/stencil swapchain pixel format %d to love PixelFormat.", depthStencilFormat);
		break;
	}
}

// if the score is nonzero then the device is suitable.
// A higher rating means generally better performance
// if the score is 0 the device is unsuitable
int Graphics::rateDeviceSuitability(VkPhysicalDevice device, bool querySwapChain)
{
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	bool hasMSFTLayeredDriver = false;
	for (const auto &extension : availableExtensions)
	{
		if (strcmp(extension.extensionName, VK_MSFT_LAYERED_DRIVER_EXTENSION_NAME) == 0)
		{
			hasMSFTLayeredDriver = true;
			break;
		}
	}

	VkPhysicalDeviceProperties2 deviceProperties2{};
	deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;

	VkPhysicalDeviceLayeredDriverPropertiesMSFT layeredDriverProperties{};
	layeredDriverProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_DRIVER_PROPERTIES_MSFT;

	if (deviceProperties.apiVersion >= VK_API_VERSION_1_1)
	{
		if (hasMSFTLayeredDriver)
			deviceProperties2.pNext = &layeredDriverProperties;

		vkGetPhysicalDeviceProperties2(device, &deviceProperties2);
	}

	int score = 2;

	// optional

	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		score += isLowPowerPreferred() ? 100 : 1000;
	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
		score += isLowPowerPreferred() ? 1000 : 100;
	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
		score += 10;

	// Reduce the score if this is something like Vulkan-on-D3D12 rather than a native driver.
	if (hasMSFTLayeredDriver && layeredDriverProperties.underlyingAPI != VK_LAYERED_DRIVER_UNDERLYING_API_NONE_MSFT)
		score /= 2;

	// definitely needed

	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU)
	{
		if (!SDL_GetHintBoolean("LOVE_GRAPHICS_VULKAN_ALLOW_SOFTWARE", false))
			score = 0;
	}

	QueueFamilyIndices indices = findQueueFamilies(device);
	if (!indices.isComplete() && (querySwapChain || !indices.graphicsFamily.hasValue))
		score = 0;

	std::set<std::string> missingExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto &extension : availableExtensions)
		missingExtensions.erase(extension.extensionName);

	if (!missingExtensions.empty())
		score = 0;

	if (missingExtensions.empty() && querySwapChain)
	{
		auto swapChainSupport = querySwapChainSupport(device);
		bool swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		if (!swapChainAdequate)
			score = 0;
	}

	if (!deviceFeatures.samplerAnisotropy)
		score = 0;

	if (!deviceFeatures.fillModeNonSolid)
		score = 0;

	return score;
}

QueueFamilyIndices Graphics::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto &queueFamily : queueFamilies)
	{
		if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT))
			indices.graphicsFamily = i;

		if (surface != VK_NULL_HANDLE)
		{
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

			if (presentSupport)
				indices.presentFamily = i;
		}

		if (indices.isComplete())
			break;

		i++;
	}

	return indices;
}

static void findOptionalDeviceExtensions(VkPhysicalDevice physicalDevice, OptionalDeviceExtensions &optionalDeviceExtensions)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

	for (const auto &extension : availableExtensions)
	{
		if (strcmp(extension.extensionName, VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME) == 0)
			optionalDeviceExtensions.extendedDynamicState = true;
		if (strcmp(extension.extensionName, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME) == 0)
			optionalDeviceExtensions.memoryRequirements2 = true;
		if (strcmp(extension.extensionName, VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME) == 0)
			optionalDeviceExtensions.dedicatedAllocation = true;
		if (strcmp(extension.extensionName, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME) == 0)
			optionalDeviceExtensions.memoryBudget = true;
		if (strcmp(extension.extensionName, VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME) == 0)
			optionalDeviceExtensions.shaderFloatControls = true;
		if (strcmp(extension.extensionName, VK_KHR_SPIRV_1_4_EXTENSION_NAME) == 0)
			optionalDeviceExtensions.spirv14 = true;
	}
}

void Graphics::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {
		indices.graphicsFamily.value,
		indices.presentFamily.value
	};

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	findOptionalDeviceExtensions(physicalDevice, optionalDeviceExtensions);

	// sanity check for dependencies.

	if (optionalDeviceExtensions.extendedDynamicState && !optionalInstanceExtensions.physicalDeviceProperties2)
		optionalDeviceExtensions.extendedDynamicState = false;
	if (optionalDeviceExtensions.dedicatedAllocation && !optionalDeviceExtensions.memoryRequirements2)
		optionalDeviceExtensions.dedicatedAllocation = false;
	if (optionalDeviceExtensions.memoryBudget && !optionalInstanceExtensions.physicalDeviceProperties2)
		optionalDeviceExtensions.memoryBudget = false;
	if (optionalDeviceExtensions.spirv14 && !optionalDeviceExtensions.shaderFloatControls)
		optionalDeviceExtensions.spirv14 = false;
	if (optionalDeviceExtensions.spirv14 && deviceApiVersion < VK_API_VERSION_1_1)
		optionalDeviceExtensions.spirv14 = false;

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.fillModeNonSolid = VK_TRUE;

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;

	std::vector<const char*> enabledExtensions(deviceExtensions.begin(), deviceExtensions.end());
	if (optionalDeviceExtensions.extendedDynamicState)
		enabledExtensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
	if (optionalDeviceExtensions.memoryRequirements2)
		enabledExtensions.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
	if (optionalDeviceExtensions.dedicatedAllocation)
		enabledExtensions.push_back(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
	if (optionalDeviceExtensions.memoryBudget)
		enabledExtensions.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
	if (optionalDeviceExtensions.shaderFloatControls)
		enabledExtensions.push_back(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
	if (optionalDeviceExtensions.spirv14)
		enabledExtensions.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
	if (deviceApiVersion >= VK_API_VERSION_1_1)
		enabledExtensions.push_back(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME);

	createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
	createInfo.ppEnabledExtensionNames = enabledExtensions.data();

	if (isDebugEnabled())
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}

	VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures{};
	extendedDynamicStateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
	extendedDynamicStateFeatures.extendedDynamicState = VK_TRUE;
	extendedDynamicStateFeatures.pNext = nullptr;

	if (optionalDeviceExtensions.extendedDynamicState)
		createInfo.pNext = &extendedDynamicStateFeatures;

	VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
	if (result != VK_SUCCESS)
		throw love::Exception("Failed to create Vulkan logical device: %s", Vulkan::getErrorString(result));

	volkLoadDevice(device);

	vkGetDeviceQueue(device, indices.graphicsFamily.value, 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value, 0, &presentQueue);
}

void Graphics::createPipelineCache()
{
	VkPipelineCacheCreateInfo cacheInfo{};
	cacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

	VkResult result = vkCreatePipelineCache(device, &cacheInfo, nullptr, &pipelineCache);
	if (result != VK_SUCCESS)
		throw love::Exception("Could not create Vulkan pipeline cache: %s", Vulkan::getErrorString(result));
}

void Graphics::initVMA()
{
	VmaAllocatorCreateInfo allocatorCreateInfo = {};
	allocatorCreateInfo.vulkanApiVersion = deviceApiVersion;
	allocatorCreateInfo.physicalDevice = physicalDevice;
	allocatorCreateInfo.device = device;
	allocatorCreateInfo.instance = instance;

	// Default of 256 MB is a little too wasteful for most love games.
	// TODO: Tune this more.
	allocatorCreateInfo.preferredLargeHeapBlockSize = 128 * 1024 * 1024;

	VmaVulkanFunctions vulkanFunctions{};

	vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
	vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
	vulkanFunctions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
	vulkanFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
	vulkanFunctions.vkAllocateMemory = vkAllocateMemory;
	vulkanFunctions.vkFreeMemory = vkFreeMemory;
	vulkanFunctions.vkMapMemory = vkMapMemory;
	vulkanFunctions.vkUnmapMemory = vkUnmapMemory;
	vulkanFunctions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
	vulkanFunctions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
	vulkanFunctions.vkBindBufferMemory = vkBindBufferMemory;
	vulkanFunctions.vkBindImageMemory = vkBindImageMemory;
	vulkanFunctions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
	vulkanFunctions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
	vulkanFunctions.vkCreateBuffer = vkCreateBuffer;
	vulkanFunctions.vkCreateImage = vkCreateImage;
	vulkanFunctions.vkDestroyBuffer = vkDestroyBuffer;
	vulkanFunctions.vkDestroyImage = vkDestroyImage;
	vulkanFunctions.vkCmdCopyBuffer = vkCmdCopyBuffer;

	vulkanFunctions.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR;
	vulkanFunctions.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR;
	vulkanFunctions.vkBindBufferMemory2KHR = vkBindBufferMemory2KHR;
	vulkanFunctions.vkBindImageMemory2KHR = vkBindImageMemory2KHR;
	vulkanFunctions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2KHR;
	vulkanFunctions.vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements;
	vulkanFunctions.vkGetDeviceImageMemoryRequirements = vkGetDeviceImageMemoryRequirements;

	allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

	allocatorCreateInfo.flags |= VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT;
	if (optionalDeviceExtensions.dedicatedAllocation)
		allocatorCreateInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
	if (optionalDeviceExtensions.memoryBudget)
		allocatorCreateInfo.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;

	VkResult result = vmaCreateAllocator(&allocatorCreateInfo, &vmaAllocator);
	if (result != VK_SUCCESS)
		throw love::Exception("Failed to create Vulkan VMA allocator: %s", Vulkan::getErrorString(result));
}

void Graphics::createSurface()
{
	auto window = Module::getInstance<love::window::Window>(M_WINDOW);
	const void *handle = window->getHandle();
	if (!SDL_Vulkan_CreateSurface((SDL_Window*)handle, instance, nullptr, &surface))
		throw love::Exception("Failed to create Vulkan window surface: %s", SDL_GetError());
}

SwapChainSupportDetails Graphics::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

void Graphics::createSwapChain()
{
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	if (extent.width > 0 && extent.height > 0)
	{
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
			imageCount = swapChainSupport.capabilities.maxImageCount;

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value, indices.presentFamily.value };

		if (indices.graphicsFamily.value != indices.presentFamily.value)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}

		createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		createInfo.compositeAlpha = chooseCompositeAlpha(swapChainSupport.capabilities);
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = swapChain;

		VkSwapchainKHR newSwapChain = VK_NULL_HANDLE;
		VkResult result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &newSwapChain);

		if (result != VK_SUCCESS)
			throw love::Exception("Failed to create Vulkan swap chain: %s", Vulkan::getErrorString(result));

		if (swapChain != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(device, swapChain, nullptr);
			swapChain = VK_NULL_HANDLE;
		}

		swapChain = newSwapChain;

		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
	}
	else
	{
		if (swapChain != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(device, swapChain, nullptr);
			swapChain = VK_NULL_HANDLE;
		}

		// Use a fake backbuffer. Creation is deferred until startRecordingGraphicsCommands
		// because newTexture needs an active command buffer to do its initial
		// layout transitions.
		swapChainImages.clear();
		extent.width = std::max(1, pixelWidth);
		extent.height = std::max(1, pixelHeight);

		if (isGammaCorrect())
			surfaceFormat.format = VK_FORMAT_R8G8B8A8_SRGB;
		else
			surfaceFormat.format = VK_FORMAT_R8G8B8A8_UNORM;
	}

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;

	switch (swapChainImageFormat)
	{
	case VK_FORMAT_B8G8R8A8_SRGB:
		swapChainPixelFormat = PIXELFORMAT_BGRA8_sRGB;
		break;
	case VK_FORMAT_B8G8R8A8_UNORM:
		swapChainPixelFormat = PIXELFORMAT_BGRA8_UNORM;
		break;
	case VK_FORMAT_R8G8B8A8_SRGB:
		swapChainPixelFormat = PIXELFORMAT_RGBA8_sRGB;
		break;
	case VK_FORMAT_R8G8B8A8_UNORM:
		swapChainPixelFormat = PIXELFORMAT_RGBA8_UNORM;
		break;
	default:
		throw love::Exception("Failed to convert vulkan depth/stencil swapchain image format %d to love PixelFormat.", swapChainImageFormat);
		break;
	}
}

VkSurfaceFormatKHR Graphics::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
	std::vector<VkFormat> formatOrder;

	// TODO: turn off GammaCorrect if a sRGB format can't be found?
	// TODO: does every platform have these formats?
	if (isGammaCorrect())
	{
		formatOrder = {
			VK_FORMAT_B8G8R8A8_SRGB,
			VK_FORMAT_R8G8B8A8_SRGB,
		};
	}
	else
	{
		formatOrder = {
			VK_FORMAT_B8G8R8A8_UNORM,
			VK_FORMAT_R8G8B8A8_SNORM,
		};
	}

	for (const auto format : formatOrder)
	{
		for (const auto &availableFormat : availableFormats)
		{
			if (availableFormat.format == format && availableFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
				return availableFormat;
		}
	}
	
	return availableFormats[0];
}

VkPresentModeKHR Graphics::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
	const auto begin = availablePresentModes.begin();
	const auto end = availablePresentModes.end();

	switch (vsync)
	{
	case -1:
		if (std::find(begin, end, VK_PRESENT_MODE_FIFO_RELAXED_KHR) != end)
			return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
		else
			return VK_PRESENT_MODE_FIFO_KHR;
	case 0:
		// Mailbox mode might be better than immediate mode for a lot of people.
		// But on at least some systems it acts as if vsync is enabled
		// https://github.com/love2d/love/issues/1852
		// TODO: is that a bug in love's code or the graphics driver / compositor?
		// Should love expose mailbox mode in an API to users in some manner,
		// instead of trying to guess what to do?
		if (std::find(begin, end, VK_PRESENT_MODE_IMMEDIATE_KHR) != end)
			return VK_PRESENT_MODE_IMMEDIATE_KHR;
		else if (std::find(begin, end, VK_PRESENT_MODE_MAILBOX_KHR) != end)
			return VK_PRESENT_MODE_MAILBOX_KHR;
		else
			return VK_PRESENT_MODE_FIFO_KHR;
	default:
		// TODO: support for swap interval = 2, etc?
		return VK_PRESENT_MODE_FIFO_KHR;
	}
}

static uint32_t clampuint32_t(uint32_t value, uint32_t min, uint32_t max)
{
	if (value < min)
		return min;

	if (value > max)
		return max;

	return value;
}

VkExtent2D Graphics::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
		return capabilities.currentExtent;
	else
	{
		VkExtent2D actualExtent = {
			static_cast<uint32_t>(pixelWidth),
			static_cast<uint32_t>(pixelHeight)
		};

		actualExtent.width = clampuint32_t(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = clampuint32_t(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

VkCompositeAlphaFlagBitsKHR Graphics::chooseCompositeAlpha(const VkSurfaceCapabilitiesKHR &capabilities)
{
	if (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
		return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	else if (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR)
		return VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
	else if (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR)
		return VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
	else if (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR)
		return VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
	else
		throw love::Exception("failed to find composite alpha");
}

void Graphics::createImageViews()
{
	swapChainImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages.at(i);
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VkResult result = vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews.at(i));
		if (result != VK_SUCCESS)
			throw love::Exception("Failed to create Vulkan swap chain image views: %s", Vulkan::getErrorString(result));
	}
}

VkFramebuffer Graphics::createFramebuffer(FramebufferConfiguration &configuration)
{
	std::vector<VkImageView> attachments;

	for (const auto &colorView : configuration.colorViews)
		attachments.push_back(colorView);

	if (configuration.staticData.depthView)
		attachments.push_back(configuration.staticData.depthView);

	// Resolve attachments after everything else to match createRenderPass.
	for (const auto &colorResolveView : configuration.colorResolveViews)
		attachments.push_back(colorResolveView);

	VkFramebufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass = configuration.staticData.renderPass;
	createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	createInfo.pAttachments = attachments.data();
	createInfo.width = configuration.staticData.width;
	createInfo.height = configuration.staticData.height;
	createInfo.layers = 1;

	VkFramebuffer frameBuffer;
	VkResult result = vkCreateFramebuffer(device, &createInfo, nullptr, &frameBuffer);
	if (result != VK_SUCCESS)
		throw love::Exception("Failed to create Vulkan framebuffer: %s", Vulkan::getErrorString(result));
	return frameBuffer;
}

VkFramebuffer Graphics::getFramebuffer(FramebufferConfiguration &configuration)
{
	VkFramebuffer framebuffer;

	auto it = framebuffers.find(configuration);
	if (it != framebuffers.end())
		framebuffer = it->second;
	else
	{
		framebuffer = createFramebuffer(configuration);
		framebuffers[configuration] = framebuffer;
	}

	framebufferUsages[framebuffer] = true;

	return framebuffer;
}

void Graphics::cleanupFramebuffers(VkImageView imageView, PixelFormat format)
{
	bool depthstencil = isPixelFormatDepthStencil(format);

	for (auto it = framebuffers.begin(); it != framebuffers.end();)
	{
		bool foundView = false;

		if (depthstencil)
		{
			if (it->first.staticData.depthView == imageView)
				foundView = true;
		}
		else
		{
			for (VkImageView view : it->first.colorViews)
			{
				if (view == imageView)
				{
					foundView = true;
					break;
				}
			}
			if (!foundView)
			{
				for (VkImageView view : it->first.colorResolveViews)
				{
					if (view == imageView)
					{
						foundView = true;
						break;
					}
				}
			}
		}

		if (foundView)
		{
			vkDestroyFramebuffer(device, it->second, nullptr);
			it = framebuffers.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void Graphics::createDefaultShaders()
{
	for (int i = 0; i < Shader::STANDARD_MAX_ENUM; i++)
	{
		auto stype = (Shader::StandardShader)i;

		if (!Shader::standardShaders[i])
		{
			std::vector<std::string> stages;
			stages.push_back(Shader::getDefaultCode(stype, SHADERSTAGE_VERTEX));
			stages.push_back(Shader::getDefaultCode(stype, SHADERSTAGE_PIXEL));
			Shader::standardShaders[i] = newShader(stages, {});
		}
	}
}

VkRenderPass Graphics::createRenderPass(RenderPassConfiguration &configuration)
{
	VkSubpassDescription subPass{};
	subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	VkSubpassDependency beginDependency{};
	beginDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	beginDependency.dstSubpass = 0;
	beginDependency.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
	beginDependency.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	beginDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		| VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	beginDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
		| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkSubpassDependency endDependency{};
	endDependency.srcSubpass = 0;
	endDependency.dstSubpass = VK_SUBPASS_EXTERNAL;
	endDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	endDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
		| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	endDependency.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
	endDependency.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

	std::vector<VkAttachmentDescription> attachments;
	std::vector<VkAttachmentReference> colorAttachmentRefs;
	std::vector<VkAttachmentReference> colorResolveAttachmentRefs;

	uint32_t attachment = 0;
	for (const auto &colorAttachment : configuration.colorAttachments)
	{
		VkAttachmentReference reference{};
		reference.attachment = attachment++;
		reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachmentRefs.push_back(reference);

		VkAttachmentDescription colorDescription{};
		colorDescription.format = colorAttachment.format;
		colorDescription.samples = colorAttachment.msaaSamples;
		colorDescription.loadOp = colorAttachment.loadOp;
		colorDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		if (colorAttachment.msaaSamples > 1)
		{
			colorDescription.initialLayout = colorAttachment.msaaLayout;
			colorDescription.finalLayout = colorAttachment.msaaLayout;
		}
		else
		{
			colorDescription.initialLayout = colorAttachment.layout;
			colorDescription.finalLayout = colorAttachment.layout;
		}
		attachments.push_back(colorDescription);

		// I had a TODO here, but I don't remember why...
		if (colorAttachment.layout != VK_IMAGE_LAYOUT_UNDEFINED)
		{
			Vulkan::addImageLayoutTransitionOptions(true, true, false, colorAttachment.layout, beginDependency.srcAccessMask, beginDependency.srcStageMask);
			Vulkan::addImageLayoutTransitionOptions(false, true, false, colorAttachment.layout, endDependency.dstAccessMask, endDependency.dstStageMask);
		}

		if (colorAttachment.msaaLayout != VK_IMAGE_LAYOUT_UNDEFINED)
		{
			Vulkan::addImageLayoutTransitionOptions(true, true, false, colorAttachment.msaaLayout, beginDependency.srcAccessMask, beginDependency.srcStageMask);
			Vulkan::addImageLayoutTransitionOptions(false, true, false, colorAttachment.msaaLayout, endDependency.dstAccessMask, endDependency.dstStageMask);
		}
	}

	subPass.colorAttachmentCount = static_cast<uint32_t>(configuration.colorAttachments.size());
	subPass.pColorAttachments = colorAttachmentRefs.data();

	VkAttachmentReference depthStencilAttachmentRef{};
	if (configuration.staticData.depthStencilAttachment.format != VK_FORMAT_UNDEFINED)
	{
		depthStencilAttachmentRef.attachment = attachment++;
		depthStencilAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		subPass.pDepthStencilAttachment = &depthStencilAttachmentRef;

		VkAttachmentDescription depthStencilAttachment{};
		depthStencilAttachment.format = configuration.staticData.depthStencilAttachment.format;
		depthStencilAttachment.samples = configuration.staticData.depthStencilAttachment.msaaSamples;
		depthStencilAttachment.loadOp = configuration.staticData.depthStencilAttachment.depthLoadOp;
		depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthStencilAttachment.stencilLoadOp = configuration.staticData.depthStencilAttachment.stencilLoadOp;
		depthStencilAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthStencilAttachment.initialLayout = configuration.staticData.depthStencilAttachment.layout;
		depthStencilAttachment.finalLayout = configuration.staticData.depthStencilAttachment.layout;
		attachments.push_back(depthStencilAttachment);

		Vulkan::addImageLayoutTransitionOptions(true, true, true, configuration.staticData.depthStencilAttachment.layout, beginDependency.srcAccessMask, beginDependency.srcStageMask);
		Vulkan::addImageLayoutTransitionOptions(false, true, true, configuration.staticData.depthStencilAttachment.layout, endDependency.dstAccessMask, endDependency.dstStageMask);
	}

	// Add resolve attachments after everything else to make pClearValues simpler to implement.
	if (configuration.staticData.resolve)
	{
		for (const auto &colorAttachment : configuration.colorAttachments)
		{
			VkAttachmentReference reference{};
			reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			if (colorAttachment.layout == VK_IMAGE_LAYOUT_UNDEFINED)
			{
				reference.attachment = VK_ATTACHMENT_UNUSED;
				colorResolveAttachmentRefs.push_back(reference);
			}
			else
			{
				reference.attachment = attachment++;
				colorResolveAttachmentRefs.push_back(reference);

				VkAttachmentDescription resolveDescription{};
				resolveDescription.format = colorAttachment.format;
				resolveDescription.samples = VK_SAMPLE_COUNT_1_BIT;
				resolveDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				resolveDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				resolveDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				resolveDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				resolveDescription.initialLayout = colorAttachment.layout;
				resolveDescription.finalLayout = colorAttachment.layout;

				attachments.push_back(resolveDescription);
			}
		}

		subPass.pResolveAttachments = colorResolveAttachmentRefs.data();
	}

	std::array<VkSubpassDependency, 2> dependencies = { beginDependency, endDependency };

	VkRenderPassCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	createInfo.pAttachments = attachments.data();
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subPass;
	createInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	createInfo.pDependencies = dependencies.data();

	VkRenderPass renderPass;
	VkResult result = vkCreateRenderPass(device, &createInfo, nullptr, &renderPass);
	if (result != VK_SUCCESS)
		throw love::Exception("Failed to create Vulkan render pass: %s", Vulkan::getErrorString(result));

	return renderPass;
}

VkRenderPass Graphics::getRenderPass(RenderPassConfiguration &configuration)
{
	VkRenderPass renderPass;
	auto it = renderPasses.find(configuration);
	if (it != renderPasses.end())
		renderPass = it->second;
	else
	{
		renderPass = createRenderPass(configuration);
		renderPasses[configuration] = renderPass;
	}

	return renderPass;
}

void Graphics::createVulkanVertexFormat(
	Shader *shader,
	const VertexAttributes &attributes,
	std::vector<VkVertexInputBindingDescription> &bindingDescriptions,
	std::vector<VkVertexInputAttributeDescription> &attributeDescriptions)
{
	std::set<uint32_t> usedBuffers;

	for (const auto &pair : shader->getVertexAttributeIndices())
	{
		int i = pair.second.index;
		uint32 bit = 1u << i;

		VkVertexInputAttributeDescription attribdesc{};
		attribdesc.location = i;

		if (attributes.enableBits & bit)
		{
			const auto &attrib = attributes.attribs[i];

			int bufferbinding = VERTEX_BUFFER_BINDING_START + attrib.bufferIndex;

			attribdesc.binding = bufferbinding;
			attribdesc.offset = attrib.offsetFromVertex;
			attribdesc.format = Vulkan::getVulkanVertexFormat(attrib.getFormat());

			if (usedBuffers.find(bufferbinding) == usedBuffers.end())
			{
				usedBuffers.insert(bufferbinding);

				VkVertexInputBindingDescription bindingdesc{};
				bindingdesc.binding = bufferbinding;
				if (attributes.instanceBits & (1u << attrib.bufferIndex))
					bindingdesc.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
				else
					bindingdesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
				bindingdesc.stride = attributes.bufferLayouts[attrib.bufferIndex].stride;
				bindingDescriptions.push_back(bindingdesc);
			}
		}
		else
		{
			attribdesc.binding = DEFAULT_VERTEX_BUFFER_BINDING;

			// Indices should match the creation parameters for defaultVertexBuffer.
			switch (pair.second.baseType)
			{
			case DATA_BASETYPE_INT:
				attribdesc.offset = defaultVertexBuffer->getDataMember(1).offset;
				attribdesc.format = Vulkan::getVulkanVertexFormat(DATAFORMAT_INT32_VEC4);
				break;
			case DATA_BASETYPE_UINT:
				attribdesc.offset = defaultVertexBuffer->getDataMember(1).offset;
				attribdesc.format = Vulkan::getVulkanVertexFormat(DATAFORMAT_UINT32_VEC4);
				break;
			case DATA_BASETYPE_FLOAT:
			default:
				if (i == ATTRIB_COLOR)
					attribdesc.offset = defaultVertexBuffer->getDataMember(2).offset;
				else
					attribdesc.offset = defaultVertexBuffer->getDataMember(0).offset;
				attribdesc.format = Vulkan::getVulkanVertexFormat(DATAFORMAT_FLOAT_VEC4);
				break;
			}

			if (usedBuffers.find(DEFAULT_VERTEX_BUFFER_BINDING) == usedBuffers.end())
			{
				usedBuffers.insert(DEFAULT_VERTEX_BUFFER_BINDING);

				VkVertexInputBindingDescription bindingdesc{};
				bindingdesc.binding = DEFAULT_VERTEX_BUFFER_BINDING;
				bindingdesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
				bindingdesc.stride = 0; // no stride, will always read the same coord multiple times.
				bindingDescriptions.push_back(bindingdesc);
			}
		}

		attributeDescriptions.push_back(attribdesc);
	}
}

void Graphics::prepareDraw(VertexAttributesID attributesID, const BufferBindings &buffers, graphics::Texture *texture, PrimitiveType primitiveType, CullMode cullmode)
{
	if (!renderPassState.active)
		startRenderPass();

	auto s = dynamic_cast<Shader*>(Shader::current);

	usedShadersInFrame.insert(s);

	GraphicsPipelineConfigurationFull configuration{};

	configuration.core.renderPass = renderPassState.beginInfo.renderPass;
	configuration.core.attributesID = attributesID;
	configuration.core.wireFrame = states.back().wireframe;
	configuration.core.blendStateKey = states.back().blend.toKey();
	configuration.core.colorChannelMask = states.back().colorMask;
	configuration.core.msaaSamples = renderPassState.msaa;
	configuration.core.numColorAttachments = renderPassState.numColorAttachments;
	configuration.core.packedColorAttachmentFormats = renderPassState.packedColorAttachmentFormats;
	configuration.core.primitiveType = primitiveType;

	VkPipeline pipeline = VK_NULL_HANDLE;

	if (optionalDeviceExtensions.extendedDynamicState)
	{
		vkCmdSetCullModeEXT(commandBuffers.at(currentFrame), Vulkan::getCullMode(cullmode));
		pipeline = s->getCachedGraphicsPipeline(this, configuration.core);
	}
	else
	{
		configuration.noDynamicState.winding = states.back().winding;
		configuration.noDynamicState.depthState.compare = states.back().depthTest;
		configuration.noDynamicState.depthState.write = states.back().depthWrite;
		configuration.noDynamicState.stencilAction = states.back().stencil.action;
		configuration.noDynamicState.stencilCompare = states.back().stencil.compare;
		configuration.noDynamicState.cullmode = cullmode;

		pipeline = s->getCachedGraphicsPipeline(this, configuration);
	}

	if (pipeline != renderPassState.pipeline)
	{
		vkCmdBindPipeline(commandBuffers.at(currentFrame), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		renderPassState.pipeline = pipeline;
	}

	s->setMainTex(texture);
	s->cmdPushDescriptorSets(commandBuffers.at(currentFrame), VK_PIPELINE_BIND_POINT_GRAPHICS);

	VkBuffer vkbuffers[BufferBindings::MAX];
	VkDeviceSize vkoffsets[BufferBindings::MAX];
	uint32 buffercount = 0;

	uint32 allbits = buffers.useBits;
	uint32 i = 0;
	while (allbits)
	{
		uint32 bit = 1u << i;

		// TODO: handle split ranges.
		if (buffers.useBits & bit)
		{
			vkbuffers[buffercount] = (VkBuffer)buffers.info[i].buffer->getHandle();
			vkoffsets[buffercount] = (VkDeviceSize)buffers.info[i].offset;
			buffercount++;
		}

		i++;
		allbits >>= 1;
	}

	if (buffercount > 0)
		vkCmdBindVertexBuffers(commandBuffers.at(currentFrame), VERTEX_BUFFER_BINDING_START, buffercount, vkbuffers, vkoffsets);
}

void Graphics::setDefaultRenderPass()
{
	uint32_t numClearValues = 2;
	renderPassState.clearColors.resize(numClearValues);

	renderPassState.beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassState.beginInfo.renderPass = VK_NULL_HANDLE;
	renderPassState.beginInfo.framebuffer = VK_NULL_HANDLE;
	renderPassState.beginInfo.renderArea.offset = { 0, 0 };
	renderPassState.beginInfo.renderArea.extent = swapChainExtent;
	renderPassState.beginInfo.clearValueCount = numClearValues;
	renderPassState.beginInfo.pClearValues = renderPassState.clearColors.data();

	renderPassState.isWindow = true;
	renderPassState.pipeline = VK_NULL_HANDLE;
	renderPassState.width = static_cast<float>(swapChainExtent.width);
	renderPassState.height = static_cast<float>(swapChainExtent.height);
	renderPassState.msaa = msaaSamples;
	renderPassState.numColorAttachments = 1;
	renderPassState.packedColorAttachmentFormats = (uint8)swapChainPixelFormat;

	RenderPassConfiguration renderPassConfiguration{};

	VkFormat dsformat = backbufferHasDepth || backbufferHasStencil ? depthStencilFormat : VK_FORMAT_UNDEFINED;
	renderPassConfiguration.staticData.depthStencilAttachment = { dsformat, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_LOAD_OP_LOAD, msaaSamples };
	if (msaaSamples & VK_SAMPLE_COUNT_1_BIT)
		renderPassConfiguration.staticData.resolve = false;
	else
		renderPassConfiguration.staticData.resolve = true;

	FramebufferConfiguration framebufferConfiguration{};
	framebufferConfiguration.staticData.depthView = depthImageView;
	framebufferConfiguration.staticData.width = swapChainExtent.width;
	framebufferConfiguration.staticData.height = swapChainExtent.height;

	if (msaaSamples & VK_SAMPLE_COUNT_1_BIT)
	{
		renderPassConfiguration.colorAttachments.push_back({ swapChainImageFormat, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_UNDEFINED, VK_ATTACHMENT_LOAD_OP_LOAD, msaaSamples });

		if (!swapChainImageViews.empty())
			framebufferConfiguration.colorViews.push_back(swapChainImageViews.at(imageIndex));
		else
			framebufferConfiguration.colorViews.push_back(fakeBackbuffer->getRenderTargetView(0, 0));
	}
	else
	{
		renderPassConfiguration.colorAttachments.push_back({ swapChainImageFormat, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ATTACHMENT_LOAD_OP_LOAD, msaaSamples });

		framebufferConfiguration.colorViews.push_back(colorImageView);
		if (!swapChainImageViews.empty())
			framebufferConfiguration.colorResolveViews.push_back(swapChainImageViews.at(imageIndex));
		else
			framebufferConfiguration.colorResolveViews.push_back(fakeBackbuffer->getRenderTargetView(0, 0));
	}

	renderPassState.renderPassConfiguration = std::move(renderPassConfiguration);
	renderPassState.framebufferConfiguration = std::move(framebufferConfiguration);

	// Can't call clear() here because it depends on current RT state, which might not be
	// set yet when this is called from within setRenderTargetsInternal.
	if (renderPassState.windowClearRequested)
	{
		if (renderPassState.mainWindowClearColorValue.hasValue)
		{
			renderPassState.renderPassConfiguration.colorAttachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			renderPassState.clearColors[0].color = Texture::getClearColor(nullptr, renderPassState.mainWindowClearColorValue.value);
		}

		if (renderPassState.mainWindowClearDepthValue.hasValue && backbufferHasDepth)
		{
			renderPassState.renderPassConfiguration.staticData.depthStencilAttachment.depthLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			renderPassState.clearColors[1].depthStencil.depth = static_cast<float>(renderPassState.mainWindowClearDepthValue.value);
		}

		if (renderPassState.mainWindowClearStencilValue.hasValue && backbufferHasStencil)
		{
			renderPassState.renderPassConfiguration.staticData.depthStencilAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			renderPassState.clearColors[1].depthStencil.stencil = static_cast<uint32_t>(renderPassState.mainWindowClearStencilValue.value);
		}
	}
}

void Graphics::setRenderPass(const RenderTargets &rts, int pixelw, int pixelh)
{
	RenderPassConfiguration renderPassConfiguration{};
	VkSampleCountFlagBits msaa = VK_SAMPLE_COUNT_1_BIT;

	for (const auto &color : rts.colors)
	{
		auto tex = (Texture *)color.texture;
		renderPassConfiguration.colorAttachments.push_back({ 
			Vulkan::getTextureFormat(tex->getPixelFormat()).internalFormat,
			tex->getImageLayout(),
			tex->getMSAAImageLayout(),
			VK_ATTACHMENT_LOAD_OP_LOAD,
			tex->getMsaaSamples() });

		if (tex->getMSAAImageLayout() != VK_IMAGE_LAYOUT_UNDEFINED && tex->getImageLayout() != VK_IMAGE_LAYOUT_UNDEFINED)
			renderPassConfiguration.staticData.resolve = true;

		msaa = tex->getMsaaSamples();
	}

	if (rts.depthStencil.texture != nullptr)
	{
		auto tex = (Texture *)rts.depthStencil.texture;
		renderPassConfiguration.staticData.depthStencilAttachment = {
			Vulkan::getTextureFormat(rts.depthStencil.texture->getPixelFormat()).internalFormat,
			tex->getImageLayout(),
			VK_ATTACHMENT_LOAD_OP_LOAD,
			VK_ATTACHMENT_LOAD_OP_LOAD,
			tex->getMsaaSamples() };

		msaa = tex->getMsaaSamples();
	}

	FramebufferConfiguration configuration{};

	for (const auto &color : rts.colors)
	{
		auto tex = (Texture*)color.texture;
		if (tex->getMSAA() > 1)
		{
			configuration.colorViews.push_back(tex->getMSAARenderTargetView(color.mipmap, color.slice));
			configuration.colorResolveViews.push_back(tex->getRenderTargetView(color.mipmap, color.slice));
		}
		else
		{
			configuration.colorViews.push_back(tex->getRenderTargetView(color.mipmap, color.slice));
		}
	}
	if (rts.depthStencil.texture != nullptr)
	{
		auto tex = (Texture*)rts.depthStencil.texture;
		if (tex->getMSAA() > 1)
			configuration.staticData.depthView = tex->getMSAARenderTargetView(rts.depthStencil.mipmap, rts.depthStencil.slice);
		else
			configuration.staticData.depthView = tex->getRenderTargetView(rts.depthStencil.mipmap, rts.depthStencil.slice);
	}

	configuration.staticData.width = static_cast<uint32_t>(pixelw);
	configuration.staticData.height = static_cast<uint32_t>(pixelh);

	uint32_t numClearValues = static_cast<uint32_t>(rts.colors.size() + 1);
	renderPassState.clearColors.resize(numClearValues);

	renderPassState.beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassState.beginInfo.renderPass = VK_NULL_HANDLE;
	renderPassState.beginInfo.framebuffer = VK_NULL_HANDLE;
	renderPassState.beginInfo.renderArea.offset = {0, 0};
	renderPassState.beginInfo.renderArea.extent.width = static_cast<uint32_t>(pixelw);
	renderPassState.beginInfo.renderArea.extent.height = static_cast<uint32_t>(pixelh);
	renderPassState.beginInfo.clearValueCount = numClearValues;
	renderPassState.beginInfo.pClearValues = renderPassState.clearColors.data();

	renderPassState.isWindow = false;
	renderPassState.renderPassConfiguration = renderPassConfiguration;
	renderPassState.framebufferConfiguration = configuration;
	renderPassState.pipeline = VK_NULL_HANDLE;
	renderPassState.width = static_cast<float>(pixelw);
	renderPassState.height = static_cast<float>(pixelh);
	renderPassState.msaa = msaa;
	renderPassState.numColorAttachments = static_cast<uint32_t>(rts.colors.size());
	renderPassState.packedColorAttachmentFormats = 0;
	for (size_t i = 0; i < rts.colors.size(); i++)
		renderPassState.packedColorAttachmentFormats |= ((uint64)rts.colors[i].texture->getPixelFormat()) << (i * 8ull);
}

void Graphics::startRenderPass()
{
	renderPassState.active = true;

	if (renderPassState.isWindow && renderPassState.windowClearRequested)
		renderPassState.windowClearRequested = false;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = renderPassState.width;
	viewport.height = renderPassState.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vkCmdSetViewport(commandBuffers.at(currentFrame), 0, 1, &viewport);

	renderPassState.beginInfo.renderPass = getRenderPass(renderPassState.renderPassConfiguration);

	renderPassState.framebufferConfiguration.staticData.renderPass = renderPassState.beginInfo.renderPass;
	renderPassState.beginInfo.framebuffer = getFramebuffer(renderPassState.framebufferConfiguration);

	vkCmdBeginRenderPass(commandBuffers.at(currentFrame), &renderPassState.beginInfo, VK_SUBPASS_CONTENTS_INLINE);

	applyScissor();
}

void Graphics::endRenderPass()
{
	renderPassState.active = false;

	vkCmdEndRenderPass(commandBuffers.at(currentFrame));

	for (auto &colorAttachment : renderPassState.renderPassConfiguration.colorAttachments)
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

	renderPassState.renderPassConfiguration.staticData.depthStencilAttachment.depthLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	renderPassState.renderPassConfiguration.staticData.depthStencilAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
}

VkSampler Graphics::createSampler(const SamplerState &samplerState)
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = Vulkan::getFilter(samplerState.magFilter);
	samplerInfo.minFilter = Vulkan::getFilter(samplerState.minFilter);
	samplerInfo.addressModeU = Vulkan::getWrapMode(samplerState.wrapU);
	samplerInfo.addressModeV = Vulkan::getWrapMode(samplerState.wrapV);
	samplerInfo.addressModeW = Vulkan::getWrapMode(samplerState.wrapW);
	samplerInfo.anisotropyEnable = samplerState.maxAnisotropy > 1 ? VK_TRUE : VK_FALSE;
	samplerInfo.maxAnisotropy = static_cast<float>(samplerState.maxAnisotropy);

	// TODO: This probably needs to branch on a pixel format to determine whether
	// it should be float vs int, and opaque vs transparent.
	bool clampone = samplerState.wrapU == SamplerState::WRAP_CLAMP_ONE
		|| samplerState.wrapV == SamplerState::WRAP_CLAMP_ONE
		|| samplerState.wrapW == SamplerState::WRAP_CLAMP_ONE;
	samplerInfo.borderColor = clampone ? VK_BORDER_COLOR_INT_OPAQUE_WHITE : VK_BORDER_COLOR_INT_OPAQUE_BLACK;

	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	if (samplerState.depthSampleMode.hasValue)
	{
		samplerInfo.compareEnable = VK_TRUE;
		// See the comment in renderstate.h
		samplerInfo.compareOp = Vulkan::getCompareOp(getReversedCompareMode(samplerState.depthSampleMode.value));
	}
	else
	{
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	}
	samplerInfo.mipmapMode = Vulkan::getMipMapMode(samplerState.mipmapFilter);
	samplerInfo.mipLodBias = samplerState.lodBias;
	samplerInfo.minLod = static_cast<float>(samplerState.minLod);
	samplerInfo.maxLod = static_cast<float>(samplerState.maxLod);

	VkSampler sampler;
	VkResult result = vkCreateSampler(device, &samplerInfo, nullptr, &sampler);
	if (result != VK_SUCCESS)
		throw love::Exception("Failed to create Vulkan sampler: %s", Vulkan::getErrorString(result));

	return sampler;
}

void Graphics::requestSwapchainRecreation()
{
	if (swapChain != VK_NULL_HANDLE)
	{
		swapChainRecreationRequested = true;
	}
}

VkSampler Graphics::getCachedSampler(const SamplerState &samplerState)
{
	auto samplerkey = samplerState.toKey();
	auto it = samplers.find(samplerkey);
	if (it != samplers.end())
		return it->second;
	else
	{
		VkSampler sampler = createSampler(samplerState);
		samplers.insert({ samplerkey, sampler });
		return sampler;
	}
}

static uint64 getDescriptorPoolsKey(int dynamicUniformBuffers, int sampledTextures, int storageTextures, int texelBuffers, int storageBuffers)
{
	return (((int64)dynamicUniformBuffers & 0xFF) << 0)
		| (((int64)sampledTextures & 0xFF) << 8)
		| (((int64)storageTextures & 0xFF) << 16)
		| (((int64)texelBuffers    & 0xFF) << 24)
		| (((int64)storageBuffers  & 0xFF) << 32);
}

SharedDescriptorPools *Graphics::acquireDescriptorPools(int dynamicUniformBuffers, int sampledTextures, int storageTextures, int texelBuffers, int storageBuffers)
{
	uint64 key = getDescriptorPoolsKey(dynamicUniformBuffers, sampledTextures, storageTextures, texelBuffers, storageBuffers);

	auto it = sharedDescriptorPools.find(key);
	if (it != sharedDescriptorPools.end())
	{
		it->second.referenceCount++;
		return it->second.pools;
	}

	auto pools = new SharedDescriptorPools(device, dynamicUniformBuffers, sampledTextures, storageTextures, texelBuffers, storageBuffers);

	SharedDescriptorPoolsRef ref{};
	ref.pools = pools;
	ref.referenceCount = 1;
	sharedDescriptorPools[key] = ref;

	return pools;
}

void Graphics::releaseDescriptorPools(SharedDescriptorPools *p)
{
	uint64 key = getDescriptorPoolsKey(p->dynamicUniformBuffers, p->sampledTextures, p->storageTextures, p->texelBuffers, p->storageBuffers);

	auto it = sharedDescriptorPools.find(key);
	if (it != sharedDescriptorPools.end())
	{
		it->second.referenceCount--;
		if (it->second.referenceCount <= 0)
		{
			delete it->second.pools;
			sharedDescriptorPools.erase(key);
		}
	}
}

VkPipeline Graphics::createGraphicsPipeline(Shader *shader, const GraphicsPipelineConfigurationCore &configuration, const GraphicsPipelineConfigurationNoDynamicState *noDynamicStateConfiguration)
{
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

	auto &shaderStages = shader->getShaderStages();

	std::vector<VkVertexInputBindingDescription> bindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	VertexAttributes vertexAttributes;
	findVertexAttributes(configuration.attributesID, vertexAttributes);

	createVulkanVertexFormat(shader, vertexAttributes, bindingDescriptions, attributeDescriptions);

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = configuration.msaaSamples;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = Vulkan::getPolygonMode(configuration.wireFrame);
	rasterizer.lineWidth = 1.0f;
	if (!optionalDeviceExtensions.extendedDynamicState)
	{
		rasterizer.cullMode = Vulkan::getCullMode(noDynamicStateConfiguration->cullmode);
		rasterizer.frontFace = Vulkan::getFrontFace(noDynamicStateConfiguration->winding);
	}

	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = Vulkan::getPrimitiveTypeTopology(configuration.primitiveType);
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	if (!optionalDeviceExtensions.extendedDynamicState)
	{
		depthStencil.depthWriteEnable = Vulkan::getBool(noDynamicStateConfiguration->depthState.write);
		depthStencil.depthCompareOp = Vulkan::getCompareOp(noDynamicStateConfiguration->depthState.compare);
	}
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f;
	depthStencil.maxDepthBounds = 1.0f;

	depthStencil.stencilTestEnable = VK_TRUE;

	if (!optionalDeviceExtensions.extendedDynamicState)
	{
		depthStencil.front.failOp = VK_STENCIL_OP_KEEP;
		depthStencil.front.passOp = Vulkan::getStencilOp(noDynamicStateConfiguration->stencilAction);
		depthStencil.front.depthFailOp = VK_STENCIL_OP_KEEP;
		depthStencil.front.compareOp = Vulkan::getCompareOp(getReversedCompareMode(noDynamicStateConfiguration->stencilCompare));

		depthStencil.back.failOp = VK_STENCIL_OP_KEEP;
		depthStencil.back.passOp = Vulkan::getStencilOp(noDynamicStateConfiguration->stencilAction);
		depthStencil.back.depthFailOp = VK_STENCIL_OP_KEEP;
		depthStencil.back.compareOp = Vulkan::getCompareOp(getReversedCompareMode(noDynamicStateConfiguration->stencilCompare));
	}

	pipelineInfo.pDepthStencilState = &depthStencil;

	BlendState blendState = BlendState::fromKey(configuration.blendStateKey);

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = Vulkan::getColorMask(configuration.colorChannelMask);
	colorBlendAttachment.blendEnable = Vulkan::getBool(blendState.enable);
	colorBlendAttachment.srcColorBlendFactor = Vulkan::getBlendFactor(blendState.srcFactorRGB);
	colorBlendAttachment.dstColorBlendFactor = Vulkan::getBlendFactor(blendState.dstFactorRGB);
	colorBlendAttachment.colorBlendOp = Vulkan::getBlendOp(blendState.operationRGB);
	colorBlendAttachment.srcAlphaBlendFactor = Vulkan::getBlendFactor(blendState.srcFactorA);
	colorBlendAttachment.dstAlphaBlendFactor = Vulkan::getBlendFactor(blendState.dstFactorA);
	colorBlendAttachment.alphaBlendOp = Vulkan::getBlendOp(blendState.operationA);

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(configuration.numColorAttachments, colorBlendAttachment);

	if (blendState.enable)
	{
		for (uint32 i = 0; i < configuration.numColorAttachments; i++)
		{
			PixelFormat format = (PixelFormat)((configuration.packedColorAttachmentFormats >> (i * 8ull)) & 0xFF);
			if (!isPixelFormatSupported(format, PIXELFORMATUSAGEFLAGS_BLEND))
				colorBlendAttachments[i].blendEnable = false;
		}
	}

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
	colorBlending.pAttachments = colorBlendAttachments.data();
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	std::vector<VkDynamicState> dynamicStates;

	if (optionalDeviceExtensions.extendedDynamicState)
		dynamicStates = {
			VK_DYNAMIC_STATE_SCISSOR,
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
			VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
			VK_DYNAMIC_STATE_STENCIL_REFERENCE,

			VK_DYNAMIC_STATE_CULL_MODE_EXT,
			VK_DYNAMIC_STATE_FRONT_FACE_EXT,
			VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE_EXT,
			VK_DYNAMIC_STATE_DEPTH_COMPARE_OP_EXT,
			VK_DYNAMIC_STATE_STENCIL_OP_EXT,
		};
	else
		dynamicStates = {
			VK_DYNAMIC_STATE_SCISSOR,
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
			VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
			VK_DYNAMIC_STATE_STENCIL_REFERENCE,
		};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = shader->getGraphicsPipelineLayout();
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;
	pipelineInfo.renderPass = configuration.renderPass;

	VkPipeline graphicsPipeline;
	VkResult result = vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineInfo, nullptr, &graphicsPipeline);
	if (result != VK_SUCCESS)
		throw love::Exception("Failed to create Vulkan graphics pipeline: %s", Vulkan::getErrorString(result));
	return graphicsPipeline;
}

VkSampleCountFlagBits Graphics::getMsaaCount(int requestedMsaa) const
{
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

	VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;

	if (counts & VK_SAMPLE_COUNT_64_BIT && requestedMsaa >= 64)
		return VK_SAMPLE_COUNT_64_BIT;
	else if (counts & VK_SAMPLE_COUNT_32_BIT && requestedMsaa >= 32)
		return VK_SAMPLE_COUNT_32_BIT;
	else if (counts & VK_SAMPLE_COUNT_16_BIT && requestedMsaa >= 16)
		return VK_SAMPLE_COUNT_16_BIT;
	else if (counts & VK_SAMPLE_COUNT_8_BIT && requestedMsaa >= 8)
		return VK_SAMPLE_COUNT_8_BIT;
	else if (counts & VK_SAMPLE_COUNT_4_BIT && requestedMsaa >= 4)
		return VK_SAMPLE_COUNT_4_BIT;
	else if (counts & VK_SAMPLE_COUNT_2_BIT && requestedMsaa >= 2)
		return VK_SAMPLE_COUNT_2_BIT;
	else
		return VK_SAMPLE_COUNT_1_BIT;
}

void Graphics::setVsync(int vsync)
{
	if (vsync != this->vsync)
	{
		this->vsync = vsync;

		// With the extension VK_EXT_swapchain_maintenance1 a swapchain recreation might not be needed
		// https://github.com/KhronosGroup/Vulkan-Docs/blob/main/proposals/VK_EXT_swapchain_maintenance1.adoc
		// However, there are not any drivers that support it, yet.
		// Reevaluate again in the future.

		requestSwapchainRecreation();
	}
}

int Graphics::getVsync() const
{
	return vsync;
}

void Graphics::mapLocalUniformData(void *data, size_t size, VkDescriptorBufferInfo &bufferInfo)
{
	size_t alignedSize = alignUp(size, minUniformBufferOffsetAlignment);

	if (localUniformBuffer->getUsableSize() < alignedSize)
		localUniformBuffer.set(new StreamBuffer(this, BUFFERUSAGE_UNIFORM, localUniformBuffer->getSize() * 2), Acquire::NORETAIN);

	auto mapInfo = localUniformBuffer->map(size);
	memcpy(mapInfo.data, data, size);

	bufferInfo.buffer = (VkBuffer)localUniformBuffer->getHandle();
	bufferInfo.offset = localUniformBuffer->unmap(size);
	bufferInfo.range = size;

	localUniformBuffer->markUsed(alignedSize);
}

void Graphics::createColorResources()
{
	if (msaaSamples & VK_SAMPLE_COUNT_1_BIT)
	{
		colorImage = VK_NULL_HANDLE;
		colorImageView = VK_NULL_HANDLE;
	} 
	else
	{
		VkFormat colorFormat = swapChainImageFormat;

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = colorFormat;
		imageInfo.extent.width = swapChainExtent.width;
		imageInfo.extent.height = swapChainExtent.height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = msaaSamples;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaAllocationCreateInfo allocationInfo{};
		allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocationInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

		VkResult result = vmaCreateImage(vmaAllocator, &imageInfo, &allocationInfo, &colorImage, &colorImageAllocation, nullptr);
		if (result != VK_SUCCESS)
			throw love::Exception("Failed to create Vulkan MSAA color image: %s", Vulkan::getErrorString(result));

		VkImageViewCreateInfo imageViewInfo{};
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.image = colorImage;
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewInfo.format = colorFormat;
		imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewInfo.subresourceRange.baseMipLevel = 0;
		imageViewInfo.subresourceRange.levelCount = 1;
		imageViewInfo.subresourceRange.baseArrayLayer = 0;
		imageViewInfo.subresourceRange.layerCount = 1;

		result = vkCreateImageView(device, &imageViewInfo, nullptr, &colorImageView);
		if (result != VK_SUCCESS)
			throw love::Exception("Failed to create Vulkan MSAA color image view: %s", Vulkan::getErrorString(result));
	}
}

VkFormat Graphics::findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (auto format : candidates)
	{
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);
		if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
			return format;
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features)
			return format;
	}

	throw love::Exception("failed to find supported format");
}

VkFormat Graphics::findDepthFormat()
{
	return findSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

void Graphics::createDepthResources()
{
	if (!backbufferHasDepth && !backbufferHasStencil)
	{
		depthImage = VK_NULL_HANDLE;
		depthImageView = VK_NULL_HANDLE;
		return;
	}

	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = depthStencilFormat;
	imageInfo.extent.width = swapChainExtent.width;
	imageInfo.extent.height = swapChainExtent.height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = msaaSamples;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VmaAllocationCreateInfo allocationInfo{};
	allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocationInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

	VkResult result = vmaCreateImage(vmaAllocator, &imageInfo, &allocationInfo, &depthImage, &depthImageAllocation, nullptr);
	if (result != VK_SUCCESS)
		throw love::Exception("Failed to create Vulkan backbuffer depth image: %s", Vulkan::getErrorString(result));

	VkImageViewCreateInfo imageViewInfo{};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.image = depthImage;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = depthStencilFormat;
	imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	if (backbufferHasDepth)
		imageViewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
	if (backbufferHasStencil)
		imageViewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;

	result = vkCreateImageView(device, &imageViewInfo, nullptr, &depthImageView);
	if (result != VK_SUCCESS)
		throw love::Exception("Failed to create Vulkan backbuffer depth image view: %s", Vulkan::getErrorString(result));
}

void Graphics::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

	VkResult result = vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool);
	if (result != VK_SUCCESS)
		throw love::Exception("Failed to create Vulkan command pool: %s", Vulkan::getErrorString(result));
}

void Graphics::createCommandBuffers()
{
	commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	VkResult result = vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data());
	if (result != VK_SUCCESS)
		throw love::Exception("Failed to allocate Vulkan command buffers: %s", Vulkan::getErrorString(result));
}

void Graphics::createSyncObjects()
{
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores.at(i)) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores.at(i)) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences.at(i)) != VK_SUCCESS)
			throw love::Exception("Failed to create Vulkan synchronization objects for a frame!");
}

void Graphics::cleanup()
{
	for (auto &cleanUpFns : cleanUpFunctions)
		for (auto &cleanUpFn : cleanUpFns)
			cleanUpFn();
	cleanUpFunctions.clear();

	vmaDestroyAllocator(vmaAllocator);

	for (const auto &s : renderFinishedSemaphores)
		vkDestroySemaphore(device, s, nullptr);
	renderFinishedSemaphores.clear();

	for (const auto &s : imageAvailableSemaphores)
		vkDestroySemaphore(device, s, nullptr);
	imageAvailableSemaphores.clear();

	for (const auto &f : inFlightFences)
		vkDestroyFence(device, f, nullptr);
	inFlightFences.clear();

	if (!commandBuffers.empty())
		vkFreeCommandBuffers(device, commandPool, (uint32)commandBuffers.size(), commandBuffers.data());
	commandBuffers.clear();

	for (auto const &p : samplers)
		vkDestroySampler(device, p.second, nullptr);
	samplers.clear();

	for (const auto &entry : renderPasses)
		vkDestroyRenderPass(device, entry.second, nullptr);
	renderPasses.clear();

	for (const auto &entry : framebuffers)
		vkDestroyFramebuffer(device, entry.second, nullptr);
	framebuffers.clear();

	if (commandPool != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool(device, commandPool, nullptr);
		commandPool = VK_NULL_HANDLE;
	}

	if (pipelineCache != VK_NULL_HANDLE)
	{
		vkDestroyPipelineCache(device, pipelineCache, nullptr);
		pipelineCache = VK_NULL_HANDLE;
	}

	if (device != VK_NULL_HANDLE)
	{
		vkDestroyDevice(device, nullptr);
		device = VK_NULL_HANDLE;
	}
}

void Graphics::cleanupSwapChain(bool destroySwapChainObject)
{
	if (colorImage)
	{
		cleanupFramebuffers(colorImageView, swapChainPixelFormat);

		vkDestroyImageView(device, colorImageView, nullptr);
		colorImageView = VK_NULL_HANDLE;

		vmaDestroyImage(vmaAllocator, colorImage, colorImageAllocation);
		colorImage = VK_NULL_HANDLE;
	}
	if (depthImage)
	{
		cleanupFramebuffers(depthImageView, depthStencilPixelFormat);

		vkDestroyImageView(device, depthImageView, nullptr);
		depthImageView = VK_NULL_HANDLE;

		vmaDestroyImage(vmaAllocator, depthImage, depthImageAllocation);
		depthImage = VK_NULL_HANDLE;
	}
	for (const auto &swapChainImageView : swapChainImageViews)
	{
		cleanupFramebuffers(swapChainImageView, swapChainPixelFormat);
		vkDestroyImageView(device, swapChainImageView, nullptr);
	}
	swapChainImageViews.clear();
	swapChainImages.clear();
	fakeBackbuffer.set(nullptr);

	if (destroySwapChainObject && swapChain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(device, swapChain, nullptr);
		swapChain = VK_NULL_HANDLE;
	}
}

void Graphics::recreateSwapChain()
{
	vkDeviceWaitIdle(device);

	cleanupSwapChain(false);

	createSwapChain();
	createImageViews();
	createColorResources();
	createDepthResources();

	transitionColorDepthLayouts = true;
}

love::graphics::Graphics *createInstance()
{
	love::graphics::Graphics *instance = nullptr;

	try
	{
		instance = new Graphics();
	}
	catch (love::Exception &e)
	{
		if (isDebugEnabled())
			printf("Cannot create Vulkan renderer: %s\n", e.what());
	}

	return instance;
}

} // vulkan
} // graphics
} // love
