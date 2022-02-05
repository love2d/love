#include "ShaderStage.h"

#include "Graphics.h"

#include <iostream>
#include <fstream>


namespace love {
	namespace graphics {
		namespace vulkan {
			ShaderStage::ShaderStage(love::graphics::Graphics* gfx, ShaderStageType stage, const std::string& glsl, bool gles, const std::string& cachekey)
				: love::graphics::ShaderStage(gfx, stage, glsl, gles, cachekey) {
				VkShaderModuleCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				createInfo.codeSize = 0;
				createInfo.pCode = nullptr;

				Graphics* vkGfx = (Graphics*)gfx;
				device = vkGfx->getDevice();

				if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
					throw love::Exception("failed to create shader module");
				}
			}

			ShaderStage::~ShaderStage() {
				// vkDestroyShaderModule(device, shaderModule, nullptr);
			}
		}
	}
}
