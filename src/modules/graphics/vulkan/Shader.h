/**
 * Copyright (c) 2006-2024 LOVE Development Team
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

#pragma once

// LÖVE
#include "common/Optional.h"
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
#include <set>


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
	Shader(StrongRef<love::graphics::ShaderStage> stages[], const CompileOptions &options);
	virtual ~Shader();

	bool loadVolatile() override;
	void unloadVolatile() override;

	VkPipeline getComputePipeline() const;

	const std::vector<VkPipelineShaderStageCreateInfo> &getShaderStages() const;

	const VkPipelineLayout getGraphicsPipelineLayout() const;

	void newFrame();

	void cmdPushDescriptorSets(VkCommandBuffer, VkPipelineBindPoint);

	void attach() override;

	ptrdiff_t getHandle() const override { return 0; }

	std::string getWarnings() const override { return ""; }

	int getVertexAttributeIndex(const std::string &name) override;

	const UniformInfo *getUniformInfo(BuiltinUniform builtin) const override;

	void updateUniform(const UniformInfo *info, int count) override;

	void sendTextures(const UniformInfo *info, graphics::Texture **textures, int count) override;
	void sendBuffers(const UniformInfo *info, love::graphics::Buffer **buffers, int count) override;

	void setVideoTextures(graphics::Texture *ytexture, graphics::Texture *cbtexture, graphics::Texture *crtexture) override;

	void setMainTex(graphics::Texture *texture);

private:
	void calculateUniformBufferSizeAligned();
	void compileShaders();
	void createDescriptorSetLayout();
	void createPipelineLayout();
	void createDescriptorPoolSizes();
	void createStreamBuffers();
	void buildLocalUniforms(spirv_cross::Compiler &comp, const spirv_cross::SPIRType &type, size_t baseoff, const std::string &basename);
	void createDescriptorPool();
	VkDescriptorSet allocateDescriptorSet();

	VkDeviceSize uniformBufferSizeAligned;

	VkPipeline computePipeline;

	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	std::vector<VkDescriptorPoolSize> descriptorPoolSizes;

	// we don't know how much memory we need per frame for the uniform buffer descriptors
	// we keep a vector of stream buffers that gets dynamically increased if more memory is needed
	std::vector<StreamBuffer*> streamBuffers;
	std::vector<std::vector<VkDescriptorPool>> descriptorPools;

	std::vector<VkDescriptorBufferInfo> descriptorBuffers;
	std::vector<VkDescriptorImageInfo> descriptorImages;
	std::vector<VkBufferView> descriptorBufferViews;
	std::vector<VkWriteDescriptorSet> descriptorWrites;

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	std::vector<VkShaderModule> shaderModules;

	Graphics *vgfx = nullptr;
	VkDevice device;

	bool isCompute = false;

	UniformInfo *builtinUniformInfo[BUILTIN_MAX_ENUM];

	std::unique_ptr<StreamBuffer> uniformBufferObjectBuffer;
	std::vector<uint8> localUniformData;
	std::vector<uint8> localUniformStagingData;
	uint32_t localUniformLocation;
	OptionalInt builtinUniformDataOffset;

	std::unordered_map<std::string, int> attributes;

	uint32_t currentFrame;
	uint32_t currentUsedUniformStreamBuffersCount;
	uint32_t currentDescriptorPool;
};

}
}
}
