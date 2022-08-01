#include "Shader.h"
#include "Graphics.h"

#include "libraries/glslang/glslang/Public/ShaderLang.h"
#include "libraries/glslang/SPIRV/GlslangToSpv.h"
#include "libraries/spirv_cross/spirv_cross.hpp"

#include <vector>

namespace love {
namespace graphics {
namespace vulkan {
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

static EShLanguage getGlslShaderType(ShaderStageType stage) {
	switch (stage) {
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
	: graphics::Shader(stages) {
	loadVolatile();
}

bool Shader::loadVolatile() {
	calculateUniformBufferSizeAligned();
	compileShaders();
	createDescriptorSetLayout();
	createPipelineLayout();
	createStreamBuffers();
	currentImage = 0;
	count = 0;

	return true;
}

void Shader::unloadVolatile() {
	if (shaderModules.size() == 0) {
		return;
	}

	auto gfx = Module::getInstance<Graphics>(Module::M_GRAPHICS);
	gfx->queueCleanUp([shaderModules = std::move(shaderModules), device = device, descriptorSetLayout = descriptorSetLayout, pipelineLayout = pipelineLayout](){
		for (const auto shaderModule : shaderModules) {
			vkDestroyShaderModule(device, shaderModule, nullptr);
		}
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	});
	for (const auto streamBufferVector : streamBuffers) {
		for (const auto streamBuffer : streamBufferVector) {
			delete streamBuffer;
		}
	}
	shaderModules.clear();
	shaderStages.clear();
	streamBuffers.clear();
}

const std::vector<VkPipelineShaderStageCreateInfo>& Shader::getShaderStages() const {
	return shaderStages;
}

const VkPipelineLayout Shader::getGraphicsPipelineLayout() const {
	return pipelineLayout;
}

static VkDescriptorImageInfo createDescriptorImageInfo(graphics::Texture* texture) {
	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	Texture* vkTexture = (Texture*)texture;
	imageInfo.imageView = (VkImageView)vkTexture->getRenderTargetHandle();
	imageInfo.sampler = (VkSampler)vkTexture->getSamplerHandle();
	return imageInfo;
}

void Shader::cmdPushDescriptorSets(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
	// detect wether a new frame has begun
	if (currentImage != imageIndex) {
		currentImage = imageIndex;
		count = 0;

		// we needed more memory last frame, let's collapse all buffers into a single one.
		if (streamBuffers.at(currentImage).size() > 1) {
			size_t newSize = 0;
			for (auto streamBuffer : streamBuffers.at(currentImage)) {
				newSize += streamBuffer->getSize();
				delete streamBuffer;
			}
			streamBuffers.at(currentImage).clear();
			streamBuffers.at(currentImage).push_back(new StreamBuffer(gfx, BUFFERUSAGE_UNIFORM, newSize));
		} 
		// no collapse necessary, can just call nextFrame to reset the current (only) streambuffer
		else {
			streamBuffers.at(currentImage).at(0)->nextFrame();
		}
	}
	// still the same frame
	else {
		auto usedStreamBufferMemory = count * uniformBufferSizeAligned;
		if (usedStreamBufferMemory >= streamBuffers.at(currentImage).back()->getSize()) {
			// we ran out of memory in the current frame, need to allocate more.
			streamBuffers.at(currentImage).push_back(new StreamBuffer(gfx, BUFFERUSAGE_UNIFORM, STREAMBUFFER_DEFAULT_SIZE * uniformBufferSizeAligned));
			count = 0;
		}
	}

	// additional data is always added onto the last stream buffer in the current frame
	auto currentStreamBuffer = streamBuffers.at(currentImage).back();

	auto mapInfo = currentStreamBuffer->map(uniformBufferSizeAligned);
	memcpy(mapInfo.data, &uniformData, uniformBufferSizeAligned);
	currentStreamBuffer->unmap(uniformBufferSizeAligned);
	currentStreamBuffer->markUsed(uniformBufferSizeAligned);

	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = (VkBuffer)currentStreamBuffer->getHandle();
	bufferInfo.offset = count * uniformBufferSizeAligned;
	bufferInfo.range = sizeof(BuiltinUniformData);
	
	auto mainTexImageInfo = createDescriptorImageInfo(mainTex);
	auto ytextureImageInfo = createDescriptorImageInfo(ytexture);
	auto cbtextureImageInfo = createDescriptorImageInfo(cbtexture);
	auto crtextureImageInfo = createDescriptorImageInfo(crtexture);

	std::array<VkWriteDescriptorSet, 5> descriptorWrite{};
	descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[0].dstSet = 0;
	descriptorWrite[0].dstBinding = 0;
	descriptorWrite[0].dstArrayElement = 0;
	descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite[0].descriptorCount = 1;
	descriptorWrite[0].pBufferInfo = &bufferInfo;

	descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[1].dstSet = 0;
	descriptorWrite[1].dstBinding = 1;
	descriptorWrite[1].dstArrayElement = 0;
	descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite[1].descriptorCount = 1;
	descriptorWrite[1].pImageInfo = &mainTexImageInfo;				

	descriptorWrite[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[2].dstSet = 0;
	descriptorWrite[2].dstBinding = 2;
	descriptorWrite[2].dstArrayElement = 0;
	descriptorWrite[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite[2].descriptorCount = 1;
	descriptorWrite[2].pImageInfo = &ytextureImageInfo;

	descriptorWrite[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[3].dstSet = 0;
	descriptorWrite[3].dstBinding = 3;
	descriptorWrite[3].dstArrayElement = 0;
	descriptorWrite[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite[3].descriptorCount = 1;
	descriptorWrite[3].pImageInfo = &cbtextureImageInfo;

	descriptorWrite[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[4].dstSet = 0;
	descriptorWrite[4].dstBinding = 4;
	descriptorWrite[4].dstArrayElement = 0;
	descriptorWrite[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite[4].descriptorCount = 1;
	descriptorWrite[4].pImageInfo = &crtextureImageInfo;

	vkCmdPushDescriptorSet(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, static_cast<uint32_t>(descriptorWrite.size()), descriptorWrite.data());

	count++;
}

Shader::~Shader() {
	unloadVolatile();
}

void Shader::attach() {
	if (Shader::current != this) {
		Graphics::flushBatchedDrawsGlobal();
		Shader::current = this;
		Vulkan::shaderSwitch();
	}
}

int Shader::getVertexAttributeIndex(const std::string& name) {
	return vertexAttributeIndices.at(name);
}

void Shader::calculateUniformBufferSizeAligned() {
	gfx = Module::getInstance<Graphics>(Module::ModuleType::M_GRAPHICS);
	auto vgfx = (Graphics*)gfx;
	auto minAlignment = vgfx->getMinUniformBufferOffsetAlignment();
	uniformBufferSizeAligned = 
		static_cast<VkDeviceSize>(
			std::ceil(
				static_cast<float>(sizeof(BuiltinUniformData)) / static_cast<float>(minAlignment)
			)
		)
		* minAlignment;
}

void Shader::compileShaders() {
	using namespace glslang;
	using namespace spirv_cross;

	TProgram* program = new TProgram();

	gfx = Module::getInstance<Graphics>(Module::ModuleType::M_GRAPHICS);
	auto vgfx = (Graphics*)gfx;
	device = vgfx->getDevice();

	mainTex = vgfx->getDefaultTexture();
	ytexture = vgfx->getDefaultTexture();
	crtexture = vgfx->getDefaultTexture();
	cbtexture = vgfx->getDefaultTexture();

	for (int i = 0; i < SHADERSTAGE_MAX_ENUM; i++) {
		if (!stages[i])
			continue;

		auto stage = (ShaderStageType)i;
		auto glslangShaderStage = getGlslShaderType(stage);
		auto tshader = new TShader(glslangShaderStage);

		tshader->setEnvInput(EShSourceGlsl, glslangShaderStage, EShClientVulkan, 450);
		tshader->setEnvClient(EShClientVulkan, EShTargetVulkan_1_2);
		tshader->setEnvTarget(EshTargetSpv, EShTargetSpv_1_5);
		tshader->setAutoMapLocations(true);
		tshader->setAutoMapBindings(true);
		tshader->setEnvInputVulkanRulesRelaxed();
		tshader->setGlobalUniformBinding(0);
		tshader->setGlobalUniformSet(0);

		auto& glsl = stages[i]->getSource();
		const char* csrc = glsl.c_str();
		const int sourceLength = static_cast<int>(glsl.length());
		tshader->setStringsWithLengths(&csrc, &sourceLength, 1);

		int defaultVersio = 450;
		EProfile defaultProfile = ECoreProfile;
		bool forceDefault = false;
		bool forwardCompat = true;

		if (!tshader->parse(&defaultTBuiltInResource, defaultVersio, defaultProfile, forceDefault, forwardCompat, EShMsgSuppressWarnings)) {
			const char* msg1 = tshader->getInfoLog();
			const char* msg2 = tshader->getInfoDebugLog();

			throw love::Exception("error while parsing shader");
		}

		program->addShader(tshader);
	}

	if (!program->link(EShMsgDefault)) {
		throw love::Exception("link failed! %s\n", program->getInfoLog());
	}

	if (!program->mapIO()) {
		throw love::Exception("mapIO failed");
	}

	for (int i = 0; i < SHADERSTAGE_MAX_ENUM; i++) {
		auto glslangStage = getGlslShaderType((ShaderStageType)i);
		auto intermediate = program->getIntermediate(glslangStage);
		if (intermediate == nullptr) {
			continue;
		}

		spv::SpvBuildLogger logger;
		glslang::SpvOptions opt;
		opt.validate = true;

		std::vector<uint32_t> spirv;
		GlslangToSpv(*intermediate, spirv, &logger, &opt);

		std::string msgs = logger.getAllMessages();

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = spirv.size() * sizeof(uint32_t);
		createInfo.pCode = spirv.data();

		Graphics* vkGfx = (Graphics*)gfx;
		auto device = vkGfx->getDevice();

		VkShaderModule shaderModule;

		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw love::Exception("failed to create shader module");
		}

		shaderModules.push_back(shaderModule);

		VkPipelineShaderStageCreateInfo shaderStageInfo{};
		shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo.stage = getStageBit((ShaderStageType)i);
		shaderStageInfo.module = shaderModule;
		shaderStageInfo.pName = "main";

		shaderStages.push_back(shaderStageInfo);
	}
}

// fixme: should generate this dynamically.
void Shader::createDescriptorSetLayout() {
	auto vgfx = (Graphics*)gfx;
	vkCmdPushDescriptorSet = vgfx->getVkCmdPushDescriptorSetKHRFunctionPointer();

	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding videoYBinding{};
	videoYBinding.binding = 2;
	videoYBinding.descriptorCount = 1;
	videoYBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	videoYBinding.pImmutableSamplers = nullptr;
	videoYBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding videoCBBinding{};
	videoCBBinding.binding = 3;
	videoCBBinding.descriptorCount = 1;
	videoCBBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	videoCBBinding.pImmutableSamplers = nullptr;
	videoCBBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding videoCRinding{};
	videoCRinding.binding = 4;
	videoCRinding.descriptorCount = 1;
	videoCRinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	videoCRinding.pImmutableSamplers = nullptr;
	videoCRinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 5> bindings = { uboLayoutBinding, samplerLayoutBinding, videoYBinding, videoCBBinding, videoCRinding };
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		throw love::Exception("failed to create descriptor set layout");
	}
}

void Shader::createPipelineLayout() {
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw love::Exception("failed to create pipeline layout");
	}
}

void Shader::createStreamBuffers() {
	auto vgfx = (Graphics*)gfx;
	const auto numImagesInFlight = vgfx->getNumImagesInFlight();
	streamBuffers.resize(numImagesInFlight);
	for (uint32_t i = 0; i < numImagesInFlight; i++) {
		streamBuffers[i].push_back(new StreamBuffer(gfx, BUFFERUSAGE_UNIFORM, STREAMBUFFER_DEFAULT_SIZE * uniformBufferSizeAligned));
	}
}

void Shader::setVideoTextures(graphics::Texture* ytexture, graphics::Texture* cbtexture, graphics::Texture* crtexture) {
	this->ytexture = ytexture;
	this->cbtexture = cbtexture;
	this->crtexture = crtexture;
}

void Shader::setUniformData(BuiltinUniformData& data) {
	uniformData = data;
}

void Shader::setMainTex(graphics::Texture* texture) {
	mainTex = texture;
}
} // vulkan
} // graphics
} // love
