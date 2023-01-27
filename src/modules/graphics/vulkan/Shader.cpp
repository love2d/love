/**
 * Copyright (c) 2006-2023 LOVE Development Team
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

#include "Shader.h"
#include "Graphics.h"

#include "libraries/glslang/glslang/Public/ShaderLang.h"
#include "libraries/glslang/SPIRV/GlslangToSpv.h"


#include <vector>

namespace love
{
namespace graphics
{
namespace vulkan
{

static const TBuiltInResource defaultTBuiltInResource = {
	/* .MaxLights = */ 32,
	/* .MaxClipPlanes = */ 6,
	/* .MaxTextureUnits = */ 32,
	/* .MaxTextureCoords = */ 32,
	/* .MaxVertexAttribs = */ 64,
	/* .MaxVertexUniformComponents = */ 16384,
	/* .MaxVaryingFloats = */ 128,
	/* .MaxVertexTextureImageUnits = */ 32,
	/* .MaxCombinedTextureImageUnits = */ 80,
	/* .MaxTextureImageUnits = */ 32,
	/* .MaxFragmentUniformComponents = */ 16384,
	/* .MaxDrawBuffers = */ 8,
	/* .MaxVertexUniformVectors = */ 4096,
	/* .MaxVaryingVectors = */ 32,
	/* .MaxFragmentUniformVectors = */ 4096,
	/* .MaxVertexOutputVectors = */ 32,
	/* .MaxFragmentInputVectors = */ 31,
	/* .MinProgramTexelOffset = */ -8,
	/* .MaxProgramTexelOffset = */ 7,
	/* .MaxClipDistances = */ 8,
	/* .MaxComputeWorkGroupCountX = */ 65535,
	/* .MaxComputeWorkGroupCountY = */ 65535,
	/* .MaxComputeWorkGroupCountZ = */ 65535,
	/* .MaxComputeWorkGroupSizeX = */ 1024,
	/* .MaxComputeWorkGroupSizeY = */ 1024,
	/* .MaxComputeWorkGroupSizeZ = */ 64,
	/* .MaxComputeUniformComponents = */ 1024,
	/* .MaxComputeTextureImageUnits = */ 32,
	/* .MaxComputeImageUniforms = */ 16,
	/* .MaxComputeAtomicCounters = */ 4096,
	/* .MaxComputeAtomicCounterBuffers = */ 8,
	/* .MaxVaryingComponents = */ 128,
	/* .MaxVertexOutputComponents = */ 128,
	/* .MaxGeometryInputComponents = */ 128,
	/* .MaxGeometryOutputComponents = */ 128,
	/* .MaxFragmentInputComponents = */ 128,
	/* .MaxImageUnits = */ 192,
	/* .MaxCombinedImageUnitsAndFragmentOutputs = */ 144,
	/* .MaxCombinedShaderOutputResources = */ 144,
	/* .MaxImageSamples = */ 32,
	/* .MaxVertexImageUniforms = */ 16,
	/* .MaxTessControlImageUniforms = */ 16,
	/* .MaxTessEvaluationImageUniforms = */ 16,
	/* .MaxGeometryImageUniforms = */ 16,
	/* .MaxFragmentImageUniforms = */ 16,
	/* .MaxCombinedImageUniforms = */ 80,
	/* .MaxGeometryTextureImageUnits = */ 16,
	/* .MaxGeometryOutputVertices = */ 256,
	/* .MaxGeometryTotalOutputComponents = */ 1024,
	/* .MaxGeometryUniformComponents = */ 1024,
	/* .MaxGeometryVaryingComponents = */ 64,
	/* .MaxTessControlInputComponents = */ 128,
	/* .MaxTessControlOutputComponents = */ 128,
	/* .MaxTessControlTextureImageUnits = */ 16,
	/* .MaxTessControlUniformComponents = */ 1024,
	/* .MaxTessControlTotalOutputComponents = */ 4096,
	/* .MaxTessEvaluationInputComponents = */ 128,
	/* .MaxTessEvaluationOutputComponents = */ 128,
	/* .MaxTessEvaluationTextureImageUnits = */ 16,
	/* .MaxTessEvaluationUniformComponents = */ 1024,
	/* .MaxTessPatchComponents = */ 120,
	/* .MaxPatchVertices = */ 32,
	/* .MaxTessGenLevel = */ 64,
	/* .MaxViewports = */ 16,
	/* .MaxVertexAtomicCounters = */ 4096,
	/* .MaxTessControlAtomicCounters = */ 4096,
	/* .MaxTessEvaluationAtomicCounters = */ 4096,
	/* .MaxGeometryAtomicCounters = */ 4096,
	/* .MaxFragmentAtomicCounters = */ 4096,
	/* .MaxCombinedAtomicCounters = */ 4096,
	/* .MaxAtomicCounterBindings = */ 8,
	/* .MaxVertexAtomicCounterBuffers = */ 8,
	/* .MaxTessControlAtomicCounterBuffers = */ 8,
	/* .MaxTessEvaluationAtomicCounterBuffers = */ 8,
	/* .MaxGeometryAtomicCounterBuffers = */ 8,
	/* .MaxFragmentAtomicCounterBuffers = */ 8,
	/* .MaxCombinedAtomicCounterBuffers = */ 8,
	/* .MaxAtomicCounterBufferSize = */ 16384,
	/* .MaxTransformFeedbackBuffers = */ 4,
	/* .MaxTransformFeedbackInterleavedComponents = */ 64,
	/* .MaxCullDistances = */ 8,
	/* .MaxCombinedClipAndCullDistances = */ 8,
	/* .MaxSamples = */ 32,
	/* .maxMeshOutputVerticesNV = */ 256,
	/* .maxMeshOutputPrimitivesNV = */ 512,
	/* .maxMeshWorkGroupSizeX_NV = */ 32,
	/* .maxMeshWorkGroupSizeY_NV = */ 1,
	/* .maxMeshWorkGroupSizeZ_NV = */ 1,
	/* .maxTaskWorkGroupSizeX_NV = */ 32,
	/* .maxTaskWorkGroupSizeY_NV = */ 1,
	/* .maxTaskWorkGroupSizeZ_NV = */ 1,
	/* .maxMeshViewCountNV = */ 4,
	/* .maxDualSourceDrawBuffersEXT = */ 1,
	/* .limits = */ {
		/* .nonInductiveForLoops = */ 1,
		/* .whileLoops = */ 1,
		/* .doWhileLoops = */ 1,
		/* .generalUniformIndexing = */ 1,
		/* .generalAttributeMatrixVectorIndexing = */ 1,
		/* .generalVaryingIndexing = */ 1,
		/* .generalSamplerIndexing = */ 1,
		/* .generalVariableIndexing = */ 1,
		/* .generalConstantMatrixVectorIndexing = */ 1,
	}
};

static const uint32_t STREAMBUFFER_DEFAULT_SIZE = 16;
static const uint32_t DESCRIPTOR_POOL_SIZE = 1;

class BindingMapper
{
public:
	uint32_t operator()(spirv_cross::CompilerGLSL &comp, std::vector<uint32_t> &spirv, const std::string &name, const spirv_cross::ID &id)
	{
		auto it = bindingMappings.find(name);
		if (it == bindingMappings.end())
		{
			auto binding = comp.get_decoration(id, spv::DecorationBinding);

			if (isFreeBinding(binding))
			{
				bindingMappings[name] = binding;
				return binding;
			}
			else
			{
				uint32_t freeBinding = getFreeBinding();

				uint32_t binaryBindingOffset;
				if (!comp.get_binary_offset_for_decoration(id, spv::DecorationBinding, binaryBindingOffset))
					throw love::Exception("could not get binary offset for binding");

				spirv[binaryBindingOffset] = freeBinding;

				bindingMappings[name] = freeBinding;

				return freeBinding;
			}
		}
		else
			return it->second;
	};


private:
	uint32_t getFreeBinding() {
		for (uint32_t i = 0;; i++)
		{
			bool free = true;
			for (const auto &entry : bindingMappings)
			{
				if (entry.second == i)
				{
					free = false;
					break;
				}
			}
			if (free)
				return i;
		}
	}

	bool isFreeBinding(uint32_t binding) {
		for (const auto &entry : bindingMappings)
		{
			if (entry.second == binding)
				return false;
		}
		return true;
	}

	std::map<std::string, uint32_t> bindingMappings;

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

Shader::Shader(StrongRef<love::graphics::ShaderStage> stages[])
	: graphics::Shader(stages)
{
	auto gfx = Module::getInstance<Graphics>(Module::ModuleType::M_GRAPHICS);
	vgfx = dynamic_cast<Graphics*>(gfx);

	loadVolatile();
}

bool Shader::loadVolatile()
{
	computePipeline = VK_NULL_HANDLE;

	for (int i = 0; i < BUILTIN_MAX_ENUM; i++)
		builtinUniformInfo[i] = nullptr;

	compileShaders();
	calculateUniformBufferSizeAligned();
	createDescriptorSetLayout();
	createPipelineLayout();
	createDescriptorPoolSizes();
	createStreamBuffers();
	descriptorSetsVector.resize(MAX_FRAMES_IN_FLIGHT);
	currentFrame = 0;
	currentUsedUniformStreamBuffersCount = 0;
	currentUsedDescriptorSetsCount = 0;
	newFrame();

	return true;
}

void Shader::unloadVolatile()
{
	if (shaderModules.empty())
		return;

	for (const auto &uniform : uniformInfos)
	{
		switch (uniform.second.baseType)
		{
		case UNIFORM_SAMPLER:
		case UNIFORM_STORAGETEXTURE:
			for (int i = 0; i < uniform.second.count; i++)
			{
				if (uniform.second.textures[i] != nullptr)
					uniform.second.textures[i]->release();
			}
			delete[] uniform.second.textures;
			break;
		case UNIFORM_TEXELBUFFER:
		case UNIFORM_STORAGEBUFFER:
			for (int i = 0; i < uniform.second.count; i++)
			{
				if (uniform.second.buffers[i] != nullptr)
					uniform.second.buffers[i]->release();
			}
			delete[] uniform.second.buffers;
			break;
		}
	}

	vgfx->queueCleanUp([shaderModules = std::move(shaderModules), device = device, descriptorSetLayout = descriptorSetLayout, pipelineLayout = pipelineLayout, descriptorPools = descriptorPools, computePipeline = computePipeline](){
		for (const auto pool : descriptorPools)
			vkDestroyDescriptorPool(device, pool, nullptr);
		for (const auto shaderModule : shaderModules)
			vkDestroyShaderModule(device, shaderModule, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		if (computePipeline != VK_NULL_HANDLE)
			vkDestroyPipeline(device, computePipeline, nullptr);
	});

	while (!freeDescriptorSets.empty())
		freeDescriptorSets.pop();

	for (const auto streamBuffer : streamBuffers)
		streamBuffer->release();

	shaderModules.clear();
	shaderStages.clear();
	streamBuffers.clear();
	descriptorPools.clear();
	descriptorSetsVector.clear();
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

	updatedUniforms.clear();
	currentUsedUniformStreamBuffersCount = 0;
	currentUsedDescriptorSetsCount = 0;

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
	else
		streamBuffers.at(0)->nextFrame();

	if (descriptorSetsVector.at(currentFrame).size() == 0)
		descriptorSetsVector.at(currentFrame).push_back(allocateDescriptorSet());

	currentDescriptorSet = descriptorSetsVector.at(currentFrame).at(0);
}

void Shader::cmdPushDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint)
{
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

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = (VkBuffer)currentStreamBuffer->getHandle();
		bufferInfo.offset = offset;
		bufferInfo.range = localUniformData.size();

		VkWriteDescriptorSet uniformWrite{};
		uniformWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		uniformWrite.dstSet = currentDescriptorSet;
		uniformWrite.dstBinding = localUniformLocation;
		uniformWrite.dstArrayElement = 0;
		uniformWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniformWrite.descriptorCount = 1;
		uniformWrite.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(device, 1, &uniformWrite, 0, nullptr);

		currentUsedUniformStreamBuffersCount++;

		updatedUniforms.insert(localUniformLocation);
	}

	static const std::vector<BuiltinUniform> builtinUniformTextures = {
		BUILTIN_TEXTURE_MAIN,
		BUILTIN_TEXTURE_VIDEO_Y,
		BUILTIN_TEXTURE_VIDEO_CB,
		BUILTIN_TEXTURE_VIDEO_CR,
	};

	for (const auto &builtin : builtinUniformTextures)
	{
		if (builtinUniformInfo[builtin] != nullptr)
		{
			auto texture = dynamic_cast<Texture*>(builtinUniformInfo[builtin]->textures[0]);

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = texture->getImageLayout();
			imageInfo.imageView = (VkImageView)texture->getRenderTargetHandle();
			imageInfo.sampler = (VkSampler)texture->getSamplerHandle();

			auto location = builtinUniformInfo[builtin]->location;

			VkWriteDescriptorSet textureWrite{};
			textureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			textureWrite.dstSet = currentDescriptorSet;
			textureWrite.dstBinding = location;
			textureWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			textureWrite.descriptorCount = 1;
			textureWrite.pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(device, 1, &textureWrite, 0, nullptr);

			updatedUniforms.insert(location);
		}
	}

	for (const auto &u : uniformInfos)
	{
		if (updatedUniforms.find(u.second.location) == updatedUniforms.end())
			updateUniform(&u.second, u.second.count, true);
	}

	vkCmdBindDescriptorSets(commandBuffer, bindPoint, pipelineLayout, 0, 1, &currentDescriptorSet, 0, nullptr);

	currentUsedDescriptorSetsCount++;

	if (currentUsedDescriptorSetsCount >= static_cast<uint32_t>(descriptorSetsVector.at(currentFrame).size()))
		descriptorSetsVector.at(currentFrame).push_back(allocateDescriptorSet());

	currentDescriptorSet = descriptorSetsVector.at(currentFrame).at(currentUsedDescriptorSetsCount);

	updatedUniforms.clear();
}

Shader::~Shader()
{
	unloadVolatile();
}

void Shader::attach()
{
	vgfx->markShaderUsed(this);

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

const Shader::UniformInfo *Shader::getUniformInfo(const std::string &name) const
{
	return &uniformInfos.at(name);
}

const Shader::UniformInfo *Shader::getUniformInfo(BuiltinUniform builtin) const
{
	return builtinUniformInfo[builtin];
}

static bool usesLocalUniformData(const graphics::Shader::UniformInfo *info)
{
	return info->baseType == graphics::Shader::UNIFORM_BOOL ||
		info->baseType == graphics::Shader::UNIFORM_FLOAT ||
		info->baseType == graphics::Shader::UNIFORM_INT ||
		info->baseType == graphics::Shader::UNIFORM_MATRIX ||
		info->baseType == graphics::Shader::UNIFORM_UINT;
}

void Shader::updateUniform(const UniformInfo *info, int count)
{
	updateUniform(info, count, false);
}

void Shader::updateUniform(const UniformInfo* info, int count, bool internal)
{
	if (!internal && current == this)
		Graphics::flushBatchedDrawsGlobal();

	if (usesLocalUniformData(info))
		memcpy(localUniformData.data(), localUniformStagingData.data(), localUniformStagingData.size());
	if (info->baseType == UNIFORM_SAMPLER || info->baseType == UNIFORM_STORAGETEXTURE)
	{
		bool isSampler = info->baseType == UNIFORM_SAMPLER;

		std::vector<VkDescriptorImageInfo> imageInfos;

		for (int i = 0; i < count; i++)
		{
			auto vkTexture = dynamic_cast<Texture*>(info->textures[i]);

			if (vkTexture == nullptr)
				throw love::Exception("uniform variable %s is not set.", info->name.c_str());

			VkDescriptorImageInfo imageInfo{};

			imageInfo.imageLayout = vkTexture->getImageLayout();
			imageInfo.imageView = (VkImageView)vkTexture->getRenderTargetHandle();
			if (isSampler)
				imageInfo.sampler = (VkSampler)vkTexture->getSamplerHandle();

			imageInfos.push_back(imageInfo);
		}

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = currentDescriptorSet;
		write.dstBinding = info->location;
		write.dstArrayElement = 0;
		if (isSampler)
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		else
			write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		write.descriptorCount = static_cast<uint32_t>(count);
		write.pImageInfo = imageInfos.data();

		vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
	}
	if (info->baseType == UNIFORM_STORAGEBUFFER)
	{
		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = currentDescriptorSet;
		write.dstBinding = info->location;
		write.dstArrayElement = 0;
		write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		write.descriptorCount = info->count;

		std::vector<VkDescriptorBufferInfo> bufferInfos;

		for (int i = 0; i < info->count; i++)
		{
			if (info->buffers[i] == nullptr)
				throw love::Exception("uniform variable %s is not set.", info->name.c_str());

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = (VkBuffer)info->buffers[i]->getHandle();;
			bufferInfo.offset = 0;
			bufferInfo.range = info->buffers[i]->getSize();

			bufferInfos.push_back(bufferInfo);
		}

		write.pBufferInfo = bufferInfos.data();

		vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
	}
	if (info->baseType == UNIFORM_TEXELBUFFER)
	{
		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = currentDescriptorSet;
		write.dstBinding = info->location;
		write.dstArrayElement = 0;
		write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		write.descriptorCount = info->count;

		std::vector<VkBufferView> bufferViews;

		for (int i = 0; i < info->count; i++)
		{
			if (info->buffers[i] == nullptr)
				throw love::Exception("uniform variable %s is not set.", info->name.c_str());

			bufferViews.push_back((VkBufferView)info->buffers[i]->getTexelBufferHandle());
		}

		write.pTexelBufferView = bufferViews.data();

		vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
	}

	updatedUniforms.insert(info->location);
}

void Shader::sendTextures(const UniformInfo *info, graphics::Texture **textures, int count)
{
	for (int i = 0; i < count; i++)
	{
		auto oldTexture = info->textures[i];
		info->textures[i] = textures[i];
		info->textures[i]->retain();
		if (oldTexture)
			oldTexture->release();
	}

	updateUniform(info, count);
}

void Shader::sendBuffers(const UniformInfo *info, love::graphics::Buffer **buffers, int count)
{
	for (int i = 0; i < count; i++)
	{
		auto oldBuffer = info->buffers[i];
		info->buffers[i] = buffers[i];
		info->buffers[i]->retain();
		if (oldBuffer)
			oldBuffer->release();
	}

	updateUniform(info, count);
}

void Shader::calculateUniformBufferSizeAligned()
{
	auto minAlignment = vgfx->getMinUniformBufferOffsetAlignment();
	size_t size = localUniformStagingData.size();
	auto factor = static_cast<VkDeviceSize>(std::ceil(
		static_cast<float>(size) / static_cast<float>(minAlignment)
	));
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

		UniformInfo u{};
		u.name = name;
		u.dataSize = memberSize;
		u.count = memberType.array.empty() ? 1 : memberType.array[0];
		u.components = 1;
		u.data = localUniformStagingData.data() + offset;

		if (memberType.columns == 1)
		{
			if (memberType.basetype == SPIRType::Int)
				u.baseType = UNIFORM_INT;
			else if (memberType.basetype == SPIRType::UInt)
				u.baseType = UNIFORM_UINT;
			else
				u.baseType = UNIFORM_FLOAT;
			u.components = memberType.vecsize;
		}
		else
		{
			u.baseType = UNIFORM_MATRIX;
			u.matrix.rows = memberType.vecsize;
			u.matrix.columns = memberType.columns;
		}

		const auto &reflectionIt = validationReflection.localUniforms.find(u.name);
		if (reflectionIt != validationReflection.localUniforms.end())
		{
			const auto &localUniform = reflectionIt->second;
			const auto &values = localUniform.initializerValues;
			if (!values.empty())
				memcpy(
					u.data,
					values.data(),
					std::min(u.dataSize, values.size() * sizeof(LocalUniformValue)));
		}

		uniformInfos[u.name] = u;

		BuiltinUniform builtin = BUILTIN_MAX_ENUM;
		if (getConstant(u.name.c_str(), builtin))
		{
			if (builtin == BUILTIN_UNIFORMS_PER_DRAW)
				builtinUniformDataOffset = offset;
			builtinUniformInfo[builtin] = &uniformInfos[u.name];
		}
	}
}

void Shader::compileShaders()
{
	using namespace glslang;
	using namespace spirv_cross;

	std::vector<TShader*> glslangShaders;

	auto program = new TProgram();

	device = vgfx->getDevice();

	const auto &enabledExtensions = vgfx->getEnabledOptionalDeviceExtensions();

	for (int i = 0; i < SHADERSTAGE_MAX_ENUM; i++)
	{
		if (!stages[i])
			continue;

		auto stage = (ShaderStageType)i;

		if (stage == SHADERSTAGE_COMPUTE)
			isCompute = true;

		auto glslangShaderStage = getGlslShaderType(stage);
		auto tshader = new TShader(glslangShaderStage);

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

		if (!tshader->parse(&defaultTBuiltInResource, defaultVersion, defaultProfile, forceDefault, forwardCompat, EShMsgSuppressWarnings))
		{
			const char *msg1 = tshader->getInfoLog();
			const char *msg2 = tshader->getInfoDebugLog();

			throw love::Exception("error while parsing shader");
		}

		program->addShader(tshader);
		glslangShaders.push_back(tshader);
	}

	if (!program->link(EShMsgDefault))
		throw love::Exception("link failed! %s\n", program->getInfoLog());

	if (!program->mapIO())
		throw love::Exception("mapIO failed");

	uniformInfos.clear();

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
				const auto& type = comp.get_type(resource.base_type_id);
				size_t uniformBufferObjectSize = comp.get_declared_struct_size(type);
				auto defaultUniformBlockSize = comp.get_declared_struct_size(type);
				localUniformStagingData.resize(defaultUniformBlockSize);
				localUniformData.resize(defaultUniformBlockSize);
				localUniformLocation = bindingMapper(comp, spirv, resource.name, resource.id);

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
			const SPIRType &basetype = comp.get_type(r.base_type_id);
			const SPIRType &type = comp.get_type(r.type_id);
			const SPIRType &imagetype = comp.get_type(basetype.image.type);

			graphics::Shader::UniformInfo info;
			info.location = bindingMapper(comp, spirv, r.name, r.id);
			info.baseType = UNIFORM_SAMPLER;
			info.name = r.name;
			info.count = type.array.empty() ? 1 : type.array[0];
			info.isDepthSampler = type.image.depth;
			info.components = 1;

			switch (imagetype.basetype)
			{
			case SPIRType::Float:
				info.dataBaseType = DATA_BASETYPE_FLOAT;
				break;
			case SPIRType::Int:
				info.dataBaseType = DATA_BASETYPE_INT;
				break;
			case SPIRType::UInt:
				info.dataBaseType = DATA_BASETYPE_UINT;
				break;
			default:
				break;
			}

			switch (basetype.image.dim)
			{
			case spv::Dim2D:
				info.textureType = basetype.image.arrayed ? TEXTURE_2D_ARRAY : TEXTURE_2D;
				info.textures = new love::graphics::Texture *[info.count];
				break;
			case spv::Dim3D:
				info.textureType = TEXTURE_VOLUME;
				info.textures = new love::graphics::Texture *[info.count];
				break;
			case spv::DimCube:
				if (basetype.image.arrayed) {
					throw love::Exception("cubemap arrays are not currently supported");
				}
				info.textureType = TEXTURE_CUBE;
				info.textures = new love::graphics::Texture *[info.count];
				break;
			case spv::DimBuffer:
				info.baseType = UNIFORM_TEXELBUFFER;
				info.buffers = new love::graphics::Buffer *[info.count];
				break;
			default:
				throw love::Exception("unknown dim");
			}

			if (info.baseType == UNIFORM_TEXELBUFFER)
			{
				for (int i = 0; i < info.count; i++)
					info.buffers[i] = nullptr;
			}
			else
			{
				auto tex = vgfx->getDefaultTexture();
				for (int i = 0; i < info.count; i++)
				{
					info.textures[i] = tex;
					tex->retain();
				}
			}

			uniformInfos[r.name] = info;
			BuiltinUniform builtin;
			if (getConstant(r.name.c_str(), builtin))
				builtinUniformInfo[builtin] = &uniformInfos[info.name];
		}

		for (const auto &r : shaderResources.storage_buffers)
		{
			const auto &type = comp.get_type(r.type_id);

			UniformInfo u{};
			u.baseType = UNIFORM_STORAGEBUFFER;
			u.components = 1;
			u.name = r.name;
			u.count = type.array.empty() ? 1 : type.array[0];

			if (!fillUniformReflectionData(u))
				continue;

			u.location = bindingMapper(comp, spirv, r.name, r.id);
			u.buffers = new love::graphics::Buffer *[u.count];

			for (int i = 0; i < u.count; i++)
				u.buffers[i] = nullptr;

			uniformInfos[u.name] = u;
		}

		for (const auto &r : shaderResources.storage_images)
		{
			const auto &type = comp.get_type(r.type_id);

			UniformInfo u{};
			u.baseType = UNIFORM_STORAGETEXTURE;
			u.components = 1;
			u.name = r.name;
			u.count = type.array.empty() ? 1 : type.array[0];

			if (!fillUniformReflectionData(u))
				continue;

			u.textures = new love::graphics::Texture *[u.count];
			u.location = bindingMapper(comp, spirv, r.name, r.id);

			for (int i = 0; i < u.count; i++)
				u.textures[i] = nullptr;

			uniformInfos[u.name] = u;
		}

		if (shaderStage == SHADERSTAGE_VERTEX)
		{
			for (const auto &r : shaderResources.stage_inputs)
			{
				const auto &name = r.name;
				const int attributeLocation = static_cast<int>(comp.get_decoration(r.id, spv::DecorationLocation));
				attributes[name] = attributeLocation;
			}
		}

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = spirv.size() * sizeof(uint32_t);
		createInfo.pCode = spirv.data();

		auto device = vgfx->getDevice();

		VkShaderModule shaderModule;

		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
			throw love::Exception("failed to create shader module");

		shaderModules.push_back(shaderModule);

		VkPipelineShaderStageCreateInfo shaderStageInfo{};
		shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo.stage = getStageBit((ShaderStageType)i);
		shaderStageInfo.module = shaderModule;
		shaderStageInfo.pName = "main";

		shaderStages.push_back(shaderStageInfo);
	}

	delete program;
	for (auto shader : glslangShaders)
		delete shader;
}

void Shader::createDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	VkShaderStageFlags stageFlags;
	if (isCompute)
		stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	else
		stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	for (auto const &entry : uniformInfos)
	{
		auto type = Vulkan::getDescriptorType(entry.second.baseType);
		if (type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		{
			VkDescriptorSetLayoutBinding layoutBinding{};

			layoutBinding.binding = entry.second.location;
			layoutBinding.descriptorType = type;
			layoutBinding.descriptorCount = entry.second.count;
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

	for (const auto &entry : uniformInfos)
	{
		VkDescriptorPoolSize size{};
		auto type = Vulkan::getDescriptorType(entry.second.baseType);
		if (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
			continue;
		}
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
		if (builtinUniformInfo[builtIns[i]] != nullptr)
		{
			textures[i]->retain();
			builtinUniformInfo[builtIns[i]]->textures[0]->release();
			builtinUniformInfo[builtIns[i]]->textures[0] = textures[i];
		}
	}
}

bool Shader::hasUniform(const std::string &name) const
{
	return uniformInfos.find(name) != uniformInfos.end();
}

void Shader::setMainTex(graphics::Texture *texture)
{
	if (builtinUniformInfo[BUILTIN_TEXTURE_MAIN] != nullptr)
	{
		texture->retain();
		builtinUniformInfo[BUILTIN_TEXTURE_MAIN]->textures[0]->release();
		builtinUniformInfo[BUILTIN_TEXTURE_MAIN]->textures[0] = texture;
	}
}

VkDescriptorSet Shader::allocateDescriptorSet()
{
	if (freeDescriptorSets.empty())
	{
		VkDescriptorPoolCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		createInfo.maxSets = DESCRIPTOR_POOL_SIZE;
		createInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
		createInfo.pPoolSizes = descriptorPoolSizes.data();

		VkDescriptorPool pool;
		if (vkCreateDescriptorPool(device, &createInfo, nullptr, &pool) != VK_SUCCESS)
			throw love::Exception("failed to create descriptor pool");
		descriptorPools.push_back(pool);

		std::vector<VkDescriptorSetLayout> layouts(DESCRIPTOR_POOL_SIZE, descriptorSetLayout);

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = pool;
		allocInfo.descriptorSetCount = DESCRIPTOR_POOL_SIZE;
		allocInfo.pSetLayouts = layouts.data();

		std::vector<VkDescriptorSet> descriptorSet;
		descriptorSet.resize(DESCRIPTOR_POOL_SIZE);
		VkResult result = vkAllocateDescriptorSets(device, &allocInfo, descriptorSet.data());
		if (result != VK_SUCCESS)
			throw love::Exception("failed to allocate descriptor set");

		for (const auto ds : descriptorSet)
			freeDescriptorSets.push(ds);
	}

	auto ds = freeDescriptorSets.front();
	freeDescriptorSets.pop();
	return ds;
}

} // vulkan
} // graphics
} // love
