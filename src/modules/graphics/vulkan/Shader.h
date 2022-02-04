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

				void attach() override {}

				ptrdiff_t getHandle() const { return 0; }

				std::string getWarnings() const override { return ""; }

				int getVertexAttributeIndex(const std::string& name) override { return 0;  }

				const UniformInfo* getUniformInfo(const std::string& name) const override { return nullptr; }
				const UniformInfo* getUniformInfo(BuiltinUniform builtin) const override { return nullptr;  }

				void updateUniform(const UniformInfo* info, int count) override {}

				void sendTextures(const UniformInfo* info, Texture** textures, int count) override {}
				void sendBuffers(const UniformInfo* info, love::graphics::Buffer** buffers, int count) override {}

				bool hasUniform(const std::string& name) const override { return false; }

				void setVideoTextures(Texture* ytexture, Texture* cbtexture, Texture* crtexture) override {}

			private:
				std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
			};
		}
	}
}

#endif
