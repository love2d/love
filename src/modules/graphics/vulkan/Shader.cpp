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

static const uint32_t DESCRIPTOR_POOL_SIZE = 1000;

SharedDescriptorPools::SharedDescriptorPools(VkDevice device, int dynamicUniformBuffers, int sampledTextures, int storageTextures, int texelBuffers, int storageBuffers)
	: device(device)
	, dynamicUniformBuffers(dynamicUniformBuffers)
	, sampledTextures(sampledTextures)
	, storageTextures(storageTextures)
	, texelBuffers(texelBuffers)
	, storageBuffers(storageBuffers)
{
	VkDescriptorPoolSize size{};

	if (dynamicUniformBuffers > 0)
	{
		size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		size.descriptorCount = dynamicUniformBuffers;
		descriptorPoolSizes.push_back(size);
	}

	if (sampledTextures > 0)
	{
		size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		size.descriptorCount = sampledTextures;
		descriptorPoolSizes.push_back(size);
	}

	if (storageTextures > 0)
	{
		size.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		size.descriptorCount = storageTextures;
		descriptorPoolSizes.push_back(size);
	}

	if (texelBuffers > 0)
	{
		size.type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
		size.descriptorCount = texelBuffers;
		descriptorPoolSizes.push_back(size);
	}

	if (storageBuffers > 0)
	{
		size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		size.descriptorCount = storageBuffers;
		descriptorPoolSizes.push_back(size);
	}

	pools.resize(MAX_FRAMES_IN_FLIGHT);
}

SharedDescriptorPools::~SharedDescriptorPools()
{
	auto vgfx = (Graphics *)Module::getInstance<Graphics>(Module::M_GRAPHICS);
	if (vgfx == nullptr)
		return;

	vgfx->queueCleanUp([device = device, descriptorPools = pools]()
	{
		for (const auto &pools : descriptorPools)
		{
			for (const auto pool : pools)
				vkDestroyDescriptorPool(device, pool, nullptr);
		}
	});
}

void SharedDescriptorPools::newFrame(uint64 frameIndex)
{
	if (!lastFrameIndex.hasValue || lastFrameIndex.value != frameIndex)
	{
		lastFrameIndex.set(frameIndex);
		currentFrame = (size_t)((currentFrame + 1) % MAX_FRAMES_IN_FLIGHT);
		currentPool = 0;
		for (VkDescriptorPool pool : pools[currentFrame])
			vkResetDescriptorPool(device, pool, 0);
	}
}

void SharedDescriptorPools::createDescriptorPool()
{
	VkDescriptorPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.maxSets = DESCRIPTOR_POOL_SIZE;
	createInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
	createInfo.pPoolSizes = descriptorPoolSizes.data();

	VkDescriptorPool pool;
	VkResult result = vkCreateDescriptorPool(device, &createInfo, nullptr, &pool);
	if (result != VK_SUCCESS)
		throw love::Exception("Failed to create Vulkan descriptor pool: %s", Vulkan::getErrorString(result));

	pools[currentFrame].push_back(pool);
}

VkDescriptorSet SharedDescriptorPools::allocateDescriptorSet(const VkDescriptorSetLayout &descriptorSetLayout)
{
	if (pools[currentFrame].empty())
		createDescriptorPool();

	while (true)
	{
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = pools[currentFrame][currentPool];
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptorSetLayout;

		VkDescriptorSet descriptorSet;
		VkResult result = vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet);

		switch (result)
		{
		case VK_SUCCESS:
			return descriptorSet;
		case VK_ERROR_OUT_OF_POOL_MEMORY:
			currentPool++;
			if (pools[currentFrame].size() <= currentPool)
				createDescriptorPool();
			continue;
		default:
			throw love::Exception("Failed to allocate Vulkan descriptor set: %s", Vulkan::getErrorString(result));
		}
	}
}

class BindingMapper
{
public:

	BindingMapper(spv::Decoration decoration)
		: decoration(decoration)
	{}

	uint32_t operator()(spirv_cross::CompilerGLSL &comp, std::vector<uint32_t> &spirv, const std::string &name, int count, const spirv_cross::ID &id)
	{
		auto it = bindingMappings.find(name);
		if (it == bindingMappings.end())
		{
			auto binding = comp.get_decoration(id, decoration);

			if (isFreeBinding(binding, count))
			{
				bindingMappings[name] = Range(binding, count);
				return binding;
			}
			else
			{
				uint32_t freeBinding = getFreeBinding(count);

				uint32_t binaryBindingOffset;
				if (!comp.get_binary_offset_for_decoration(id, decoration, binaryBindingOffset))
					throw love::Exception("could not get binary offset for uniform %s binding", name.c_str());

				spirv[binaryBindingOffset] = freeBinding;

				bindingMappings[name] = Range(freeBinding, count);

				return freeBinding;
			}
		}
		else
		{
			auto binding = (uint32_t)it->second.getOffset();

			uint32_t binaryBindingOffset;
			if (!comp.get_binary_offset_for_decoration(id, decoration, binaryBindingOffset))
				throw love::Exception("could not get binary offset for uniform %s binding", name.c_str());

			spirv[binaryBindingOffset] = binding;

			return binding;
		}
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

	spv::Decoration decoration;
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
		throw love::Exception("Invalid shader stage type: %d", type);
	}
}

static VkShaderStageFlags getStageFlags(ShaderStageMask mask)
{
	VkShaderStageFlags flags = 0;
	if (mask & SHADERSTAGEMASK_VERTEX)
		flags |= VK_SHADER_STAGE_VERTEX_BIT;
	if (mask & SHADERSTAGEMASK_PIXEL)
		flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
	if (mask & SHADERSTAGEMASK_COMPUTE)
		flags |= VK_SHADER_STAGE_COMPUTE_BIT;
	return flags;
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
		throw love::Exception("Unknown shader stage type: %d", stage);
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
	, builtinUniformInfo()
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
	createDescriptorSetLayout();
	createPipelineLayout();
	acquireDescriptorPools();
	newFrame(vgfx->getRealFrameIndex());

	return true;
}

void Shader::unloadVolatile()
{
	if (shaderModules.empty())
		return;

	vgfx->releaseDescriptorPools(descriptorPools);
	descriptorPools = nullptr;

	vgfx->queueCleanUp([shaderModules = std::move(shaderModules), device = device, descriptorSetLayout = descriptorSetLayout, pipelineLayout = pipelineLayout,
		computePipeline = computePipeline,
		graphicsPipelinesCore = std::move(graphicsPipelinesDynamicState), graphicsPipelinesFull = std::move(graphicsPipelinesNoDynamicState)]() {
		for (const auto shaderModule : shaderModules)
			vkDestroyShaderModule(device, shaderModule, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		if (computePipeline != VK_NULL_HANDLE)
			vkDestroyPipeline(device, computePipeline, nullptr);
		for (const auto &kvp : graphicsPipelinesCore)
			vkDestroyPipeline(device, kvp.second, nullptr);
		for (const auto &kvp : graphicsPipelinesFull)
			vkDestroyPipeline(device, kvp.second, nullptr);
	});

	shaderModules.clear();
	shaderStages.clear();
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

void Shader::newFrame(uint64 graphicsFrameIndex)
{
	currentDescriptorSet = VK_NULL_HANDLE;
	resourceDescriptorsDirty = true;

	descriptorPools->newFrame(graphicsFrameIndex);
}

void Shader::cmdPushDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint)
{
	bool useLocalUniformOffset = false;
	uint32 localUniformOffset = 0;

	if (!localUniformData.empty())
	{
		if (builtinUniformDataOffset.hasValue)
		{
			auto builtinData = vgfx->getCurrentBuiltinUniformData();
			auto dst = (BuiltinUniformData *) (localUniformData.data() + builtinUniformDataOffset.value);
			memcpy(dst, &builtinData, sizeof(builtinData));
		}

		VkDescriptorBufferInfo info = {};
		vgfx->mapLocalUniformData(localUniformData.data(), localUniformData.size(), info);

		// This is a dynamic uniform buffer, so the offset is specified in BindDescriptorSets
		// and it only needs to update the descriptor sets if the buffer changes.
		if (info.buffer != descriptorBuffers[0].buffer)
			resourceDescriptorsDirty = true;

		descriptorBuffers[0].buffer = info.buffer;
		descriptorBuffers[0].range = info.range;
		descriptorBuffers[0].offset = 0;

		useLocalUniformOffset = true;
		localUniformOffset = info.offset;
	}

	// Sampler updates need to happen here because the handles may change after sendTextures.
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

			auto sampler = (VkSampler)vkTexture->getSamplerHandle();

			VkDescriptorImageInfo &imageInfo = descriptorImages[info.bindingStartIndex + i];
			if (sampler != imageInfo.sampler)
			{
				imageInfo.sampler = sampler;
				resourceDescriptorsDirty = true;
			}
		}
	}

	if (resourceDescriptorsDirty || currentDescriptorSet == VK_NULL_HANDLE)
	{
		currentDescriptorSet = descriptorPools->allocateDescriptorSet(descriptorSetLayout);

		for (auto &write : descriptorWrites)
			write.dstSet = currentDescriptorSet;

		vkUpdateDescriptorSets(device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
		resourceDescriptorsDirty = false;
	}

	vkCmdBindDescriptorSets(commandBuffer, bindPoint, pipelineLayout, 0, 1, &currentDescriptorSet, useLocalUniformOffset ? 1 : 0, &localUniformOffset);
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
}

int Shader::getVertexAttributeIndex(const std::string &name)
{
	auto it = attributes.find(name);
	return it == attributes.end() ? -1 : it->second.index;
}

const Shader::UniformInfo *Shader::getUniformInfo(BuiltinUniform builtin) const
{
	return builtinUniformInfo[builtin];
}

void Shader::updateUniform(const UniformInfo *info, int count)
{
	if (current == this)
		Graphics::flushBatchedDrawsGlobal();

	count = std::min(count, info->count);

	if (info->data != nullptr)
	{
		size_t offset = (const uint8*)info->data - localUniformStagingData.data();
		uint8 *dst = localUniformData.data() + offset;
		copyToUniformBuffer(info, info->data, dst, count);
	}
}

void Shader::applyTexture(const UniformInfo *info, int i, love::graphics::Texture *texture, UniformType /*basetype*/, bool isdefault)
{
	setTextureDescriptor(info, (isdefault && (info->access & ACCESS_WRITE) != 0) ? nullptr : texture, i);
}

void Shader::applyBuffer(const UniformInfo *info, int i, love::graphics::Buffer *buffer, UniformType /*basetype*/, bool isdefault)
{
	setBufferDescriptor(info, (isdefault && (info->access & ACCESS_WRITE) != 0) ? nullptr : buffer, i);
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
			if (memberType.op == spv::OpTypeArray)
			{
				size_t arraystride = comp.type_struct_member_array_stride(type, uindex);
				for (uint32 i = 0; i < memberType.array[0]; i++)
				{
					std::string structname = name + "[" + std::to_string(i) + "].";
					buildLocalUniforms(comp, memberType, offset + i * arraystride, structname);
				}
			}
			else
			{
				std::string structname = name + ".";
				buildLocalUniforms(comp, memberType, offset, structname);
			}
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
			{
				memcpy(
					u.data,
					values.data(),
					std::min(u.dataSize, values.size() * sizeof(LocalUniformValue)));

				uint8 *dst = localUniformData.data() + offset;
				copyToUniformBuffer(&u, u.data, dst, u.count);
			}
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

		if (!tshader->parse(GetResources(), defaultVersion, defaultProfile, forceDefault, forwardCompat, EShMsgSuppressWarnings))
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

	BindingMapper bindingMapper(spv::DecorationBinding);
	BindingMapper ioLocationMapper(spv::DecorationLocation);
	BindingMapper vertexInputLocationMapper(spv::DecorationLocation);

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

		std::vector<uint32> spirv;

		GlslangToSpv(*intermediate, spirv, &logger, &opt);

		auto compiler = std::make_unique<spirv_cross::CompilerGLSL>(spirv);
		auto &comp = *compiler;

		// We aren't recompiling the SPIR-V to something else, so
		// set_enabled_interface_variables wouldn't do much.
		// Vulkan has various rules about making sure bindings to inputs and
		// resources are valid, so we can't skip inactive ones here.
		// Unfortunately GlslangToSpv doesn't strip unused resources even
		// though it knows about them...
		auto active = compiler->get_active_interface_variables();
		auto shaderResources = comp.get_shader_resources();

		for (const auto &resource : shaderResources.uniform_buffers)
		{
			// TODO: Do something smarter here.
			if (active.find(resource.id) == active.end())
				continue;

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
			}
			else
				throw love::Exception("unimplemented: non default uniform blocks.");
		}

		for (const auto &r : shaderResources.sampled_images)
		{
			// TODO: Do something smarter here.
			if (active.find(r.id) == active.end())
				continue;

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
			// TODO: Do something smarter here.
			if (active.find(r.id) == active.end())
				continue;

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
			// TODO: Do something smarter here.
			if (active.find(r.id) == active.end())
				continue;

			std::string name = canonicaliizeUniformName(r.name);
			const auto &uniformit = reflection.storageTextures.find(name);
			if (uniformit == reflection.storageTextures.end())
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
			// Use the mapper on known used inputs first, so their bindings get
			// put into the map without being changed.
			for (const auto &r : shaderResources.stage_inputs)
			{
				auto it = reflection.vertexInputs.find(r.name);
				if (it != reflection.vertexInputs.end() && it->second >= 0)
					vertexInputLocationMapper(comp, spirv, r.name, 1, r.id);
			}

			for (const auto &r : shaderResources.stage_inputs)
			{
				// Don't skip unused inputs, vulkan still needs to have valid
				// bindings for them. This will also avoid shuffling intentional
				// used bindings because of the earlier loop.
				int index = (int)vertexInputLocationMapper(comp, spirv, r.name, 1, r.id);

				DataBaseType basetype = DATA_BASETYPE_FLOAT;

				switch (comp.get_type(r.base_type_id).basetype)
				{
				case spirv_cross::SPIRType::Int:
					basetype = DATA_BASETYPE_INT;
					break;
				case spirv_cross::SPIRType::UInt:
					basetype = DATA_BASETYPE_UINT;
					break;
				default:
					break;
				}

				attributes[r.name] = { index, basetype };
			}

			for (const auto &r : shaderResources.stage_outputs)
			{
				const auto &type = comp.get_type(r.base_type_id);
				int count = type.array.empty() ? 1 : type.array[0];
				if (type.op == spv::OpTypeMatrix)
					count *= type.columns;

				ioLocationMapper(comp, spirv, r.name, count, r.id);
			}
		}
		else if (shaderStage == SHADERSTAGE_PIXEL)
		{
			for (const auto &r : shaderResources.stage_inputs)
			{
				const auto &type = comp.get_type(r.base_type_id);
				int count = type.array.empty() ? 1 : type.array[0];
				if (type.op == spv::OpTypeMatrix)
					count *= type.columns;

				ioLocationMapper(comp, spirv, r.name, count, r.id);
			}
		}

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = spirv.size() * sizeof(uint32_t);
		createInfo.pCode = spirv.data();

		VkShaderModule shaderModule;

		VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
		if (result != VK_SUCCESS)
			throw love::Exception("Failed to create Vulkan shader module: %s", Vulkan::getErrorString(result));

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

	for (const auto &kvp : reflection.allUniforms)
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

	allTextureInfo.clear();
	allTextureInfo.reserve(numTextures);
	storageBufferInfo.clear();
	storageBufferInfo.reserve(numBuffers);

	if (localUniformData.size() > 0)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.range = localUniformData.size();

		descriptorBuffers.push_back(bufferInfo);
		storageBufferInfo.push_back({ nullptr, ACCESS_READ }); // Dummy value.

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstBinding = localUniformLocation;
		write.dstArrayElement = 0;
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		write.descriptorCount = 1;
		write.pBufferInfo = &descriptorBuffers.back();
		descriptorWrites.push_back(write);
	}

	for (auto &u : reflection.sampledTextures)
	{
		UniformInfo &info = u.second;
		if (!info.active)
			continue;

		info.bindingStartIndex = (int)descriptorImages.size();

		for (int i = 0; i < info.count; i++)
		{
			VkDescriptorImageInfo imageInfo{};
			descriptorImages.push_back(imageInfo);

			allTextureInfo.push_back({ nullptr, info.access });

			auto texture = activeTextures[info.resourceIndex + i];
			if (texture != nullptr)
				setTextureDescriptor(&info, texture, i);
		}

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstBinding = info.location;
		write.dstArrayElement = 0;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.descriptorCount = static_cast<uint32_t>(info.count);
		write.pImageInfo = &descriptorImages[info.bindingStartIndex];

		descriptorWrites.push_back(write);
	}

	for (auto &u : reflection.storageTextures)
	{
		UniformInfo &info = u.second;
		if (!info.active)
			continue;

		info.bindingStartIndex = (int)descriptorImages.size();

		for (int i = 0; i < info.count; i++)
		{
			VkDescriptorImageInfo imageInfo{};
			descriptorImages.push_back(imageInfo);

			allTextureInfo.push_back({ nullptr, info.access });

			auto texture = activeTextures[info.resourceIndex + i];
			if (texture != nullptr)
				setTextureDescriptor(&info, texture, i);
		}

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstBinding = info.location;
		write.dstArrayElement = 0;
		write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		write.descriptorCount = static_cast<uint32_t>(info.count);
		write.pImageInfo = &descriptorImages[info.bindingStartIndex];

		descriptorWrites.push_back(write);
	}

	for (auto &u : reflection.texelBuffers)
	{
		UniformInfo &info = u.second;
		if (!info.active)
			continue;

		info.bindingStartIndex = (int)descriptorBufferViews.size();

		for (int i = 0; i < info.count; i++)
		{
			descriptorBufferViews.push_back(VK_NULL_HANDLE);

			auto buffer = activeBuffers[info.resourceIndex + i];
			if (buffer != nullptr)
				setBufferDescriptor(&info, buffer, i);
		}

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstBinding = info.location;
		write.dstArrayElement = 0;
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
		write.descriptorCount = info.count;
		write.pTexelBufferView = &descriptorBufferViews[info.bindingStartIndex];

		descriptorWrites.push_back(write);
	}

	for (auto &u : reflection.storageBuffers)
	{
		UniformInfo &info = u.second;
		if (!info.active)
			continue;

		info.bindingStartIndex = (int)descriptorBuffers.size();

		for (int i = 0; i < info.count; i++)
		{
			VkDescriptorBufferInfo bufferInfo{};
			descriptorBuffers.push_back(bufferInfo);

			storageBufferInfo.push_back({ nullptr, info.access });

			auto buffer = activeBuffers[info.resourceIndex + i];
			if (buffer != nullptr)
				setBufferDescriptor(&info, buffer, i);
		}

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstBinding = info.location;
		write.dstArrayElement = 0;
		write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		write.descriptorCount = info.count;
		write.pBufferInfo = &descriptorBuffers[info.bindingStartIndex];

		descriptorWrites.push_back(write);
	}

	resourceDescriptorsDirty = true;
}

void Shader::createDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	for (auto const &entry : reflection.allUniforms)
	{
		if (!entry.second->active)
			continue;

		auto type = Vulkan::getDescriptorType(entry.second->baseType);
		if (type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
		{
			VkDescriptorSetLayoutBinding layoutBinding{};

			layoutBinding.binding = entry.second->location;
			layoutBinding.descriptorType = type;
			layoutBinding.descriptorCount = entry.second->count;
			layoutBinding.stageFlags = getStageFlags((ShaderStageMask)entry.second->stageMask);

			bindings.push_back(layoutBinding);
		}
	}

	if (!localUniformStagingData.empty())
	{
		VkDescriptorSetLayoutBinding uniformBinding{};
		uniformBinding.binding = localUniformLocation;
		uniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		uniformBinding.descriptorCount = 1;
		if (isCompute)
			uniformBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		else
			uniformBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		bindings.push_back(uniformBinding);
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	VkResult result = vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout);
	if (result != VK_SUCCESS)
		throw love::Exception("Failed to create Vulkan descriptor set layout: %s", Vulkan::getErrorString(result));
}

void Shader::createPipelineLayout()
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;

	VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout);
	if (result != VK_SUCCESS)
		throw love::Exception("Failed to create Vulkan pipeline layout: %s", Vulkan::getErrorString(result));

	if (isCompute)
	{
		assert(shaderStages.size() == 1);

		VkComputePipelineCreateInfo computeInfo{};
		computeInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		computeInfo.stage = shaderStages.at(0);
		computeInfo.layout = pipelineLayout;

		result = vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &computeInfo, nullptr, &computePipeline);
		if (result != VK_SUCCESS)
			throw love::Exception("Failed to create Vulkan compute pipeline: %s", Vulkan::getErrorString(result));
	}
}

static int getDescriptorPoolSize(const std::map<std::string, Shader::UniformInfo> &uniforms)
{
	int size = 0;
	for (const auto &entry : uniforms)
	{
		if (entry.second.active)
			size += entry.second.count;
	}
	return size;
}

void Shader::acquireDescriptorPools()
{
	int dynamicUniformBuffers = 0;
	if (!localUniformData.empty())
		dynamicUniformBuffers++;

	int sampledTextures = getDescriptorPoolSize(reflection.sampledTextures);
	int storageTextures = getDescriptorPoolSize(reflection.storageTextures);
	int texelBuffers = getDescriptorPoolSize(reflection.texelBuffers);
	int storageBuffers = getDescriptorPoolSize(reflection.storageBuffers);

	descriptorPools = vgfx->acquireDescriptorPools(dynamicUniformBuffers, sampledTextures, storageTextures, texelBuffers, storageBuffers);
}

void Shader::setMainTex(graphics::Texture *texture)
{
	const UniformInfo *u = builtinUniformInfo[BUILTIN_TEXTURE_MAIN];
	if (u != nullptr)
	{
		auto prevtexture = activeTextures[u->resourceIndex];
		if (texture != nullptr)
			texture->retain();
		if (prevtexture)
			prevtexture->release();
		activeTextures[u->resourceIndex] = texture;

		if (texture != prevtexture)
			setTextureDescriptor(u, texture, 0);
	}
}

void Shader::setTextureDescriptor(const UniformInfo *info, love::graphics::Texture *texture, int index)
{
	auto vkTexture = dynamic_cast<Texture*>(texture);

	VkDescriptorImageInfo &imageInfo = descriptorImages[info->bindingStartIndex + index];

	// Samplers may change after this call, so they're set just before the
	// descriptor set is used instead of here.
	VkImageView view = vkTexture != nullptr ? (VkImageView)vkTexture->getHandle() : VK_NULL_HANDLE;
	if (view != imageInfo.imageView)
	{
		imageInfo.imageLayout = vkTexture != nullptr ? vkTexture->getImageLayout() : VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.imageView = view;
		allTextureInfo[info->bindingStartIndex + index].texture = texture;
		resourceDescriptorsDirty = true;
	}
}

void Shader::setBufferDescriptor(const UniformInfo *info, love::graphics::Buffer *buffer, int index)
{
	if (info->baseType == UNIFORM_STORAGEBUFFER)
	{
		VkDescriptorBufferInfo &bufferInfo = descriptorBuffers[info->bindingStartIndex + index];
		VkBuffer vkbuffer = buffer != nullptr ? (VkBuffer)buffer->getHandle() : VK_NULL_HANDLE;
		VkDeviceSize range = buffer != nullptr ? buffer->getSize() : 0;
		if (vkbuffer != bufferInfo.buffer || bufferInfo.offset != 0 || range != bufferInfo.range)
		{
			bufferInfo.buffer = vkbuffer;
			bufferInfo.offset = 0;
			bufferInfo.range = range;
			storageBufferInfo[info->bindingStartIndex + index].buffer = buffer;
			resourceDescriptorsDirty = true;
		}
	}
	else if (info->baseType == UNIFORM_TEXELBUFFER)
	{
		VkBufferView view = buffer != nullptr ? (VkBufferView)buffer->getTexelBufferHandle() : VK_NULL_HANDLE;
		if (view != descriptorBufferViews[info->bindingStartIndex + index])
		{
			descriptorBufferViews[info->bindingStartIndex + index] = view;
			resourceDescriptorsDirty = true;
		}
	}
}

VkPipeline Shader::getCachedGraphicsPipeline(Graphics *vgfx, const GraphicsPipelineConfigurationCore &configuration)
{
	auto it = graphicsPipelinesDynamicState.find(configuration);
	if (it != graphicsPipelinesDynamicState.end())
		return it->second;

	VkPipeline pipeline = vgfx->createGraphicsPipeline(this, configuration, nullptr);
	graphicsPipelinesDynamicState.insert({ configuration, pipeline });
	
	return pipeline;
}

VkPipeline Shader::getCachedGraphicsPipeline(Graphics *vgfx, const GraphicsPipelineConfigurationFull &configuration)
{
	auto it = graphicsPipelinesNoDynamicState.find(configuration);
	if (it != graphicsPipelinesNoDynamicState.end())
		return it->second;

	VkPipeline pipeline = vgfx->createGraphicsPipeline(this, configuration.core, &configuration.noDynamicState);
	graphicsPipelinesNoDynamicState.insert({ configuration, pipeline });
	
	return pipeline;
}

} // vulkan
} // graphics
} // love
