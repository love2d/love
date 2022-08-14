#ifndef LOVE_GRAPHICS_VULKAN_SHADER_H
#define LOVE_GRAPHICS_VULKAN_SHADER_H

#include <graphics/Shader.h>
#include <graphics/vulkan/ShaderStage.h>
#include "Vulkan.h"

#include <vulkan/vulkan.h>
#include "libraries/spirv_cross/spirv_reflect.hpp"

#include <map>
#include <memory>
#include <iostream>
#include <unordered_map>


namespace love {
namespace graphics {
namespace vulkan {
class Shader final : public graphics::Shader, public Volatile {
public:
	Shader(StrongRef<love::graphics::ShaderStage> stages[]);
	virtual ~Shader();

	bool loadVolatile() override;
	void unloadVolatile() override;

	const std::vector<VkPipelineShaderStageCreateInfo>& getShaderStages() const;

	const VkPipelineLayout getGraphicsPipelineLayout() const;

	void cmdPushDescriptorSets(VkCommandBuffer, uint32_t currentFrame);

	void attach() override;

	ptrdiff_t getHandle() const { return 0; }

	std::string getWarnings() const override { return ""; }

	int getVertexAttributeIndex(const std::string& name) override;

	const UniformInfo* getUniformInfo(const std::string& name) const override;
	const UniformInfo* getUniformInfo(BuiltinUniform builtin) const override;

	// Not needed right now, since the logic that links the values of the uniforms to the shader is done in cmdPushDescriptorSets
	// which gets called from the vulkan::Graphics class whenever a draw call happens.
	// I'll have to reevaluate the use of this function in the future though.
	void updateUniform(const UniformInfo* info, int count) override {}

	void sendTextures(const UniformInfo* info, graphics::Texture** textures, int count) override;
	void sendBuffers(const UniformInfo* info, love::graphics::Buffer** buffers, int count) override {}

	bool hasUniform(const std::string& name) const override;

	void setVideoTextures(graphics::Texture* ytexture, graphics::Texture* cbtexture, graphics::Texture* crtexture) override;

	// fixme: use normal methods for this in the future.
	void setUniformData(BuiltinUniformData& data);
	void setMainTex(graphics::Texture* texture);

private:
	void calculateUniformBufferSizeAligned();
	void compileShaders();
	void createDescriptorSetLayout();
	void createPipelineLayout();
	void createStreamBuffers();
	void buildLocalUniforms(
		spirv_cross::Compiler& comp, 
		const spirv_cross::SPIRType& type, 
		size_t baseoff, 
		const std::string& basename);

	VkDeviceSize uniformBufferSizeAligned;
	PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSet;

	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;

	// we don't know how much memory we need per frame for the uniform buffer descriptors
	// we keep a vector of stream buffers per frame in flight
	// that gets dynamically increased if more memory is needed
	std::vector<std::vector<StreamBuffer*>> streamBuffers;

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	std::vector<VkShaderModule> shaderModules;
	Graphics* gfx;
	VkDevice device;

	std::unordered_map<std::string, graphics::Shader::UniformInfo> uniformInfos;
	UniformInfo* builtinUniformInfo[BUILTIN_MAX_ENUM];

	std::unique_ptr<StreamBuffer> uniformBufferObjectBuffer;
	std::vector<uint8> localUniformStagingData;
	size_t builtinUniformDataOffset;

	uint32_t currentFrame;
	// todo: give this variable a better name
	uint32_t count;
};
}
}
}

#endif
