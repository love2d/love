#include "Graphics.h"
#include "Buffer.h"
#include "SDL_vulkan.h"
#include "window/Window.h"
#include "common/Exception.h"
#include "Shader.h"

#include <vector>
#include <cstring>
#include <set>
#include <fstream>
#include <iostream>


namespace love {
	namespace graphics {
		namespace vulkan {
			const std::vector<const char*> validationLayers = {
				"VK_LAYER_KHRONOS_validation"
			};

			const std::vector<const char*> deviceExtensions = {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};

#ifdef NDEBUG
			const bool enableValidationLayers = false;
#else
			const bool enableValidationLayers = true;
#endif

			const int MAX_FRAMES_IN_FLIGHT = 2;

			static std::vector<char> readFile(const std::string& filename) {
				std::ifstream file(filename, std::ios::ate | std::ios::binary);

				if (!file.is_open()) {
					throw std::runtime_error("failed to open file!");
				}

				size_t fileSize = (size_t)file.tellg();
				std::vector<char> buffer(fileSize);

				file.seekg(0);
				file.read(buffer.data(), fileSize);

				file.close();

				return buffer;
			}

			const char* Graphics::getName() const {
				return "love.graphics.vulkan";
			}

			Graphics::Graphics() {
			}

			void Graphics::initVulkan() {
				if (!init) {
					init = true;
					createVulkanInstance();
					createSurface();
					pickPhysicalDevice();
					createLogicalDevice();
					createSwapChain();
					createImageViews();
					createRenderPass();
					createDefaultShaders();
					createDescriptorSetLayout();
					createGraphicsPipeline();
					createFramebuffers();
					createCommandPool();
					createCommandBuffers();
					createUniformBuffers();
					createDescriptorPool();
					createDescriptorSets();
					createSyncObjects();
					startRecordingGraphicsCommands();
				}
			}

			Graphics::~Graphics() {
				cleanup();
			}

			// START OVERRIDEN FUNCTIONS

			love::graphics::Buffer* Graphics::newBuffer(const love::graphics::Buffer::Settings& settings, const std::vector<love::graphics::Buffer::DataDeclaration>& format, const void* data, size_t size, size_t arraylength) {
				std::cout << "newBuffer ";
				return nullptr;
			}

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

				std::cout << "beginCommandBuffer(imageIndex=" << imageIndex << ") ";
				if (vkBeginCommandBuffer(commandBuffers.at(imageIndex), &beginInfo) != VK_SUCCESS) {
					throw love::Exception("failed to begin recording command buffer");
				}

				VkRenderPassBeginInfo renderPassInfo{};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = renderPass;
				renderPassInfo.framebuffer = swapChainFramBuffers.at(imageIndex);
				renderPassInfo.renderArea.offset = { 0, 0 };
				renderPassInfo.renderArea.extent = swapChainExtent;
				renderPassInfo.clearValueCount = 1;
				renderPassInfo.pClearValues = &clearColor;

				const auto& commandBuffer = commandBuffers.at(imageIndex);

				vkCmdBeginRenderPass(commandBuffers.at(imageIndex), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
				vkCmdBindPipeline(commandBuffers.at(imageIndex), VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
			}

			void Graphics::endRecordingGraphicsCommands() {
				const auto& commandBuffer = commandBuffers.at(imageIndex);

				std::cout << "endCommandBuffer(imageIndex=" << imageIndex << ") ";
				vkCmdEndRenderPass(commandBuffers.at(imageIndex));
				if (vkEndCommandBuffer(commandBuffers.at(imageIndex)) != VK_SUCCESS) {
					throw love::Exception("failed to record command buffer");
				}
			}

			void Graphics::present(void* screenshotCallbackdata) {
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

				std::cout << "present" << std::endl;

				startRecordingGraphicsCommands();
			}

			void Graphics::setViewportSize(int width, int height, int pixelwidth, int pixelheight) {
				std::cout << "setViewPortSize";
				recreateSwapChain();
			}

			bool Graphics::setMode(void* context, int width, int height, int pixelwidth, int pixelheight, bool windowhasstencil, int msaa) { 
				std::cout << "setMode ";

				if (batchedDrawState.vb[0] == nullptr)
				{
					// Initial sizes that should be good enough for most cases. It will
					// resize to fit if needed, later.
					batchedDrawState.vb[0] = new StreamBuffer(device, physicalDevice, BUFFERUSAGE_VERTEX, 1024 * 1024 * 1);
					batchedDrawState.vb[1] = new StreamBuffer(device, physicalDevice, BUFFERUSAGE_VERTEX, 256 * 1024 * 1);
					batchedDrawState.indexBuffer = new StreamBuffer(device, physicalDevice, BUFFERUSAGE_INDEX, sizeof(uint16) * LOVE_UINT16_MAX);
				}

				return true;
			}

			void Graphics::draw(const DrawIndexedCommand& cmd) { 
				std::cout << "drawIndexed ";

				std::vector<VkBuffer> buffers;
				std::vector<VkDeviceSize> offsets;
				buffers.push_back((VkBuffer)cmd.buffers->info[0].buffer->getHandle());
				offsets.push_back((VkDeviceSize) 0);

				prepareDraw(currentFrame);

				vkCmdBindDescriptorSets(commandBuffers.at(imageIndex), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets.at(currentFrame), 0, nullptr);
				vkCmdBindVertexBuffers(commandBuffers.at(imageIndex), 0, 1, buffers.data(), offsets.data());
				vkCmdBindIndexBuffer(commandBuffers.at(imageIndex), (VkBuffer) cmd.indexBuffer->getHandle(), 0, VK_INDEX_TYPE_UINT16);
				vkCmdDrawIndexed(commandBuffers.at(imageIndex), static_cast<uint32_t>(cmd.indexCount), 1, 0, 0, 0);
			}

			graphics::StreamBuffer* Graphics::newStreamBuffer(BufferUsage type, size_t size) { 
				std::cout << "newStreamBuffer ";
				return new StreamBuffer(device, physicalDevice, type, size); 
			}

			// END IMPLEMENTATION OVERRIDDEN FUNCTIONS

			void Graphics::prepareDraw(uint32_t currentImage) {
				auto& buffer = uniformBuffers.at(currentImage);

				Shader::UniformBufferObject ubo{};
				ubo.windowWidth = static_cast<float>(swapChainExtent.width);
				ubo.windowHeight = static_cast<float>(swapChainExtent.height);

				auto mappedInfo = buffer->map(0);
				memcpy(mappedInfo.data, &ubo, sizeof(ubo));
				buffer->unmap(0);
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
				appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);	//todo, same as above
				appInfo.apiVersion = VK_API_VERSION_1_0;

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

				return score;
			}

			Graphics::QueueFamilyIndices Graphics::findQueueFamilies(VkPhysicalDevice device) {
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

				VkPhysicalDeviceFeatures deviceFeatures{};

				VkDeviceCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
				createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
				createInfo.pQueueCreateInfos = queueCreateInfos.data();
				createInfo.pEnabledFeatures = &deviceFeatures;

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

			void Graphics::createSurface() {
				auto window = Module::getInstance<love::window::Window>(M_WINDOW);
				const void* handle = window->getHandle();
				if (SDL_Vulkan_CreateSurface((SDL_Window*)handle, instance, &surface) != SDL_TRUE) {
					throw love::Exception("failed to create window surface");
				}
			}

			Graphics::SwapChainSupportDetails Graphics::querySwapChainSupport(VkPhysicalDevice device) {
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

			void Graphics::createRenderPass() {
				VkAttachmentDescription colorAttachment{};
				colorAttachment.format = swapChainImageFormat;
				colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
				colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

				VkAttachmentReference colorAttachmentRef{};
				colorAttachmentRef.attachment = 0;
				colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				VkSubpassDescription subpass{};
				subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				subpass.colorAttachmentCount = 1;
				subpass.pColorAttachments = &colorAttachmentRef;

				VkSubpassDependency dependency{};
				dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
				dependency.dstSubpass = 0;
				dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependency.srcAccessMask = 0;
				dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

				VkRenderPassCreateInfo renderPassInfo{};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
				renderPassInfo.attachmentCount = 1;
				renderPassInfo.pAttachments = &colorAttachment;
				renderPassInfo.subpassCount = 1;
				renderPassInfo.pSubpasses = &subpass;
				renderPassInfo.dependencyCount = 1;
				renderPassInfo.pDependencies = &dependency;

				if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
					throw love::Exception("failed to create render pass");
				}
			}

			static VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code) {
				VkShaderModuleCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				createInfo.codeSize = code.size();
				createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

				VkShaderModule shaderModule;
				if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
					throw love::Exception("failed to create shader module");
				}

				return shaderModule;
			}

			void Graphics::createDefaultShaders() {
				for (int i = 0; i < Shader::STANDARD_MAX_ENUM; i++) {
					auto stype = (Shader::StandardShader)i;

					if (!Shader::standardShaders[i]) {
						std::vector<std::string> stages;
						stages.push_back(Shader::getDefaultCode(stype, SHADERSTAGE_VERTEX));
						stages.push_back(Shader::getDefaultCode(stype, SHADERSTAGE_PIXEL));
						Shader::standardShaders[i] = newShader(stages, true);
					}
				}
			}

			void Graphics::createDescriptorSetLayout() {
				VkDescriptorSetLayoutBinding uboLayoutBinding{};
				uboLayoutBinding.binding = 0;
				uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				uboLayoutBinding.descriptorCount = 1;
				uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

				VkDescriptorSetLayoutCreateInfo layoutInfo{};
				layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				layoutInfo.bindingCount = 1;
				layoutInfo.pBindings = &uboLayoutBinding;

				if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
					throw love::Exception("failed to create descriptor set layout");
				}
			}

			void Graphics::createUniformBuffers() {
				VkDeviceSize bufferSize = sizeof(Shader::UniformBufferObject);
				
				for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
					uniformBuffers.push_back(std::make_unique<StreamBuffer>(device, physicalDevice, BUFFERUSAGE_UNIFORM, bufferSize));
				}
			}

			void Graphics::createDescriptorPool() {
				VkDescriptorPoolSize poolSize{};
				poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

				VkDescriptorPoolCreateInfo poolInfo{};
				poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				poolInfo.poolSizeCount = 1;
				poolInfo.pPoolSizes = &poolSize;
				poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

				if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
					throw love::Exception("failed to create descriptor pool");
				}
			}

			void Graphics::createDescriptorSets() {
				std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
				VkDescriptorSetAllocateInfo allocInfo{};
				allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				allocInfo.descriptorPool = descriptorPool;
				allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
				allocInfo.pSetLayouts = layouts.data();

				descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
				if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
					throw love::Exception("failed to allocate descriptor sets");
				}

				for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
					VkDescriptorBufferInfo bufferInfo{};
					bufferInfo.buffer = (VkBuffer) uniformBuffers.at(i)->getHandle();
					bufferInfo.offset = 0;
					bufferInfo.range = sizeof(Shader::UniformBufferObject);

					VkWriteDescriptorSet descriptorWrite{};
					descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptorWrite.dstSet = descriptorSets[i];
					descriptorWrite.dstBinding = 0;
					descriptorWrite.dstArrayElement = 0;
					descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					descriptorWrite.descriptorCount = 1;
					descriptorWrite.pBufferInfo = &bufferInfo;

					vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
				}
			}

			void Graphics::createGraphicsPipeline() {
				auto shader = reinterpret_cast<love::graphics::vulkan::Shader*>(love::graphics::vulkan::Shader::standardShaders[Shader::STANDARD_DEFAULT]);
				auto shaderStages = shader->getShaderStages();

				VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
				vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

				VkVertexInputBindingDescription vertexBindingDescription;
				vertexBindingDescription.binding = 0;
				vertexBindingDescription.stride = 2 * sizeof(float);	// just position for now
				vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

				// todo later
				VkVertexInputAttributeDescription positionInputAttributeDescription;
				positionInputAttributeDescription.binding = 0;
				positionInputAttributeDescription.location = 0;
				positionInputAttributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
				positionInputAttributeDescription.offset = 0;

				vertexInputInfo.vertexBindingDescriptionCount = 1;
				vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDescription;
				vertexInputInfo.vertexAttributeDescriptionCount = 1;
				vertexInputInfo.pVertexAttributeDescriptions = &positionInputAttributeDescription;

				VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
				inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
				inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
				inputAssembly.primitiveRestartEnable = VK_FALSE;
				
				VkViewport viewport{};
				viewport.x = 0.0f;
				viewport.y = 0.0f;
				viewport.width = (float)swapChainExtent.width;
				viewport.height = (float)swapChainExtent.height;
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;

				VkRect2D scissor{};
				scissor.offset = { 0, 0 };
				scissor.extent = swapChainExtent;

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
				rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
				rasterizer.lineWidth = 1.0f;
				rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
				rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
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
				colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
				colorBlendAttachment.blendEnable = VK_FALSE;

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
				pipelineInfo.renderPass = renderPass;
				pipelineInfo.subpass = 0;
				pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
				pipelineInfo.basePipelineIndex = -1;

				if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
					throw love::Exception("failed to create graphics pipeline");
				}
			}

			void Graphics::createFramebuffers() {
				swapChainFramBuffers.resize(swapChainImageViews.size());
				for (size_t i = 0; i < swapChainImageViews.size(); i++) {
					VkImageView attachments[] = {
						swapChainImageViews.at(i)
					};

					VkFramebufferCreateInfo framebufferInfo{};
					framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
					framebufferInfo.renderPass = renderPass;
					framebufferInfo.attachmentCount = 1;
					framebufferInfo.pAttachments = attachments;
					framebufferInfo.width = swapChainExtent.width;
					framebufferInfo.height = swapChainExtent.height;
					framebufferInfo.layers = 1;

					if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramBuffers.at(i)) != VK_SUCCESS) {
						throw love::Exception("failed to create framebuffers");
					}
				}
			}

			void Graphics::createCommandPool() {
				QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

				VkCommandPoolCreateInfo poolInfo{};
				poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
				poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

				if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
					throw love::Exception("failed to create command pool");
				}
			}

			void Graphics::createCommandBuffers() {
				commandBuffers.resize(swapChainFramBuffers.size());

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

			void Graphics::cleanup() {
				vkDeviceWaitIdle(device);

				cleanupSwapChain();

				for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
					vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
					vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
					vkDestroyFence(device, inFlightFences[i], nullptr);
				}

				vkDestroyDescriptorPool(device, descriptorPool, nullptr);

				vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
				vkDestroyCommandPool(device, commandPool, nullptr);
				vkDestroyDevice(device, nullptr);
				vkDestroySurfaceKHR(instance, surface, nullptr);
				vkDestroyInstance(instance, nullptr);
			}

			void Graphics::cleanupSwapChain() {
				for (size_t i = 0; i < swapChainFramBuffers.size(); i++) {
					vkDestroyFramebuffer(device, swapChainFramBuffers[i], nullptr);
				}
				vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
				vkDestroyPipeline(device, graphicsPipeline, nullptr);
				vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
				vkDestroyRenderPass(device, renderPass, nullptr);
				for (size_t i = 0; i < swapChainImageViews.size(); i++) {
					vkDestroyImageView(device, swapChainImageViews[i], nullptr);
				}
				vkDestroySwapchainKHR(device, swapChain, nullptr);
				uniformBuffers.clear();
			}

			void Graphics::recreateSwapChain() {
				vkDeviceWaitIdle(device);

				cleanupSwapChain();

				createSwapChain();
				createImageViews();
				createRenderPass();
				createGraphicsPipeline();
				createFramebuffers();
				createUniformBuffers();
				createDescriptorPool();
				createDescriptorSets();
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
