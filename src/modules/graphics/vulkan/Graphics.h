#ifndef LOVE_GRAPHICS_VULKAN_GRAPHICS_H
#define LOVE_GRAPHICS_VULKAN_GRAPHICS_H

// löve
#include "graphics/Graphics.h"
#include "StreamBuffer.h"
#include "ShaderStage.h"
#include "Shader.h"
#include "Texture.h"
#include <common/config.h>

// libraries
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"
#include "libraries/xxHash/xxhash.h"

// c++
#include <optional>
#include <iostream>
#include <memory>
#include <functional>


namespace love {
namespace graphics {
namespace vulkan {
struct RenderPassConfiguration {
    VkFormat frameBufferFormat;

    bool operator==(const RenderPassConfiguration& conf) const {
        return memcmp(this, &conf, sizeof(RenderPassConfiguration)) == 0;
    }
};

struct RenderPassConfigurationHasher {
    size_t operator()(const RenderPassConfiguration &configuration) const {
        return XXH32(&configuration, sizeof(RenderPassConfiguration), 0);
    }
};

struct GraphicsPipelineConfiguration {
    VkRenderPass renderPass;
	VertexAttributes vertexAttributes;
	Shader* shader = nullptr;
	PrimitiveType primitiveType = PRIMITIVE_MAX_ENUM;
	bool wireFrame;
	BlendState blendState;
	ColorChannelMask colorChannelMask;
	Winding winding;
	CullMode cullmode;
	float viewportWidth;
	float viewportHeight;
	std::optional<Rect> scissorRect;

	GraphicsPipelineConfiguration() {
		memset(this, 0, sizeof(GraphicsPipelineConfiguration));
	}

	bool operator==(const GraphicsPipelineConfiguration& other) const {
		return memcmp(this, &other, sizeof(GraphicsPipelineConfiguration)) == 0;
	}
};

struct GraphicsPipelineConfigurationHasher {
	size_t operator() (const GraphicsPipelineConfiguration &configuration) const {
		return XXH32(&configuration, sizeof(GraphicsPipelineConfiguration), 0);
	}
};

struct SamplerStateHasher {
	size_t operator()(const SamplerState &samplerState) const {
		return XXH32(&samplerState, sizeof(SamplerState), 0);
	}
};

struct BatchedDrawBuffers {
	StreamBuffer* vertexBuffer1;
	StreamBuffer* vertexBuffer2;
	StreamBuffer* indexBuffer;
	StreamBuffer* constantColorBuffer;

	~BatchedDrawBuffers() {
		delete vertexBuffer1;
		delete vertexBuffer2;
		delete indexBuffer;
		delete constantColorBuffer;
	}
};

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities{};
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class Graphics final : public love::graphics::Graphics {
public:
	Graphics() = default;

	virtual ~Graphics();

	const char* getName() const override;
	const VkDevice getDevice() const;
	const VkPhysicalDevice getPhysicalDevice() const;
	const VmaAllocator getVmaAllocator() const;

	// implementation for virtual functions
	love::graphics::Texture* newTexture(const love::graphics::Texture::Settings& settings, const love::graphics::Texture::Slices* data = nullptr) override;
	love::graphics::Buffer* newBuffer(const love::graphics::Buffer::Settings& settings, const std::vector<love::graphics::Buffer::DataDeclaration>& format, const void* data, size_t size, size_t arraylength) override;
	void clear(OptionalColorD color, OptionalInt stencil, OptionalDouble depth) override;
	void clear(const std::vector<OptionalColorD>& colors, OptionalInt stencil, OptionalDouble depth) override;
	Matrix4 computeDeviceProjection(const Matrix4& projection, bool rendertotexture) const override;
	void discard(const std::vector<bool>& colorbuffers, bool depthstencil) override { }
	void present(void* screenshotCallbackdata) override;
	void setViewportSize(int width, int height, int pixelwidth, int pixelheight) override;
	bool setMode(void* context, int width, int height, int pixelwidth, int pixelheight, bool windowhasstencil, int msaa) override;
	void unSetMode() override;
	void setActive(bool active) override;
	int getRequestedBackbufferMSAA() const override { return 0; }
	int getBackbufferMSAA() const  override { return 0; }
	void setColor(Colorf c) override;
	void setScissor(const Rect& rect) override;
	void setScissor() override;
	void setStencilMode(StencilAction action, CompareMode compare, int value, love::uint32 readmask, love::uint32 writemask) override { }
	void setDepthMode(CompareMode compare, bool write) override { }
	void setFrontFaceWinding(Winding winding) override;
	void setColorMask(ColorChannelMask mask) override;
	void setBlendState(const BlendState& blend) override;
	void setPointSize(float size) override;
	void setWireframe(bool enable) override;
	PixelFormat getSizedFormat(PixelFormat format, bool rendertarget, bool readable) const override;
	bool isPixelFormatSupported(PixelFormat format, uint32 usage, bool sRGB = false) override;
	Renderer getRenderer() const override;
	bool usesGLSLES() const override;
	RendererInfo getRendererInfo() const override;
	void draw(const DrawCommand& cmd) override;
	void draw(const DrawIndexedCommand& cmd) override;
	void drawQuads(int start, int count, const VertexAttributes& attributes, const BufferBindings& buffers, graphics::Texture* texture) override;

	GraphicsReadback* newReadbackInternal(ReadbackMethod method, love::graphics::Buffer* buffer, size_t offset, size_t size, data::ByteData* dest, size_t destoffset) override { return nullptr;  };
	GraphicsReadback* newReadbackInternal(ReadbackMethod method, love::graphics::Texture* texture, int slice, int mipmap, const Rect& rect, image::ImageData* dest, int destx, int desty) { return nullptr; }

	VkCommandBuffer getDataTransferCommandBuffer();
	void queueCleanUp(std::function<void()> cleanUp);

	uint32_t getNumImagesInFlight() const;
	const VkDeviceSize getMinUniformBufferOffsetAlignment() const;
	graphics::Texture* getDefaultTexture() const;
	VkSampler getCachedSampler(const SamplerState&);

protected:
	graphics::ShaderStage* newShaderStageInternal(ShaderStageType stage, const std::string& cachekey, const std::string& source, bool gles) override { 
		return new ShaderStage(this, stage, source, gles, cachekey); 
	}
	graphics::Shader* newShaderInternal(StrongRef<love::graphics::ShaderStage> stages[SHADERSTAGE_MAX_ENUM]) override { 
		return new Shader(stages);
	}
	graphics::StreamBuffer* newStreamBuffer(BufferUsage type, size_t size) override;
	bool dispatch(int x, int y, int z) override { return false; }
	void initCapabilities() override;
	void getAPIStats(int& shaderswitches) const override;
	void setRenderTargetsInternal(const RenderTargets& rts, int pixelw, int pixelh, bool hasSRGBtexture) override;

private:
	void createVulkanInstance();
	bool checkValidationSupport();
	void pickPhysicalDevice();
	int rateDeviceSuitability(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	void createLogicalDevice();
	void initVMA();
	void createSurface();
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void createSwapChain();
	void createImageViews();
    std::vector<VkFramebuffer> createSwapChainFramebuffers(VkRenderPass);
    VkFramebuffer getSwapChainFramebuffer(VkRenderPass);
	void createDefaultShaders();
    VkRenderPass createRenderPass(RenderPassConfiguration);
	VkPipeline createGraphicsPipeline(GraphicsPipelineConfiguration);
	void createCommandPool();
	void createCommandBuffers();
	void createSyncObjects();
	void createDefaultTexture();
	void cleanup();
	void cleanupSwapChain();
	void recreateSwapChain();
	void startRecordingGraphicsCommands();
	void endRecordingGraphicsCommands();
	void ensureGraphicsPipelineConfiguration(GraphicsPipelineConfiguration);
	graphics::Shader::BuiltinUniformData getCurrentBuiltinUniformData();
	void updatedBatchedDrawBuffers();
	bool usesConstantVertexColor(const VertexAttributes&);
	void createVulkanVertexFormat(
		VertexAttributes vertexAttributes, 
		std::vector<VkVertexInputBindingDescription> &bindingDescriptions, 
		std::vector<VkVertexInputAttributeDescription> &attributeDescriptions);
	void prepareDraw(const VertexAttributes& attributes, const BufferBindings& buffers, graphics::Texture* texture, PrimitiveType, CullMode);
	void startRenderPass(Texture*, uint32_t w, uint32_t h);
	void endRenderPass();
	VkSampler createSampler(const SamplerState&);

	VkInstance instance = VK_NULL_HANDLE;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VkQueue graphicsQueue = VK_NULL_HANDLE;
	VkQueue presentQueue = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkSwapchainKHR swapChain = VK_NULL_HANDLE;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat = VK_FORMAT_UNDEFINED;
	VkExtent2D swapChainExtent = VkExtent2D();
	std::vector<VkImageView> swapChainImageViews;
    std::unordered_map<VkRenderPass, std::vector<VkFramebuffer>> swapChainFramebufferVector;
	VkPipeline currentGraphicsPipeline = VK_NULL_HANDLE;
    VkRenderPass currentRenderPass = VK_NULL_HANDLE;
    std::unordered_map<RenderPassConfiguration, VkRenderPass, RenderPassConfigurationHasher> renderPasses;
	std::unordered_map<GraphicsPipelineConfiguration, VkPipeline, GraphicsPipelineConfigurationHasher> graphicsPipelines;
	std::unordered_map<SamplerState, VkSampler, SamplerStateHasher> samplers;
	VkCommandPool commandPool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkCommandBuffer> dataTransferCommandBuffers;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;
	VkDeviceSize minUniformBufferOffsetAlignment = 0;
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
	graphics::Texture* currentTexture = nullptr;

	// render pass variables.
	VkFormat currentFramebufferOutputFormat = VK_FORMAT_UNDEFINED;
	Texture* renderTargetTexture = nullptr;
	float currentViewportWidth = 0;
	float currentViewportHeight = 0;
};
} // vulkan
} // graphics
} // love

#endif
