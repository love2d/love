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
#include <optional>
#include <iostream>
#include <memory>
#include <functional>
#include <set>


namespace love
{
namespace graphics
{
namespace vulkan
{

struct RenderPassConfiguration
{
	std::vector<VkFormat> colorFormats;

	struct StaticRenderPassConfiguration
	{
		VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
		VkFormat depthFormat = VK_FORMAT_UNDEFINED;
		bool resolve = false;
	} staticData;

    bool operator==(const RenderPassConfiguration &conf) const
	{
		return colorFormats == conf.colorFormats && 
			(memcmp(&staticData, &conf.staticData, sizeof(StaticRenderPassConfiguration)) == 0);
    }
};

struct RenderPassConfigurationHasher
{
    size_t operator()(const RenderPassConfiguration &configuration) const
	{
		size_t hashes[] = { 
			XXH32(configuration.colorFormats.data(), configuration.colorFormats.size() * sizeof(VkFormat), 0),
			XXH32(&configuration.staticData, sizeof(configuration.staticData), 0),
		};
		return XXH32(hashes, sizeof(hashes), 0);
    }
};

struct FramebufferConfiguration
{
	std::vector<VkImageView> colorViews;

	struct StaticFramebufferConfiguration
	{
		VkImageView depthView = VK_NULL_HANDLE;
		VkImageView resolveView = VK_NULL_HANDLE;

		uint32_t width = 0;
		uint32_t height = 0;

		VkRenderPass renderPass = VK_NULL_HANDLE;
	} staticData;

	bool operator==(const FramebufferConfiguration &conf) const
	{
		return colorViews == conf.colorViews &&
			(memcmp(&staticData, &conf.staticData, sizeof(StaticFramebufferConfiguration)) == 0);
	}
};

struct FramebufferConfigurationHasher
{
	size_t operator()(const FramebufferConfiguration &configuration) const
	{
		size_t hashes[] = {
			XXH32(configuration.colorViews.data(), configuration.colorViews.size() * sizeof(VkImageView), 0),
			XXH32(&configuration.staticData, sizeof(configuration.staticData), 0),
		};

		return XXH32(hashes, sizeof(hashes), 0);
	}
};

struct OptionalInstanceExtensions
{
    bool physicalDeviceProperties2 = false;
};

struct OptionalDeviceFeatures
{
	bool extendedDynamicState = false;
};

struct OptionalDeviceExtensionFunctions
{
	// extended dynamic state
	PFN_vkCmdSetCullModeEXT vkCmdSetCullModeEXT = nullptr;
	PFN_vkCmdSetDepthBoundsTestEnableEXT vkCmdSetDepthBoundsTestEnableEXT = nullptr;
	PFN_vkCmdSetDepthCompareOpEXT vkCmdSetDepthCompareOpEXT = nullptr;
	PFN_vkCmdSetDepthTestEnableEXT vkCmdSetDepthTestEnableEXT = nullptr;
	PFN_vkCmdSetDepthWriteEnableEXT vkCmdSetDepthWriteEnableEXT = nullptr;
	PFN_vkCmdSetFrontFaceEXT vkCmdSetFrontFaceEXT = nullptr;
	PFN_vkCmdSetPrimitiveTopologyEXT vkCmdSetPrimitiveTopologyEXT = nullptr;
	PFN_vkCmdSetScissorWithCountEXT vkCmdSetScissorWithCountEXT = nullptr;
	PFN_vkCmdSetStencilOpEXT vkCmdSetStencilOpEXT = nullptr;
	PFN_vkCmdSetStencilTestEnableEXT vkCmdSetStencilTestEnableEXT = nullptr;
	PFN_vkCmdSetViewportWithCountEXT vkCmdSetViewportWithCountEXT = nullptr;
};

struct GraphicsPipelineConfiguration
{
    VkRenderPass renderPass;
	VertexAttributes vertexAttributes;
	Shader* shader = nullptr;
	bool wireFrame;
	BlendState blendState;
	ColorChannelMask colorChannelMask;
	VkSampleCountFlagBits msaaSamples;
	uint32_t numColorAttachments;
	PrimitiveType primitiveType;

	struct DynamicState
	{
		CullMode cullmode = CULL_NONE;
		Winding winding = WINDING_MAX_ENUM;
		StencilAction stencilAction = STENCIL_MAX_ENUM;
		CompareMode stencilCompare = COMPARE_MAX_ENUM;
		DepthState depthState{};
	} dynamicState;

	GraphicsPipelineConfiguration()
	{
		memset(this, 0, sizeof(GraphicsPipelineConfiguration));
	}

	bool operator==(const GraphicsPipelineConfiguration &other) const
	{
		return memcmp(this, &other, sizeof(GraphicsPipelineConfiguration)) == 0;
	}
};

struct GraphicsPipelineConfigurationHasher
{
	size_t operator() (const GraphicsPipelineConfiguration &configuration) const
	{
		return XXH32(&configuration, sizeof(GraphicsPipelineConfiguration), 0);
	}
};

struct SamplerStateHasher
{
	size_t operator()(const SamplerState &samplerState) const
	{
		return XXH32(&samplerState, sizeof(SamplerState), 0);
	}
};

struct BatchedDrawBuffers
{
	StreamBuffer* vertexBuffer1;
	StreamBuffer* vertexBuffer2;
	StreamBuffer* indexBuffer;
	StreamBuffer* constantColorBuffer;

	~BatchedDrawBuffers()
	{
		delete vertexBuffer1;
		delete vertexBuffer2;
		delete indexBuffer;
		delete constantColorBuffer;
	}
};

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() const
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
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
	VkRenderPass renderPass = VK_NULL_HANDLE;
	VkPipeline pipeline = VK_NULL_HANDLE;
	std::vector<VkImage> transitionImages;
	uint32_t numColorAttachments = 0;
	float width = 0.0f;
	float height = 0.0f;
	VkSampleCountFlagBits msaa = VK_SAMPLE_COUNT_1_BIT;
};

class Graphics final : public love::graphics::Graphics
{
public:
#ifdef LOVE_ANDROID
	Graphics() {
		auto result = volkInitialize();
		if (result != VK_SUCCESS) {
			throw love::Exception("could not initialize volk");
		}
	}
#else
	Graphics() = default;
#endif

	virtual ~Graphics();

	const char *getName() const override;
	const VkDevice getDevice() const;
	const VkPhysicalDevice getPhysicalDevice() const;
	const VmaAllocator getVmaAllocator() const;

	// implementation for virtual functions
	love::graphics::Texture *newTexture(const love::graphics::Texture::Settings &settings, const love::graphics::Texture::Slices *data) override;
	love::graphics::Buffer *newBuffer(const love::graphics::Buffer::Settings &settings, const std::vector<love::graphics::Buffer::DataDeclaration>& format, const void *data, size_t size, size_t arraylength) override;
	void clear(OptionalColorD color, OptionalInt stencil, OptionalDouble depth) override;
	void clear(const std::vector<OptionalColorD> &colors, OptionalInt stencil, OptionalDouble depth) override;
	Matrix4 computeDeviceProjection(const Matrix4 &projection, bool rendertotexture) const override;
	void discard(const std::vector<bool> &colorbuffers, bool depthstencil) override { }
	void present(void *screenshotCallbackdata) override;
	void setViewportSize(int width, int height, int pixelwidth, int pixelheight) override;
	bool setMode(void *context, int width, int height, int pixelwidth, int pixelheight, bool windowhasstencil, int msaa) override;
	void unSetMode() override;
	void setActive(bool active) override;
	int getRequestedBackbufferMSAA() const override;
	int getBackbufferMSAA() const  override;
	void setColor(Colorf c) override;
	void setScissor(const Rect &rect) override;
	void setScissor() override;
	void setStencilMode(StencilAction action, CompareMode compare, int value, love::uint32 readmask, love::uint32 writemask) override;
	void setDepthMode(CompareMode compare, bool write) override;
	void setFrontFaceWinding(Winding winding) override;
	void setColorMask(ColorChannelMask mask) override;
	void setBlendState(const BlendState &blend) override;
	void setPointSize(float size) override;
	void setWireframe(bool enable) override;
	PixelFormat getSizedFormat(PixelFormat format, bool rendertarget, bool readable) const override;
	bool isPixelFormatSupported(PixelFormat format, uint32 usage, bool sRGB) override;
	Renderer getRenderer() const override;
	bool usesGLSLES() const override;
	RendererInfo getRendererInfo() const override;
	void draw(const DrawCommand &cmd) override;
	void draw(const DrawIndexedCommand &cmd) override;
	void drawQuads(int start, int count, const VertexAttributes &attributes, const BufferBindings &buffers, graphics::Texture *texture) override;

	graphics::GraphicsReadback *newReadbackInternal(ReadbackMethod method, love::graphics::Buffer *buffer, size_t offset, size_t size, data::ByteData *dest, size_t destoffset) override;
	graphics::GraphicsReadback *newReadbackInternal(ReadbackMethod method, love::graphics::Texture *texture, int slice, int mipmap, const Rect &rect, image::ImageData *dest, int destx, int desty) override;

	VkCommandBuffer getCommandBufferForDataTransfer();

	void queueCleanUp(std::function<void()> cleanUp);
	void addReadbackCallback(std::function<void()> callback);

	void submitGpuCommands(bool present);

	uint32_t getNumImagesInFlight() const;
	uint32_t getFrameIndex() const;

	const VkDeviceSize getMinUniformBufferOffsetAlignment() const;
	graphics::Texture *getDefaultTexture() const;
	VkSampler getCachedSampler(const SamplerState&);

	void setComputeShader(Shader*);
	std::set<Shader*> &getUsedShadersInFrame();

	const OptionalDeviceFeatures &getOptionalDeviceFeatures() const;
	const OptionalDeviceExtensionFunctions &getExtensionFunctions() const;

	graphics::Shader::BuiltinUniformData getCurrentBuiltinUniformData();

protected:
	graphics::ShaderStage *newShaderStageInternal(ShaderStageType stage, const std::string &cachekey, const std::string &source, bool gles) override;
	graphics::Shader *newShaderInternal(StrongRef<love::graphics::ShaderStage> stages[SHADERSTAGE_MAX_ENUM]) override;
	graphics::StreamBuffer *newStreamBuffer(BufferUsage type, size_t size) override;
	bool dispatch(int x, int y, int z) override;
	void initCapabilities() override;
	void getAPIStats(int &shaderswitches) const override;
	void setRenderTargetsInternal(const RenderTargets &rts, int pixelw, int pixelh, bool hasSRGBtexture) override;

private:
	void createVulkanInstance();
	bool checkValidationSupport();
	void pickPhysicalDevice();
	void getMaxUsableSampleCount();
	int rateDeviceSuitability(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	void createLogicalDevice();
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
	void createDefaultRenderPass();
	void createDefaultFramebuffers();
    VkFramebuffer createFramebuffer(FramebufferConfiguration &);
    VkFramebuffer getFramebuffer(FramebufferConfiguration &);
	void createDefaultShaders();
    VkRenderPass createRenderPass(RenderPassConfiguration &);
	VkPipeline createGraphicsPipeline(GraphicsPipelineConfiguration &);
	void createColorResources();
	VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat findDepthFormat();
	void createDepthResources();
	void createCommandPool();
	void createCommandBuffers();
	void createSyncObjects();
	void createDefaultTexture();
	void cleanup();
	void cleanupSwapChain();
	void recreateSwapChain();
	void initDynamicState();
	void beginFrame();
	void startRecordingGraphicsCommands(bool newFrame);
	void endRecordingGraphicsCommands(bool present);
	void ensureGraphicsPipelineConfiguration(GraphicsPipelineConfiguration &);
	void updatedBatchedDrawBuffers();
	bool usesConstantVertexColor(const VertexAttributes &);
	void createVulkanVertexFormat(
		VertexAttributes vertexAttributes, 
		std::vector<VkVertexInputBindingDescription> &bindingDescriptions, 
		std::vector<VkVertexInputAttributeDescription> &attributeDescriptions);
	void prepareDraw(const VertexAttributes &attributes, const BufferBindings &buffers, graphics::Texture *texture, PrimitiveType, CullMode);
	void setRenderPass(const RenderTargets &rts, int pixelw, int pixelh, bool hasSRGBtexture);
	void setDefaultRenderPass();
	void startRenderPass();
	void endRenderPass();
	VkSampler createSampler(const SamplerState&);

	VkInstance instance = VK_NULL_HANDLE;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	int requestedMsaa = 0;
	VkDevice device = VK_NULL_HANDLE; 
	OptionalInstanceExtensions optionalInstanceExtensions;
	OptionalDeviceFeatures optionalDeviceFeatures;
	OptionalDeviceExtensionFunctions ext;
	VkQueue graphicsQueue = VK_NULL_HANDLE;
	VkQueue presentQueue = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
	VkSurfaceTransformFlagBitsKHR preTransform = {};
	Matrix4 displayRotation;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat = VK_FORMAT_UNDEFINED;
	VkExtent2D swapChainExtent = VkExtent2D();
	std::vector<VkImageView> swapChainImageViews;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkImage colorImage = VK_NULL_HANDLE;
	VkImageView colorImageView = VK_NULL_HANDLE;
	VmaAllocation colorImageAllocation = VK_NULL_HANDLE;
	VkImage depthImage = VK_NULL_HANDLE;
	VkImageView depthImageView = VK_NULL_HANDLE;
	VmaAllocation depthImageAllocation = VK_NULL_HANDLE;
	VkRenderPass defaultRenderPass = VK_NULL_HANDLE;
	std::vector<VkFramebuffer> defaultFramebuffers;
    std::unordered_map<RenderPassConfiguration, VkRenderPass, RenderPassConfigurationHasher> renderPasses;
	std::unordered_map<FramebufferConfiguration, VkFramebuffer, FramebufferConfigurationHasher> framebuffers;
	std::unordered_map<GraphicsPipelineConfiguration, VkPipeline, GraphicsPipelineConfigurationHasher> graphicsPipelines;
	std::unordered_map<SamplerState, VkSampler, SamplerStateHasher> samplers;
	VkCommandPool commandPool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> commandBuffers;
	Shader* computeShader = nullptr;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;
	VkDeviceSize minUniformBufferOffsetAlignment = 0;
	bool imageRequested = false;
	size_t currentFrame = 0;
	uint32_t imageIndex = 0;
	bool framebufferResized = false;
	VmaAllocator vmaAllocator = VK_NULL_HANDLE;
	std::unique_ptr<Texture> standardTexture = nullptr;
	// we need an array of draw buffers, since the frames are being rendered asynchronously
	// and we can't (or shouldn't) update the contents of the buffers while they're still in flight / being rendered.
	std::vector<BatchedDrawBuffers> batchedDrawBuffers;
	// functions that need to be called to cleanup objects that were needed for rendering a frame.
	// just like batchedDrawBuffers we need a vector for each frame in flight.
	std::vector<std::vector<std::function<void()>>> cleanUpFunctions;
	std::vector<std::vector<std::function<void()>>> readbackCallbacks;
	std::set<Shader*> usedShadersInFrame;
	RenderpassState renderPassState;
};

} // vulkan
} // graphics
} // love
