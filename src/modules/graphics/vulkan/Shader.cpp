#include "Shader.h"

#include "libraries/glslang/glslang/Public/ShaderLang.h"
#include "libraries/glslang/SPIRV/GlslangToSpv.h"
#include <vector>

namespace love {
	namespace graphics {
		namespace vulkan {
			static VkShaderStageFlagBits getStageBit(ShaderStageType type) {
				switch (type) {
				case SHADERSTAGE_VERTEX:
					return VK_SHADER_STAGE_VERTEX_BIT;
				case SHADERSTAGE_PIXEL:
					return VK_SHADER_STAGE_FRAGMENT_BIT;
				case SHADERSTAGE_COMPUTE:
					return VK_SHADER_STAGE_COMPUTE_BIT;
				}
				throw love::Exception("invalid type");
			}

			Shader::Shader(StrongRef<love::graphics::ShaderStage> stages[])
				: graphics::Shader(stages) {

				if (false) {
					for (int i = 0; i < SHADERSTAGE_MAX_ENUM; i++) {
						if (!stages[i]) 
							continue;
					
						auto stage = dynamic_cast<ShaderStage*>(stages[i].get());

						VkPipelineShaderStageCreateInfo shaderStageInfo{};
						shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
						shaderStageInfo.stage = getStageBit(stage->getStageType());
						shaderStageInfo.module = stage->getShaderModule();
						shaderStageInfo.pName = "main";

						shaderStages.push_back(shaderStageInfo);
					}
				}
			}
		}
	}
}