#include "Graphics.h"
#include "Buffer.h"
#include "GraphicsReadback.h"
#include "SDL_vulkan.h"
#include "window/Window.h"
#include "common/Exception.h"
#include "Shader.h"
#include "graphics/Texture.h"
#include "Vulkan.h"
#include "common/version.h"
#include "common/pixelformat.h"

#include <algorithm>
#include <vector>
#include <cstring>
#include <set>
#include <fstream>
#include <sstream>
#include <iostream>
#include <array>


namespace love
{
namespace graphics
{
namespace vulkan
{

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

constexpr uint32_t vulkanApiVersion = VK_API_VERSION_1_0;

const char *Graphics::getName() const
{
	return "love.graphics.vulkan";
}

const VkDevice Graphics::getDevice() const
{
	return device;
}

const VkPhysicalDevice Graphics::getPhysicalDevice() const
{
	return physicalDevice;
}

const VmaAllocator Graphics::getVmaAllocator() const
{
	return vmaAllocator;
}

Graphics::~Graphics()
{
	// We already cleaned those up by clearing out batchedDrawBuffers.
	// We set them to nullptr here so the base class doesn't crash
	// when it tries to free this.
	batchedDrawState.vb[0] = nullptr;
	batchedDrawState.vb[1] = nullptr;
	batchedDrawState.indexBuffer = nullptr;
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
	if (!renderPassState.active)
		startRenderPass();

	VkClearAttachment attachment{};

	if (color.hasValue)
	{
		attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		attachment.clearValue.color.float32[0] = static_cast<float>(color.value.r);
		attachment.clearValue.color.float32[1] = static_cast<float>(color.value.g);
		attachment.clearValue.color.float32[2] = static_cast<float>(color.value.b);
		attachment.clearValue.color.float32[3] = static_cast<float>(color.value.a);
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

void Graphics::clear(const std::vector<OptionalColorD> &colors, OptionalInt stencil, OptionalDouble depth)
{
	if (!renderPassState.active)
		startRenderPass();

	std::vector<VkClearAttachment> attachments;
	for (const auto &color : colors)
	{
		VkClearAttachment attachment{};
		if (color.hasValue)
		{
			attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			attachment.clearValue.color.float32[0] = static_cast<float>(color.value.r);
			attachment.clearValue.color.float32[1] = static_cast<float>(color.value.g);
			attachment.clearValue.color.float32[2] = static_cast<float>(color.value.b);
			attachment.clearValue.color.float32[3] = static_cast<float>(color.value.a);
		}
		attachments.push_back(attachment);
	}

	VkClearAttachment depthStencilAttachment{};

	if (stencil.hasValue)
	{
		depthStencilAttachment.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
		depthStencilAttachment.clearValue.depthStencil.stencil = static_cast<uint32_t>(stencil.value);
	}
	if (depth.hasValue)
	{
		depthStencilAttachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		depthStencilAttachment.clearValue.depthStencil.depth = static_cast<float>(depth.value);
	}

	attachments.push_back(depthStencilAttachment);

	VkClearRect rect{};
	rect.layerCount = 1;
	rect.rect.extent.width = static_cast<uint32_t>(renderPassState.width);
	rect.rect.extent.height = static_cast<uint32_t>(renderPassState.height);

	vkCmdClearAttachments(commandBuffers[currentFrame], static_cast<uint32_t>(attachments.size()), attachments.data(), 1, &rect);
}

void Graphics::submitGpuCommands(bool present)
{
	flushBatchedDraws();

	endRecordingGraphicsCommands(present);

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

	if (present)
	{
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(device, 1, &inFlightFences[currentFrame]);
		fence = inFlightFences[currentFrame];
	}

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, fence) != VK_SUCCESS)
		throw love::Exception("failed to submit draw command buffer");
	
	if (!present)
	{
		vkQueueWaitIdle(graphicsQueue);

		for (auto &callbacks : readbackCallbacks)
		{
			for (const auto &callback : callbacks)
				callback();
			callbacks.clear();
		}

		startRecordingGraphicsCommands(false);
	}
}

void Graphics::present(void *screenshotCallbackdata)
{
	if (!isActive())
		return;

	submitGpuCommands(true);

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	VkSemaphore waitSemaphores[] = { renderFinishedSemaphores.at(currentFrame) };

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = waitSemaphores;

	VkSwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
	{
		framebufferResized = false;
		recreateSwapChain();
	}
	else if (result != VK_SUCCESS)
		throw love::Exception("failed to present swap chain image");

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	beginFrame();

	updatedBatchedDrawBuffers();
}

void Graphics::setViewportSize(int width, int height, int pixelwidth, int pixelheight)
{
	this->width = width;
	this->height = height;
	this->pixelWidth = pixelwidth;
	this->pixelHeight = pixelheight;

	resetProjection();
}

bool Graphics::setMode(void *context, int width, int height, int pixelwidth, int pixelheight, bool windowhasstencil, int msaa)
{
	requestedMsaa = msaa;

	cleanUpFunctions.clear();
	cleanUpFunctions.resize(MAX_FRAMES_IN_FLIGHT);

	readbackCallbacks.clear();
	readbackCallbacks.resize(MAX_FRAMES_IN_FLIGHT);

	createVulkanInstance();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	initVMA();
	initCapabilities();
	createSwapChain();
	createImageViews();
	createSyncObjects();
	createColorResources();
	createDepthResources();
	createDefaultRenderPass();
	createDefaultFramebuffers();
	createCommandPool();
	createCommandBuffers();

	float whiteColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	batchedDrawBuffers.clear();
	batchedDrawBuffers.reserve(MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		batchedDrawBuffers.emplace_back();
		// Initial sizes that should be good enough for most cases. It will
		// resize to fit if needed, later.
		batchedDrawBuffers[i].vertexBuffer1 = new StreamBuffer(this, BUFFERUSAGE_VERTEX, 1024 * 1024 * 1);
		batchedDrawBuffers[i].vertexBuffer2 = new StreamBuffer(this, BUFFERUSAGE_VERTEX, 256 * 1024 * 1);
		batchedDrawBuffers[i].indexBuffer = new StreamBuffer(this, BUFFERUSAGE_INDEX, sizeof(uint16) * LOVE_UINT16_MAX);

		// sometimes the VertexColor is not set, so we manually adjust it to white color
		batchedDrawBuffers[i].constantColorBuffer = new StreamBuffer(this, BUFFERUSAGE_VERTEX, sizeof(whiteColor));
		auto mapInfo = batchedDrawBuffers[i].constantColorBuffer->map(sizeof(whiteColor));
		memcpy(mapInfo.data, whiteColor, sizeof(whiteColor));
		batchedDrawBuffers[i].constantColorBuffer->unmap(sizeof(whiteColor));
		batchedDrawBuffers[i].constantColorBuffer->markUsed(sizeof(whiteColor));
	}
	updatedBatchedDrawBuffers();

	beginFrame();

	createDefaultTexture();
	createDefaultShaders();
	Shader::current = Shader::standardShaders[graphics::Shader::StandardShader::STANDARD_DEFAULT];
	createQuadIndexBuffer();

	restoreState(states.back());

	setViewportSize(width, height, pixelwidth, pixelheight);

	Vulkan::resetShaderSwitches();

	currentFrame = 0;

	created = true;
	drawCalls = 0;
	drawCallsBatched = 0;

	return true;
}

void Graphics::initCapabilities()
{
	// todo
	capabilities.features[FEATURE_MULTI_RENDER_TARGET_FORMATS] = false;
	capabilities.features[FEATURE_CLAMP_ZERO] = false;
	capabilities.features[FEATURE_CLAMP_ONE] = false;
	capabilities.features[FEATURE_BLEND_MINMAX] = false;
	capabilities.features[FEATURE_LIGHTEN] = false;
	capabilities.features[FEATURE_FULL_NPOT] = false;
	capabilities.features[FEATURE_PIXEL_SHADER_HIGHP] = true;
	capabilities.features[FEATURE_SHADER_DERIVATIVES] = true;
	capabilities.features[FEATURE_GLSL3] = true;
	capabilities.features[FEATURE_GLSL4] = true;
	capabilities.features[FEATURE_INSTANCING] = true;
	capabilities.features[FEATURE_TEXEL_BUFFER] = false;
	capabilities.features[FEATURE_INDEX_BUFFER_32BIT] = true;
	capabilities.features[FEATURE_COPY_BUFFER] = true;
	capabilities.features[FEATURE_COPY_BUFFER_TO_TEXTURE] = true;
	capabilities.features[FEATURE_COPY_TEXTURE_TO_BUFFER] = true;
	capabilities.features[FEATURE_COPY_RENDER_TARGET_TO_BUFFER] = true;
	static_assert(FEATURE_MAX_ENUM == 17, "Graphics::initCapabilities must be updated when adding a new graphics feature!");

	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);

	capabilities.limits[LIMIT_POINT_SIZE] = properties.limits.pointSizeRange[1];
	capabilities.limits[LIMIT_TEXTURE_SIZE] = properties.limits.maxImageDimension2D;
	capabilities.limits[LIMIT_TEXTURE_LAYERS] = properties.limits.maxImageArrayLayers;
	capabilities.limits[LIMIT_VOLUME_TEXTURE_SIZE] = properties.limits.maxImageDimension3D;
	capabilities.limits[LIMIT_CUBE_TEXTURE_SIZE] = properties.limits.maxImageDimensionCube;
	capabilities.limits[LIMIT_TEXEL_BUFFER_SIZE] = properties.limits.maxTexelBufferElements;	// ?
	capabilities.limits[LIMIT_SHADER_STORAGE_BUFFER_SIZE] = properties.limits.maxStorageBufferRange;	// ?
	capabilities.limits[LIMIT_THREADGROUPS_X] = properties.limits.maxComputeWorkGroupSize[0];	// this is correct?
	capabilities.limits[LIMIT_THREADGROUPS_Y] = properties.limits.maxComputeWorkGroupSize[1];
	capabilities.limits[LIMIT_THREADGROUPS_Z] = properties.limits.maxComputeWorkGroupSize[2];
	capabilities.limits[LIMIT_RENDER_TARGETS] = properties.limits.maxColorAttachments;
	capabilities.limits[LIMIT_TEXTURE_MSAA] = 1;	// todo
	capabilities.limits[LIMIT_ANISOTROPY] = properties.limits.maxSamplerAnisotropy;
	static_assert(LIMIT_MAX_ENUM == 13, "Graphics::initCapabilities must be updated when adding a new system limit!");

	capabilities.textureTypes[TEXTURE_2D] = true;
	capabilities.textureTypes[TEXTURE_2D_ARRAY] = true;
	capabilities.textureTypes[TEXTURE_VOLUME] = false;
	capabilities.textureTypes[TEXTURE_CUBE] = true;
}

void Graphics::getAPIStats(int &shaderswitches) const
{
	shaderswitches = static_cast<int>(Vulkan::getNumShaderSwitches());
}

void Graphics::unSetMode()
{
	created = false;
	vkDeviceWaitIdle(device);
	Volatile::unloadAll();
	cleanup();
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

	if (optionalDeviceFeatures.extendedDynamicState)
		ext.vkCmdSetFrontFaceEXT(
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
	info.device = deviceProperties.deviceName;
	info.vendor = Vulkan::getVendorName(deviceProperties.vendorID);
	info.version = Vulkan::getVulkanApiVersion(deviceProperties.apiVersion);

	std::stringstream ss;
	ss << "Vulkan( ";
	if (optionalDeviceFeatures.extendedDynamicState)
		ss << VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME << " ";
	if (optionalDeviceFeatures.pushDescriptor)
		ss << VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME << " ";
	ss << ")";

	info.name = ss.str();

	return info;
}

void Graphics::draw(const DrawCommand &cmd)
{
	prepareDraw(*cmd.attributes, *cmd.buffers, cmd.texture, cmd.primitiveType, cmd.cullMode);

	vkCmdDraw(commandBuffers.at(currentFrame), static_cast<uint32_t>(cmd.vertexCount), static_cast<uint32_t>(cmd.instanceCount), static_cast<uint32_t>(cmd.vertexStart), 0);
	drawCalls++;
}

void Graphics::draw(const DrawIndexedCommand &cmd)
{
	prepareDraw(*cmd.attributes, *cmd.buffers, cmd.texture, cmd.primitiveType, cmd.cullMode);

	vkCmdBindIndexBuffer(commandBuffers.at(currentFrame), (VkBuffer)cmd.indexBuffer->getHandle(), static_cast<VkDeviceSize>(cmd.indexBufferOffset), Vulkan::getVulkanIndexBufferType(cmd.indexType));
	vkCmdDrawIndexed(commandBuffers.at(currentFrame), static_cast<uint32_t>(cmd.indexCount), static_cast<uint32_t>(cmd.instanceCount), 0, 0, 0);
	drawCalls++;
}

void Graphics::drawQuads(int start, int count, const VertexAttributes &attributes, const BufferBindings &buffers, graphics::Texture *texture)
{
	const int MAX_VERTICES_PER_DRAW = LOVE_UINT16_MAX;
	const int MAX_QUADS_PER_DRAW = MAX_VERTICES_PER_DRAW / 4;

	prepareDraw(attributes, buffers, texture, PRIMITIVE_TRIANGLES, CULL_BACK);

	vkCmdBindIndexBuffer(commandBuffers.at(currentFrame), (VkBuffer)quadIndexBuffer->getHandle(), 0, Vulkan::getVulkanIndexBufferType(INDEX_UINT16));

	int baseVertex = start * 4;

	for (int quadindex = 0; quadindex < count; quadindex += MAX_QUADS_PER_DRAW)
	{
		int quadcount = std::min(MAX_QUADS_PER_DRAW, count - quadindex);

		vkCmdDrawIndexed(commandBuffers.at(currentFrame), static_cast<uint32_t>(quadcount * 6), 1, 0, baseVertex, 0);
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

	VkRect2D scissor = computeScissor(rect,
                                      static_cast<double>(swapChainExtent.width),
                                      static_cast<double>(swapChainExtent.height),
									  getCurrentDPIScale(),
                                      preTransform);
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
	flushBatchedDraws();

	vkCmdSetStencilWriteMask(commandBuffers.at(currentFrame), VK_STENCIL_FACE_FRONT_AND_BACK, writemask);
	vkCmdSetStencilCompareMask(commandBuffers.at(currentFrame), VK_STENCIL_FACE_FRONT_AND_BACK, readmask);
	vkCmdSetStencilReference(commandBuffers.at(currentFrame), VK_STENCIL_FACE_FRONT_AND_BACK, value);

	if (optionalDeviceFeatures.extendedDynamicState)
		ext.vkCmdSetStencilOpEXT(
			commandBuffers.at(currentFrame),
			VK_STENCIL_FACE_FRONT_AND_BACK,
			VK_STENCIL_OP_KEEP, Vulkan::getStencilOp(action),
			VK_STENCIL_OP_KEEP, Vulkan::getCompareOp(compare));

	states.back().stencil.action = action;
	states.back().stencil.compare = compare;
	states.back().stencil.value = value;
	states.back().stencil.readMask = readmask;
	states.back().stencil.writeMask = writemask;
}

void Graphics::setDepthMode(CompareMode compare, bool write)
{
	flushBatchedDraws();

	if (optionalDeviceFeatures.extendedDynamicState)
	{
		ext.vkCmdSetDepthCompareOpEXT(
			commandBuffers.at(currentFrame), Vulkan::getCompareOp(compare));

		ext.vkCmdSetDepthWriteEnableEXT(
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

PixelFormat Graphics::getSizedFormat(PixelFormat format, bool rendertarget, bool readable) const
{
	switch (format)
	{
	case PIXELFORMAT_NORMAL:
		if (isGammaCorrect())
			return PIXELFORMAT_RGBA8_UNORM_sRGB;
		else
			return PIXELFORMAT_RGBA8_UNORM;
	case PIXELFORMAT_HDR:
		return PIXELFORMAT_RGBA16_FLOAT;
	default:
		return format;
	}
}

bool Graphics::isPixelFormatSupported(PixelFormat format, uint32 usage, bool sRGB)
{
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

bool Graphics::dispatch(int x, int y, int z)
{
	if (renderPassState.active)
		endRenderPass();

	vkCmdBindPipeline(commandBuffers.at(currentFrame), VK_PIPELINE_BIND_POINT_COMPUTE, computeShader->getComputePipeline());

	computeShader->cmdPushDescriptorSets(commandBuffers.at(currentFrame), currentFrame, VK_PIPELINE_BIND_POINT_COMPUTE);

	vkCmdDispatch(commandBuffers.at(currentFrame), static_cast<uint32_t>(x), static_cast<uint32_t>(y), static_cast<uint32_t>(z));

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

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapChainExtent.width);
	viewport.height = static_cast<float>(swapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vkCmdSetViewport(commandBuffers.at(currentFrame), 0, 1, &viewport);

	vkCmdSetStencilWriteMask(commandBuffers.at(currentFrame), VK_STENCIL_FACE_FRONT_AND_BACK, states.back().stencil.writeMask);
	vkCmdSetStencilCompareMask(commandBuffers.at(currentFrame), VK_STENCIL_FACE_FRONT_AND_BACK, states.back().stencil.readMask);
	vkCmdSetStencilReference(commandBuffers.at(currentFrame), VK_STENCIL_FACE_FRONT_AND_BACK, states.back().stencil.value);

	if (optionalDeviceFeatures.extendedDynamicState)
	{
		 ext.vkCmdSetStencilOpEXT(
			commandBuffers.at(currentFrame),
			VK_STENCIL_FACE_FRONT_AND_BACK,
			VK_STENCIL_OP_KEEP, Vulkan::getStencilOp(states.back().stencil.action),
			VK_STENCIL_OP_KEEP, Vulkan::getCompareOp(states.back().stencil.compare));

		ext.vkCmdSetDepthCompareOpEXT(
			commandBuffers.at(currentFrame), Vulkan::getCompareOp(states.back().depthTest));

		ext.vkCmdSetDepthWriteEnableEXT(
			commandBuffers.at(currentFrame), Vulkan::getBool(states.back().depthWrite));

		ext.vkCmdSetFrontFaceEXT(
			commandBuffers.at(currentFrame), Vulkan::getFrontFace(states.back().winding));
	}
}

void Graphics::beginFrame()
{
	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

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

	startRecordingGraphicsCommands(true);

	Vulkan::cmdTransitionImageLayout(
		commandBuffers.at(currentFrame),
		swapChainImages[imageIndex],
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	Vulkan::resetShaderSwitches();
}

void Graphics::startRecordingGraphicsCommands(bool newFrame)
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

void Graphics::endRecordingGraphicsCommands(bool present) {
	if (renderPassState.active)
		endRenderPass();

	if (present)
		Vulkan::cmdTransitionImageLayout(
			commandBuffers.at(currentFrame), 
			swapChainImages[imageIndex], 
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	if (vkEndCommandBuffer(commandBuffers.at(currentFrame)) != VK_SUCCESS)
		throw love::Exception("failed to record command buffer");
}

void Graphics::updatedBatchedDrawBuffers()
{
	batchedDrawState.vb[0] = batchedDrawBuffers[currentFrame].vertexBuffer1;
	batchedDrawState.vb[0]->nextFrame();
	batchedDrawState.vb[1] = batchedDrawBuffers[currentFrame].vertexBuffer2;
	batchedDrawState.vb[1]->nextFrame();
	batchedDrawState.indexBuffer = batchedDrawBuffers[currentFrame].indexBuffer;
	batchedDrawState.indexBuffer->nextFrame();
}

uint32_t Graphics::getNumImagesInFlight() const
{
	return MAX_FRAMES_IN_FLIGHT;
}

const VkDeviceSize Graphics::getMinUniformBufferOffsetAlignment() const
{
	return minUniformBufferOffsetAlignment;
}

graphics::Texture *Graphics::getDefaultTexture() const
{
	return dynamic_cast<graphics::Texture*>(standardTexture.get());
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

static void checkOptionalInstanceExtensions(OptionalInstanceExtensions &ext)
{
	uint32_t count;

	vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);

	std::vector<VkExtensionProperties> extensions(count);

	vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());

	for (const auto& extension : extensions)
		if (strcmp(extension.extensionName, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) == 0)
			ext.physicalDeviceProperties2 = true;
}

void Graphics::createVulkanInstance()
{
	if (enableValidationLayers && !checkValidationSupport())
		throw love::Exception("validation layers requested, but not available");

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "LOVE";
	appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);	//todo, get this version from somewhere else?
	appInfo.pEngineName = "LOVE Engine";
	appInfo.engineVersion = VK_MAKE_API_VERSION(0, VERSION_MAJOR, VERSION_MINOR, VERSION_REV);
	appInfo.apiVersion = vulkanApiVersion;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.pNext = nullptr;

	auto window = Module::getInstance<love::window::Window>(M_WINDOW);
	const void *handle = window->getHandle();

	unsigned int count;
	if (SDL_Vulkan_GetInstanceExtensions((SDL_Window*)handle, &count, nullptr) != SDL_TRUE)
		throw love::Exception("couldn't retrieve sdl vulkan extensions");

	std::vector<const char*> extensions = {};

	checkOptionalInstanceExtensions(optionalInstanceExtensions);

	if (optionalInstanceExtensions.physicalDeviceProperties2)
		extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

	size_t additional_extension_count = extensions.size();
	extensions.resize(additional_extension_count + count);

	if (SDL_Vulkan_GetInstanceExtensions((SDL_Window*)handle, &count, extensions.data() + additional_extension_count) != SDL_TRUE)
		throw love::Exception("couldn't retrieve sdl vulkan extensions");

	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
		throw love::Exception("couldn't create vulkan instance");

#ifdef LOVE_ANDROID
	volkLoadInstance(instance);
#endif
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
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
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

	getMaxUsableSampleCount();
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
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
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

static void findOptionalDeviceExtensions(VkPhysicalDevice physicalDevice, OptionalDeviceFeatures &optionalDeviceFeatures)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

	for (const auto& extension : availableExtensions)
	{
		if (strcmp(extension.extensionName, VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME) == 0)
			optionalDeviceFeatures.extendedDynamicState = true;
		if (strcmp(extension.extensionName, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME) == 0)
			optionalDeviceFeatures.pushDescriptor = true;
	}
}

void Graphics::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value()};

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

	findOptionalDeviceExtensions(physicalDevice, optionalDeviceFeatures);

    if (optionalDeviceFeatures.extendedDynamicState && !optionalInstanceExtensions.physicalDeviceProperties2)
        optionalDeviceFeatures.extendedDynamicState = false;

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.fillModeNonSolid = VK_TRUE;

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;

	std::vector<const char*> enabledExtensions(deviceExtensions.begin(), deviceExtensions.end());

	if (optionalDeviceFeatures.extendedDynamicState)
		enabledExtensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
	if (optionalDeviceFeatures.pushDescriptor)
		enabledExtensions.push_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);

	createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
	createInfo.ppEnabledExtensionNames = enabledExtensions.data();

	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
		createInfo.enabledLayerCount = 0;

	VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures{};
	extendedDynamicStateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
	extendedDynamicStateFeatures.extendedDynamicState = Vulkan::getBool(optionalDeviceFeatures.extendedDynamicState);
	extendedDynamicStateFeatures.pNext = nullptr;

	createInfo.pNext = &extendedDynamicStateFeatures;

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
		throw love::Exception("failed to create logical device");

#ifdef LOVE_ANDROID
    volkLoadDevice(device);
#endif

	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);

	if (optionalDeviceFeatures.extendedDynamicState)
	{
#ifdef LOVE_ANDROID
        ext.vkCmdSetCullModeEXT = vkCmdSetCullModeEXT;
		ext.vkCmdSetDepthBoundsTestEnableEXT = vkCmdSetDepthBoundsTestEnableEXT;
		ext.vkCmdSetDepthBoundsTestEnableEXT = vkCmdSetCullModeEXT;
		ext.vkCmdSetDepthTestEnableEXT = vkCmdSetDepthTestEnableEXT;
		ext.vkCmdSetDepthWriteEnableEXT = vkCmdSetDepthWriteEnableEXT;
		ext.vkCmdSetFrontFaceEXT = vkCmdSetFrontFaceEXT;
		ext.vkCmdSetPrimitiveTopologyEXT = vkCmdSetPrimitiveTopologyEXT;
		ext.vkCmdSetScissorWithCountEXT = vkCmdSetScissorWithCountEXT;
		ext.vkCmdSetStencilOpEXT = vkCmdSetStencilOpEXT;
		ext.vkCmdSetStencilTestEnableEXT = vkCmdSetStencilTestEnableEXT;
		ext.vkCmdSetViewportWithCountEXT = vkCmdSetViewportWithCountEXT;
#else
		ext.vkCmdSetCullModeEXT = (PFN_vkCmdSetCullModeEXT)vkGetDeviceProcAddr(device, "vkCmdSetCullModeEXT");
		ext.vkCmdSetDepthBoundsTestEnableEXT = (PFN_vkCmdSetDepthBoundsTestEnableEXT)vkGetDeviceProcAddr(device, "vkCmdSetDepthBoundsTestEnableEXT");
		ext.vkCmdSetDepthCompareOpEXT = (PFN_vkCmdSetDepthCompareOpEXT)vkGetDeviceProcAddr(device, "vkCmdSetDepthCompareOpEXT");
		ext.vkCmdSetDepthTestEnableEXT = (PFN_vkCmdSetDepthTestEnableEXT)vkGetDeviceProcAddr(device, "vkCmdSetDepthTestEnableEXT");
		ext.vkCmdSetDepthWriteEnableEXT = (PFN_vkCmdSetDepthWriteEnableEXT)vkGetDeviceProcAddr(device, "vkCmdSetDepthWriteEnableEXT");
		ext.vkCmdSetFrontFaceEXT = (PFN_vkCmdSetFrontFaceEXT)vkGetDeviceProcAddr(device, "vkCmdSetFrontFaceEXT");
		ext.vkCmdSetPrimitiveTopologyEXT = (PFN_vkCmdSetPrimitiveTopologyEXT)vkGetDeviceProcAddr(device, "vkCmdSetPrimitiveTopologyEXT");
		ext.vkCmdSetScissorWithCountEXT = (PFN_vkCmdSetScissorWithCountEXT)vkGetDeviceProcAddr(device, "vkCmdSetScissorWithCountEXT");
		ext.vkCmdSetStencilOpEXT = (PFN_vkCmdSetStencilOpEXT)vkGetDeviceProcAddr(device, "vkCmdSetStencilOpEXT");
		ext.vkCmdSetStencilTestEnableEXT = (PFN_vkCmdSetStencilTestEnableEXT)vkGetDeviceProcAddr(device, "vkCmdSetStencilTestEnableEXT");
		ext.vkCmdSetViewportWithCountEXT = (PFN_vkCmdSetViewportWithCountEXT)vkGetDeviceProcAddr(device, "vkCmdSetViewportWithCountEXT");
#endif
	}
	if (optionalDeviceFeatures.pushDescriptor)
	{
#ifdef LOVE_ANDROID
		ext.vkCmdPushDescriptorSetKHR = vkCmdPushDescriptorSetKHR;
#else
		ext.vkCmdPushDescriptorSetKHR = (PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr(device, "vkCmdPushDescriptorSetKHR");
#endif
	}
}

void Graphics::initVMA()
{
	VmaAllocatorCreateInfo allocatorCreateInfo = {};
	allocatorCreateInfo.vulkanApiVersion = vulkanApiVersion;
	allocatorCreateInfo.physicalDevice = physicalDevice;
	allocatorCreateInfo.device = device;
	allocatorCreateInfo.instance = instance;
#ifdef LOVE_ANDROID
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
#else
	VmaVulkanFunctions vulkanFunctions{};
	vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
	vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;
	allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;
#endif

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

    if (swapChainSupport.capabilities.currentTransform & VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR ||
        swapChainSupport.capabilities.currentTransform & VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR)
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
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily)
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
	for (const auto& availableFormat : availableFormats)
		// fixme: what if this format and colorspace is not available?
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return availableFormat;

	return availableFormats[0];
}

VkPresentModeKHR Graphics::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
	int vsync = Vulkan::getVsync();

	auto begin = availablePresentModes.begin();
	auto end = availablePresentModes.end();

	switch (vsync) {
	case -1:
		if (std::find(begin, end, VK_PRESENT_MODE_FIFO_RELAXED_KHR) != availablePresentModes.end())
			return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
		else
			return VK_PRESENT_MODE_FIFO_KHR;
	case 0:
		if (std::find(begin, end, VK_PRESENT_MODE_MAILBOX_KHR) != availablePresentModes.end())
			return VK_PRESENT_MODE_MAILBOX_KHR;
		else
		{
			if (std::find(begin, end, VK_PRESENT_MODE_IMMEDIATE_KHR) != availablePresentModes.end())
				return VK_PRESENT_MODE_IMMEDIATE_KHR;
			else
				return VK_PRESENT_MODE_FIFO_KHR;
		}
	default:
		return VK_PRESENT_MODE_FIFO_KHR;
	}
}

VkExtent2D Graphics::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
		return capabilities.currentExtent;
	else
	{
		auto window = Module::getInstance<love::window::Window>(M_WINDOW);
		const void *handle = window->getHandle();

		int width, height;
		SDL_Vulkan_GetDrawableSize((SDL_Window*)handle, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

VkCompositeAlphaFlagBitsKHR Graphics::chooseCompositeAlpha(const VkSurfaceCapabilitiesKHR& capabilities)
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

void Graphics::createDefaultRenderPass()
{
	RenderPassConfiguration renderPassConfiguration{};
	renderPassConfiguration.colorFormats.push_back(swapChainImageFormat);
	renderPassConfiguration.staticData.msaaSamples = msaaSamples;
	renderPassConfiguration.staticData.depthFormat = findDepthFormat();
	if (msaaSamples & VK_SAMPLE_COUNT_1_BIT)
		renderPassConfiguration.staticData.resolve = false;
	else
		renderPassConfiguration.staticData.resolve = true;
	defaultRenderPass = createRenderPass(renderPassConfiguration);
}

void Graphics::createDefaultFramebuffers()
{
	defaultFramebuffers.clear();

	for (const auto view : swapChainImageViews)
	{
		FramebufferConfiguration configuration{};
		configuration.staticData.renderPass = defaultRenderPass;
		configuration.staticData.width = swapChainExtent.width;
		configuration.staticData.height = swapChainExtent.height;
		configuration.staticData.depthView = depthImageView;
		if (msaaSamples & VK_SAMPLE_COUNT_1_BIT)
			configuration.colorViews.push_back(view);
		else
		{
			configuration.colorViews.push_back(colorImageView);
			configuration.staticData.resolveView = view;
		}
		defaultFramebuffers.push_back(createFramebuffer(configuration));
	}
}

VkFramebuffer Graphics::createFramebuffer(FramebufferConfiguration &configuration)
{
	std::vector<VkImageView> attachments;

	for (const auto& colorView : configuration.colorViews)
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
	auto it = framebuffers.find(configuration);
	if (it != framebuffers.end())
		return it->second;
	else
	{
		VkFramebuffer framebuffer = createFramebuffer(configuration);
		framebuffers[configuration] = framebuffer;
		return framebuffer;
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

	std::vector<VkAttachmentDescription> attachments;
	std::vector<VkAttachmentReference> colorAttachmentRefs;

	uint32_t attachment = 0;
	for (const auto &colorFormat : configuration.colorFormats)
	{
		VkAttachmentReference reference{};
		reference.attachment = attachment++;
		reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachmentRefs.push_back(reference);

		VkAttachmentDescription colorDescription{};
		colorDescription.format = colorFormat;
		colorDescription.samples = configuration.staticData.msaaSamples;
		colorDescription.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
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
	if (configuration.staticData.depthFormat != VK_FORMAT_UNDEFINED)
	{
		depthStencilAttachmentRef.attachment = attachment++;
		depthStencilAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		subPass.pDepthStencilAttachment = &depthStencilAttachmentRef;

		VkAttachmentDescription depthStencilAttachment{};
		depthStencilAttachment.format = configuration.staticData.depthFormat;
		depthStencilAttachment.samples = configuration.staticData.msaaSamples;
		depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthStencilAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthStencilAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthStencilAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
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
		colorAttachmentResolve.format = configuration.colorFormats.at(0);
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
	readbackDependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
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

bool Graphics::usesConstantVertexColor(const VertexAttributes &vertexAttributes)
{
	return !!(vertexAttributes.enableBits & (1u << ATTRIB_COLOR));
}

void Graphics::createVulkanVertexFormat(
	VertexAttributes vertexAttributes,
	std::vector<VkVertexInputBindingDescription> &bindingDescriptions,
	std::vector<VkVertexInputAttributeDescription> &attributeDescriptions)
{
	std::set<uint32_t> usedBuffers;

	auto allBits = vertexAttributes.enableBits;

	bool usesColor = false;

	uint8_t highestBufferBinding = 0;

	// change to loop like in opengl implementation ?
	for (uint32_t i = 0; i < VertexAttributes::MAX; i++)
	{
		uint32 bit = 1u << i;
		if (allBits & bit)
		{
			if (i == ATTRIB_COLOR)
				usesColor = true;

			auto attrib = vertexAttributes.attribs[i];
			auto bufferBinding = attrib.bufferIndex;
			if (usedBuffers.find(bufferBinding) == usedBuffers.end())	// use .contains() when c++20 is enabled
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
	}

	// do we need to use a constant VertexColor?
	if (!usesColor)
	{
		// FIXME: is there a case where gaps happen between buffer bindings?
		// then this doesn't work. We might need to enable null buffers again.
		const auto constantColorBufferBinding = highestBufferBinding + 1;

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

	GraphicsPipelineConfiguration configuration{};

    configuration.renderPass = renderPassState.renderPass;
	configuration.vertexAttributes = attributes;
	configuration.shader = (Shader*)Shader::current;
	configuration.wireFrame = states.back().wireframe;
	configuration.blendState = states.back().blend;
	configuration.colorChannelMask = states.back().colorMask;
	configuration.msaaSamples = renderPassState.msaa;
	configuration.numColorAttachments = renderPassState.numColorAttachments;
	configuration.primitiveType = primitiveType;

	if (optionalDeviceFeatures.extendedDynamicState)
		ext.vkCmdSetCullModeEXT(commandBuffers.at(currentFrame), Vulkan::getCullMode(cullmode));
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
		if (buffers.useBits & (1u << i))
		{
			bufferVector.push_back((VkBuffer)buffers.info[i].buffer->getHandle());
			offsets.push_back((VkDeviceSize)buffers.info[i].offset);
		}

	if (usesConstantVertexColor(attributes))
	{
		bufferVector.push_back((VkBuffer)batchedDrawBuffers[currentFrame].constantColorBuffer->getHandle());
		offsets.push_back((VkDeviceSize)0);
	}

	auto currentUniformData = getCurrentBuiltinUniformData();
	configuration.shader->setUniformData(currentUniformData);
	if (texture == nullptr)
		configuration.shader->setMainTex(standardTexture.get());
	else
		configuration.shader->setMainTex(texture);

	ensureGraphicsPipelineConfiguration(configuration);

	configuration.shader->cmdPushDescriptorSets(commandBuffers.at(currentFrame), static_cast<uint32_t>(currentFrame), VK_PIPELINE_BIND_POINT_GRAPHICS);
	vkCmdBindVertexBuffers(commandBuffers.at(currentFrame), 0, static_cast<uint32_t>(bufferVector.size()), bufferVector.data(), offsets.data());
}

void Graphics::setDefaultRenderPass()
{
	renderPassState.beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassState.beginInfo.renderPass = defaultRenderPass;
	renderPassState.beginInfo.framebuffer = defaultFramebuffers[imageIndex];
	renderPassState.beginInfo.renderArea.offset = { 0, 0 };
	renderPassState.beginInfo.renderArea.extent = swapChainExtent;
	renderPassState.beginInfo.clearValueCount = 0;

	renderPassState.renderPass = defaultRenderPass;
	renderPassState.pipeline = VK_NULL_HANDLE;
	renderPassState.width = static_cast<float>(swapChainExtent.width);
	renderPassState.height = static_cast<float>(swapChainExtent.height);
	renderPassState.msaa = msaaSamples;
	renderPassState.numColorAttachments = 1;
	renderPassState.transitionImages.clear();

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = renderPassState.width;
	viewport.height = renderPassState.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vkCmdSetViewport(commandBuffers.at(currentFrame), 0, 1, &viewport);
}

void Graphics::setRenderPass(const RenderTargets &rts, int pixelw, int pixelh, bool hasSRGBtexture)
{
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(pixelw);
	viewport.height = static_cast<float>(pixelh);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vkCmdSetViewport(commandBuffers.at(currentFrame), 0, 1, &viewport);

	auto currentCommandBuffer = commandBuffers.at(currentFrame);

	// fixme: hasSRGBtexture
	// fixme: msaaSamples
	RenderPassConfiguration renderPassConfiguration{};
	for (const auto &color : rts.colors)
	{
		// fixme: use mipmap and slice.
		color.mipmap;
		color.slice;
		renderPassConfiguration.colorFormats.push_back(Vulkan::getTextureFormat(color.texture->getPixelFormat()).internalFormat);
	}
	if (rts.depthStencil.texture != nullptr)
	{
		// fixme: use mipmap and slice:
		rts.depthStencil.mipmap;
		rts.depthStencil.slice;
		if (rts.depthStencil.texture != nullptr)
			renderPassConfiguration.staticData.depthFormat =  Vulkan::getTextureFormat(rts.depthStencil.texture->getPixelFormat()).internalFormat;
	}

    VkRenderPass renderPass;
	auto it = renderPasses.find(renderPassConfiguration);
	if (it != renderPasses.end())
		renderPass = it->second;
	else
	{
		renderPass = createRenderPass(renderPassConfiguration);
		renderPasses[renderPassConfiguration] = renderPass;
	}

	FramebufferConfiguration configuration{};

	std::vector<VkImage> transitionImages;

	for (const auto& color : rts.colors)
	{
		configuration.colorViews.push_back((VkImageView)color.texture->getRenderTargetHandle());
		transitionImages.push_back((VkImage) color.texture->getHandle());
	}
	if (rts.depthStencil.texture != nullptr)
		// fixme: layout transition of depth stencil image?
		configuration.staticData.depthView = (VkImageView)rts.depthStencil.texture->getRenderTargetHandle();

	configuration.staticData.renderPass = renderPass;
	configuration.staticData.width = static_cast<uint32_t>(pixelw);
	configuration.staticData.height = static_cast<uint32_t>(pixelh);
	VkFramebuffer framebuffer = getFramebuffer(configuration);

	renderPassState.beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassState.beginInfo.renderPass = renderPass;
	renderPassState.beginInfo.framebuffer = framebuffer;
	renderPassState.beginInfo.renderArea.offset = {0, 0};
	renderPassState.beginInfo.renderArea.extent.width = static_cast<uint32_t>(pixelw);
	renderPassState.beginInfo.renderArea.extent.height = static_cast<uint32_t>(pixelh);
	renderPassState.beginInfo.clearValueCount = 0;

	renderPassState.renderPass = renderPass;
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

	for (const auto& image : renderPassState.transitionImages)
		Vulkan::cmdTransitionImageLayout(commandBuffers.at(currentFrame), image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	vkCmdBeginRenderPass(commandBuffers.at(currentFrame), &renderPassState.beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Graphics::endRenderPass()
{
	renderPassState.active = false;

	vkCmdEndRenderPass(commandBuffers.at(currentFrame));

	for (const auto &image : renderPassState.transitionImages)
		Vulkan::cmdTransitionImageLayout(commandBuffers.at(currentFrame), image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

VkSampler Graphics::createSampler(const SamplerState &samplerState)
{
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = Vulkan::getFilter(samplerState.magFilter);
	samplerInfo.minFilter = Vulkan::getFilter(samplerState.minFilter);
	samplerInfo.addressModeU = Vulkan::getWrapMode(samplerState.wrapU);
	samplerInfo.addressModeV = Vulkan::getWrapMode(samplerState.wrapV);
	samplerInfo.addressModeW = Vulkan::getWrapMode(samplerState.wrapW);
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = static_cast<float>(samplerState.maxAnisotropy);
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
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
	samplerInfo.mipmapMode = Vulkan::getMipMapMode(samplerState.mipmapFilter);
	samplerInfo.mipLodBias = samplerState.lodBias;
	samplerInfo.minLod = static_cast<float>(samplerState.minLod);
	samplerInfo.maxLod = static_cast<float>(samplerState.maxLod);

	VkSampler sampler;
	if (vkCreateSampler(device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
		throw love::Exception("failed to create sampler");

	return sampler;
}

void Graphics::setComputeShader(Shader *shader)
{
	computeShader = shader;
}

const OptionalDeviceFeatures &Graphics::getOptionalDeviceFeatures() const
{
	return optionalDeviceFeatures;
}

const OptionalDeviceExtensionFunctions &Graphics::getExtensionFunctions() const
{
	return ext;
}

VkSampler Graphics::getCachedSampler(const SamplerState &samplerState)
{
	auto it = samplers.find(samplerState);
	if (it != samplers.end())
		return it->second;
	else
	{
		VkSampler sampler = createSampler(samplerState);
		samplers.insert({samplerState, sampler});
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
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = Vulkan::getPolygonMode(configuration.wireFrame);
	rasterizer.lineWidth = 1.0f;
	if (!optionalDeviceFeatures.extendedDynamicState)
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
	if (!optionalDeviceFeatures.extendedDynamicState)
	{
		depthStencil.depthWriteEnable = Vulkan::getBool(configuration.dynamicState.depthState.write);
		depthStencil.depthCompareOp = Vulkan::getCompareOp(configuration.dynamicState.depthState.compare);
	}
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f;
	depthStencil.maxDepthBounds = 1.0f;

	depthStencil.stencilTestEnable = VK_TRUE;

	if (!optionalDeviceFeatures.extendedDynamicState)
	{
		depthStencil.front.failOp = VK_STENCIL_OP_KEEP;
		depthStencil.front.passOp = Vulkan::getStencilOp(configuration.dynamicState.stencilAction);
		depthStencil.front.depthFailOp = VK_STENCIL_OP_KEEP;
		depthStencil.front.compareOp = Vulkan::getCompareOp(configuration.dynamicState.stencilCompare);

		depthStencil.back.failOp = VK_STENCIL_OP_KEEP;
		depthStencil.back.passOp = Vulkan::getStencilOp(configuration.dynamicState.stencilAction);
		depthStencil.back.depthFailOp = VK_STENCIL_OP_KEEP;
		depthStencil.back.compareOp = Vulkan::getCompareOp(configuration.dynamicState.stencilCompare);
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
	
	if (optionalDeviceFeatures.extendedDynamicState)
		dynamicStates = {
			VK_DYNAMIC_STATE_SCISSOR,
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
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
	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
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
		}
	}
	else
	{
		VkPipeline pipeline = createGraphicsPipeline(configuration);
		graphicsPipelines.insert({configuration, pipeline});
		vkCmdBindPipeline(commandBuffers.at(currentFrame), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		renderPassState.pipeline = pipeline;
	}
}

void Graphics::getMaxUsableSampleCount()
{
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

	VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;

	if (counts & VK_SAMPLE_COUNT_64_BIT && requestedMsaa >= 64)
		msaaSamples = VK_SAMPLE_COUNT_64_BIT;
	else if (counts & VK_SAMPLE_COUNT_32_BIT && requestedMsaa >= 32)
		msaaSamples = VK_SAMPLE_COUNT_32_BIT;
	else if (counts & VK_SAMPLE_COUNT_16_BIT && requestedMsaa >= 16)
		msaaSamples = VK_SAMPLE_COUNT_16_BIT;
	else if (counts & VK_SAMPLE_COUNT_8_BIT && requestedMsaa >= 8)
		msaaSamples = VK_SAMPLE_COUNT_8_BIT;
	else if (counts & VK_SAMPLE_COUNT_4_BIT && requestedMsaa >= 4)
		msaaSamples = VK_SAMPLE_COUNT_4_BIT;
	else if (counts & VK_SAMPLE_COUNT_2_BIT && requestedMsaa >= 2)
		msaaSamples = VK_SAMPLE_COUNT_2_BIT;
	else
		msaaSamples = VK_SAMPLE_COUNT_1_BIT;
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

		vmaCreateImage(vmaAllocator, &imageInfo, &allocationInfo, &colorImage, &colorImageAllocation, nullptr);

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

		vkCreateImageView(device, &imageViewInfo, nullptr, &colorImageView);
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
	VkFormat depthFormat = findDepthFormat();

	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = depthFormat;
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

	vmaCreateImage(vmaAllocator, &imageInfo, &allocationInfo, &depthImage, &depthImageAllocation, nullptr);

	VkImageViewCreateInfo imageViewInfo{};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.image = depthImage;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = depthFormat;
	imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;

	vkCreateImageView(device, &imageViewInfo, nullptr, &depthImageView);
}

void Graphics::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
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
	standardTexture.reset((Texture*)newTexture(settings, nullptr));
	uint8_t whitePixels[] = {255, 255, 255, 255};
	standardTexture->replacePixels(whitePixels, sizeof(whitePixels), 0, 0, { 0, 0, 1, 1 }, false);
}

void Graphics::cleanup()
{
	cleanupSwapChain();

	for (auto &cleanUpFns : cleanUpFunctions)
		for (auto &cleanUpFn : cleanUpFns)
			cleanUpFn();
	cleanUpFunctions.clear();

	vmaDestroyAllocator(vmaAllocator);
	batchedDrawBuffers.clear();
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

	for (const auto &[key, val] : renderPasses)
		vkDestroyRenderPass(device, val, nullptr);

	// fixme: maybe we should clean up some pipelines if they haven't been used in a while.
	for (auto const &p : graphicsPipelines)
		vkDestroyPipeline(device, p.second, nullptr);
	graphicsPipelines.clear();

	vkDestroyCommandPool(device, commandPool, nullptr);
	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
}

void Graphics::cleanupSwapChain()
{
	for (const auto &framebuffer : defaultFramebuffers)
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	vkDestroyRenderPass(device, defaultRenderPass, nullptr);
	vkDestroyImageView(device, colorImageView, nullptr);
	vmaDestroyImage(vmaAllocator, colorImage, colorImageAllocation);
	vkDestroyImageView(device, depthImageView, nullptr);
	vmaDestroyImage(vmaAllocator, depthImage, depthImageAllocation);
	for (const auto &[key, val] : framebuffers)
		vkDestroyFramebuffer(device, val, nullptr);
    framebuffers.clear();
    for (const auto &swapChainImageView : swapChainImageViews)
        vkDestroyImageView(device, swapChainImageView, nullptr);
    swapChainImageViews.clear();
	vkDestroySwapchainKHR(device, swapChain, nullptr);
}

void Graphics::recreateSwapChain()
{
	vkDeviceWaitIdle(device);

	cleanupSwapChain();

	createSwapChain();
	createImageViews();
	createColorResources();
	createDepthResources();
	createDefaultRenderPass();
	createDefaultFramebuffers();
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
