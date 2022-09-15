#pragma once

// LÖVE
#include "graphics/Shader.h"
#include "graphics/vulkan/ShaderStage.h"
#include "Vulkan.h"

// Libraries
#include "VulkanWrapper.h"
#include "libraries/spirv_cross/spirv_reflect.hpp"

// C++
#include <map>
#include <memory>
#include <unordered_map>
#include <queue>
#include <optional>


namespace love
{
namespace graphics
{
namespace vulkan
{

class Graphics;

class Shader final
	: public graphics::Shader
	, public Volatile
{
public:
	Shader(StrongRef<love::graphics::ShaderStage> stages[]);
	virtual ~Shader();

	bool loadVolatile() override;
	void unloadVolatile() override;

	VkPipeline getComputePipeline() const;

	const std::vector<VkPipelineShaderStageCreateInfo> &getShaderStages() const;

	const VkPipelineLayout getGraphicsPipelineLayout() const;

	void newFrame(uint32_t frameIndex);

	void cmdPushDescriptorSets(VkCommandBuffer, VkPipelineBindPoint);

	void attach() override;

	ptrdiff_t getHandle() const { return 0; }

	std::string getWarnings() const override { return ""; }

	int getVertexAttributeIndex(const std::string &name) override;

	const UniformInfo *getUniformInfo(const std::string &name) const override;
	const UniformInfo *getUniformInfo(BuiltinUniform builtin) const override;

	void updateUniform(const UniformInfo *info, int count) override;

	void sendTextures(const UniformInfo *info, graphics::Texture **textures, int count) override;
	void sendBuffers(const UniformInfo *info, love::graphics::Buffer **buffers, int count) override {}

	bool hasUniform(const std::string &name) const override;

	void setVideoTextures(graphics::Texture *ytexture, graphics::Texture *cbtexture, graphics::Texture *crtexture) override;

	void setMainTex(graphics::Texture *texture);

private:
	void calculateUniformBufferSizeAligned();
	void compileShaders();
	void createDescriptorSetLayout();
	void createPipelineLayout();
	void createDescriptorPoolSizes();
	void createStreamBuffers();
	void buildLocalUniforms(
		spirv_cross::Compiler &comp, 
		const spirv_cross::SPIRType &type, 
		size_t baseoff, 
		const std::string &basename);

	VkDescriptorSet allocateDescriptorSet();

	VkDeviceSize uniformBufferSizeAligned;

	VkPipeline computePipeline;

	bool useBuiltinUniformPushConstant = false;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	std::vector<VkDescriptorPoolSize> descriptorPoolSizes;

	// we don't know how much memory we need per frame for the uniform buffer descriptors
	// we keep a vector of stream buffers per frame in flight
	// that gets dynamically increased if more memory is needed
	std::vector<std::vector<StreamBuffer*>> streamBuffers;
	std::vector<VkDescriptorPool> descriptorPools;
	std::queue<VkDescriptorSet> freeDescriptorSets;
	std::vector<std::vector<VkDescriptorSet>> descriptorSetsVector;

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	std::vector<VkShaderModule> shaderModules;

	Graphics *vgfx = nullptr;
	VkDevice device;

	bool isCompute = false;

	std::unordered_map<std::string, graphics::Shader::UniformInfo> uniformInfos;
	UniformInfo *builtinUniformInfo[BUILTIN_MAX_ENUM];

	std::unique_ptr<StreamBuffer> uniformBufferObjectBuffer;
	std::vector<uint8> localUniformData;
	std::vector<uint8> localUniformStagingData;
	uint32_t uniformLocation;
	std::optional<size_t> builtinUniformDataOffset;

	std::unordered_map<std::string, int> attributes;

	VkDescriptorSet currentDescriptorSet;

	uint32_t currentFrame;
	uint32_t currentUsedUniformStreamBuffersCount;
	uint32_t currentUsedDescriptorSetsCount;
};

}
}
}
