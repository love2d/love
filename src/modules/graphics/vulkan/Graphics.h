#ifndef LOVE_GRAPHICS_VULKAN_GRAPHICS_H
#define LOVE_GRAPHICS_VULKAN_GRAPHICS_H

#include "graphics/Graphics.h"
#include "StreamBuffer.h"
#include "ShaderStage.h"
#include "Shader.h"
#include "Texture.h"
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

#include <common/config.h>

#include <optional>
#include <iostream>
#include <memory>


namespace love {
	namespace graphics {
		namespace vulkan {
			struct GraphicsPipelineConfiguration {
				std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions;
				std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;
				Shader* shader = nullptr;
				PrimitiveType primitiveType = PRIMITIVE_MAX_ENUM;
				VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
				BlendState blendState;
				ColorChannelMask colorChannelMask;
				Winding winding;
				CullMode cullmode;

				friend static bool operator==(const GraphicsPipelineConfiguration& first, const GraphicsPipelineConfiguration& other);
			};

			struct DecriptorSetConfiguration {
				graphics::Texture* texture;
				graphics::StreamBuffer* buffer;

				bool operator==(const DecriptorSetConfiguration& conf2) {
					return this->texture == conf2.texture && this->buffer == conf2.buffer;
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
				void clear(OptionalColorD color, OptionalInt stencil, OptionalDouble depth) override { std::cout << "clear1 "; }
				void clear(const std::vector<OptionalColorD>& colors, OptionalInt stencil, OptionalDouble depth) override { std::cout << "clear2 "; }
				Matrix4 computeDeviceProjection(const Matrix4& projection, bool rendertotexture) const override;
				void discard(const std::vector<bool>& colorbuffers, bool depthstencil) override { std::cout << "discard "; }
				void present(void* screenshotCallbackdata) override;
				void setViewportSize(int width, int height, int pixelwidth, int pixelheight) override;
				bool setMode(void* context, int width, int height, int pixelwidth, int pixelheight, bool windowhasstencil, int msaa) override;
				void unSetMode() override;
				void setActive(bool active) override { std::cout << "setActive "; }
				int getRequestedBackbufferMSAA() const override { std::cout << "getRequestedBackbufferMSAA "; return 0; }
				int getBackbufferMSAA() const  override { std::cout << "getBackbufferMSAA "; return 0; }
				void setColor(Colorf c) override;
				void setScissor(const Rect& rect) override { std::cout << "setScissor "; }
				void setScissor() override { std::cout << "setScissor2 "; }
				void setStencilMode(StencilAction action, CompareMode compare, int value, love::uint32 readmask, love::uint32 writemask) override { std::cout << "setStencilMode "; }
				void setDepthMode(CompareMode compare, bool write) override { std::cout << "setDepthMode "; }
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

				VkCommandBuffer beginSingleTimeCommands();
				void endSingleTimeCommands(VkCommandBuffer);

			protected:
				graphics::ShaderStage* newShaderStageInternal(ShaderStageType stage, const std::string& cachekey, const std::string& source, bool gles) override { 
					std::cout << "newShaderStageInternal "; 
					return new ShaderStage(this, stage, source, gles, cachekey); 
				}
				graphics::Shader* newShaderInternal(StrongRef<love::graphics::ShaderStage> stages[SHADERSTAGE_MAX_ENUM]) override { 
					std::cout << "newShaderInternal "; 
					return new Shader(stages);
				}
				graphics::StreamBuffer* newStreamBuffer(BufferUsage type, size_t size) override;
				bool dispatch(int x, int y, int z) override { std::cout << "dispatch "; return false; }
				void initCapabilities() override;
				void getAPIStats(int& shaderswitches) const override { std::cout << "getAPIStats "; }
				void setRenderTargetsInternal(const RenderTargets& rts, int pixelw, int pixelh, bool hasSRGBtexture) override { std::cout << "setRenderTargetsInternal "; }

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
				void createRenderPass();
				void createDefaultShaders();
				void createDescriptorSetLayout();
				void createDescriptorPool();
				std::vector<VkDescriptorSet> createDescriptorSets(DecriptorSetConfiguration);
				VkPipeline createGraphicsPipeline(GraphicsPipelineConfiguration);
				void createFramebuffers();
				void createCommandPool();
				void createCommandBuffers();
				void createSyncObjects();
				void createDefaultTexture();
				void createQuadIndexBuffer();
				void cleanup();
				void cleanupSwapChain();
				void recreateSwapChain();
				void startRecordingGraphicsCommands();
				void endRecordingGraphicsCommands();
				void ensureGraphicsPipelineConfiguration(GraphicsPipelineConfiguration);
				graphics::StreamBuffer* createUniformBufferFromData(graphics::Shader::BuiltinUniformData);
				graphics::Shader::BuiltinUniformData getCurrentBuiltinUniformData();
				void setTexture(graphics::Texture* texture);
				void updatedBatchedDrawBuffers();
				VkDescriptorSet* getDescriptorSet(int currentFrame);
				graphics::StreamBuffer* getUniformBuffer();
				void createVulkanVertexFormat(VertexAttributes vertexAttributes, bool& useConstantVertexColor, GraphicsPipelineConfiguration& configuration);
				void prepareDraw(const VertexAttributes& attributes, const BufferBindings& buffers, graphics::Texture* texture, PrimitiveType, CullMode);

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
				VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
				VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
				VkRenderPass renderPass = VK_NULL_HANDLE;
				VkPipeline currentGraphicsPipeline = VK_NULL_HANDLE;
				std::vector<std::pair<GraphicsPipelineConfiguration, VkPipeline>> graphicsPipelines;	// FIXME improve performance by using a hash map
				std::vector<VkPipelineLayout> graphicsPipelineLayouts;
				std::vector<VkFramebuffer> swapChainFramBuffers;
				VkCommandPool commandPool = VK_NULL_HANDLE;
				std::vector<VkCommandBuffer> commandBuffers;
				VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
				VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
				std::vector<VkSemaphore> imageAvailableSemaphores;
				std::vector<VkSemaphore> renderFinishedSemaphores;
				std::vector<VkFence> inFlightFences;
				std::vector<VkFence> imagesInFlight;
				size_t currentFrame = 0;
				uint32_t imageIndex = 0;
				bool framebufferResized = false;
				VmaAllocator vmaAllocator = VK_NULL_HANDLE;
				std::unique_ptr<Texture> standardTexture = nullptr;
				std::unique_ptr<StreamBuffer> quadIndexBuffer = nullptr;
				// we need an array of draw buffers, since the frames are being rendered asynchronously
				// and we can't (or shouldn't) update the contents of the buffers while they're still in flight / being rendered.
				std::vector<BatchedDrawBuffers> batchedDrawBuffers;
				graphics::Texture* currentTexture = nullptr;
				std::vector<std::pair<graphics::Shader::BuiltinUniformData, graphics::StreamBuffer*>> uniformBufferMap;
				std::vector<std::pair<DecriptorSetConfiguration, std::vector<VkDescriptorSet>>> descriptorSetsMap;
				VkPolygonMode currentPolygonMode = VK_POLYGON_MODE_FILL;
			};
		}
	}
}

#endif
