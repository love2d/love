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

#include "graphics/vertex.h"
#include "Shader.h"
#include "Graphics.h"
#include "common/Range.h"

#include "libraries/glslang/glslang/Public/ShaderLang.h"
#include "libraries/glslang/glslang/Public/ResourceLimits.h"
#include "libraries/glslang/SPIRV/GlslangToSpv.h"

#include <array>

namespace love
{
namespace graphics
{
namespace vulkan
{

static const uint32_t STREAMBUFFER_DEFAULT_SIZE = 16;
static const uint32_t DESCRIPTOR_POOL_SIZE = 1000;

class BindingMapper
{
public:
	uint32_t operator()(spirv_cross::CompilerGLSL &comp, std::vector<uint32_t> &spirv, const std::string &name, int count, const spirv_cross::ID &id)
	{
		auto it = bindingMappings.find(name);
		if (it == bindingMappings.end())
		{
			auto binding = comp.get_decoration(id, spv::DecorationBinding);

			if (isFreeBinding(binding, count))
			{
				bindingMappings[name] = Range(binding, count);
				return binding;
			}
			else
			{
				uint32_t freeBinding = getFreeBinding(count);

				uint32_t binaryBindingOffset;
				if (!comp.get_binary_offset_for_decoration(id, spv::DecorationBinding, binaryBindingOffset))
					throw love::Exception("could not get binary offset for uniform %s binding", name.c_str());

				spirv[binaryBindingOffset] = freeBinding;

				bindingMappings[name] = Range(freeBinding, count);

				return freeBinding;
			}
		}
		else
			return (uint32_t)it->second.getOffset();
	};


private:
	uint32_t getFreeBinding(int count)
	{
		for (uint32_t i = 0;; i++)
		{
			if (isFreeBinding(i, count))
				return i;
		}
	}

	bool isFreeBinding(uint32_t binding, int count)
	{
		Range r(binding, count);
		for (const auto &entry : bindingMappings)
		{
			if (entry.second.intersects(r))
				return false;
		}
		return true;
	}

	std::map<std::string, Range> bindingMappings;

};

static VkShaderStageFlagBits getStageBit(ShaderStageType type)
{
	switch (type)
	{
	case SHADERSTAGE_VERTEX:
		return VK_SHADER_STAGE_VERTEX_BIT;
	case SHADERSTAGE_PIXEL:
		return VK_SHADER_STAGE_FRAGMENT_BIT;
	case SHADERSTAGE_COMPUTE:
		return VK_SHADER_STAGE_COMPUTE_BIT;
	default:
		throw love::Exception("invalid type");
	}
}

static EShLanguage getGlslShaderType(ShaderStageType stage)
{
	switch (stage)
	{
	case SHADERSTAGE_VERTEX:
		return EShLangVertex;
	case SHADERSTAGE_PIXEL:
		return EShLangFragment;
	case SHADERSTAGE_COMPUTE:
		return EShLangCompute;
	default:
		throw love::Exception("unkonwn shader stage type");
	}
}

static bool usesLocalUniformData(const graphics::Shader::UniformInfo *info)
{
	return info->baseType == graphics::Shader::UNIFORM_BOOL ||
		info->baseType == graphics::Shader::UNIFORM_FLOAT ||
		info->baseType == graphics::Shader::UNIFORM_INT ||
		info->baseType == graphics::Shader::UNIFORM_MATRIX ||
		info->baseType == graphics::Shader::UNIFORM_UINT;
}

Shader::Shader(StrongRef<love::graphics::ShaderStage> stages[], const CompileOptions &options)
	: graphics::Shader(stages, options)
{
	auto gfx = Module::getInstance<Graphics>(Module::ModuleType::M_GRAPHICS);
	vgfx = dynamic_cast<Graphics*>(gfx);

	loadVolatile();
}

bool Shader::loadVolatile()
{
	device = vgfx->getDevice();

	computePipeline = VK_NULL_HANDLE;

	for (int i = 0; i < BUILTIN_MAX_ENUM; i++)
		builtinUniformInfo[i] = nullptr;

	compileShaders();
	calculateUniformBufferSizeAligned();
	createDescriptorSetLayout();
	createPipelineLayout();
	createDescriptorPoolSizes();
	createStreamBuffers();
	descriptorPools.resize(MAX_FRAMES_IN_FLIGHT);
	currentFrame = 0;
	currentUsedUniformStreamBuffersCount = 0;
	newFrame();

	return true;
}

void Shader::unloadVolatile()
{
	if (shaderModules.empty())
		return;

	vgfx->queueCleanUp([shaderModules = std::move(shaderModules), device = device, descriptorSetLayout = descriptorSetLayout, pipelineLayout = pipelineLayout, descriptorPools = descriptorPools, computePipeline = computePipeline](){
		for (const auto &pools : descriptorPools)
		{
			for (const auto pool : pools)
				vkDestroyDescriptorPool(device, pool, nullptr);
		}
		for (const auto shaderModule : shaderModules)
			vkDestroyShaderModule(device, shaderModule, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		if (computePipeline != VK_NULL_HANDLE)
			vkDestroyPipeline(device, computePipeline, nullptr);
	});

	for (const auto streamBuffer : streamBuffers)
		streamBuffer->release();

	shaderModules.clear();
	shaderStages.clear();
	streamBuffers.clear();
	descriptorPools.clear();
}

const std::vector<VkPipelineShaderStageCreateInfo> &Shader::getShaderStages() const
{
	return shaderStages;
}

const VkPipelineLayout Shader::getGraphicsPipelineLayout() const
{
	return pipelineLayout;
}

VkPipeline Shader::getComputePipeline() const
{
	return computePipeline;
}

void Shader::newFrame()
{
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	currentUsedUniformStreamBuffersCount = 0;
	currentDescriptorPool = 0;

	if (streamBuffers.size() > 1)
	{
		size_t newSize = 0;
		for (auto streamBuffer : streamBuffers)
		{
			newSize += streamBuffer->getSize();
			streamBuffer->release();
		}
		streamBuffers.clear();
		streamBuffers.push_back(new StreamBuffer(vgfx, BUFFERUSAGE_UNIFORM, newSize));
	}
	else if (streamBuffers.size() == 1)
		streamBuffers.at(0)->nextFrame();

	for (VkDescriptorPool pool : descriptorPools[currentFrame])
		vkResetDescriptorPool(device, pool, 0);
}

void Shader::cmdPushDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint)
{
	int imageIndex = 0;
	int bufferIndex = 0;
	int bufferViewIndex = 0;

	if (!localUniformData.empty())
	{
		auto usedStreamBufferMemory = currentUsedUniformStreamBuffersCount * uniformBufferSizeAligned;
		if (usedStreamBufferMemory >= streamBuffers.back()->getSize())
		{
			streamBuffers.push_back(new StreamBuffer(vgfx, BUFFERUSAGE_UNIFORM, STREAMBUFFER_DEFAULT_SIZE * uniformBufferSizeAligned));
			currentUsedUniformStreamBuffersCount = 0;
		}

		if (builtinUniformDataOffset.hasValue)
		{
			auto builtinData = vgfx->getCurrentBuiltinUniformData();
			auto dst = localUniformData.data() + builtinUniformDataOffset.value;
			memcpy(dst, &builtinData, sizeof(builtinData));
		}

		auto currentStreamBuffer = streamBuffers.back();

		auto mapInfo = currentStreamBuffer->map(uniformBufferSizeAligned);
		memcpy(mapInfo.data, localUniformData.data(), localUniformData.size());
		auto offset = currentStreamBuffer->unmap(uniformBufferSizeAligned);
		currentStreamBuffer->markUsed(uniformBufferSizeAligned);

		VkDescriptorBufferInfo &bufferInfo = descriptorBuffers[bufferIndex++];
		bufferInfo.buffer = (VkBuffer)currentStreamBuffer->getHandle();
		bufferInfo.offset = offset;
		bufferInfo.range = localUniformData.size();

		currentUsedUniformStreamBuffersCount++;
	}

	// TODO: iteration order must match the order at the end of compileShaders right now.
	// TODO: We can store data via setTextures and setBuffers instead of iterating over
	// everything here.
	for (const auto &u : reflection.sampledTextures)
	{
		const auto &info = u.second;
		if (!info.active)
			continue;

		for (int i = 0; i < info.count; i++)
		{
			auto vkTexture = dynamic_cast<Texture*>(activeTextures[info.resourceIndex + i]);

			if (vkTexture == nullptr)
				throw love::Exception("uniform variable %s is not set.", info.name.c_str());

			VkDescriptorImageInfo &imageInfo = descriptorImages[imageIndex++];

			imageInfo.imageLayout = vkTexture->getImageLayout();
			imageInfo.imageView = (VkImageView)vkTexture->getRenderTargetHandle();
			imageInfo.sampler = (VkSampler)vkTexture->getSamplerHandle();
		}
	}

	for (const auto &u : reflection.storageTextures)
	{
		const auto &info = u.second;
		if (!info.active)
			continue;

		for (int i = 0; i < info.count; i++)
		{
			auto vkTexture = dynamic_cast<Texture*>(activeTextures[info.resourceIndex + i]);

			if (vkTexture == nullptr)
				throw love::Exception("uniform variable %s is not set.", info.name.c_str());

			VkDescriptorImageInfo &imageInfo = descriptorImages[imageIndex++];

			imageInfo.imageLayout = vkTexture->getImageLayout();
			imageInfo.imageView = (VkImageView)vkTexture->getRenderTargetHandle();
		}
	}

	for (const auto &u : reflection.texelBuffers)
	{
		const auto &info = u.second;
		if (!info.active)
			continue;

		for (int i = 0; i < info.count; i++)
		{
			auto b = activeBuffers[info.resourceIndex + i];
			if (b == nullptr)
				throw love::Exception("uniform variable %s is not set.", info.name.c_str());

			descriptorBufferViews[bufferViewIndex++] = (VkBufferView)b->getTexelBufferHandle();
		}
	}

	for (const auto &u : reflection.storageBuffers)
	{
		const auto &info = u.second;
		if (!info.active)
			continue;

		for (int i = 0; i < info.count; i++)
		{
			auto b = activeBuffers[info.resourceIndex + i];
			if (b == nullptr)
				throw love::Exception("uniform variable %s is not set.", info.name.c_str());

			VkDescriptorBufferInfo &bufferInfo = descriptorBuffers[bufferIndex++];
			bufferInfo.buffer = (VkBuffer)b->getHandle();
			bufferInfo.offset = 0;
			bufferInfo.range = b->getSize();
		}
	}

	VkDescriptorSet currentDescriptorSet = allocateDescriptorSet();

	for (auto &write : descriptorWrites)
		write.dstSet = currentDescriptorSet;

	vkUpdateDescriptorSets(device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);

	vkCmdBindDescriptorSets(commandBuffer, bindPoint, pipelineLayout, 0, 1, &currentDescriptorSet, 0, nullptr);
}

Shader::~Shader()
{
	unloadVolatile();
}

void Shader::attach()
{
	if (!isCompute)
	{
		if (Shader::current != this)
		{
			Graphics::flushBatchedDrawsGlobal();
			Shader::current = this;
			Vulkan::shaderSwitch();
		}
	}
	else
		vgfx->setComputeShader(this);
}

int Shader::getVertexAttributeIndex(const std::string &name)
{
	auto it = attributes.find(name);
	return it == attributes.end() ? -1 : it->second;
}

const Shader::UniformInfo *Shader::getUniformInfo(BuiltinUniform builtin) const
{
	return builtinUniformInfo[builtin];
}

void Shader::updateUniform(const UniformInfo *info, int count)
{
	if (current == this)
		Graphics::flushBatchedDrawsGlobal();

	if (usesLocalUniformData(info))
		memcpy(localUniformData.data(), localUniformStagingData.data(), localUniformStagingData.size());
}

void Shader::sendTextures(const UniformInfo *info, graphics::Texture **textures, int count)
{
	if (current == this)
		Graphics::flushBatchedDrawsGlobal();

	for (int i = 0; i < count; i++)
	{
		int resourceindex = info->resourceIndex + i;
		auto oldTexture = activeTextures[resourceindex];
		activeTextures[resourceindex] = textures[i];
		activeTextures[resourceindex]->retain();
		if (oldTexture)
			oldTexture->release();
	}
}

void Shader::sendBuffers(const UniformInfo *info, love::graphics::Buffer **buffers, int count)
{
	if (current == this)
		Graphics::flushBatchedDrawsGlobal();

	for (int i = 0; i < count; i++)
	{
		int resourceindex = info->resourceIndex + i;
		auto oldBuffer = activeBuffers[resourceindex];
		activeBuffers[resourceindex] = buffers[i];
		activeBuffers[resourceindex]->retain();
		if (oldBuffer)
			oldBuffer->release();
	}
}

void Shader::calculateUniformBufferSizeAligned()
{
	auto minAlignment = vgfx->getMinUniformBufferOffsetAlignment();
	size_t size = localUniformStagingData.size();
	auto factor = static_cast<VkDeviceSize>(std::ceil(static_cast<float>(size) / static_cast<float>(minAlignment)));
	uniformBufferSizeAligned = factor * minAlignment;
}

void Shader::buildLocalUniforms(spirv_cross::Compiler &comp, const spirv_cross::SPIRType &type, size_t baseoff, const std::string &basename)
{
	using namespace spirv_cross;

	const auto &membertypes = type.member_types;

	for (size_t uindex = 0; uindex < membertypes.size(); uindex++)
	{
		const auto &memberType = comp.get_type(membertypes[uindex]);
		size_t memberSize = comp.get_declared_struct_member_size(type, uindex);
		size_t offset = baseoff + comp.type_struct_member_offset(type, uindex);

		std::string name = basename + comp.get_member_name(type.self, uindex);

		switch (memberType.basetype)
		{
		case SPIRType::Struct:
			name += ".";
			buildLocalUniforms(comp, memberType, offset, name);
			continue;
		case SPIRType::Int:
		case SPIRType::UInt:
		case SPIRType::Float:
			break;
		default:
			continue;
		}

		name = canonicaliizeUniformName(name);

		auto uniformit = reflection.allUniforms.find(name);
		if (uniformit == reflection.allUniforms.end())
		{
			handleUnknownUniformName(name.c_str());
			continue;
		}

		UniformInfo &u = *(uniformit->second);

		u.active = true;
		u.dataSize = memberSize;
		u.data = localUniformStagingData.data() + offset;

		const auto &valuesit = reflection.localUniformInitializerValues.find(name);
		if (valuesit != reflection.localUniformInitializerValues.end())
		{
			const auto &values = valuesit->second;
			if (!values.empty())
				memcpy(
					u.data,
					values.data(),
					std::min(u.dataSize, values.size() * sizeof(LocalUniformValue)));
		}

		BuiltinUniform builtin = BUILTIN_MAX_ENUM;
		if (getConstant(u.name.c_str(), builtin))
		{
			if (builtin == BUILTIN_UNIFORMS_PER_DRAW)
				builtinUniformDataOffset = offset;
			builtinUniformInfo[builtin] = &u;
		}
	}
}

void Shader::compileShaders()
{
	using namespace glslang;
	using namespace spirv_cross;

	std::vector<std::unique_ptr<TShader>> glslangShaders;

	auto program = std::make_unique<TProgram>();

	const auto &enabledExtensions = vgfx->getEnabledOptionalDeviceExtensions();

	for (int i = 0; i < SHADERSTAGE_MAX_ENUM; i++)
	{
		if (!stages[i])
			continue;

		auto stage = (ShaderStageType)i;

		if (stage == SHADERSTAGE_COMPUTE)
			isCompute = true;

		auto glslangShaderStage = getGlslShaderType(stage);
		auto tshader = std::make_unique<TShader>(glslangShaderStage);

		tshader->setEnvInput(EShSourceGlsl, glslangShaderStage, EShClientVulkan, 450);
		tshader->setEnvClient(EShClientVulkan, EShTargetVulkan_1_2);
		if (enabledExtensions.spirv14)
			tshader->setEnvTarget(EshTargetSpv, EShTargetSpv_1_4);
		else
			tshader->setEnvTarget(EshTargetSpv, EShTargetSpv_1_0);
		tshader->setAutoMapLocations(true);
		tshader->setAutoMapBindings(true);
		tshader->setEnvInputVulkanRulesRelaxed();
		tshader->setGlobalUniformBinding(0);
		tshader->setGlobalUniformSet(0);

		auto &glsl = stages[i]->getSource();
		const char *csrc = glsl.c_str();
		const int sourceLength = static_cast<int>(glsl.length());
		tshader->setStringsWithLengths(&csrc, &sourceLength, 1);

		int defaultVersion = 450;
		EProfile defaultProfile = ECoreProfile;
		bool forceDefault = false;
		bool forwardCompat = true;

		if (!tshader->parse(GetDefaultResources(), defaultVersion, defaultProfile, forceDefault, forwardCompat, EShMsgSuppressWarnings))
		{
			const char *stageName = "unknown";
			ShaderStage::getConstant(stage, stageName);

			std::string err = "Error parsing " + std::string(stageName) + " shader:\n\n"
				+ std::string(tshader->getInfoLog()) + "\n"
				+ std::string(tshader->getInfoDebugLog());

			throw love::Exception("%s", err.c_str());
		}

		program->addShader(tshader.get());
		glslangShaders.push_back(std::move(tshader));
	}

	if (!program->link(EShMsgDefault))
		throw love::Exception("link failed! %s\n", program->getInfoLog());

	if (!program->mapIO())
		throw love::Exception("mapIO failed");

	BindingMapper bindingMapper;

	for (int i = 0; i < SHADERSTAGE_MAX_ENUM; i++)
	{
		auto shaderStage = (ShaderStageType)i;
		auto glslangStage = getGlslShaderType(shaderStage);
		auto intermediate = program->getIntermediate(glslangStage);

		if (intermediate == nullptr)
			continue;

		spv::SpvBuildLogger logger;
		glslang::SpvOptions opt;
		opt.validate = true;

		std::vector<uint32_t> spirv;
		GlslangToSpv(*intermediate, spirv, &logger, &opt);

		spirv_cross::CompilerGLSL comp(spirv);

		// we only care about variables that are actually getting used.
		auto active = comp.get_active_interface_variables();
		auto shaderResources = comp.get_shader_resources(active);
		comp.set_enabled_interface_variables(std::move(active));

		for (const auto &resource : shaderResources.uniform_buffers)
		{
			if (resource.name == "gl_DefaultUniformBlock")
			{
				const auto &type = comp.get_type(resource.base_type_id);
				size_t defaultUniformBlockSize = comp.get_declared_struct_size(type);

				localUniformStagingData.resize(defaultUniformBlockSize);
				localUniformData.resize(defaultUniformBlockSize);
				localUniformLocation = bindingMapper(comp, spirv, resource.name, 1, resource.id);

				memset(localUniformStagingData.data(), 0, defaultUniformBlockSize);
				memset(localUniformData.data(), 0, defaultUniformBlockSize);

				std::string basename("");
				buildLocalUniforms(comp, type, 0, basename);

				memcpy(localUniformData.data(), localUniformStagingData.data(), localUniformStagingData.size());
			}
			else
				throw love::Exception("unimplemented: non default uniform blocks.");
		}

		for (const auto &r : shaderResources.sampled_images)
		{
			std::string name = canonicaliizeUniformName(r.name);
			auto uniformit = reflection.allUniforms.find(name);
			if (uniformit == reflection.allUniforms.end())
			{
				handleUnknownUniformName(name.c_str());
				continue;
			}

			UniformInfo &u = *(uniformit->second);
			u.active = true;
			u.location = bindingMapper(comp, spirv, name, u.count, r.id);

			BuiltinUniform builtin;
			if (getConstant(name.c_str(), builtin))
				builtinUniformInfo[builtin] = &u;
		}

		for (const auto &r : shaderResources.storage_buffers)
		{
			std::string name = canonicaliizeUniformName(r.name);
			const auto &uniformit = reflection.storageBuffers.find(name);
			if (uniformit == reflection.storageBuffers.end())
			{
				handleUnknownUniformName(name.c_str());
				continue;
			}

			UniformInfo &u = uniformit->second;
			u.active = true;
			u.location = bindingMapper(comp, spirv, name, u.count, r.id);
		}

		for (const auto &r : shaderResources.storage_images)
		{
			std::string name = canonicaliizeUniformName(r.name);
			const auto &uniformit = reflection.storageBuffers.find(name);
			if (uniformit == reflection.storageBuffers.end())
			{
				handleUnknownUniformName(name.c_str());
				continue;
			}

			UniformInfo &u = uniformit->second;
			u.active = true;
			u.location = bindingMapper(comp, spirv, name, u.count, r.id);
		}

		if (shaderStage == SHADERSTAGE_VERTEX)
		{
			int nextAttributeIndex = ATTRIB_MAX_ENUM;

			for (const auto &r : shaderResources.stage_inputs)
			{
				int index;

				BuiltinVertexAttribute builtinAttribute;
				if (graphics::getConstant(r.name.c_str(), builtinAttribute))
					index = (int)builtinAttribute;
				else
					index = nextAttributeIndex++;

				uint32_t locationOffset;
				if (!comp.get_binary_offset_for_decoration(r.id, spv::DecorationLocation, locationOffset))
					throw love::Exception("could not get binary offset for vertex attribute %s location", r.name.c_str());

				spirv[locationOffset] = (uint32_t)index;

				attributes[r.name] = index;
			}
		}

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = spirv.size() * sizeof(uint32_t);
		createInfo.pCode = spirv.data();

		VkShaderModule shaderModule;

		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
			throw love::Exception("failed to create shader module");

		std::string debugname = getShaderStageDebugName(shaderStage);
		if (!debugname.empty() && vgfx->getEnabledOptionalInstanceExtensions().debugInfo)
		{
			auto device = vgfx->getDevice();

			VkDebugUtilsObjectNameInfoEXT nameInfo{};
			nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
			nameInfo.objectType = VK_OBJECT_TYPE_SHADER_MODULE;
			nameInfo.objectHandle = (uint64_t)shaderModule;
			nameInfo.pObjectName = debugname.c_str();
			vkSetDebugUtilsObjectNameEXT(device, &nameInfo);
		}

		shaderModules.push_back(shaderModule);

		VkPipelineShaderStageCreateInfo shaderStageInfo{};
		shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo.stage = getStageBit((ShaderStageType)i);
		shaderStageInfo.module = shaderModule;
		shaderStageInfo.pName = "main";

		shaderStages.push_back(shaderStageInfo);
	}

	int numBuffers = 0;
	int numTextures = 0;
	int numBufferViews = 0;

	if (localUniformData.size() > 0)
		numBuffers++;

	for (const auto kvp : reflection.allUniforms)
	{
		if (!kvp.second->active)
			continue;

		switch (kvp.second->baseType)
		{
		case UNIFORM_SAMPLER:
		case UNIFORM_STORAGETEXTURE:
			numTextures += kvp.second->count;
			break;
		case UNIFORM_STORAGEBUFFER:
			numBuffers += kvp.second->count;
			break;
		case UNIFORM_TEXELBUFFER:
			numBufferViews += kvp.second->count;
			break;
		default:
			continue;
		}
	}

	descriptorWrites.clear();

	descriptorBuffers.clear();
	descriptorBuffers.reserve(numBuffers);

	descriptorImages.clear();
	descriptorImages.reserve(numTextures);

	descriptorBufferViews.clear();
	descriptorBufferViews.reserve(numBufferViews);

	if (localUniformData.size() > 0)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.range = localUniformData.size();

		descriptorBuffers.push_back(bufferInfo);

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstBinding = localUniformLocation;
		write.dstArrayElement = 0;
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write.descriptorCount = 1;
		write.pBufferInfo = &descriptorBuffers.back();
		descriptorWrites.push_back(write);
	}

	for (const auto &u : reflection.sampledTextures)
	{
		const UniformInfo &info = u.second;
		if (!info.active)
			continue;

		for (int i = 0; i < info.count; i++)
		{
			VkDescriptorImageInfo imageInfo{};
			descriptorImages.push_back(imageInfo);
		}

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstBinding = info.location;
		write.dstArrayElement = 0;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.descriptorCount = static_cast<uint32_t>(info.count);
		write.pImageInfo = &descriptorImages[descriptorImages.size() - info.count];

		descriptorWrites.push_back(write);
	}

	for (const auto &u : reflection.storageTextures)
	{
		const UniformInfo &info = u.second;
		if (!info.active)
			continue;

		for (int i = 0; i < info.count; i++)
		{
			VkDescriptorImageInfo imageInfo{};
			descriptorImages.push_back(imageInfo);
		}

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstBinding = info.location;
		write.dstArrayElement = 0;
		write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		write.descriptorCount = static_cast<uint32_t>(info.count);
		write.pImageInfo = &descriptorImages[descriptorImages.size() - info.count];

		descriptorWrites.push_back(write);
	}

	for (const auto &u : reflection.texelBuffers)
	{
		const UniformInfo &info = u.second;
		if (!info.active)
			continue;

		for (int i = 0; i < info.count; i++)
			descriptorBufferViews.push_back(VK_NULL_HANDLE);

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstBinding = info.location;
		write.dstArrayElement = 0;
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
		write.descriptorCount = info.count;
		write.pTexelBufferView = &descriptorBufferViews[descriptorBufferViews.size() - info.count];

		descriptorWrites.push_back(write);
	}

	for (const auto &u : reflection.storageBuffers)
	{
		const UniformInfo &info = u.second;
		if (!info.active)
			continue;

		for (int i = 0; i < info.count; i++)
		{
			VkDescriptorBufferInfo bufferInfo{};
			descriptorBuffers.push_back(bufferInfo);
		}

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstBinding = info.location;
		write.dstArrayElement = 0;
		write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		write.descriptorCount = info.count;
		write.pBufferInfo = &descriptorBuffers[descriptorBuffers.size() - info.count];

		descriptorWrites.push_back(write);
	}
}

void Shader::createDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	VkShaderStageFlags stageFlags;
	if (isCompute)
		stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	else
		stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	for (auto const &entry : reflection.allUniforms)
	{
		if (!entry.second->active)
			continue;

		auto type = Vulkan::getDescriptorType(entry.second->baseType);
		if (type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		{
			VkDescriptorSetLayoutBinding layoutBinding{};

			layoutBinding.binding = entry.second->location;
			layoutBinding.descriptorType = type;
			layoutBinding.descriptorCount = entry.second->count;
			layoutBinding.stageFlags = stageFlags;

			bindings.push_back(layoutBinding);
		}
	}

	if (!localUniformStagingData.empty())
	{
		VkDescriptorSetLayoutBinding uniformBinding{};
		uniformBinding.binding = localUniformLocation;
		uniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniformBinding.descriptorCount = 1;
		uniformBinding.stageFlags = stageFlags;
		bindings.push_back(uniformBinding);
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		throw love::Exception("failed to create descriptor set layout");
}

void Shader::createPipelineLayout()
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		throw love::Exception("failed to create pipeline layout");

	if (isCompute)
	{
		assert(shaderStages.size() == 1);

		VkComputePipelineCreateInfo computeInfo{};
		computeInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		computeInfo.stage = shaderStages.at(0);
		computeInfo.layout = pipelineLayout;

		if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &computeInfo, nullptr, &computePipeline) != VK_SUCCESS)
			throw love::Exception("failed to create compute pipeline");
	}
}

void Shader::createDescriptorPoolSizes()
{
	if (!localUniformData.empty())
	{
		VkDescriptorPoolSize size{};
		size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		size.descriptorCount = 1;

		descriptorPoolSizes.push_back(size);
	}

	for (const auto &entry : reflection.allUniforms)
	{
		if (entry.second->location < 0)
			continue;

		VkDescriptorPoolSize size{};
		auto type = Vulkan::getDescriptorType(entry.second->baseType);
		if (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			continue;

		size.type = type;
		size.descriptorCount = 1;
		descriptorPoolSizes.push_back(size);
	}
}

void Shader::createStreamBuffers()
{
	size_t size = STREAMBUFFER_DEFAULT_SIZE * uniformBufferSizeAligned;
	if (size > 0)
		streamBuffers.push_back(new StreamBuffer(vgfx, BUFFERUSAGE_UNIFORM, size));
}

void Shader::setVideoTextures(graphics::Texture *ytexture, graphics::Texture *cbtexture, graphics::Texture *crtexture)
{
	std::array<graphics::Texture*, 3> textures = {
		ytexture, cbtexture, crtexture
	};

	std::array<BuiltinUniform, 3> builtIns = {
		BUILTIN_TEXTURE_VIDEO_Y,
		BUILTIN_TEXTURE_VIDEO_CB,
		BUILTIN_TEXTURE_VIDEO_CR,
	};

	static_assert(textures.size() == builtIns.size(), "expected number of textures to be the same");

	for (size_t i = 0; i < textures.size(); i++)
	{
		const UniformInfo *u = builtinUniformInfo[builtIns[i]];
		if (u != nullptr)
		{
			textures[i]->retain();
			if (activeTextures[u->resourceIndex])
				activeTextures[u->resourceIndex]->release();
			activeTextures[u->resourceIndex] = textures[i];
		}
	}
}

void Shader::setMainTex(graphics::Texture *texture)
{
	const UniformInfo *u = builtinUniformInfo[BUILTIN_TEXTURE_MAIN];
	if (u != nullptr)
	{
		texture->retain();
		if (activeTextures[u->resourceIndex])
			activeTextures[u->resourceIndex]->release();
		activeTextures[u->resourceIndex] = texture;
	}
}

void Shader::createDescriptorPool()
{
	VkDescriptorPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.maxSets = DESCRIPTOR_POOL_SIZE;
	createInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
	createInfo.pPoolSizes = descriptorPoolSizes.data();

	VkDescriptorPool pool;
	if (vkCreateDescriptorPool(device, &createInfo, nullptr, &pool) != VK_SUCCESS)
		throw love::Exception("failed to create descriptor pool");

	descriptorPools[currentFrame].push_back(pool);
}

VkDescriptorSet Shader::allocateDescriptorSet()
{
	if (descriptorPools[currentFrame].empty())
		createDescriptorPool();

	while (true)
	{
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPools[currentFrame][currentDescriptorPool];
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptorSetLayout;

		VkDescriptorSet descriptorSet;
		VkResult result = vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet);

		switch (result)
		{
		case VK_SUCCESS:
			return descriptorSet;
		case VK_ERROR_OUT_OF_POOL_MEMORY:
			currentDescriptorPool++;
			if (descriptorPools[currentFrame].size() <= currentDescriptorPool)
				createDescriptorPool();
			continue;
		default:
			throw love::Exception("failed to allocate descriptor set");
		}
	}
}

} // vulkan
} // graphics
} // love
