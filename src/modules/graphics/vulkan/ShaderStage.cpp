#include "ShaderStage.h"

#include "Graphics.h"

#include <iostream>
#include <fstream>


namespace love {
	namespace graphics {
		namespace vulkan {
			static int someIndex = 0;

			static std::string getFileEnding(ShaderStageType type) {
				switch (type) {
				case SHADERSTAGE_VERTEX:
					return ".vert";
				case SHADERSTAGE_PIXEL:
					return ".frag";
				default:
					throw love::Exception("unsupported shader stage type");
				}
			}

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

			static int shaderSourceId = 0;

			std::vector<char> compileShader(const std::string& glsl, ShaderStageType stage) {
				// fixme: use glslang or shaderc for this

				std::string inputFileName = std::string("temp") + std::to_string(shaderSourceId++) + getFileEnding(stage);
				std::string outputFileName = std::string("temp.spv");

				std::ofstream out(inputFileName);
				out << glsl;
				out.close();

				std::string command = std::string("glslc -fauto-bind-uniforms ") + inputFileName + " -o " + outputFileName;
				system(command.c_str());

				return readFile(outputFileName);
			}

			ShaderStage::ShaderStage(love::graphics::Graphics* gfx, ShaderStageType stage, const std::string& glsl, bool gles, const std::string& cachekey)
				: love::graphics::ShaderStage(gfx, stage, glsl, gles, cachekey) {
				auto code = compileShader(glsl, stage);

				VkShaderModuleCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				createInfo.codeSize = code.size();
				createInfo.pCode = reinterpret_cast<uint32_t*>(code.data());

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
