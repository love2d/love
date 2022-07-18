#include "Graphics.h"
#include "Buffer.h"
#include "SDL_vulkan.h"
#include "window/Window.h"
#include "common/Exception.h"
#include "Shader.h"
#include "graphics/Texture.h"
#include "Vulkan.h"
#include "common/version.h"

#include <algorithm>
#include <vector>
#include <cstring>
#include <set>
#include <fstream>
#include <iostream>
#include <array>


namespace love {
	namespace graphics {
		namespace vulkan {
			static VkIndexType getVulkanIndexBufferType(IndexDataType type) {
				switch (type) {
				case INDEX_UINT16: return VK_INDEX_TYPE_UINT16;
				case INDEX_UINT32: return VK_INDEX_TYPE_UINT32;
				default:
					throw love::Exception("unknown Index Data type");
				}
			}

			const std::vector<const char*> validationLayers = {
				"VK_LAYER_KHRONOS_validation"
			};

			const std::vector<const char*> deviceExtensions = {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};

#ifdef NDEBUG
			constexpr bool enableValidationLayers = false;
#else
			constexpr bool enableValidationLayers = true;
#endif

			constexpr int MAX_FRAMES_IN_FLIGHT = 2;

			constexpr uint32_t vulkanApiVersion = VK_API_VERSION_1_3;

			const char* Graphics::getName() const {
				return "love.graphics.vulkan";
			}

			const VkDevice Graphics::getDevice() const {
				return device;
			}

			const VkPhysicalDevice Graphics::getPhysicalDevice() const {
				return physicalDevice;
			}

			const VmaAllocator Graphics::getVmaAllocator() const {
				return vmaAllocator;
			}

			Graphics::~Graphics() {
				// We already cleaned those up by clearing out batchedDrawBuffers. 
				// We set them to nullptr here so the base class doesn't crash
				// when it tries to free this.
				batchedDrawState.vb[0] = nullptr;
				batchedDrawState.vb[1] = nullptr;
				batchedDrawState.indexBuffer = nullptr;
			}

			// START OVERRIDEN FUNCTIONS

			love::graphics::Texture* Graphics::newTexture(const love::graphics::Texture::Settings& settings, const love::graphics::Texture::Slices* data) {
				return new Texture(this, settings, data);
			}

			love::graphics::Buffer* Graphics::newBuffer(const love::graphics::Buffer::Settings& settings, const std::vector<love::graphics::Buffer::DataDeclaration>& format, const void* data, size_t size, size_t arraylength) {
				return new Buffer(this, settings, format, data, size, arraylength);
			}

			// FIXME: clear stencil and depth missing.
			void Graphics::clear(OptionalColorD color, OptionalInt stencil, OptionalDouble depth) {
				VkClearAttachment attachment{};

				if (color.hasValue) {
					attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					attachment.clearValue.color.float32[0] = static_cast<float>(color.value.r);
					attachment.clearValue.color.float32[1] = static_cast<float>(color.value.g);
					attachment.clearValue.color.float32[2] = static_cast<float>(color.value.b);
					attachment.clearValue.color.float32[3] = static_cast<float>(color.value.a);
				}

				VkClearRect rect{};
				rect.layerCount = 1;
				rect.rect.extent.width = currentViewportWidth;
				rect.rect.extent.height = currentViewportHeight;

				vkCmdClearAttachments(commandBuffers[imageIndex], 1, &attachment, 1, &rect);
			}

			void Graphics::clear(const std::vector<OptionalColorD>& colors, OptionalInt stencil, OptionalDouble depth) {
				std::vector<VkClearAttachment> attachments;
				for (const auto& color : colors) {
					VkClearAttachment attachment{};
					if (color.hasValue) {
						attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
						attachment.clearValue.color.float32[0] = static_cast<float>(color.value.r);
						attachment.clearValue.color.float32[1] = static_cast<float>(color.value.g);
						attachment.clearValue.color.float32[2] = static_cast<float>(color.value.b);
						attachment.clearValue.color.float32[3] = static_cast<float>(color.value.a);
					}
					attachments.push_back(attachment);
				}

				VkClearRect rect{};
				rect.layerCount = 1;
				rect.rect.extent.width = currentViewportWidth;
				rect.rect.extent.height = currentViewportHeight;

				vkCmdClearAttachments(commandBuffers[imageIndex], static_cast<uint32_t>(attachments.size()), attachments.data(), 1, &rect);
			}

			void Graphics::present(void* screenshotCallbackdata) {
				if (!isActive()) {
					return;
				}

				flushBatchedDraws();

				endRecordingGraphicsCommands();

				if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
					vkWaitForFences(device, 1, &imagesInFlight.at(imageIndex), VK_TRUE, UINT64_MAX);
				}
				imagesInFlight[imageIndex] = inFlightFences[currentFrame];

				VkSubmitInfo submitInfo{};
				submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

				VkSemaphore waitSemaphores[] = { imageAvailableSemaphores.at(currentFrame) };
				VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
				submitInfo.waitSemaphoreCount = 1;
				submitInfo.pWaitSemaphores = waitSemaphores;
				submitInfo.pWaitDstStageMask = waitStages;

				submitInfo.commandBufferCount = 1;
				submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

				VkSemaphore signalSemaphores[] = { renderFinishedSemaphores.at(currentFrame) };
				submitInfo.signalSemaphoreCount = 1;
				submitInfo.pSignalSemaphores = signalSemaphores;

				vkResetFences(device, 1, &inFlightFences[currentFrame]);

				if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences.at(currentFrame)) != VK_SUCCESS) {
					throw love::Exception("failed to submit draw command buffer");
				}

				VkPresentInfoKHR presentInfo{};
				presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

				presentInfo.waitSemaphoreCount = 1;
				presentInfo.pWaitSemaphores = signalSemaphores;

				VkSwapchainKHR swapChains[] = { swapChain };
				presentInfo.swapchainCount = 1;
				presentInfo.pSwapchains = swapChains;

				presentInfo.pImageIndices = &imageIndex;

				VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);

				if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
					framebufferResized = false;
					recreateSwapChain();
				}
				else if (result != VK_SUCCESS) {
					throw love::Exception("failed to present swap chain image");
				}
				
				currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

				updatedBatchedDrawBuffers();
				startRecordingGraphicsCommands();
			}

			void Graphics::setViewportSize(int width, int height, int pixelwidth, int pixelheight) {
				this->width = width;
				this->height = height;
				this->pixelWidth = pixelwidth;
				this->pixelHeight = pixelheight;

				resetProjection();
			}

			bool Graphics::setMode(void* context, int width, int height, int pixelwidth, int pixelheight, bool windowhasstencil, int msaa) {
				createVulkanInstance();
				createSurface();
				pickPhysicalDevice();
				createLogicalDevice();
				initVMA();
				initCapabilities();
				createSwapChain();
				createImageViews();
				createDefaultShaders();
				createDescriptorSetLayout();
				createCommandPool();
				createCommandBuffers();
				createDefaultTexture();
				createQuadIndexBuffer();
				createDescriptorPool();
				createSyncObjects();
				startRecordingGraphicsCommands();
				currentFrame = 0;

				created = true;

				float whiteColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

				batchedDrawBuffers.clear();
				batchedDrawBuffers.reserve(MAX_FRAMES_IN_FLIGHT);
				for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
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

				Shader::current = Shader::standardShaders[graphics::Shader::StandardShader::STANDARD_DEFAULT];
				currentPolygonMode = VK_POLYGON_MODE_FILL;
				restoreState(states.back());
				
				setViewportSize(width, height, pixelwidth, pixelheight);
				renderTargetTexture = nullptr;
				currentViewportWidth = 0.0f;
				currentViewportHeight = 0.0f;

				Vulkan::resetShaderSwitches();

				return true;
			}

			void Graphics::initCapabilities() {
				// todo
				capabilities.features[FEATURE_MULTI_RENDER_TARGET_FORMATS] = false;
				capabilities.features[FEATURE_CLAMP_ZERO] = false;
				capabilities.features[FEATURE_CLAMP_ONE] = false;
				capabilities.features[FEATURE_BLEND_MINMAX] = false;
				capabilities.features[FEATURE_LIGHTEN] = false;
				capabilities.features[FEATURE_FULL_NPOT] = false;
				capabilities.features[FEATURE_PIXEL_SHADER_HIGHP] = true;
				capabilities.features[FEATURE_SHADER_DERIVATIVES] = false;
				capabilities.features[FEATURE_GLSL3] = true;
				capabilities.features[FEATURE_GLSL4] = true;
				capabilities.features[FEATURE_INSTANCING] = false;
				capabilities.features[FEATURE_TEXEL_BUFFER] = false;
				capabilities.features[FEATURE_INDEX_BUFFER_32BIT] = true;
				capabilities.features[FEATURE_COPY_BUFFER] = false;
				capabilities.features[FEATURE_COPY_BUFFER_TO_TEXTURE] = false;
				capabilities.features[FEATURE_COPY_TEXTURE_TO_BUFFER] = false;
				capabilities.features[FEATURE_COPY_RENDER_TARGET_TO_BUFFER] = false;
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
				capabilities.limits[LIMIT_THREADGROUPS_X] = 0;  // todo
				capabilities.limits[LIMIT_THREADGROUPS_Y] = 0;  // todo
				capabilities.limits[LIMIT_THREADGROUPS_Z] = 0;  // todo
				capabilities.limits[LIMIT_RENDER_TARGETS] = 1;	// todo
				capabilities.limits[LIMIT_TEXTURE_MSAA] = 1;	// todo
				capabilities.limits[LIMIT_ANISOTROPY] = 1.0f;	// todo
				static_assert(LIMIT_MAX_ENUM == 13, "Graphics::initCapabilities must be updated when adding a new system limit!");

				capabilities.textureTypes[TEXTURE_2D] = true;
				capabilities.textureTypes[TEXTURE_VOLUME] = false;
				capabilities.textureTypes[TEXTURE_2D_ARRAY] = false;
				capabilities.textureTypes[TEXTURE_CUBE] = false;
			}

			void Graphics::getAPIStats(int& shaderswitches) const {
				shaderswitches = Vulkan::getNumShaderSwitches();
			}

			void Graphics::unSetMode() {
				created = false;
				vkDeviceWaitIdle(device);
				Volatile::unloadAll();
				cleanup();
			}

			void Graphics::setActive(bool enable) {
				flushBatchedDraws();
				active = enable;
			}

			void Graphics::setFrontFaceWinding(Winding winding) {
				const auto& currentState = states.back();

				if (currentState.winding == winding) {
					return;
				}

				flushBatchedDraws();

				states.back().winding = winding;
			}

			void Graphics::setColorMask(ColorChannelMask mask) {
				flushBatchedDraws();

				states.back().colorMask = mask;
			}

			void Graphics::setBlendState(const BlendState& blend) {
				flushBatchedDraws();

				states.back().blend = blend;
			}

			void Graphics::setPointSize(float size) {
				if (size != states.back().pointSize)
					flushBatchedDraws();

				states.back().pointSize = size;
			}

			bool Graphics::usesGLSLES() const {
				return false;
			}
			
			Graphics::RendererInfo Graphics::getRendererInfo() const {
				VkPhysicalDeviceProperties deviceProperties;
				vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

				Graphics::RendererInfo info;
				info.device = deviceProperties.deviceName;
				info.vendor = Vulkan::getVendorName(deviceProperties.vendorID);
				info.version = Vulkan::getVulkanApiVersion(deviceProperties.apiVersion);
				info.name = "Vulkan";

				return info;
			}

			void Graphics::draw(const DrawCommand& cmd) {
				prepareDraw(*cmd.attributes, *cmd.buffers, cmd.texture, cmd.primitiveType, cmd.cullMode);

				vkCmdDraw(commandBuffers.at(imageIndex), static_cast<uint32_t>(cmd.vertexCount), static_cast<uint32_t>(cmd.instanceCount), static_cast<uint32_t>(cmd.vertexStart), 0);
			}

			void Graphics::draw(const DrawIndexedCommand& cmd) {
				prepareDraw(*cmd.attributes, *cmd.buffers, cmd.texture, cmd.primitiveType, cmd.cullMode);

				vkCmdBindIndexBuffer(commandBuffers.at(imageIndex), (VkBuffer)cmd.indexBuffer->getHandle(), static_cast<VkDeviceSize>(cmd.indexBufferOffset), getVulkanIndexBufferType(cmd.indexType));
				vkCmdDrawIndexed(commandBuffers.at(imageIndex), static_cast<uint32_t>(cmd.indexCount), static_cast<uint32_t>(cmd.instanceCount), 0, 0, 0);
			}

			void Graphics::drawQuads(int start, int count, const VertexAttributes& attributes, const BufferBindings& buffers, graphics::Texture* texture) {
				const int MAX_VERTICES_PER_DRAW = LOVE_UINT16_MAX;
				const int MAX_QUADS_PER_DRAW = MAX_VERTICES_PER_DRAW / 4;

				prepareDraw(attributes, buffers, texture, PRIMITIVE_TRIANGLES, CULL_BACK);

				vkCmdBindIndexBuffer(commandBuffers.at(imageIndex), (VkBuffer)quadIndexBuffer->getHandle(), 0, getVulkanIndexBufferType(INDEX_UINT16));

				int baseVertex = start * 4;

				for (int quadindex = 0; quadindex < count; quadindex += MAX_QUADS_PER_DRAW) {
					int quadcount = std::min(MAX_QUADS_PER_DRAW, count - quadindex);

					vkCmdDrawIndexed(commandBuffers.at(imageIndex), static_cast<uint32_t>(quadcount * 6), 1, 0, baseVertex, 0);
					baseVertex += quadcount * 4;
				}
			}

			void Graphics::setColor(Colorf c) {
				c.r = std::min(std::max(c.r, 0.0f), 1.0f);
				c.g = std::min(std::max(c.g, 0.0f), 1.0f);
				c.b = std::min(std::max(c.b, 0.0f), 1.0f);
				c.a = std::min(std::max(c.a, 0.0f), 1.0f);

				states.back().color = c;
			}

			void Graphics::setScissor(const Rect& rect) {
				flushBatchedDraws();
				
				states.back().scissor = true;
				states.back().scissorRect = rect;
			}

			void Graphics::setScissor() {
				flushBatchedDraws();

				states.back().scissor = false;
			}

			void Graphics::setWireframe(bool enable) {
				flushBatchedDraws();

				if (enable) {
					currentPolygonMode = VK_POLYGON_MODE_LINE;
				}
				else {
					currentPolygonMode = VK_POLYGON_MODE_FILL;
				}

				states.back().wireframe = enable;
			}

			PixelFormat Graphics::getSizedFormat(PixelFormat format, bool rendertarget, bool readable) const { 
				switch (format) {
				PIXELFORMAT_NORMAL:
					if (isGammaCorrect()) {
						return PIXELFORMAT_RGBA8_UNORM_sRGB;
					}
					else {
						return PIXELFORMAT_RGBA8_UNORM;
					}
				case PIXELFORMAT_HDR:
					return PIXELFORMAT_RGBA16_FLOAT;
				default:
					return format;
				}
			}

			bool Graphics::isPixelFormatSupported(PixelFormat format, uint32 usage, bool sRGB) { 
				return true;
			}

			Renderer Graphics::getRenderer() const {
				return RENDERER_VULKAN;
			}

			graphics::StreamBuffer* Graphics::newStreamBuffer(BufferUsage type, size_t size) {
				return new StreamBuffer(this, type, size);
			}

			Matrix4 Graphics::computeDeviceProjection(const Matrix4& projection, bool rendertotexture) const { 
				uint32 flags = DEVICE_PROJECTION_DEFAULT;
				return calculateDeviceProjection(projection, flags);
			}
			
			void Graphics::setRenderTargetsInternal(const RenderTargets& rts, int pixelw, int pixelh, bool hasSRGBtexture) {
				endRenderPass();

				if (rts.colors.size() == 0) {
					startRenderPass(nullptr, swapChainExtent.width, swapChainExtent.height);
				} else {
					auto& firstRenderTarget = rts.getFirstTarget();
					startRenderPass(static_cast<Texture*>(firstRenderTarget.texture), pixelw, pixelh);
				}
			}

			// END IMPLEMENTATION OVERRIDDEN FUNCTIONS

			void Graphics::startRecordingGraphicsCommands() {
				vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

				while (true) {
					VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
					if (result == VK_ERROR_OUT_OF_DATE_KHR) {
						recreateSwapChain();
						continue;
					}
					else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
						throw love::Exception("failed to acquire swap chain image");
					}

					break;
				}

				VkCommandBufferBeginInfo beginInfo{};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.flags = 0;
				beginInfo.pInheritanceInfo = nullptr;

				if (vkBeginCommandBuffer(commandBuffers.at(imageIndex), &beginInfo) != VK_SUCCESS) {
					throw love::Exception("failed to begin recording command buffer");
				}

				Vulkan::cmdTransitionImageLayout(commandBuffers.at(imageIndex), swapChainImages[imageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

				startRenderPass(nullptr, swapChainExtent.width, swapChainExtent.height);

				Vulkan::resetShaderSwitches();
			}

			void Graphics::endRecordingGraphicsCommands() {
				const auto& commandBuffer = commandBuffers.at(imageIndex);

				endRenderPass();

				Vulkan::cmdTransitionImageLayout(commandBuffers.at(imageIndex), swapChainImages[imageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

				if (vkEndCommandBuffer(commandBuffers.at(imageIndex)) != VK_SUCCESS) {
					throw love::Exception("failed to record command buffer");
				}
			}

			void Graphics::setTexture(graphics::Texture* texture) {
				currentTexture = texture;
			}

			void Graphics::updatedBatchedDrawBuffers() {
				batchedDrawState.vb[0] = batchedDrawBuffers[currentFrame].vertexBuffer1;
				batchedDrawState.vb[0]->nextFrame();
				batchedDrawState.vb[1] = batchedDrawBuffers[currentFrame].vertexBuffer2; 
				batchedDrawState.vb[1]->nextFrame();
				batchedDrawState.indexBuffer = batchedDrawBuffers[currentFrame].indexBuffer;
				batchedDrawState.indexBuffer->nextFrame();
			}

			VkDescriptorSet* Graphics::getDescriptorSet(int currentFrame) {
				DecriptorSetConfiguration config{};
				config.texture = currentTexture;
				config.buffer = getUniformBuffer();
				for (auto i = 0; i < descriptorSetsMap.size(); i++) {
					if (descriptorSetsMap[i].first == config) {
						return &descriptorSetsMap[i].second[currentFrame];
					}
				}
				auto descriptorSets = createDescriptorSets(config);
				descriptorSetsMap.push_back(std::make_pair(config, descriptorSets));
				return &descriptorSetsMap.back().second[currentFrame];
			}

			graphics::StreamBuffer* Graphics::getUniformBuffer() {
				auto data = getCurrentBuiltinUniformData();
				for (auto &it : uniformBufferMap) {
					if (it.first == data) {
						return it.second;
					}
				}
				auto buffer = createUniformBufferFromData(data);
				uniformBufferMap.push_back(std::make_pair(data, buffer));
				return buffer;
			}

			VkCommandBuffer Graphics::beginSingleTimeCommands() {
				VkCommandBufferAllocateInfo allocInfo{};
				allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				allocInfo.commandPool = commandPool;
				allocInfo.commandBufferCount = 1;

				VkCommandBuffer commandBuffer;
				vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

				VkCommandBufferBeginInfo beginInfo{};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

				vkBeginCommandBuffer(commandBuffer, &beginInfo);

				return commandBuffer;
			}

			void Graphics::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
				vkEndCommandBuffer(commandBuffer);

				VkSubmitInfo submitInfo{};
				submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submitInfo.commandBufferCount = 1;
				submitInfo.pCommandBuffers = &commandBuffer;

				vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
				vkQueueWaitIdle(graphicsQueue);
				vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
			}

			graphics::Shader::BuiltinUniformData Graphics::getCurrentBuiltinUniformData() {
				love::graphics::Shader::BuiltinUniformData data;

				data.transformMatrix = getTransform();
				data.projectionMatrix = getDeviceProjection();

				// The normal matrix is the transpose of the inverse of the rotation portion
				// (top-left 3x3) of the transform matrix.
				{
					Matrix3 normalmatrix = Matrix3(data.transformMatrix).transposedInverse();
					const float* e = normalmatrix.getElements();
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

				data.screenSizeParams.x = swapChainExtent.width;
				data.screenSizeParams.y = swapChainExtent.height;

				data.screenSizeParams.z = 1.0f;
				data.screenSizeParams.w = 0.0f;

				data.constantColor = getColor();
				gammaCorrectColor(data.constantColor);

				return data;
			}

			graphics::StreamBuffer* Graphics::createUniformBufferFromData(graphics::Shader::BuiltinUniformData data) {
				auto buffer = newStreamBuffer(BUFFERUSAGE_UNIFORM, sizeof(data));
				auto mappedInfo = buffer->map(0);
				memcpy(mappedInfo.data, &data, sizeof(data));
				buffer->unmap(0);

				return buffer;
			}

			void Graphics::createVulkanInstance() {
				if (enableValidationLayers && !checkValidationSupport()) {
					throw love::Exception("validation layers requested, but not available");
				}

				VkApplicationInfo appInfo{};
				appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
				appInfo.pApplicationName = "LOVE";
				appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);	//todo, get this version from somewhere else?
				appInfo.pEngineName = "LOVE Engine";
				appInfo.engineVersion = VK_MAKE_VERSION(VERSION_MAJOR, VERSION_MINOR, VERSION_REV);
				appInfo.apiVersion = vulkanApiVersion;

				VkInstanceCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
				createInfo.pApplicationInfo = &appInfo;
				createInfo.pNext = nullptr;

				auto window = Module::getInstance<love::window::Window>(M_WINDOW);
				const void* handle = window->getHandle();

				unsigned int count;
				if (SDL_Vulkan_GetInstanceExtensions((SDL_Window*)handle, &count, nullptr) != SDL_TRUE) {
					throw love::Exception("couldn't retrieve sdl vulkan extensions");
				}

				std::vector<const char*> extensions = {};	// can add more here
				size_t addition_extension_count = extensions.size();
				extensions.resize(addition_extension_count + count);

				if (SDL_Vulkan_GetInstanceExtensions((SDL_Window*)handle, &count, extensions.data() + addition_extension_count) != SDL_TRUE) {
					throw love::Exception("couldn't retrieve sdl vulkan extensions");
				}

				createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
				createInfo.ppEnabledExtensionNames = extensions.data();

				if (enableValidationLayers) {
					createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
					createInfo.ppEnabledLayerNames = validationLayers.data();
				}
				else {
					createInfo.enabledLayerCount = 0;
					createInfo.ppEnabledLayerNames = nullptr;
				}

				if (vkCreateInstance(
					&createInfo, 
					nullptr, 
					&instance) != VK_SUCCESS) {
					throw love::Exception("couldn't create vulkan instance");
				}
			}

			bool Graphics::checkValidationSupport() {
				uint32_t layerCount;
				vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

				std::vector<VkLayerProperties> availableLayers(layerCount);
				vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

				for (const char* layerName : validationLayers) {
					bool layerFound = false;

					for (const auto& layerProperties : availableLayers) {
						if (strcmp(layerName, layerProperties.layerName) == 0) {
							layerFound = true;
							break;
						}
					}

					if (!layerFound) {
						return false;
					}
				}

				return true;
			}

			void Graphics::pickPhysicalDevice() {
				uint32_t deviceCount = 0;
				vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

				if (deviceCount == 0) {
					throw love::Exception("failed to find GPUs with Vulkan support");
				}

				std::vector<VkPhysicalDevice> devices(deviceCount);
				vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

				std::multimap<int, VkPhysicalDevice> candidates;

				for (const auto& device : devices) {
					int score = rateDeviceSuitability(device);
					candidates.insert(std::make_pair(score, device));
				}

				if (candidates.rbegin()->first > 0) {
					physicalDevice = candidates.rbegin()->second;
				}
				else {
					throw love::Exception("failed to find a suitable gpu");
				}
			}

			bool Graphics::checkDeviceExtensionSupport(VkPhysicalDevice device) {
				uint32_t extensionCount;
				vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

				std::vector<VkExtensionProperties> availableExtensions(extensionCount);
				vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

				std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

				for (const auto& extension : availableExtensions) {
					requiredExtensions.erase(extension.extensionName);
				}

				return requiredExtensions.empty();
			}

			// if the score is nonzero then the device is suitable.
			// A higher rating means generally better performance
			// if the score is 0 the device is unsuitable
			int Graphics::rateDeviceSuitability(VkPhysicalDevice device) {
				VkPhysicalDeviceProperties deviceProperties;
				VkPhysicalDeviceFeatures deviceFeatures;
				vkGetPhysicalDeviceProperties(device, &deviceProperties);
				vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

				int score = 1;

				// optional 

				if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
					score += 1000;
				}
				if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
					score += 100;
				}
				if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU) {
					score += 10;
				}

				// definitely needed

				QueueFamilyIndices indices = findQueueFamilies(device);
				if (!indices.isComplete()) {
					score = 0;
				}

				bool extensionsSupported = checkDeviceExtensionSupport(device);
				if (!extensionsSupported) {
					score = 0;
				}

				if (extensionsSupported) {
					auto swapChainSupport = querySwapChainSupport(device);
					bool swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
					if (!swapChainAdequate) {
						score = 0;
					}
				}

				if (!deviceFeatures.samplerAnisotropy) {
					score = 0;
				}

				if (!deviceFeatures.fillModeNonSolid) {
					score = 0;
				}

				return score;
			}

			QueueFamilyIndices Graphics::findQueueFamilies(VkPhysicalDevice device) {
				QueueFamilyIndices indices;

				uint32_t queueFamilyCount = 0;
				vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

				std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
				vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

				int i = 0;
				for (const auto& queueFamily : queueFamilies) {
					if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
						indices.graphicsFamily = i;
					}

					VkBool32 presentSupport = false;
					vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

					if (presentSupport) {
						indices.presentFamily = i;
					}

					if (indices.isComplete()) {
						break;
					}

					i++;
				}

				return indices;
			}

			void Graphics::createLogicalDevice() {
				QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

				std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
				std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

				float queuePriority = 1.0f;
				for (uint32_t queueFamily : uniqueQueueFamilies) {
					VkDeviceQueueCreateInfo queueCreateInfo{};
					queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
					queueCreateInfo.queueFamilyIndex = queueFamily;
					queueCreateInfo.queueCount = 1;
					queueCreateInfo.pQueuePriorities = &queuePriority;
					queueCreateInfos.push_back(queueCreateInfo);
				}

				VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeature{};
				dynamicRenderingFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
				dynamicRenderingFeature.dynamicRendering = VK_TRUE;

				VkPhysicalDeviceFeatures deviceFeatures{};
				deviceFeatures.samplerAnisotropy = VK_TRUE;

				VkDeviceCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
				createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
				createInfo.pQueueCreateInfos = queueCreateInfos.data();
				createInfo.pEnabledFeatures = &deviceFeatures;
				createInfo.pNext = &dynamicRenderingFeature;

				createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
				createInfo.ppEnabledExtensionNames = deviceExtensions.data();

				// can this be removed?
				if (enableValidationLayers) {
					createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
					createInfo.ppEnabledLayerNames = validationLayers.data();
				}
				else {
					createInfo.enabledLayerCount = 0;
				}

				if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
					throw love::Exception("failed to create logical device");
				}

				vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
				vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
			}

			void Graphics::initVMA() {
				VmaVulkanFunctions vulkanFunctions = {};
				vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
				vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

				VmaAllocatorCreateInfo allocatorCreateInfo = {};
				allocatorCreateInfo.vulkanApiVersion = vulkanApiVersion;
				allocatorCreateInfo.physicalDevice = physicalDevice;
				allocatorCreateInfo.device = device;
				allocatorCreateInfo.instance = instance;
				allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

				if (vmaCreateAllocator(&allocatorCreateInfo, &vmaAllocator) != VK_SUCCESS) {
					throw love::Exception("failed to create vma allocator");
				}
			}

			void Graphics::createSurface() {
				auto window = Module::getInstance<love::window::Window>(M_WINDOW);
				const void* handle = window->getHandle();
				if (SDL_Vulkan_CreateSurface((SDL_Window*)handle, instance, &surface) != SDL_TRUE) {
					throw love::Exception("failed to create window surface");
				}
			}

			SwapChainSupportDetails Graphics::querySwapChainSupport(VkPhysicalDevice device) {
				SwapChainSupportDetails details;

				vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

				uint32_t formatCount;
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

				if (formatCount != 0) {
					details.formats.resize(formatCount);
					vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
				}

				uint32_t presentModeCount;
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

				if (presentModeCount != 0) {
					details.presentModes.resize(presentModeCount);
					vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
				}

				return details;
			}

			void Graphics::createSwapChain() {
				SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

				VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
				VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
				VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

				uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
				if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
					imageCount = swapChainSupport.capabilities.maxImageCount;
				}

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

				if (indices.graphicsFamily != indices.presentFamily) {
					createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
					createInfo.queueFamilyIndexCount = 2;
					createInfo.pQueueFamilyIndices = queueFamilyIndices;
				}
				else {
					createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
					createInfo.queueFamilyIndexCount = 0;
					createInfo.pQueueFamilyIndices = nullptr;
				}

				createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
				createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
				createInfo.presentMode = presentMode;
				createInfo.clipped = VK_TRUE;
				createInfo.oldSwapchain = VK_NULL_HANDLE;

				if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
					throw love::Exception("failed to create swap chain");
				}

				vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
				swapChainImages.resize(imageCount);
				vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

				swapChainImageFormat = surfaceFormat.format;
				swapChainExtent = extent;
			}

			VkSurfaceFormatKHR Graphics::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
				for (const auto& availableFormat : availableFormats) {
					if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
						return availableFormat;
					}
				}

				return availableFormats[0];
			}

			VkPresentModeKHR Graphics::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
				// needed ?
				for (const auto& availablePresentMode : availablePresentModes) {
					if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
						return availablePresentMode;
					}
				}

				return VK_PRESENT_MODE_FIFO_KHR;
			}

			VkExtent2D Graphics::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
				if (capabilities.currentExtent.width != UINT32_MAX) {
					return capabilities.currentExtent;
				}
				else {
					auto window = Module::getInstance<love::window::Window>(M_WINDOW);
					const void* handle = window->getHandle();

					int width, height;
					// is this the equivalent of glfwGetFramebufferSize ?
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

			void Graphics::createImageViews() {
				swapChainImageViews.resize(swapChainImages.size());

				for (size_t i = 0; i < swapChainImages.size(); i++) {
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

					if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews.at(i)) != VK_SUCCESS) {
						throw love::Exception("failed to create image views");
					}
				}
			}

			void Graphics::createDefaultShaders() {
				for (int i = 0; i < Shader::STANDARD_MAX_ENUM; i++) {
					auto stype = (Shader::StandardShader)i;

					if (!Shader::standardShaders[i]) {
						std::vector<std::string> stages;
						stages.push_back(Shader::getDefaultCode(stype, SHADERSTAGE_VERTEX));
						stages.push_back(Shader::getDefaultCode(stype, SHADERSTAGE_PIXEL));
						Shader::standardShaders[i] = newShader(stages, {});
					}
				}
			}

			void Graphics::createDescriptorSetLayout() {
				VkDescriptorSetLayoutBinding uboLayoutBinding{};
				uboLayoutBinding.binding = 0;
				uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				uboLayoutBinding.descriptorCount = 1;
				uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

				VkDescriptorSetLayoutBinding samplerLayoutBinding{};
				samplerLayoutBinding.binding = 1;
				samplerLayoutBinding.descriptorCount = 1;
				samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				samplerLayoutBinding.pImmutableSamplers = nullptr;
				samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

				std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
				VkDescriptorSetLayoutCreateInfo layoutInfo{};
				layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
				layoutInfo.pBindings = bindings.data();

				if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
					throw love::Exception("failed to create descriptor set layout");
				}
			}

			void Graphics::createDescriptorPool() {
				std::array<VkDescriptorPoolSize, 2> poolSizes{};
				poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
				poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

				VkDescriptorPoolCreateInfo poolInfo{};
				poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
				poolInfo.pPoolSizes = poolSizes.data();
				// FIXME: When using more than 128 textures at once we will run out of memory.
				// we probably want to reuse descriptors per flight image
				// and use multiple pools in case of too many allocations
				poolInfo.maxSets = 128 * static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

				if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
					throw love::Exception("failed to create descriptor pool");
				}
			}

			std::vector<VkDescriptorSet> Graphics::createDescriptorSets(DecriptorSetConfiguration config) {
				std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
				VkDescriptorSetAllocateInfo allocInfo{};
				allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				allocInfo.descriptorPool = descriptorPool;
				allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
				allocInfo.pSetLayouts = layouts.data();

				std::vector<VkDescriptorSet> newDescriptorSets;

				newDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
				VkResult result = vkAllocateDescriptorSets(device, &allocInfo, newDescriptorSets.data());
				if (result != VK_SUCCESS) {
					switch (result) {
					case VK_ERROR_OUT_OF_HOST_MEMORY:
						throw love::Exception("failed to allocate descriptor sets: out of host memory");
					case VK_ERROR_OUT_OF_DEVICE_MEMORY:
						throw love::Exception("failed to allocate descriptor sets: out of device memory");
					case VK_ERROR_FRAGMENTED_POOL:
						throw love::Exception("failed to allocate descriptor sets: fragmented pool");
					case VK_ERROR_OUT_OF_POOL_MEMORY:
						throw love::Exception("failed to allocate descriptor sets: out of pool memory");
					default:
						throw love::Exception("failed to allocate descriptor sets");
					}
				}

				for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
					VkDescriptorBufferInfo bufferInfo{};
					bufferInfo.buffer = (VkBuffer)config.buffer->getHandle();
					bufferInfo.offset = 0;
					bufferInfo.range = sizeof(graphics::Shader::BuiltinUniformData);

					VkDescriptorImageInfo imageInfo{};
					imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
					Texture* vkTexture = (Texture*)config.texture;
					imageInfo.imageView = vkTexture->getImageView();
					imageInfo.sampler = vkTexture->getSampler();

					std::array<VkWriteDescriptorSet, 2> descriptorWrite{};
					descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptorWrite[0].dstSet = newDescriptorSets[i];
					descriptorWrite[0].dstBinding = 0;
					descriptorWrite[0].dstArrayElement = 0;
					descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					descriptorWrite[0].descriptorCount = 1;
					descriptorWrite[0].pBufferInfo = &bufferInfo;

					descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptorWrite[1].dstSet = newDescriptorSets[i];
					descriptorWrite[1].dstBinding = 1;
					descriptorWrite[1].dstArrayElement = 0;
					descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					descriptorWrite[1].descriptorCount = 1;
					descriptorWrite[1].pImageInfo = &imageInfo;

					vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrite.size()), descriptorWrite.data(), 0, nullptr);
				}

				return newDescriptorSets;
			}

			void Graphics::createVulkanVertexFormat(
				VertexAttributes vertexAttributes, 
				bool& useConstantVertexColor,
				GraphicsPipelineConfiguration& configuration) {
				std::set<uint32_t> usedBuffers;
				std::vector<VkVertexInputBindingDescription> bindingDescriptions;
				std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

				auto allBits = vertexAttributes.enableBits;

				bool usesColor = false;

				uint8_t highestBufferBinding = 0;
				
				for (uint32_t i = 0; i < VertexAttributes::MAX; i++) {	// change to loop like in opengl implementation ?
					uint32 bit = 1u << i;
					if (allBits & bit) {
						if (i == ATTRIB_COLOR) {
							usesColor = true;
						}

						auto attrib = vertexAttributes.attribs[i];
						auto bufferBinding = attrib.bufferIndex;
						if (usedBuffers.find(bufferBinding) == usedBuffers.end()) {	// use .contains() when c++20 is enabled
							usedBuffers.insert(bufferBinding);

							VkVertexInputBindingDescription bindingDescription{};
							bindingDescription.binding = bufferBinding;
							if (vertexAttributes.instanceBits & (1u << bufferBinding)) {
								bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
							}
							else {
								bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
							}
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
				if (!usesColor) {
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

					useConstantVertexColor = true;
				}
				else {
					useConstantVertexColor = false;
				}

				configuration.vertexInputBindingDescriptions = bindingDescriptions;
				configuration.vertexInputAttributeDescriptions = attributeDescriptions;
			}

			void Graphics::prepareDraw(const VertexAttributes& attributes, const BufferBindings& buffers, graphics::Texture* texture, PrimitiveType primitiveType, CullMode cullmode) {
				GraphicsPipelineConfiguration configuration;
				configuration.shader = (Shader*)Shader::current;
				configuration.primitiveType = primitiveType;
				configuration.polygonMode = currentPolygonMode;
				configuration.blendState = states.back().blend;
				configuration.winding = states.back().winding;
				configuration.cullmode = cullmode;
				configuration.framebufferFormat = currentFramebufferOutputFormat;
				configuration.viewportWidth = currentViewportWidth;
				configuration.viewportHeight = currentViewportHeight;
				if (states.back().scissor) {
					configuration.scissorRect = states.back().scissorRect;
				}
				else {
					configuration.scissorRect = std::nullopt;
				}
				std::vector<VkBuffer> bufferVector;

				std::vector<VkDeviceSize> offsets;

				bool useConstantColorBuffer;
				createVulkanVertexFormat(attributes, useConstantColorBuffer, configuration);

				for (uint32_t i = 0; i < VertexAttributes::MAX; i++) {
					if (buffers.useBits & (1u << i)) {
						bufferVector.push_back((VkBuffer)buffers.info[i].buffer->getHandle());
						offsets.push_back((VkDeviceSize)buffers.info[i].offset);
					}
				}

				if (useConstantColorBuffer) {
					bufferVector.push_back((VkBuffer)batchedDrawBuffers[currentFrame].constantColorBuffer->getHandle());
					offsets.push_back((VkDeviceSize)0);
				}

				if (texture == nullptr) {
					setTexture(standardTexture.get());
				}
				else {
					setTexture(texture);
				}

				ensureGraphicsPipelineConfiguration(configuration);

				vkCmdBindDescriptorSets(commandBuffers.at(imageIndex), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, getDescriptorSet(currentFrame), 0, nullptr);
				vkCmdBindVertexBuffers(commandBuffers.at(imageIndex), 0, static_cast<uint32_t>(bufferVector.size()), bufferVector.data(), offsets.data());
			}

			void Graphics::startRenderPass(Texture* texture, uint32_t w, uint32_t h) {
				VkRenderingAttachmentInfo colorAttachmentInfo{};
				colorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
				colorAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				colorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				if (texture) {
					colorAttachmentInfo.imageView = texture->getImageView();
					auto vulkanFormat = Vulkan::getTextureFormat(texture->getPixelFormat());
					currentFramebufferOutputFormat = vulkanFormat.internalFormat;

					renderTargetTexture = texture;
				} else {
					colorAttachmentInfo.imageView = swapChainImageViews[imageIndex];
					currentFramebufferOutputFormat = swapChainImageFormat;

					renderTargetTexture = nullptr;
				}
				colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

				VkRenderingInfo renderingInfo{};
				renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
				renderingInfo.renderArea.extent.width = w;
				renderingInfo.renderArea.extent.height = h;
				renderingInfo.layerCount = 1;
				renderingInfo.colorAttachmentCount = 1;
				renderingInfo.pColorAttachments = &colorAttachmentInfo;

				currentViewportWidth = (float)w;
				currentViewportHeight = (float)h;

				if (renderTargetTexture) {
					Vulkan::cmdTransitionImageLayout(commandBuffers.at(imageIndex), (VkImage)texture->getHandle(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
				}

				vkCmdBeginRendering(commandBuffers.at(imageIndex), &renderingInfo);

				currentGraphicsPipeline = VK_NULL_HANDLE;
			}

			void Graphics::endRenderPass() {
				vkCmdEndRendering(commandBuffers.at(imageIndex));

				if (renderTargetTexture) {
					Vulkan::cmdTransitionImageLayout(commandBuffers.at(imageIndex), (VkImage)renderTargetTexture->getHandle(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
					renderTargetTexture = nullptr;
				}
			}

			VkPipeline Graphics::createGraphicsPipeline(GraphicsPipelineConfiguration configuration) {
				auto &shaderStages = configuration.shader->getShaderStages();

				VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
				vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
				vertexInputInfo.vertexBindingDescriptionCount = configuration.vertexInputBindingDescriptions.size();
				vertexInputInfo.pVertexBindingDescriptions = configuration.vertexInputBindingDescriptions.data();
				vertexInputInfo.vertexAttributeDescriptionCount = configuration.vertexInputAttributeDescriptions.size();
				vertexInputInfo.pVertexAttributeDescriptions = configuration.vertexInputAttributeDescriptions.data();

				VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
				inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
				inputAssembly.topology = Vulkan::getPrimitiveTypeTopology(configuration.primitiveType);
				inputAssembly.primitiveRestartEnable = VK_FALSE;
				
				VkViewport viewport{};
				viewport.x = 0.0f;
				viewport.y = 0.0f;
				viewport.width = configuration.viewportWidth;
				viewport.height = configuration.viewportHeight;
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;

				VkRect2D scissor{};
				if (configuration.scissorRect.has_value()) {
					scissor.offset.x = configuration.scissorRect.value().x;
					scissor.offset.y = configuration.scissorRect.value().y;
					scissor.extent.width = static_cast<uint32_t>(configuration.scissorRect.value().w);
					scissor.extent.height = static_cast<uint32_t>(configuration.scissorRect.value().h);
				}
				else {
					scissor.offset = { 0, 0 };
					scissor.extent = swapChainExtent;
				}

				VkPipelineViewportStateCreateInfo viewportState{};
				viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
				viewportState.viewportCount = 1;
				viewportState.pViewports = &viewport;
				viewportState.scissorCount = 1;
				viewportState.pScissors = &scissor;

				VkPipelineRasterizationStateCreateInfo rasterizer{};
				rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
				rasterizer.depthClampEnable = VK_FALSE;
				rasterizer.rasterizerDiscardEnable = VK_FALSE;
				rasterizer.polygonMode = configuration.polygonMode;
				rasterizer.lineWidth = 1.0f;
				rasterizer.cullMode = Vulkan::getCullMode(configuration.cullmode);
				rasterizer.frontFace = Vulkan::getFrontFace(configuration.winding);
				rasterizer.depthBiasEnable = VK_FALSE;
				rasterizer.depthBiasConstantFactor = 0.0f;
				rasterizer.depthBiasClamp = 0.0f;
				rasterizer.depthBiasSlopeFactor = 0.0f;

				VkPipelineMultisampleStateCreateInfo multisampling{};
				multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
				multisampling.sampleShadingEnable = VK_FALSE;
				multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
				multisampling.minSampleShading = 1.0f; // Optional
				multisampling.pSampleMask = nullptr; // Optional
				multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
				multisampling.alphaToOneEnable = VK_FALSE; // Optional

				VkPipelineColorBlendAttachmentState colorBlendAttachment{};
				colorBlendAttachment.colorWriteMask = Vulkan::getColorMask(configuration.colorChannelMask);
				colorBlendAttachment.blendEnable = Vulkan::getBool(configuration.blendState.enable);
				colorBlendAttachment.srcColorBlendFactor = Vulkan::getBlendFactor(configuration.blendState.srcFactorRGB);
				colorBlendAttachment.dstColorBlendFactor = Vulkan::getBlendFactor(configuration.blendState.dstFactorRGB);
				colorBlendAttachment.colorBlendOp = Vulkan::getBlendOp(configuration.blendState.operationRGB);
				colorBlendAttachment.srcAlphaBlendFactor = Vulkan::getBlendFactor(configuration.blendState.srcFactorA);
				colorBlendAttachment.dstAlphaBlendFactor = Vulkan::getBlendFactor(configuration.blendState.dstFactorA);
				colorBlendAttachment.alphaBlendOp = Vulkan::getBlendOp(configuration.blendState.operationA);

				VkPipelineColorBlendStateCreateInfo colorBlending{};
				colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
				colorBlending.logicOpEnable = VK_FALSE;
				colorBlending.logicOp = VK_LOGIC_OP_COPY;
				colorBlending.attachmentCount = 1;
				colorBlending.pAttachments = &colorBlendAttachment;
				colorBlending.blendConstants[0] = 0.0f;
				colorBlending.blendConstants[1] = 0.0f;
				colorBlending.blendConstants[2] = 0.0f;
				colorBlending.blendConstants[3] = 0.0f;

				VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
				pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
				pipelineLayoutInfo.setLayoutCount = 1;
				pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
				pipelineLayoutInfo.pushConstantRangeCount = 0;

				if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
					throw love::Exception("failed to create pipeline layout");
				}
				graphicsPipelineLayouts.push_back(pipelineLayout);

				VkFormat framebufferOutputFormat = configuration.framebufferFormat;

				VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo{};
				pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
				pipelineRenderingCreateInfo.colorAttachmentCount = 1;
				pipelineRenderingCreateInfo.pColorAttachmentFormats = &framebufferOutputFormat;

				VkGraphicsPipelineCreateInfo pipelineInfo{};
				pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
				pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
				pipelineInfo.pStages = shaderStages.data();
				pipelineInfo.pVertexInputState = &vertexInputInfo;
				pipelineInfo.pInputAssemblyState = &inputAssembly;
				pipelineInfo.pViewportState = &viewportState;
				pipelineInfo.pRasterizationState = &rasterizer;
				pipelineInfo.pMultisampleState = &multisampling;
				pipelineInfo.pDepthStencilState = nullptr;
				pipelineInfo.pColorBlendState = &colorBlending;
				pipelineInfo.pDynamicState = nullptr;
				pipelineInfo.layout = pipelineLayout;
				pipelineInfo.subpass = 0;
				pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
				pipelineInfo.basePipelineIndex = -1;
				pipelineInfo.pNext = &pipelineRenderingCreateInfo;

				VkPipeline graphicsPipeline;
				if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
					throw love::Exception("failed to create graphics pipeline");
				}
				return graphicsPipeline;
			}

			void Graphics::ensureGraphicsPipelineConfiguration(GraphicsPipelineConfiguration configuration) {
				VkPipeline pipeline = VK_NULL_HANDLE;
				for (auto const& p : graphicsPipelines) {
					if (p.first == configuration) {
						pipeline = p.second;
						break;
					}
				}
				if (pipeline != VK_NULL_HANDLE) {
					if (currentGraphicsPipeline != pipeline) {
						vkCmdBindPipeline(commandBuffers.at(imageIndex), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
						currentGraphicsPipeline = pipeline;
					}
				} else {
					VkPipeline newPipeLine = createGraphicsPipeline(configuration);
					graphicsPipelines.push_back(std::make_pair(configuration, newPipeLine));
					vkCmdBindPipeline(commandBuffers.at(imageIndex), VK_PIPELINE_BIND_POINT_GRAPHICS, newPipeLine);
					currentGraphicsPipeline = newPipeLine;
				}
			}

			void Graphics::createCommandPool() {
				QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

				VkCommandPoolCreateInfo poolInfo{};
				poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
				poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

				if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
					throw love::Exception("failed to create command pool");
				}
			}

			void Graphics::createCommandBuffers() {
				commandBuffers.resize(swapChainImages.size());

				VkCommandBufferAllocateInfo allocInfo{};
				allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				allocInfo.commandPool = commandPool;
				allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

				if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
					throw love::Exception("failed to allocate command buffers");
				}
			}

			void Graphics::createSyncObjects() {
				imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
				renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
				inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
				imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

				VkSemaphoreCreateInfo semaphoreInfo{};
				semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

				VkFenceCreateInfo fenceInfo{};
				fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

				for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
					if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores.at(i)) != VK_SUCCESS ||
						vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores.at(i)) != VK_SUCCESS ||
						vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences.at(i)) != VK_SUCCESS) {
						throw love::Exception("failed to create synchronization objects for a frame!");
					}
				}
			}

			void Graphics::createDefaultTexture() {
				Texture::Settings settings;
				standardTexture.reset((Texture*)newTexture(settings));
			}

			void Graphics::createQuadIndexBuffer() {
				if (quadIndexBuffer != nullptr)
					return;

				size_t size = sizeof(uint16) * getIndexCount(TRIANGLEINDEX_QUADS, LOVE_UINT16_MAX);
				quadIndexBuffer.reset((StreamBuffer*)newStreamBuffer(BUFFERUSAGE_INDEX, size));
				auto map = quadIndexBuffer->map(size);
				fillIndices(TRIANGLEINDEX_QUADS, 0, LOVE_UINT16_MAX, (uint16*)map.data);
				quadIndexBuffer->unmap(size);
			}

			bool operator==(const GraphicsPipelineConfiguration& first, const GraphicsPipelineConfiguration& other) {
				if (first.scissorRect != other.scissorRect) {
					return false;
				}
				if (first.viewportHeight != other.viewportHeight) {
					return false;
				}
				if (first.viewportWidth != other.viewportWidth) {
					return false;
				}
				if (first.framebufferFormat != other.framebufferFormat) {
					return false;
				}
				if (first.cullmode != other.cullmode) {
					return false;
				}
				if (first.winding != other.winding) {
					return false;
				}
				if (first.colorChannelMask != other.colorChannelMask) {
					return false;
				}
				if (!(first.blendState == other.blendState)) {	// not sure why != doesn't work
					return false;
				}
				if (first.polygonMode != other.polygonMode) {
					return false;
				}
				if (first.primitiveType != other.primitiveType) {
					return false;
				}
				if (first.shader != other.shader) {
					return false;
				}
				if (first.vertexInputAttributeDescriptions.size() != other.vertexInputAttributeDescriptions.size()) {
					return false;
				}
				if (first.vertexInputBindingDescriptions.size() != other.vertexInputBindingDescriptions.size()) {
					return false;
				}
				for (uint32_t i = 0; i < first.vertexInputAttributeDescriptions.size(); i++) {
					const VkVertexInputAttributeDescription& x = first.vertexInputAttributeDescriptions[i];
					const VkVertexInputAttributeDescription& y = other.vertexInputAttributeDescriptions[i];
					if (x.binding != y.binding) {
						return false;
					}
					if (x.location != y.location) {
						return false;
					}
					if (x.offset != y.offset) {
						return false;
					}
					if (x.format != y.format) {
						return false;
					}
				}
				for (uint32_t i = 0; i < first.vertexInputBindingDescriptions.size(); i++) {
					const VkVertexInputBindingDescription& x = first.vertexInputBindingDescriptions[i];
					const VkVertexInputBindingDescription& y = other.vertexInputBindingDescriptions[i];
					if (x.binding != y.binding) {
						return false;
					}
					if (x.inputRate != y.inputRate) {
						return false;
					}
					if (x.stride != y.stride) {
						return false;
					}
				}
				return true;
			}

			void Graphics::cleanup() {
				cleanupSwapChain();

				vmaDestroyAllocator(vmaAllocator);
				batchedDrawBuffers.clear();
				for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
					vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
					vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
					vkDestroyFence(device, inFlightFences[i], nullptr);
				}

				vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
				vkDestroyCommandPool(device, commandPool, nullptr);
				vkDestroyDevice(device, nullptr);
				vkDestroySurfaceKHR(instance, surface, nullptr);
				vkDestroyInstance(instance, nullptr);
			}

			void Graphics::cleanupSwapChain() {
				vkDestroyDescriptorPool(device, descriptorPool, nullptr);
				vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
				for (auto const& p : graphicsPipelines) {
					vkDestroyPipeline(device, p.second, nullptr);
				}
				graphicsPipelines.clear();
				currentGraphicsPipeline = VK_NULL_HANDLE;
				for (const auto pipelineLayout : graphicsPipelineLayouts) {
					vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
				}
				graphicsPipelineLayouts.clear();
				for (size_t i = 0; i < swapChainImageViews.size(); i++) {
					vkDestroyImageView(device, swapChainImageViews[i], nullptr);
				}
				vkDestroySwapchainKHR(device, swapChain, nullptr);
				for (auto p : uniformBufferMap) {
					delete p.second;
				}
				uniformBufferMap.clear();
				descriptorSetsMap.clear();
			}

			void Graphics::recreateSwapChain() {
				vkDeviceWaitIdle(device);

				cleanupSwapChain();

				createSwapChain();
				createImageViews();
				createDescriptorPool();
				createCommandBuffers();
				startRecordingGraphicsCommands();
			}

			love::graphics::Graphics* createInstance() {
				love::graphics::Graphics* instance = nullptr;

				try {
					instance = new Graphics();
				}
				catch (love::Exception& e) {
					printf("Cannot create Vulkan renderer: %s\n", e.what());
				}

				return instance;
			}
		}
	}
}
