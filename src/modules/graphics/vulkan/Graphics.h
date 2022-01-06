#ifndef LOVE_GRAPHICS_VULKAN_GRAPHICS_H
#define LOVE_GRAPHICS_VULKAN_GRAPHICS_H

#include "graphics/Graphics.h"
#include <vulkan/vulkan.h>

#include <common/config.h>

#include <optional>
#include <iostream>


namespace love {
	namespace graphics {
		namespace vulkan {
			class Graphics final : public love::graphics::Graphics {
			public:
				Graphics();

				void initVulkan();

				virtual ~Graphics();

				const char* getName() const override;

				const VkDevice getDevice() const {
					return device;
				}

				const VkPhysicalDevice getPhysicalDevice() const {
					return physicalDevice;
				}

				// implementation for virtual functions
				Texture* newTexture(const Texture::Settings& settings, const Texture::Slices* data = nullptr) override { return nullptr;  }
				love::graphics::Buffer* newBuffer(const love::graphics::Buffer::Settings& settings, const std::vector<love::graphics::Buffer::DataDeclaration>& format, const void* data, size_t size, size_t arraylength) override;
				void clear(OptionalColorD color, OptionalInt stencil, OptionalDouble depth) override {}
				void clear(const std::vector<OptionalColorD>& colors, OptionalInt stencil, OptionalDouble depth) override {}
				Matrix4 computeDeviceProjection(const Matrix4& projection, bool rendertotexture) const override { return Matrix4(); }
				void discard(const std::vector<bool>& colorbuffers, bool depthstencil) override { }
				void present(void* screenshotCallbackdata) override;
				void setViewportSize(int width, int height, int pixelwidth, int pixelheight) override;
				bool setMode(void* context, int width, int height, int pixelwidth, int pixelheight, bool windowhasstencil, int msaa) override { return false;  }
				void unSetMode() override {}
				void setActive(bool active) override {}
				int getRequestedBackbufferMSAA() const override { return 0;  }
				int getBackbufferMSAA() const  override { return 0;  }
				void setColor(Colorf c) override {}
				void setScissor(const Rect& rect) override {}
				void setScissor() override {}
				void setStencilMode(StencilAction action, CompareMode compare, int value, love::uint32 readmask, love::uint32 writemask) override {}
				void setDepthMode(CompareMode compare, bool write) override {}
				void setFrontFaceWinding(Winding winding) override {}
				void setColorMask(ColorChannelMask mask) override {}
				void setBlendState(const BlendState& blend) override {}
				void setPointSize(float size) override {}
				void setWireframe(bool enable) override {}
				PixelFormat getSizedFormat(PixelFormat format, bool rendertarget, bool readable) const override { return PIXELFORMAT_UNKNOWN;  }
				bool isPixelFormatSupported(PixelFormat format, PixelFormatUsageFlags usage, bool sRGB = false) override { return false;  }
				Renderer getRenderer() const override { return RENDERER_VULKAN; }
				bool usesGLSLES() const override { return false;  }
				RendererInfo getRendererInfo() const override { return {};  }
				void draw(const DrawCommand& cmd) override {}
				void draw(const DrawIndexedCommand& cmd) override {}
				void drawQuads(int start, int count, const VertexAttributes& attributes, const BufferBindings& buffers, Texture* texture) override {}

			protected:
				ShaderStage* newShaderStageInternal(ShaderStageType stage, const std::string& cachekey, const std::string& source, bool gles) override { return nullptr; }
				Shader* newShaderInternal(StrongRef<love::graphics::ShaderStage> stages[SHADERSTAGE_MAX_ENUM]) override { return nullptr;  }
				StreamBuffer* newStreamBuffer(BufferUsage type, size_t size) override { return nullptr;  }
				bool dispatch(int x, int y, int z) override { return false;  }
				void initCapabilities() override {}
				void getAPIStats(int& shaderswitches) const override {}
				void setRenderTargetsInternal(const RenderTargets& rts, int pixelw, int pixelh, bool hasSRGBtexture) override {}

			private:
				bool init = false;
				// vulkan specific member functions and variables

				struct QueueFamilyIndices {
					std::optional<uint32_t> graphicsFamily;
					std::optional<uint32_t> presentFamily;

					bool isComplete() {
						return graphicsFamily.has_value() && presentFamily.has_value();
					}
				};

				struct SwapChainSupportDetails {
					VkSurfaceCapabilitiesKHR capabilities;
					std::vector<VkSurfaceFormatKHR> formats;
					std::vector<VkPresentModeKHR> presentModes;
				};

				void createVulkanInstance();
				bool checkValidationSupport();
				void pickPhysicalDevice();
				int rateDeviceSuitability(VkPhysicalDevice device);
				QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
				void createLogicalDevice();
				void createSurface();
				bool checkDeviceExtensionSupport(VkPhysicalDevice device);
				SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
				VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
				VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
				VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
				void createSwapChain();
				void createImageViews();
				void createRenderPass();
				void createGraphicsPipeline();
				void createFramebuffers();
				void createCommandPool();
				void createCommandBuffers();
				void createSyncObjects();
				void cleanup();
				void cleanupSwapChain();
				void recreateSwapChain();

				VkInstance instance;
				VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
				VkDevice device;
				VkQueue graphicsQueue;
				VkQueue presentQueue;
				VkSurfaceKHR surface;
				VkSwapchainKHR swapChain;
				std::vector<VkImage> swapChainImages;
				VkFormat swapChainImageFormat;
				VkExtent2D swapChainExtent;
				std::vector<VkImageView> swapChainImageViews;
				VkPipelineLayout pipelineLayout;
				VkRenderPass renderPass;
				VkPipeline graphicsPipeline;
				std::vector<VkFramebuffer> swapChainFramBuffers;
				VkCommandPool commandPool;
				std::vector<VkCommandBuffer> commandBuffers;

				std::vector<VkSemaphore> imageAvailableSemaphores;
				std::vector<VkSemaphore> renderFinishedSemaphores;
				std::vector<VkFence> inFlightFences;
				std::vector<VkFence> imagesInFlight;
				size_t currentFrame = 0;
				bool framebufferResized = false;
			};
		}
	}
}

#endif
