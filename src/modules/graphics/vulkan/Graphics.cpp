/**
 * Copyright (c) 2006-2023 LOVE Development Team
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
#include "window/Window.h"
#include "Buffer.h"
#include "Graphics.h"
#include "GraphicsReadback.h"
#include "Shader.h"
#include "Vulkan.h"

#include <SDL_vulkan.h>

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

constexpr uint32_t USAGES_POLL_INTERVAL = 5000;

const char *Graphics::getName() const
{
	return "love.graphics.vulkan";
}

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
{
	if (SDL_Vulkan_LoadLibrary(nullptr))
		throw love::Exception("could not find vulkan");

	volkInitializeCustom((PFN_vkGetInstanceProcAddr)SDL_Vulkan_GetVkGetInstanceProcAddr());

	if (isDebugEnabled() && !checkValidationSupport())
		throw love::Exception("validation layers requested, but not available");

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
	unsigned int count;
	if (SDL_Vulkan_GetInstanceExtensions(nullptr, &count, nullptr) != SDL_TRUE)
		throw love::Exception("couldn't retrieve sdl vulkan extensions");

	std::vector<const char*> extensions = {};

	checkOptionalInstanceExtensions(optionalInstanceExtensions);

	if (optionalInstanceExtensions.physicalDeviceProperties2)
		extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
	if (optionalInstanceExtensions.debugInfo)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	size_t additional_extension_count = extensions.size();
	extensions.resize(additional_extension_count + count);

	if (SDL_Vulkan_GetInstanceExtensions(nullptr, &count, extensions.data() + additional_extension_count) != SDL_TRUE)
		throw love::Exception("couldn't retrieve sdl vulkan extensions");

	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	if (isDebugEnabled())
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
		throw love::Exception("couldn't create vulkan instance");

	volkLoadInstance(instance);
}

Graphics::~Graphics()
{
	defaultConstantTexCoord.set(nullptr);
	defaultConstantColor.set(nullptr);
	defaultTexture.set(nullptr);

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

love::graphics::Buffer *Graphics::newBuffer(const love::graphics::Buffer::Settings &settings, const std::vector<love::graphics::Buffer::DataDeclaration> &format, const void *data, size_t size, size_t arraylength)
{
	return new Buffer(this, settings, format, data, size, arraylength);
}

void Graphics::clear(OptionalColorD color, OptionalInt stencil, OptionalDouble depth)
{
	if (!color.hasValue && !stencil.hasValue && !depth.hasValue)
		return;

	flushBatchedDraws();

	if (renderPassState.active)
	{
		VkClearAttachment attachment{};

		if (color.hasValue)
		{
			Colorf cf((float)color.value.r, (float)color.value.g, (float)color.value.b, (float)color.value.a);
			gammaCorrectColor(cf);

			attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			attachment.clearValue.color.float32[0] = static_cast<float>(cf.r);
			attachment.clearValue.color.float32[1] = static_cast<float>(cf.g);
			attachment.clearValue.color.float32[2] = static_cast<float>(cf.b);
			attachment.clearValue.color.float32[3] = static_cast<float>(cf.a);
		}

		VkClearAttachment depthStencilAttachment{};

		if (stencil.hasValue)
		{
			depthStencilAttachment.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
			depthStencilAttachment.clearValue.depthStencil.stencil = static_cast<uint32_t>(stencil.value);
		}
		if (depth.hasValue)
		{
			depthStencilAttachment.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
			depthStencilAttachment.clearValue.depthStencil.depth = static_cast<float>(depth.value);
		}

		std::array<VkClearAttachment, 2> attachments = {
			attachment,
			depthStencilAttachment
		};

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
		if (color.hasValue)
		{
			renderPassState.renderPassConfiguration.colorAttachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

			Colorf cf((float)color.value.r, (float)color.value.g, (float)color.value.b, (float)color.value.a);
			gammaCorrectColor(cf);

			renderPassState.clearColors[0].color.float32[0] = static_cast<float>(cf.r);
			renderPassState.clearColors[0].color.float32[1] = static_cast<float>(cf.g);
			renderPassState.clearColors[0].color.float32[2] = static_cast<float>(cf.b);
			renderPassState.clearColors[0].color.float32[3] = static_cast<float>(cf.a);
		}

		if (depth.hasValue)
		{
			renderPassState.renderPassConfiguration.staticData.depthStencilAttachment.depthLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			renderPassState.clearColors[1].depthStencil.depth = static_cast<float>(depth.value);
		}

		if (stencil.hasValue)
		{
			renderPassState.renderPassConfiguration.staticData.depthStencilAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			renderPassState.clearColors[1].depthStencil.stencil = static_cast<uint32_t>(stencil.value);
		}

		if (renderPassState.isWindow)
		{
			renderPassState.windowClearRequested = true;
			renderPassState.mainWindowClearColorValue = color;
			renderPassState.mainWindowClearDepthValue = depth;
			renderPassState.mainWindowClearStencilValue = stencil;
		}
		else
			startRenderPass();
	}
}

void Graphics::clear(const std::vector<OptionalColorD> &colors, OptionalInt stencil, OptionalDouble depth)
{
	if (colors.empty() && !stencil.hasValue && !depth.hasValue)
		return;

	flushBatchedDraws();

	if (renderPassState.active)
	{
		std::vector<VkClearAttachment> attachments;
		for (const auto &color : colors)
		{
			VkClearAttachment attachment{};
			if (color.hasValue)
			{
				Colorf cf((float)color.value.r, (float)color.value.g, (float)color.value.b, (float)color.value.a);
				gammaCorrectColor(cf);

				attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				attachment.clearValue.color.float32[0] = static_cast<float>(cf.r);
				attachment.clearValue.color.float32[1] = static_cast<float>(cf.g);
				attachment.clearValue.color.float32[2] = static_cast<float>(cf.b);
				attachment.clearValue.color.float32[3] = static_cast<float>(cf.a);
			}
			attachments.push_back(attachment);
		}

		VkClearAttachment depthStencilAttachment{};

		if (stencil.hasValue)
		{
			depthStencilAttachment.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			depthStencilAttachment.clearValue.depthStencil.stencil = static_cast<uint32_t>(stencil.value);
		}
		if (depth.hasValue)
		{
			depthStencilAttachment.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
			depthStencilAttachment.clearValue.depthStencil.depth = static_cast<float>(depth.value);
		}

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
		for (size_t i = 0; i < colors.size(); i++)
		{
			if (colors[i].hasValue)
			{
				renderPassState.renderPassConfiguration.colorAttachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

				auto &color = colors[i];
				Colorf cf((float)color.value.r, (float)color.value.g, (float)color.value.b, (float)color.value.a);
				gammaCorrectColor(cf);

				renderPassState.clearColors[i].color.float32[0] = static_cast<float>(cf.r);
				renderPassState.clearColors[i].color.float32[1] = static_cast<float>(cf.g);
				renderPassState.clearColors[i].color.float32[2] = static_cast<float>(cf.b);
				renderPassState.clearColors[i].color.float32[3] = static_cast<float>(cf.a);
			}
		}

		if (depth.hasValue)
		{
			renderPassState.renderPassConfiguration.staticData.depthStencilAttachment.depthLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			renderPassState.clearColors[colors.size()].depthStencil.depth = static_cast<float>(depth.value);
		}

		if (stencil.hasValue)
		{
			renderPassState.renderPassConfiguration.staticData.depthStencilAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			renderPassState.clearColors[colors.size()].depthStencil.stencil = static_cast<uint32_t>(stencil.value);
		}

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

	if (submitMode == SUBMIT_PRESENT)
	{
		if (pendingScreenshotCallbacks.empty())
			Vulkan::cmdTransitionImageLayout(
				commandBuffers.at(currentFrame),
				swapChainImages.at(imageIndex),
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		else
		{
			Vulkan::cmdTransitionImageLayout(
				commandBuffers.at(currentFrame),
				swapChainImages.at(imageIndex),
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

			Vulkan::cmdTransitionImageLayout(
				commandBuffers.at(currentFrame),
				screenshotReadbackBuffers.at(currentFrame).image,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

			VkImageBlit blit{};
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.layerCount = 1;
			blit.srcOffsets[1] = {
				static_cast<int>(swapChainExtent.width),
				static_cast<int>(swapChainExtent.height),
				1
			};
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.layerCount = 1;
			blit.dstOffsets[1] = {
				static_cast<int>(swapChainExtent.width),
				static_cast<int>(swapChainExtent.height),
				1
			};

			vkCmdBlitImage(
				commandBuffers.at(currentFrame),
				swapChainImages.at(imageIndex), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				screenshotReadbackBuffers.at(currentFrame).image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
				1, &blit,
				VK_FILTER_NEAREST);

			Vulkan::cmdTransitionImageLayout(
				commandBuffers.at(currentFrame),
				swapChainImages.at(imageIndex),
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

			Vulkan::cmdTransitionImageLayout(
				commandBuffers.at(currentFrame),
				screenshotReadbackBuffers.at(currentFrame).image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
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
				screenshotReadbackBuffers.at(currentFrame).image,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				screenshotReadbackBuffers.at(currentFrame).buffer,
				1, &region);

			addReadbackCallback([
				w = swapChainExtent.width,
				h = swapChainExtent.height,
				pendingScreenshotCallbacks = pendingScreenshotCallbacks,
				screenShotReadbackBuffer = screenshotReadbackBuffers.at(currentFrame),
				screenshotCallbackData = screenshotCallbackData]() {
				auto imageModule = Module::getInstance<love::image::Image>(M_IMAGE);

				for (const auto &info : pendingScreenshotCallbacks)
				{
					image::ImageData *img = imageModule->newImageData(
						w,
						h,
						PIXELFORMAT_RGBA8_UNORM,
						screenShotReadbackBuffer.allocationInfo.pMappedData);
					info.callback(&info, img, screenshotCallbackData);
					img->release();
				}
			});

			pendingScreenshotCallbacks.clear();
		}
	}

	endRecordingGraphicsCommands();

	if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
		vkWaitForFences(device, 1, &imagesInFlight.at(imageIndex), VK_TRUE, UINT64_MAX);
	imagesInFlight[imageIndex] = inFlightFences[currentFrame];

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
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(device, 1, &inFlightFences[currentFrame]);
		fence = inFlightFences[currentFrame];
	}

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, fence) != VK_SUCCESS)
		throw love::Exception("failed to submit draw command buffer");
	
	if (submitMode == SUBMIT_NOPRESENT || submitMode == SUBMIT_RESTART)
	{
		vkQueueWaitIdle(graphicsQueue);

		for (auto &callbacks : readbackCallbacks)
		{
			for (const auto &callback : callbacks)
				callback();
			callbacks.clear();
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

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderFinishedSemaphores.at(currentFrame);
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapChain;
	presentInfo.pImageIndices = &imageIndex;

	VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);

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

	frameCounter++;
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	beginFrame();
}

void Graphics::setViewportSize(int width, int height, int pixelwidth, int pixelheight)
{
	if (swapChain != VK_NULL_HANDLE && (pixelwidth != this->pixelWidth || pixelheight != this->pixelHeight || width != this->width || height != this->height))
		requestSwapchainRecreation();

	this->width = width;
	this->height = height;
	this->pixelWidth = pixelwidth;
	this->pixelHeight = pixelheight;

	if (!isRenderTargetActive())
		resetProjection();
}

bool Graphics::setMode(void *context, int width, int height, int pixelwidth, int pixelheight, bool windowhasstencil, int msaa)
{
	requestedMsaa = msaa;
	windowHasStencil = windowhasstencil;

	// Must be called before the swapchain is created.
	setViewportSize(width, height, pixelwidth, pixelheight);

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
	createScreenshotCallbackBuffers();
	createColorResources();
	createDepthResources();
	transitionColorDepthLayouts = true;

	if (createBaseObjects)
	{
		createCommandPool();
		createCommandBuffers();
		createSyncObjects();
	}

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

		// sometimes the VertexTexCoord is not set, so we manually adjust it to (0, 0)
		if (defaultConstantTexCoord == nullptr)
		{
			float zeroTexCoord[2] = { 0.0f, 0.0f };
			Buffer::DataDeclaration format("ConstantTexCoord", DATAFORMAT_FLOAT_VEC2);
			Buffer::Settings settings(BUFFERUSAGEFLAG_VERTEX, BUFFERDATAUSAGE_STATIC);
			defaultConstantTexCoord = newBuffer(settings, { format }, zeroTexCoord, sizeof(zeroTexCoord), 1);
		}

		// sometimes the VertexColor is not set, so we manually adjust it to white color
		if (defaultConstantColor == nullptr)
		{
			uint8 whiteColor[] = { 255, 255, 255, 255 };
			Buffer::DataDeclaration format("ConstantColor", DATAFORMAT_UNORM8_VEC4);
			Buffer::Settings settings(BUFFERUSAGEFLAG_VERTEX, BUFFERDATAUSAGE_STATIC);
			defaultConstantColor = newBuffer(settings, { format }, whiteColor, sizeof(whiteColor), 1);
		}

		createDefaultTexture();
		createDefaultShaders();
		Shader::current = Shader::standardShaders[Shader::StandardShader::STANDARD_DEFAULT];
		createQuadIndexBuffer();
		createFanIndexBuffer();

		frameCounter = 0;
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
	capabilities.features[FEATURE_BLEND_MINMAX] = true;
	capabilities.features[FEATURE_LIGHTEN] = true;
	capabilities.features[FEATURE_FULL_NPOT] = true;
	capabilities.features[FEATURE_PIXEL_SHADER_HIGHP] = true;
	capabilities.features[FEATURE_SHADER_DERIVATIVES] = true;
	capabilities.features[FEATURE_GLSL3] = true;
	capabilities.features[FEATURE_GLSL4] = true;
	capabilities.features[FEATURE_INSTANCING] = true;
	capabilities.features[FEATURE_TEXEL_BUFFER] = true;
	capabilities.features[FEATURE_INDEX_BUFFER_32BIT] = true;
	capabilities.features[FEATURE_COPY_BUFFER] = true;
	capabilities.features[FEATURE_COPY_BUFFER_TO_TEXTURE] = true;
	capabilities.features[FEATURE_COPY_TEXTURE_TO_BUFFER] = true;
	capabilities.features[FEATURE_COPY_RENDER_TARGET_TO_BUFFER] = true;
	capabilities.features[FEATURE_MIPMAP_RANGE] = true;
	capabilities.features[FEATURE_INDIRECT_DRAW] = true;
	static_assert(FEATURE_MAX_ENUM == 19, "Graphics::initCapabilities must be updated when adding a new graphics feature!");

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
	submitGpuCommands(SUBMIT_NOPRESENT);

	created = false;

	cleanupSwapChain();
	vkDestroySurfaceKHR(instance, surface, nullptr);
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
	prepareDraw(*cmd.attributes, *cmd.buffers, cmd.texture, cmd.primitiveType, cmd.cullMode);

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
	prepareDraw(*cmd.attributes, *cmd.buffers, cmd.texture, cmd.primitiveType, cmd.cullMode);

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

void Graphics::drawQuads(int start, int count, const VertexAttributes &attributes, const BufferBindings &buffers, graphics::Texture *texture)
{
	const int MAX_VERTICES_PER_DRAW = LOVE_UINT16_MAX;
	const int MAX_QUADS_PER_DRAW = MAX_VERTICES_PER_DRAW / 4;

	prepareDraw(attributes, buffers, texture, PRIMITIVE_TRIANGLES, CULL_BACK);

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

static VkRect2D computeScissor(const Rect &r, double bufferWidth, double bufferHeight, double dpiScale, VkSurfaceTransformFlagBitsKHR preTransform)
{
	double x = static_cast<double>(r.x) * dpiScale;
	double y = static_cast<double>(r.y) * dpiScale;
	double w = static_cast<double>(r.w) * dpiScale;
	double h = static_cast<double>(r.h) * dpiScale;

	double scissorX, scissorY, scissorW, scissorH;

	switch (preTransform)
	{
	case VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR:
		scissorX = bufferWidth - h - y;
		scissorY = x;
		scissorW = h;
		scissorH = w;
		break;
	case VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR:
		scissorX = bufferWidth - w - x;
		scissorY = bufferHeight - h - y;
		scissorW = w;
		scissorH = h;
		break;
	case VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR:
		scissorX = y;
		scissorY = bufferHeight - w - x;
		scissorW = h;
		scissorH = w;
		break;
	default:
		scissorX = x;
		scissorY = y;
		scissorW = w;
		scissorH = h;
		break;
	}

	VkRect2D scissor = { 
		{static_cast<int32_t>(scissorX), static_cast<int32_t>(scissorY)},
		{static_cast<uint32_t>(scissorW), static_cast<uint32_t>(scissorH)}
	};
	return scissor;
}

void Graphics::setScissor(const Rect &rect)
{
	flushBatchedDraws();

	VkRect2D scissor = computeScissor(rect, static_cast<double>(swapChainExtent.width), static_cast<double>(swapChainExtent.height), getCurrentDPIScale(), preTransform);
	vkCmdSetScissor(commandBuffers.at(currentFrame), 0, 1, &scissor);

	states.back().scissor = true;
	states.back().scissorRect = rect;
}

void Graphics::setScissor()
{
	flushBatchedDraws();

	states.back().scissor = false;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;

	vkCmdSetScissor(commandBuffers.at(currentFrame), 0, 1, &scissor);
}

void Graphics::setStencilMode(StencilAction action, CompareMode compare, int value, love::uint32 readmask, love::uint32 writemask)
{
	if (action != STENCIL_KEEP)
	{
		const auto& rts = states.back().renderTargets;
		auto dsTexture = rts.depthStencil.texture.get();

		if (!isRenderTargetActive() && !windowHasStencil)
			throw love::Exception("The window must have stenciling enabled to draw to the main screen's stencil buffer");
		else if (isRenderTargetActive() && (rts.temporaryRTFlags & TEMPORARY_RT_STENCIL) == 0 && (dsTexture == nullptr || !isPixelFormatStencil(dsTexture->getPixelFormat())))
			throw love::Exception("drawing to the stencil buffer with a render target active requires either stencil=true or a custom stencil-type to be used, in setRenderTarget");
	}

	flushBatchedDraws();

	vkCmdSetStencilWriteMask(commandBuffers.at(currentFrame), VK_STENCIL_FRONT_AND_BACK, writemask);
	
	vkCmdSetStencilCompareMask(commandBuffers.at(currentFrame), VK_STENCIL_FRONT_AND_BACK, readmask);
	vkCmdSetStencilReference(commandBuffers.at(currentFrame), VK_STENCIL_FRONT_AND_BACK, value);

	if (optionalDeviceExtensions.extendedDynamicState)
		vkCmdSetStencilOpEXT(
			commandBuffers.at(currentFrame),
			VK_STENCIL_FRONT_AND_BACK,
			VK_STENCIL_OP_KEEP, Vulkan::getStencilOp(action),
			VK_STENCIL_OP_KEEP, Vulkan::getCompareOp(getReversedCompareMode(compare)));

	states.back().stencil.action = action;
	states.back().stencil.compare = compare;
	states.back().stencil.value = value;
	states.back().stencil.readMask = readmask;
	states.back().stencil.writeMask = writemask;
}

void Graphics::setDepthMode(CompareMode compare, bool write)
{
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

	if (usage & PIXELFORMATUSAGE_LINEAR)
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

graphics::Shader *Graphics::newShaderInternal(StrongRef<love::graphics::ShaderStage> stages[SHADERSTAGE_MAX_ENUM])
{
	return new Shader(stages);
}

graphics::StreamBuffer *Graphics::newStreamBuffer(BufferUsage type, size_t size)
{
	return new StreamBuffer(this, type, size);
}

bool Graphics::dispatch(love::graphics::Shader *shader, int x, int y, int z)
{
	usedShadersInFrame.insert(computeShader);

	if (renderPassState.active)
		endRenderPass();

	vkCmdBindPipeline(commandBuffers.at(currentFrame), VK_PIPELINE_BIND_POINT_COMPUTE, computeShader->getComputePipeline());

	computeShader->cmdPushDescriptorSets(commandBuffers.at(currentFrame), VK_PIPELINE_BIND_POINT_COMPUTE);

	// TODO: does this need any layout transitions?
	vkCmdDispatch(commandBuffers.at(currentFrame), (uint32) x, (uint32) y, (uint32) z);

	return true;
}

bool Graphics::dispatch(love::graphics::Shader *shader, love::graphics::Buffer *indirectargs, size_t argsoffset)
{
	usedShadersInFrame.insert(computeShader);

	if (renderPassState.active)
		endRenderPass();

	vkCmdBindPipeline(commandBuffers.at(currentFrame), VK_PIPELINE_BIND_POINT_COMPUTE, computeShader->getComputePipeline());

	computeShader->cmdPushDescriptorSets(commandBuffers.at(currentFrame), VK_PIPELINE_BIND_POINT_COMPUTE);

	// TODO: does this need any layout transitions?
	vkCmdDispatchIndirect(commandBuffers.at(currentFrame), (VkBuffer) indirectargs->getHandle(), argsoffset);

	return true;
}

Matrix4 Graphics::computeDeviceProjection(const Matrix4 &projection, bool rendertotexture) const
{
	uint32 flags = DEVICE_PROJECTION_DEFAULT;
	return calculateDeviceProjection(projection, flags);
}

void Graphics::setRenderTargetsInternal(const RenderTargets &rts, int pixelw, int pixelh, bool hasSRGBtexture)
{
	if (renderPassState.active)
		endRenderPass();

	bool isWindow = rts.getFirstTarget().texture == nullptr;
	if (isWindow)
		setDefaultRenderPass();
	else
		setRenderPass(rts, pixelw, pixelh, hasSRGBtexture);
}

// END IMPLEMENTATION OVERRIDDEN FUNCTIONS

void Graphics::initDynamicState()
{
	if (states.back().scissor)
		setScissor(states.back().scissorRect);
	else
		setScissor();

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

void Graphics::beginFrame()
{
	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	if (frameCounter >= USAGES_POLL_INTERVAL)
	{
		cleanupUnusedObjects();
		frameCounter = 0;
	}

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

	for (auto &readbackCallback : readbackCallbacks.at(currentFrame))
		readbackCallback();
	readbackCallbacks.at(currentFrame).clear();

	for (auto &cleanUpFn : cleanUpFunctions.at(currentFrame))
		cleanUpFn();
	cleanUpFunctions.at(currentFrame).clear();

	startRecordingGraphicsCommands();

	Vulkan::cmdTransitionImageLayout(
		commandBuffers.at(currentFrame),
		swapChainImages[imageIndex],
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	if (transitionColorDepthLayouts)
	{
		Vulkan::cmdTransitionImageLayout(
			commandBuffers.at(currentFrame),
			depthImage,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		if (colorImage)
			Vulkan::cmdTransitionImageLayout(
				commandBuffers.at(currentFrame),
				colorImage,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		transitionColorDepthLayouts = false;
	}

	Vulkan::resetShaderSwitches();

	for (const auto shader : usedShadersInFrame)
		shader->newFrame();
	usedShadersInFrame.clear();
}

void Graphics::startRecordingGraphicsCommands()
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	if (vkBeginCommandBuffer(commandBuffers.at(currentFrame), &beginInfo) != VK_SUCCESS)
		throw love::Exception("failed to begin recording command buffer");

	initDynamicState();

	setDefaultRenderPass();
}

void Graphics::endRecordingGraphicsCommands() {
	if (renderPassState.active)
		endRenderPass();

	if (vkEndCommandBuffer(commandBuffers.at(currentFrame)) != VK_SUCCESS)
		throw love::Exception("failed to record command buffer");
}

const VkDeviceSize Graphics::getMinUniformBufferOffsetAlignment() const
{
	return minUniformBufferOffsetAlignment;
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
	data.projectionMatrix = displayRotation * getDeviceProjection();

	// The normal matrix is the transpose of the inverse of the rotation portion
	// (top-left 3x3) of the transform matrix.
	{
		Matrix3 normalmatrix = Matrix3(data.transformMatrix).transposedInverse();
		const float *e = normalmatrix.getElements();
		for (int i = 0; i < 3; i++)
		{
			data.normalMatrix[i].x = e[i * 3 + 0];
			data.normalMatrix[i].y = e[i * 3 + 1];
			data.normalMatrix[i].z = e[i * 3 + 2];
			data.normalMatrix[i].w = 0.0f;
		}
	}

	// Store DPI scale in an unused component of another vector.
	data.normalMatrix[0].w = (float)getCurrentDPIScale();

	// Same with point size.
	data.normalMatrix[1].w = getPointSize();

	data.screenSizeParams.x = static_cast<float>(swapChainExtent.width);
	data.screenSizeParams.y = static_cast<float>(swapChainExtent.height);

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
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0)
		throw love::Exception("failed to find GPUs with Vulkan support");

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	std::multimap<int, VkPhysicalDevice> candidates;

	for (const auto &device : devices)
	{
		int score = rateDeviceSuitability(device);
		candidates.insert(std::make_pair(score, device));
	}

	if (candidates.rbegin()->first > 0)
		physicalDevice = candidates.rbegin()->second;
	else
		throw love::Exception("failed to find a suitable gpu");

	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);
	minUniformBufferOffsetAlignment = properties.limits.minUniformBufferOffsetAlignment;
	deviceApiVersion = properties.apiVersion;

	depthStencilFormat = findDepthFormat();
}

bool Graphics::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto &extension : availableExtensions)
		requiredExtensions.erase(extension.extensionName);

	return requiredExtensions.empty();
}

// if the score is nonzero then the device is suitable.
// A higher rating means generally better performance
// if the score is 0 the device is unsuitable
int Graphics::rateDeviceSuitability(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	int score = 1;

	// optional

	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		score += 1000;
	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
		score += 100;
	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
		score += 10;

	// definitely needed

	QueueFamilyIndices indices = findQueueFamilies(device);
	if (!indices.isComplete())
		score = 0;

	bool extensionsSupported = checkDeviceExtensionSupport(device);
	if (!extensionsSupported)
		score = 0;

	if (extensionsSupported)
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

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (presentSupport)
			indices.presentFamily = i;

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

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
		throw love::Exception("failed to create logical device");

	volkLoadDevice(device);

	vkGetDeviceQueue(device, indices.graphicsFamily.value, 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value, 0, &presentQueue);
}

void Graphics::createPipelineCache()
{
	VkPipelineCacheCreateInfo cacheInfo{};
	cacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

	if (vkCreatePipelineCache(device, &cacheInfo, nullptr, &pipelineCache) != VK_SUCCESS)
		throw love::Exception("could not create pipeline cache");
}

void Graphics::initVMA()
{
	VmaAllocatorCreateInfo allocatorCreateInfo = {};
	allocatorCreateInfo.vulkanApiVersion = deviceApiVersion;
	allocatorCreateInfo.physicalDevice = physicalDevice;
	allocatorCreateInfo.device = device;
	allocatorCreateInfo.instance = instance;

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

	if (vmaCreateAllocator(&allocatorCreateInfo, &vmaAllocator) != VK_SUCCESS)
		throw love::Exception("failed to create vma allocator");
}

void Graphics::createSurface()
{
	auto window = Module::getInstance<love::window::Window>(M_WINDOW);
	const void *handle = window->getHandle();
	if (SDL_Vulkan_CreateSurface((SDL_Window*)handle, instance, &surface) != SDL_TRUE)
		throw love::Exception("failed to create window surface");
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

	if ((swapChainSupport.capabilities.currentTransform & VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR) ||
		(swapChainSupport.capabilities.currentTransform & VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR))
	{
		uint32_t width, height;
		width = extent.width;
		height = extent.height;
		extent.width = height;
		extent.height = width;
	}

	auto currentTransform = swapChainSupport.capabilities.currentTransform;
	constexpr float PI = 3.14159265358979323846f;
	float angle = 0.0f;
	if (currentTransform & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		angle = 0.0f;
	else if (currentTransform & VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR)
		angle = -PI / 2.0f;
	else if (currentTransform & VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR)
		angle = -PI;
	else if (currentTransform & VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR)
		angle = -3.0f * PI / 2.0f;

	float data[] = {
		cosf(angle), -sinf(angle), 0.0f, 0.0f,
		sinf(angle), cosf(angle), 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};
	displayRotation = Matrix4(data);

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

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = chooseCompositeAlpha(swapChainSupport.capabilities);
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
		throw love::Exception("failed to create swap chain");

	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
	preTransform = swapChainSupport.capabilities.currentTransform;
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

		if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews.at(i)) != VK_SUCCESS)
			throw love::Exception("failed to create image views");
	}
}

void Graphics::createScreenshotCallbackBuffers()
{
	screenshotReadbackBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
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
			&screenshotReadbackBuffers.at(i).buffer,
			&screenshotReadbackBuffers.at(i).allocation,
			&screenshotReadbackBuffers.at(i).allocationInfo);

		if (result != VK_SUCCESS)
			throw love::Exception("failed to create screenshot readback buffer");

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		imageInfo.extent = {
			swapChainExtent.width,
			swapChainExtent.height,
			1
		};
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaAllocationCreateInfo imageAllocCreateInfo{};

		result = vmaCreateImage(
			vmaAllocator, 
			&imageInfo, 
			&imageAllocCreateInfo, 
			&screenshotReadbackBuffers.at(i).image, 
			&screenshotReadbackBuffers.at(i).imageAllocation, 
			nullptr);

		if (result != VK_SUCCESS)
			throw love::Exception("failed to create screenshot readback image");
	}
}

VkFramebuffer Graphics::createFramebuffer(FramebufferConfiguration &configuration)
{
	std::vector<VkImageView> attachments;

	for (const auto &colorView : configuration.colorViews)
		attachments.push_back(colorView);

	if (configuration.staticData.depthView)
		attachments.push_back(configuration.staticData.depthView);

	if (configuration.staticData.resolveView)
		attachments.push_back(configuration.staticData.resolveView);

	VkFramebufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass = configuration.staticData.renderPass;
	createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	createInfo.pAttachments = attachments.data();
	createInfo.width = configuration.staticData.width;
	createInfo.height = configuration.staticData.height;
	createInfo.layers = 1;

	VkFramebuffer frameBuffer;
	if (vkCreateFramebuffer(device, &createInfo, nullptr, &frameBuffer) != VK_SUCCESS)
		throw love::Exception("failed to create framebuffer");
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

	std::vector<VkAttachmentDescription> attachments;
	std::vector<VkAttachmentReference> colorAttachmentRefs;

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
		colorDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachments.push_back(colorDescription);
	}

	subPass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
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
		depthStencilAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthStencilAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments.push_back(depthStencilAttachment);
	}

	VkAttachmentReference colorAttachmentResolveRef{};
	if (configuration.staticData.resolve)
	{
		colorAttachmentResolveRef.attachment = attachment++;
		colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		subPass.pResolveAttachments = &colorAttachmentResolveRef;

		VkAttachmentDescription colorAttachmentResolve{};
		colorAttachmentResolve.format = configuration.colorAttachments.at(0).format;
		colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachments.push_back(colorAttachmentResolve);
	}

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT;
	dependency.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkSubpassDependency readbackDependency{};
	readbackDependency.srcSubpass = 0;
	readbackDependency.dstSubpass = VK_SUBPASS_EXTERNAL;
	readbackDependency.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	readbackDependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	readbackDependency.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
	readbackDependency.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

	std::array<VkSubpassDependency, 2> dependencies = { dependency, readbackDependency };

	VkRenderPassCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	createInfo.pAttachments = attachments.data();
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subPass;
	createInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	createInfo.pDependencies = dependencies.data();

	VkRenderPass renderPass;
	if (vkCreateRenderPass(device, &createInfo, nullptr, &renderPass) != VK_SUCCESS)
		throw love::Exception("failed to create render pass");

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

	renderPassUsages[renderPass] = true;

	return renderPass;
}

void Graphics::createVulkanVertexFormat(
	VertexAttributes vertexAttributes,
	std::vector<VkVertexInputBindingDescription> &bindingDescriptions,
	std::vector<VkVertexInputAttributeDescription> &attributeDescriptions)
{
	std::set<uint32_t> usedBuffers;

	auto enableBits = vertexAttributes.enableBits;
	auto allBits = enableBits;

	bool usesColor = false;
	bool usesTexCoord = false;

	uint8_t highestBufferBinding = 0;

	uint32_t i = 0;
	while (allBits)
	{
		uint32 bit = 1u << i;
		if (enableBits & bit)
		{
			if (i == ATTRIB_TEXCOORD)
				usesTexCoord = true;
			if (i == ATTRIB_COLOR)
				usesColor = true;

			auto attrib = vertexAttributes.attribs[i];
			auto bufferBinding = attrib.bufferIndex;
			if (usedBuffers.find(bufferBinding) == usedBuffers.end())
			{
				usedBuffers.insert(bufferBinding);

				VkVertexInputBindingDescription bindingDescription{};
				bindingDescription.binding = bufferBinding;
				if (vertexAttributes.instanceBits & (1u << bufferBinding))
					bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
				else
					bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
				bindingDescription.stride = vertexAttributes.bufferLayouts[bufferBinding].stride;
				bindingDescriptions.push_back(bindingDescription);

				highestBufferBinding = std::max(highestBufferBinding, bufferBinding);
			}

			VkVertexInputAttributeDescription attributeDescription{};
			attributeDescription.location = i;
			attributeDescription.binding = bufferBinding;
			attributeDescription.offset = attrib.offsetFromVertex;
			attributeDescription.format = Vulkan::getVulkanVertexFormat(attrib.format);

			attributeDescriptions.push_back(attributeDescription);
		}

		i++;
		allBits >>= 1;
	}

	if (!usesTexCoord)
	{
		// FIXME: is there a case where gaps happen between buffer bindings?
		// then this doesn't work. We might need to enable null buffers again.
		const auto constantTexCoordBufferBinding = ++highestBufferBinding;

		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = constantTexCoordBufferBinding;
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		bindingDescription.stride = 0;	// no stride, will always read the same coord multiple times.
		bindingDescriptions.push_back(bindingDescription);

		VkVertexInputAttributeDescription attributeDescription{};
		attributeDescription.binding = constantTexCoordBufferBinding;
		attributeDescription.location = ATTRIB_TEXCOORD;
		attributeDescription.offset = 0;
		attributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions.push_back(attributeDescription);
	}

	if (!usesColor)
	{
		// FIXME: is there a case where gaps happen between buffer bindings?
		// then this doesn't work. We might need to enable null buffers again.
		const auto constantColorBufferBinding = ++highestBufferBinding;

		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = constantColorBufferBinding;
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		bindingDescription.stride = 0;	// no stride, will always read the same color multiple times.
		bindingDescriptions.push_back(bindingDescription);

		VkVertexInputAttributeDescription attributeDescription{};
		attributeDescription.binding = constantColorBufferBinding;
		attributeDescription.location = ATTRIB_COLOR;
		attributeDescription.offset = 0;
		attributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions.push_back(attributeDescription);
	}
}

void Graphics::prepareDraw(const VertexAttributes &attributes, const BufferBindings &buffers, graphics::Texture *texture, PrimitiveType primitiveType, CullMode cullmode)
{
	if (!renderPassState.active)
		startRenderPass();

	usedShadersInFrame.insert((dynamic_cast<Shader*>(Shader::current)));

	GraphicsPipelineConfiguration configuration{};

	configuration.renderPass = renderPassState.beginInfo.renderPass;
	configuration.vertexAttributes = attributes;
	configuration.shader = (Shader*)Shader::current;
	configuration.wireFrame = states.back().wireframe;
	configuration.blendState = states.back().blend;
	configuration.colorChannelMask = states.back().colorMask;
	configuration.msaaSamples = renderPassState.msaa;
	configuration.numColorAttachments = renderPassState.numColorAttachments;
	configuration.primitiveType = primitiveType;

	if (optionalDeviceExtensions.extendedDynamicState)
		vkCmdSetCullModeEXT(commandBuffers.at(currentFrame), Vulkan::getCullMode(cullmode));
	else
	{
		configuration.dynamicState.winding = states.back().winding;
		configuration.dynamicState.depthState.compare = states.back().depthTest;
		configuration.dynamicState.depthState.write = states.back().depthWrite;
		configuration.dynamicState.stencilAction = states.back().stencil.action;
		configuration.dynamicState.stencilCompare = states.back().stencil.compare;
		configuration.dynamicState.cullmode = cullmode;
	}

	std::vector<VkBuffer> bufferVector;
	std::vector<VkDeviceSize> offsets;

	for (uint32_t i = 0; i < VertexAttributes::MAX; i++)
	{
		if (buffers.useBits & (1u << i))
		{
			bufferVector.push_back((VkBuffer)buffers.info[i].buffer->getHandle());
			offsets.push_back((VkDeviceSize)buffers.info[i].offset);
		}
	}

	if (!(attributes.enableBits & (1u << ATTRIB_TEXCOORD)))
	{
		bufferVector.push_back((VkBuffer)defaultConstantTexCoord->getHandle());
		offsets.push_back((VkDeviceSize)0);
	}

	if (!(attributes.enableBits & (1u << ATTRIB_COLOR)))
	{
		bufferVector.push_back((VkBuffer)defaultConstantColor->getHandle());
		offsets.push_back((VkDeviceSize)0);
	}

	if (texture == nullptr)
		configuration.shader->setMainTex(defaultTexture);
	else
		configuration.shader->setMainTex(texture);

	ensureGraphicsPipelineConfiguration(configuration);

	configuration.shader->cmdPushDescriptorSets(commandBuffers.at(currentFrame), VK_PIPELINE_BIND_POINT_GRAPHICS);
	vkCmdBindVertexBuffers(commandBuffers.at(currentFrame), 0, static_cast<uint32_t>(bufferVector.size()), bufferVector.data(), offsets.data());
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
	renderPassState.transitionImages.clear();

	RenderPassConfiguration renderPassConfiguration{};
	renderPassConfiguration.colorAttachments.push_back({ swapChainImageFormat, VK_ATTACHMENT_LOAD_OP_LOAD, msaaSamples });
	renderPassConfiguration.staticData.depthStencilAttachment = { depthStencilFormat, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_LOAD_OP_LOAD, msaaSamples };
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
		framebufferConfiguration.colorViews.push_back(swapChainImageViews.at(imageIndex));
		framebufferConfiguration.staticData.resolveView = VK_NULL_HANDLE;
	}
	else
	{
		framebufferConfiguration.colorViews.push_back(colorImageView);
		framebufferConfiguration.staticData.resolveView = swapChainImageViews.at(imageIndex);
	}

	renderPassState.renderPassConfiguration = std::move(renderPassConfiguration);
	renderPassState.framebufferConfiguration = std::move(framebufferConfiguration);

	if (renderPassState.windowClearRequested)
		clear(renderPassState.mainWindowClearColorValue, renderPassState.mainWindowClearStencilValue, renderPassState.mainWindowClearDepthValue);
}

void Graphics::setRenderPass(const RenderTargets &rts, int pixelw, int pixelh, bool hasSRGBtexture)
{
	// fixme: hasSRGBtexture
	RenderPassConfiguration renderPassConfiguration{};
	for (const auto &color : rts.colors)
		renderPassConfiguration.colorAttachments.push_back({ 
			Vulkan::getTextureFormat(color.texture->getPixelFormat()).internalFormat,
			VK_ATTACHMENT_LOAD_OP_LOAD,
			dynamic_cast<Texture*>(color.texture)->getMsaaSamples() });
	if (rts.depthStencil.texture != nullptr)
		renderPassConfiguration.staticData.depthStencilAttachment = {
			Vulkan::getTextureFormat(rts.depthStencil.texture->getPixelFormat()).internalFormat,
			VK_ATTACHMENT_LOAD_OP_LOAD,
			VK_ATTACHMENT_LOAD_OP_LOAD,
			dynamic_cast<Texture*>(rts.depthStencil.texture)->getMsaaSamples() };

	FramebufferConfiguration configuration{};

	std::vector<VkImage> transitionImages;

	for (const auto &color : rts.colors)
	{
		configuration.colorViews.push_back(dynamic_cast<Texture*>(color.texture)->getRenderTargetView(color.mipmap, color.slice));
		transitionImages.push_back((VkImage) color.texture->getHandle());
	}
	if (rts.depthStencil.texture != nullptr)
		configuration.staticData.depthView = dynamic_cast<Texture*>(rts.depthStencil.texture)->getRenderTargetView(rts.depthStencil.mipmap, rts.depthStencil.slice);

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
	renderPassState.msaa = VK_SAMPLE_COUNT_1_BIT;
	renderPassState.numColorAttachments = static_cast<uint32_t>(rts.colors.size());
	renderPassState.transitionImages = std::move(transitionImages);
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

	for (const auto &image : renderPassState.transitionImages)
		Vulkan::cmdTransitionImageLayout(commandBuffers.at(currentFrame), image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	vkCmdBeginRenderPass(commandBuffers.at(currentFrame), &renderPassState.beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Graphics::endRenderPass()
{
	renderPassState.active = false;

	vkCmdEndRenderPass(commandBuffers.at(currentFrame));

	for (const auto &image : renderPassState.transitionImages)
		Vulkan::cmdTransitionImageLayout(commandBuffers.at(currentFrame), image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

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
	samplerInfo.anisotropyEnable = VK_TRUE;
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
		samplerInfo.compareOp = Vulkan::getCompareOp(samplerState.depthSampleMode.value);
	}
	else
	{
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	}
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.mipmapMode = Vulkan::getMipMapMode(samplerState.mipmapFilter);
	samplerInfo.mipLodBias = samplerState.lodBias;
	samplerInfo.minLod = static_cast<float>(samplerState.minLod);
	samplerInfo.maxLod = static_cast<float>(samplerState.maxLod);

	VkSampler sampler;
	if (vkCreateSampler(device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
		throw love::Exception("failed to create sampler");

	return sampler;
}

template<typename Configuration, typename ObjectHandle, typename ConfigurationHasher, typename Deleter>
static void eraseUnusedObjects(
	std::unordered_map<Configuration, ObjectHandle, ConfigurationHasher> &objects,
	std::unordered_map<ObjectHandle, bool> &usages,
	Deleter deleter,
	VkDevice device)
{
	std::vector<Configuration> deletionKeys;

	for (const auto &entry : objects)
	{
		if (!usages[entry.second])
		{
			deletionKeys.push_back(entry.first);
			usages.erase(entry.second);
			deleter(device, entry.second, nullptr);
		}
		else
			usages[entry.second] = false;
	}

	for (const auto &key : deletionKeys)
		objects.erase(key);
}

void Graphics::cleanupUnusedObjects()
{
	eraseUnusedObjects(renderPasses, renderPassUsages, vkDestroyRenderPass, device);
	eraseUnusedObjects(framebuffers, framebufferUsages, vkDestroyFramebuffer, device);
	eraseUnusedObjects(graphicsPipelines, pipelineUsages, vkDestroyPipeline, device);
}

void Graphics::requestSwapchainRecreation()
{
	if (swapChain != VK_NULL_HANDLE)
	{
		swapChainRecreationRequested = true;
	}
}

void Graphics::setComputeShader(Shader *shader)
{
	computeShader = shader;
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

VkPipeline Graphics::createGraphicsPipeline(GraphicsPipelineConfiguration &configuration)
{
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

	auto &shaderStages = configuration.shader->getShaderStages();

	std::vector<VkVertexInputBindingDescription> bindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	createVulkanVertexFormat(configuration.vertexAttributes, bindingDescriptions, attributeDescriptions);

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
		rasterizer.cullMode = Vulkan::getCullMode(configuration.dynamicState.cullmode);
		rasterizer.frontFace = Vulkan::getFrontFace(configuration.dynamicState.winding);
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
		depthStencil.depthWriteEnable = Vulkan::getBool(configuration.dynamicState.depthState.write);
		depthStencil.depthCompareOp = Vulkan::getCompareOp(configuration.dynamicState.depthState.compare);
	}
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f;
	depthStencil.maxDepthBounds = 1.0f;

	depthStencil.stencilTestEnable = VK_TRUE;

	if (!optionalDeviceExtensions.extendedDynamicState)
	{
		depthStencil.front.failOp = VK_STENCIL_OP_KEEP;
		depthStencil.front.passOp = Vulkan::getStencilOp(configuration.dynamicState.stencilAction);
		depthStencil.front.depthFailOp = VK_STENCIL_OP_KEEP;
		depthStencil.front.compareOp = Vulkan::getCompareOp(getReversedCompareMode(configuration.dynamicState.stencilCompare));

		depthStencil.back.failOp = VK_STENCIL_OP_KEEP;
		depthStencil.back.passOp = Vulkan::getStencilOp(configuration.dynamicState.stencilAction);
		depthStencil.back.depthFailOp = VK_STENCIL_OP_KEEP;
		depthStencil.back.compareOp = Vulkan::getCompareOp(getReversedCompareMode(configuration.dynamicState.stencilCompare));
	}

	pipelineInfo.pDepthStencilState = &depthStencil;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = Vulkan::getColorMask(configuration.colorChannelMask);
	colorBlendAttachment.blendEnable = Vulkan::getBool(configuration.blendState.enable);
	colorBlendAttachment.srcColorBlendFactor = Vulkan::getBlendFactor(configuration.blendState.srcFactorRGB);
	colorBlendAttachment.dstColorBlendFactor = Vulkan::getBlendFactor(configuration.blendState.dstFactorRGB);
	colorBlendAttachment.colorBlendOp = Vulkan::getBlendOp(configuration.blendState.operationRGB);
	colorBlendAttachment.srcAlphaBlendFactor = Vulkan::getBlendFactor(configuration.blendState.srcFactorA);
	colorBlendAttachment.dstAlphaBlendFactor = Vulkan::getBlendFactor(configuration.blendState.dstFactorA);
	colorBlendAttachment.alphaBlendOp = Vulkan::getBlendOp(configuration.blendState.operationA);

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(configuration.numColorAttachments, colorBlendAttachment);

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
	pipelineInfo.layout = configuration.shader->getGraphicsPipelineLayout();
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;
	pipelineInfo.renderPass = configuration.renderPass;

	VkPipeline graphicsPipeline;
	if (vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		throw love::Exception("failed to create graphics pipeline");
	return graphicsPipeline;
}

void Graphics::ensureGraphicsPipelineConfiguration(GraphicsPipelineConfiguration &configuration) {
	auto it = graphicsPipelines.find(configuration);
	if (it != graphicsPipelines.end())
	{
		if (it->second != renderPassState.pipeline)
		{
			vkCmdBindPipeline(commandBuffers.at(currentFrame), VK_PIPELINE_BIND_POINT_GRAPHICS, it->second);
			renderPassState.pipeline = it->second;
			pipelineUsages[it->second] = true;
		}
	}
	else
	{
		VkPipeline pipeline = createGraphicsPipeline(configuration);
		graphicsPipelines.insert({configuration, pipeline});
		vkCmdBindPipeline(commandBuffers.at(currentFrame), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		renderPassState.pipeline = pipeline;
		pipelineUsages[pipeline] = true;
	}
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

		if (vmaCreateImage(vmaAllocator, &imageInfo, &allocationInfo, &colorImage, &colorImageAllocation, nullptr))
			throw love::Exception("failed to create color image");

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

		if (vkCreateImageView(device, &imageViewInfo, nullptr, &colorImageView) != VK_SUCCESS)
			throw love::Exception("failed to create color image view");
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

	if (vmaCreateImage(vmaAllocator, &imageInfo, &allocationInfo, &depthImage, &depthImageAllocation, nullptr) != VK_SUCCESS)
		throw love::Exception("failed to create depth image");

	VkImageViewCreateInfo imageViewInfo{};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.image = depthImage;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = depthStencilFormat;
	imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	if (windowHasStencil)
		imageViewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(device, &imageViewInfo, nullptr, &depthImageView) != VK_SUCCESS)
		throw love::Exception("failed to create depth image view");
}

void Graphics::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
		throw love::Exception("failed to create command pool");
}

void Graphics::createCommandBuffers()
{
	commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		throw love::Exception("failed to allocate command buffers");
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
			throw love::Exception("failed to create synchronization objects for a frame!");
}

void Graphics::createDefaultTexture()
{
	Texture::Settings settings;
	defaultTexture.set(newTexture(settings, nullptr), Acquire::NORETAIN);

	uint8_t whitePixels[] = {255, 255, 255, 255};
	defaultTexture->replacePixels(whitePixels, sizeof(whitePixels), 0, 0, { 0, 0, 1, 1 }, false);
}

void Graphics::cleanup()
{
	for (auto &cleanUpFns : cleanUpFunctions)
		for (auto &cleanUpFn : cleanUpFns)
			cleanUpFn();
	cleanUpFunctions.clear();

	vmaDestroyAllocator(vmaAllocator);
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}

	vkFreeCommandBuffers(device, commandPool, MAX_FRAMES_IN_FLIGHT, commandBuffers.data());

	for (auto const &p : samplers)
		vkDestroySampler(device, p.second, nullptr);
	samplers.clear();

	for (const auto &entry : renderPasses)
		vkDestroyRenderPass(device, entry.second, nullptr);
	renderPasses.clear();

	for (const auto &entry : framebuffers)
		vkDestroyFramebuffer(device, entry.second, nullptr);
	framebuffers.clear();
		
	for (auto const &p : graphicsPipelines)
		vkDestroyPipeline(device, p.second, nullptr);
	graphicsPipelines.clear();

	vkDestroyCommandPool(device, commandPool, nullptr);
	vkDestroyPipelineCache(device, pipelineCache, nullptr);
	vkDestroyDevice(device, nullptr);
}

void Graphics::cleanupSwapChain()
{
	for (const auto &readbackBuffer : screenshotReadbackBuffers)
	{
		vmaDestroyBuffer(vmaAllocator, readbackBuffer.buffer, readbackBuffer.allocation);
		vmaDestroyImage(vmaAllocator, readbackBuffer.image, readbackBuffer.imageAllocation);
	}
	if (colorImage)
	{
		vkDestroyImageView(device, colorImageView, nullptr);
		vmaDestroyImage(vmaAllocator, colorImage, colorImageAllocation);
	}
	vkDestroyImageView(device, depthImageView, nullptr);
	vmaDestroyImage(vmaAllocator, depthImage, depthImageAllocation);
	for (const auto &swapChainImageView : swapChainImageViews)
		vkDestroyImageView(device, swapChainImageView, nullptr);
	swapChainImageViews.clear();
	vkDestroySwapchainKHR(device, swapChain, nullptr);

	swapChain = VK_NULL_HANDLE;
}

void Graphics::recreateSwapChain()
{
	vkDeviceWaitIdle(device);

	cleanupSwapChain();

	createSwapChain();
	createImageViews();
	createScreenshotCallbackBuffers();
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
		printf("Cannot create Vulkan renderer: %s\n", e.what());
	}

	return instance;
}

} // vulkan
} // graphics
} // love
