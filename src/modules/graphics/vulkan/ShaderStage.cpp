#include "ShaderStage.h"

#include "Graphics.h"

#include <shaderc/shaderc.hpp>

#include <iostream>
#include <fstream>


namespace love {
	namespace graphics {
		namespace vulkan {
			static shaderc_shader_kind getShaderStage(ShaderStageType stage) {
				switch (stage) {
				case SHADERSTAGE_VERTEX: return shaderc_vertex_shader;
				case SHADERSTAGE_PIXEL: return shaderc_fragment_shader;
				case SHADERSTAGE_COMPUTE: return shaderc_compute_shader;
				default:
					throw love::Exception("unknown exception");
				}
			}

			ShaderStage::ShaderStage(love::graphics::Graphics* gfx, ShaderStageType stage, const std::string& glsl, bool gles, const std::string& cachekey)
				: love::graphics::ShaderStage(gfx, stage, glsl, gles, cachekey) {
				using namespace shaderc;

				Compiler compiler{};
				auto result = compiler.CompileGlslToSpv(glsl, shaderc_vertex_shader, "shader.glsl");
				std::vector<uint32_t> code(result.begin(), result.end());

				VkShaderModuleCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				createInfo.codeSize = code.size() * sizeof(unsigned int);
				createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

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
