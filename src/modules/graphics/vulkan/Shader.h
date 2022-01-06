#ifndef LOVE_GRAPHICS_VULKAN_SHADER_H
#define LOVE_GRAPHICS_VULKAN_SHADER_H

#include <graphics/Shader.h>
#include <graphics/vulkan/ShaderStage.h>
#include "libraries/glslang/glslang/Public/ShaderLang.h"
#include "libraries/glslang/SPIRV/GlslangToSpv.h"
#include <vulkan/vulkan.h>


namespace love {
	namespace graphics {
		namespace vulkan {
			class Shader final : public graphics::Shader {
			public:
				Shader(StrongRef<love::graphics::ShaderStage> stages[]);
				virtual ~Shader() = default;

				const std::vector<VkPipelineShaderStageCreateInfo>& getShaderStages() const {
					return shaderStages;
				}

			private:
				std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
			};
		}
	}
}

#endif
