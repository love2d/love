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

// löve
#include "common/config.h"
#include "graphics/Graphics.h"
#include "StreamBuffer.h"
#include "ShaderStage.h"
#include "Shader.h"
#include "Texture.h"

// libraries
#include "VulkanWrapper.h"
#include "libraries/xxHash/xxhash.h"

// c++
#include <iostream>
#include <memory>
#include <functional>
#include <set>
#include <tuple>

namespace love
{
namespace graphics
{
namespace vulkan
{

struct ColorAttachment
{
	VkFormat format = VK_FORMAT_UNDEFINED;
	VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkImageLayout msaaLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

	bool operator==(const ColorAttachment &attachment) const
	{
		return format == attachment.format &&
			layout == attachment.layout &&
			msaaLayout == attachment.msaaLayout &&
			loadOp == attachment.loadOp &&
			msaaSamples == attachment.msaaSamples;
	}
};

struct DepthStencilAttachment
{
	VkFormat format = VK_FORMAT_UNDEFINED;
	VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkAttachmentLoadOp depthLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

	bool operator==(const DepthStencilAttachment &attachment) const
	{
		return format == attachment.format &&
			layout == attachment.layout &&
			depthLoadOp == attachment.depthLoadOp &&
			stencilLoadOp == attachment.stencilLoadOp &&
			msaaSamples == attachment.msaaSamples;
	}
};

struct RenderPassConfiguration
{
	std::vector<ColorAttachment> colorAttachments;

	struct StaticRenderPassConfiguration
	{
		DepthStencilAttachment depthStencilAttachment;
		bool resolve = false;
	} staticData;

	bool operator==(const RenderPassConfiguration &conf) const
	{
		return colorAttachments == conf.colorAttachments && 
			(memcmp(&staticData, &conf.staticData, sizeof(StaticRenderPassConfiguration)) == 0);
	}
};

struct RenderPassConfigurationHasher
{
	size_t operator()(const RenderPassConfiguration &configuration) const
	{
		size_t hashes[] = { 
			XXH32(configuration.colorAttachments.data(), configuration.colorAttachments.size() * sizeof(ColorAttachment), 0),
			XXH32(&configuration.staticData, sizeof(configuration.staticData), 0),
		};
		return XXH32(hashes, sizeof(hashes), 0);
	}
};

struct FramebufferConfiguration
{
	std::vector<VkImageView> colorViews;
	std::vector<VkImageView> colorResolveViews;

	struct StaticFramebufferConfiguration
	{
		VkImageView depthView = VK_NULL_HANDLE;

		uint32_t width = 0;
		uint32_t height = 0;

		VkRenderPass renderPass = VK_NULL_HANDLE;
	} staticData;

	bool operator==(const FramebufferConfiguration &conf) const
	{
		return colorViews == conf.colorViews && colorResolveViews == conf.colorResolveViews &&
			(memcmp(&staticData, &conf.staticData, sizeof(StaticFramebufferConfiguration)) == 0);
	}
};

struct FramebufferConfigurationHasher
{
	size_t operator()(const FramebufferConfiguration &configuration) const
	{
		size_t hashes[] = {
			XXH32(configuration.colorViews.data(), configuration.colorViews.size() * sizeof(VkImageView), 0),
			XXH32(configuration.colorResolveViews.data(), configuration.colorResolveViews.size() * sizeof(VkImageView), 0),
			XXH32(&configuration.staticData, sizeof(configuration.staticData), 0),
		};

		return XXH32(hashes, sizeof(hashes), 0);
	}
};

struct OptionalInstanceExtensions
{
	// VK_KHR_get_physical_device_properties2
	bool physicalDeviceProperties2 = false;

	// VK_EXT_debug_info
	bool debugInfo = false;
};

struct OptionalDeviceExtensions
{
	// VK_EXT_extended_dynamic_state
	bool extendedDynamicState = false;

	// VK_KHR_get_memory_requirements2
	bool memoryRequirements2 = false;

	// VK_KHR_dedicated_allocation
	bool dedicatedAllocation = false;

	// VK_EXT_memory_budget
	bool memoryBudget = false;

	// VK_KHR_shader_float_controls
	bool shaderFloatControls = false;

	// VK_KHR_spirv_1_4
	bool spirv14 = false;
};

struct QueueFamilyIndices
{
	Optional<uint32_t> graphicsFamily;
	Optional<uint32_t> presentFamily;

	bool isComplete() const
	{
		return graphicsFamily.hasValue && presentFamily.hasValue;
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities{};
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct RenderpassState
{
	bool active = false;
	VkRenderPassBeginInfo beginInfo{};
	bool isWindow = false;
	RenderPassConfiguration renderPassConfiguration{};
	FramebufferConfiguration framebufferConfiguration{};
	VkPipeline pipeline = VK_NULL_HANDLE;
	uint32_t numColorAttachments = 0;
	uint64 packedColorAttachmentFormats = 0;
	float width = 0.0f;
	float height = 0.0f;
	VkSampleCountFlagBits msaa = VK_SAMPLE_COUNT_1_BIT;
	std::vector<VkClearValue> clearColors;

	bool windowClearRequested = false;
	OptionalColorD mainWindowClearColorValue;
	OptionalDouble mainWindowClearDepthValue;
	OptionalInt mainWindowClearStencilValue;
};

enum SubmitMode
{
	SUBMIT_PRESENT,
	SUBMIT_NOPRESENT,
	SUBMIT_RESTART,
	SUBMIT_MAXENUM,
};

class Graphics final : public love::graphics::Graphics
{
public:
	Graphics();
	~Graphics();

	// implementation for virtual functions
	love::graphics::Texture *newTexture(const love::graphics::Texture::Settings &settings, const love::graphics::Texture::Slices *data) override;
	love::graphics::Texture *newTextureView(love::graphics::Texture *base, const Texture::ViewSettings &viewsettings) override;
	love::graphics::Buffer *newBuffer(const love::graphics::Buffer::Settings &settings, const std::vector<love::graphics::Buffer::DataDeclaration>& format, const void *data, size_t size, size_t arraylength) override;
	graphics::GraphicsReadback *newReadbackInternal(ReadbackMethod method, love::graphics::Buffer *buffer, size_t offset, size_t size, data::ByteData *dest, size_t destoffset) override;
	graphics::GraphicsReadback *newReadbackInternal(ReadbackMethod method, love::graphics::Texture *texture, int slice, int mipmap, const Rect &rect, image::ImageData *dest, int destx, int desty) override;
	void clear(OptionalColorD color, OptionalInt stencil, OptionalDouble depth) override;
	void clear(const std::vector<OptionalColorD> &colors, OptionalInt stencil, OptionalDouble depth) override;
	void discard(const std::vector<bool>& colorbuffers, bool depthstencil) override;
	void present(void *screenshotCallbackdata) override;
	void backbufferChanged(int width, int height, int pixelwidth, int pixelheight, bool backbufferstencil, bool backbufferdepth, int msaa) override;
	bool setMode(void *context, int width, int height, int pixelwidth, int pixelheight, bool backbufferstencil, bool backbufferdepth, int msaa) override;
	void unSetMode() override;
	void setActive(bool active) override;
	int getRequestedBackbufferMSAA() const override;
	int getBackbufferMSAA() const  override;
	void setColor(Colorf c) override;
	void setScissor(const Rect &rect) override;
	void setScissor() override;
	void setStencilState(const StencilState &s) override;
	void setDepthMode(CompareMode compare, bool write) override;
	void setFrontFaceWinding(Winding winding) override;
	void setColorMask(ColorChannelMask mask) override;
	void setBlendState(const BlendState &blend) override;
	void setPointSize(float size) override;
	void setWireframe(bool enable) override;
	bool isPixelFormatSupported(PixelFormat format, uint32 usage) override;
	Renderer getRenderer() const override;
	bool usesGLSLES() const override;
	RendererInfo getRendererInfo() const override;
	void draw(const DrawCommand &cmd) override;
	void draw(const DrawIndexedCommand &cmd) override;
	void drawQuads(int start, int count, VertexAttributesID attributesID, const BufferBindings &buffers, graphics::Texture *texture) override;

	// internal functions.

	VkDevice getDevice() const;
	VmaAllocator getVmaAllocator() const;
	VkCommandBuffer getCommandBufferForDataTransfer();
	void queueCleanUp(std::function<void()> cleanUp);
	void addReadbackCallback(std::function<void()> callback);
	void submitGpuCommands(SubmitMode, void *screenshotCallbackData = nullptr);
	VkSampler getCachedSampler(const SamplerState &sampler);
	graphics::Shader::BuiltinUniformData getCurrentBuiltinUniformData();
	const OptionalDeviceExtensions &getEnabledOptionalDeviceExtensions() const;
	const OptionalInstanceExtensions &getEnabledOptionalInstanceExtensions() const;
	VkSampleCountFlagBits getMsaaCount(int requestedMsaa) const;
	void setVsync(int vsync);
	int getVsync() const;
	void mapLocalUniformData(void *data, size_t size, VkDescriptorBufferInfo &bufferInfo);

	void cleanupFramebuffers(VkImageView imageView, PixelFormat format);

	VkPipeline createGraphicsPipeline(Shader *shader, const GraphicsPipelineConfigurationCore &configuration, const GraphicsPipelineConfigurationNoDynamicState *noDynamicStateConfiguration);

	uint32 getDeviceApiVersion() const { return deviceApiVersion; }

protected:
	graphics::ShaderStage *newShaderStageInternal(ShaderStageType stage, const std::string &cachekey, const std::string &source, bool gles) override;
	graphics::Shader *newShaderInternal(StrongRef<love::graphics::ShaderStage> stages[SHADERSTAGE_MAX_ENUM], const Shader::CompileOptions &options) override;
	graphics::StreamBuffer *newStreamBuffer(BufferUsage type, size_t size) override;
	bool dispatch(love::graphics::Shader *shader, int x, int y, int z) override;
	bool dispatch(love::graphics::Shader *shader, love::graphics::Buffer *indirectargs, size_t argsoffset) override;
	void initCapabilities() override;
	void getAPIStats(int &shaderswitches) const override;
	void setRenderTargetsInternal(const RenderTargets &rts, int pixelw, int pixelh, bool hasSRGBtexture) override;

private:
	bool checkValidationSupport();
	void pickPhysicalDevice();
	int rateDeviceSuitability(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	void createLogicalDevice();
	void createPipelineCache();
	void initVMA();
	void createSurface();
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
	VkCompositeAlphaFlagBitsKHR chooseCompositeAlpha(const VkSurfaceCapabilitiesKHR &capabilities);
	void createSwapChain();
	void createImageViews();
	VkFramebuffer createFramebuffer(FramebufferConfiguration &configuration);
	VkFramebuffer getFramebuffer(FramebufferConfiguration &configuration);
	void createDefaultShaders();
	VkRenderPass createRenderPass(RenderPassConfiguration &configuration);
	VkRenderPass getRenderPass(RenderPassConfiguration &configuration);
	void createColorResources();
	VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat findDepthFormat();
	void createDepthResources();
	void createCommandPool();
	void createCommandBuffers();
	void createSyncObjects();
	void cleanup();
	void cleanupSwapChain();
	void recreateSwapChain();
	void initDynamicState();
	void beginFrame();
	void startRecordingGraphicsCommands();
	void endRecordingGraphicsCommands();
	void createVulkanVertexFormat(
		Shader *shader,
		const VertexAttributes &attributes, 
		std::vector<VkVertexInputBindingDescription> &bindingDescriptions, 
		std::vector<VkVertexInputAttributeDescription> &attributeDescriptions);
	void prepareDraw(
		VertexAttributesID attributesID,
		const BufferBindings &buffers, graphics::Texture *texture,
		PrimitiveType, CullMode);
	void setRenderPass(const RenderTargets &rts, int pixelw, int pixelh);
	void setDefaultRenderPass();
	void startRenderPass();
	void endRenderPass();
	void applyScissor();
	VkSampler createSampler(const SamplerState &sampler);
	void requestSwapchainRecreation();

	VkInstance instance = VK_NULL_HANDLE;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	uint32_t deviceApiVersion = VK_API_VERSION_1_0;
	int requestedMsaa = 0;
	VkDevice device = VK_NULL_HANDLE; 
	OptionalInstanceExtensions optionalInstanceExtensions;
	OptionalDeviceExtensions optionalDeviceExtensions;
	VkQueue graphicsQueue = VK_NULL_HANDLE;
	VkQueue presentQueue = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkSwapchainKHR swapChain = VK_NULL_HANDLE;
	std::vector<VkImage> swapChainImages;
	StrongRef<Texture> fakeBackbuffer;
	VkFormat swapChainImageFormat = VK_FORMAT_UNDEFINED;
	PixelFormat swapChainPixelFormat = PIXELFORMAT_UNKNOWN;
	VkFormat depthStencilFormat = VK_FORMAT_UNDEFINED;
	PixelFormat depthStencilPixelFormat = PIXELFORMAT_UNKNOWN;
	VkExtent2D swapChainExtent = VkExtent2D();
	std::vector<VkImageView> swapChainImageViews;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkImage colorImage = VK_NULL_HANDLE;
	VkImageView colorImageView = VK_NULL_HANDLE;
	VmaAllocation colorImageAllocation = VK_NULL_HANDLE;
	VkImage depthImage = VK_NULL_HANDLE;
	VkImageView depthImageView = VK_NULL_HANDLE;
	VmaAllocation depthImageAllocation = VK_NULL_HANDLE;
	VkPipelineCache pipelineCache = VK_NULL_HANDLE;
	std::unordered_map<RenderPassConfiguration, VkRenderPass, RenderPassConfigurationHasher> renderPasses;
	std::unordered_map<FramebufferConfiguration, VkFramebuffer, FramebufferConfigurationHasher> framebuffers;
	std::unordered_map<VkFramebuffer, bool> framebufferUsages;
	std::unordered_map<uint64, VkSampler> samplers;
	VkCommandPool commandPool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;
	int vsync = 1;
	VkDeviceSize minUniformBufferOffsetAlignment = 0;
	bool imageRequested = false;
	size_t currentFrame = 0;
	uint32_t imageIndex = 0;
	bool swapChainRecreationRequested = false;
	bool transitionColorDepthLayouts = false;
	VmaAllocator vmaAllocator = VK_NULL_HANDLE;
	StrongRef<love::graphics::Buffer> defaultVertexBuffer;
	StrongRef<StreamBuffer> localUniformBuffer;
	// functions that need to be called to cleanup objects that were needed for rendering a frame.
	// We need a vector for each frame in flight.
	std::vector<std::vector<std::function<void()>>> cleanUpFunctions;
	std::vector<std::vector<std::function<void()>>> readbackCallbacks;
	std::set<StrongRef<Shader>> usedShadersInFrame;
	RenderpassState renderPassState;
};

} // vulkan
} // graphics
} // love
