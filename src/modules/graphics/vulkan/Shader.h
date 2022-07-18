#ifndef LOVE_GRAPHICS_VULKAN_SHADER_H
#define LOVE_GRAPHICS_VULKAN_SHADER_H

#include <graphics/Shader.h>
#include <graphics/vulkan/ShaderStage.h>
#include "Vulkan.h"
#include <vulkan/vulkan.h>

#include <map>
#include <iostream>


namespace love {
	namespace graphics {
		namespace vulkan {
			class Shader final : public graphics::Shader, public Volatile {
			public:
				Shader(StrongRef<love::graphics::ShaderStage> stages[]);
				virtual ~Shader();

				bool loadVolatile() override;
				void unloadVolatile() override;

				const std::vector<VkPipelineShaderStageCreateInfo>& getShaderStages() const {
					return shaderStages;
				}

				void attach() override;

				ptrdiff_t getHandle() const { return 0; }

				std::string getWarnings() const override { return ""; }

				int getVertexAttributeIndex(const std::string& name) override;

				const UniformInfo* getUniformInfo(const std::string& name) const override { return nullptr; }
				const UniformInfo* getUniformInfo(BuiltinUniform builtin) const override { return nullptr;  }

				void updateUniform(const UniformInfo* info, int count) override {}

				void sendTextures(const UniformInfo* info, graphics::Texture** textures, int count) override {}
				void sendBuffers(const UniformInfo* info, love::graphics::Buffer** buffers, int count) override {}

				bool hasUniform(const std::string& name) const override { return false; }

				void setVideoTextures(graphics::Texture* ytexture, graphics::Texture* cbtexture, graphics::Texture* crtexture) override {}

			private:
				std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
				std::vector<VkShaderModule> shaderModules;

				std::map<std::string, int> vertexAttributeIndices = {
					{ "VertexPosition", 0 },
					{ "VertexTexCoord", 1 },
					{ "VertexColor", 2 }
				};

				std::map<std::string, int> uniformBindings = {
					{ "love_UniformsPerDraw", 0 },
					{ "love_VideoYChannel", 1 },
					{ "love_VideoCbChannel", 2 },
					{ "love_VideoCrChannel", 3 },
					{ "MainTex", 4 }
				};

				std::map<std::string, UniformInfo> uniforms;
			};
		}
	}
}

#endif
