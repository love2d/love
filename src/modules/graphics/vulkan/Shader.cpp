#include "Shader.h"
#include "Graphics.h"

#include "libraries/glslang/glslang/Public/ShaderLang.h"
#include "libraries/glslang/SPIRV/GlslangToSpv.h"


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
static const uint32_t DESCRIPTOR_POOL_SIZE = 1;

static VkShaderStageFlagBits getStageBit(ShaderStageType type) {
	switch (type) {
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
	for (int i = 0; i < BUILTIN_MAX_ENUM; i++) {
		builtinUniformInfo[i] = nullptr;
	}

	compileShaders();
	calculateUniformBufferSizeAligned();
	createDescriptorSetLayout();
	createPipelineLayout();
	createStreamBuffers();
	descriptorSetsVector.resize(((Graphics*)gfx)->getNumImagesInFlight());
	currentFrame = 0;
	currentUsedUniformStreamBuffersCount = 0;
	currentUsedDescriptorSetsCount = 0;
	currentAllocatedDescriptorSets = DESCRIPTOR_POOL_SIZE;

	return true;
}

void Shader::unloadVolatile() {
	if (shaderModules.empty()) {
		return;
	}

	auto gfx = Module::getInstance<Graphics>(Module::M_GRAPHICS);
	gfx->queueCleanUp([shaderModules = std::move(shaderModules), device = device, descriptorSetLayout = descriptorSetLayout, pipelineLayout = pipelineLayout, descriptorPools = descriptorPools](){
		for (const auto pool : descriptorPools) {
			vkDestroyDescriptorPool(device, pool, nullptr);
		}
		for (const auto shaderModule : shaderModules) {
			vkDestroyShaderModule(device, shaderModule, nullptr);
		}
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	});
	for (const auto &streamBufferVector : streamBuffers) {
		for (const auto streamBuffer : streamBufferVector) {
			delete streamBuffer;
		}
	}
	shaderModules.clear();
	shaderStages.clear();
	streamBuffers.clear();
	descriptorPools.clear();
	descriptorSetsVector.clear();
}

const std::vector<VkPipelineShaderStageCreateInfo>& Shader::getShaderStages() const {
	return shaderStages;
}

const VkPipelineLayout Shader::getGraphicsPipelineLayout() const {
	return pipelineLayout;
}

static VkDescriptorImageInfo* createDescriptorImageInfo(graphics::Texture* texture) {
	Texture* vkTexture = (Texture*)texture;

	VkDescriptorImageInfo* imageInfo = new VkDescriptorImageInfo();

	imageInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo->imageView = (VkImageView)vkTexture->getRenderTargetHandle();
	imageInfo->sampler = (VkSampler)vkTexture->getSamplerHandle();

	return imageInfo;
}

void Shader::cmdPushDescriptorSets(VkCommandBuffer commandBuffer, uint32_t frameIndex) {
	// detect wether a new frame has begun
	if (currentFrame != frameIndex) {
		currentFrame = frameIndex;

		currentUsedUniformStreamBuffersCount = 0;
		currentUsedDescriptorSetsCount = 0;

		// we needed more memory last frame, let's collapse all buffers into a single one.
		if (streamBuffers.at(currentFrame).size() > 1) {
			size_t newSize = 0;
			for (auto streamBuffer : streamBuffers.at(currentFrame)) {
				newSize += streamBuffer->getSize();
				delete streamBuffer;
			}
			streamBuffers.at(currentFrame).clear();
			streamBuffers.at(currentFrame).push_back(new StreamBuffer(gfx, BUFFERUSAGE_UNIFORM, newSize));
		}
		// no collapse necessary, can just call nextFrame to reset the current (only) streambuffer
		else {
			streamBuffers.at(currentFrame).at(0)->nextFrame();
		}
	}
	// still the same frame
	else {
		auto usedStreamBufferMemory = currentUsedUniformStreamBuffersCount * uniformBufferSizeAligned;
		if (usedStreamBufferMemory >= streamBuffers.at(currentFrame).back()->getSize()) {
			// we ran out of memory in the current frame, need to allocate more.
			streamBuffers.at(currentFrame).push_back(new StreamBuffer(gfx, BUFFERUSAGE_UNIFORM, STREAMBUFFER_DEFAULT_SIZE * uniformBufferSizeAligned));
			currentUsedUniformStreamBuffersCount = 0;
		}
	}

	if (currentUsedDescriptorSetsCount >= static_cast<uint32_t>(descriptorSetsVector.at(currentFrame).size())) {
		descriptorSetsVector.at(currentFrame).push_back(allocateDescriptorSet());
	}

	// additional data is always added onto the last stream buffer in the current frame
	auto currentStreamBuffer = streamBuffers.at(currentFrame).back();

	auto mapInfo = currentStreamBuffer->map(uniformBufferSizeAligned);
	memcpy(mapInfo.data, localUniformStagingData.data(), uniformBufferSizeAligned);
	currentStreamBuffer->unmap(uniformBufferSizeAligned);
	currentStreamBuffer->markUsed(uniformBufferSizeAligned);

	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = (VkBuffer)currentStreamBuffer->getHandle();
	bufferInfo.offset = currentUsedUniformStreamBuffersCount * uniformBufferSizeAligned;
	bufferInfo.range = sizeof(BuiltinUniformData);
	
	VkDescriptorSet currentDescriptorSet = descriptorSetsVector.at(currentFrame).at(currentUsedDescriptorSetsCount);

	std::vector<VkWriteDescriptorSet> descriptorWrite{};

	// uniform buffer update always happens
	// (are there cases without ubos at all?)
	VkWriteDescriptorSet uniformWrite{};
	uniformWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uniformWrite.dstSet = currentDescriptorSet;
	uniformWrite.dstBinding = builtinUniformInfo[BUILTIN_UNIFORMS_PER_DRAW]->location;
	uniformWrite.dstArrayElement = 0;
	uniformWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformWrite.descriptorCount = 1;
	uniformWrite.pBufferInfo = &bufferInfo;			

	descriptorWrite.push_back(uniformWrite);

	std::vector<VkDescriptorImageInfo*> imageInfos;
	
	// update everything other than uniform buffers (since that's already taken care of.
	for (const auto& [key, val] : uniformInfos) {
		// fixme: other types.
		if (val.baseType == UNIFORM_SAMPLER) {
			VkWriteDescriptorSet write{};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstSet = currentDescriptorSet;
			write.dstBinding = val.location;
			write.dstArrayElement = 0;
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.descriptorCount = 1;

			VkDescriptorImageInfo* imageInfo = createDescriptorImageInfo(val.textures[0]);	// fixme: arrays
			imageInfos.push_back(imageInfo);

			write.pImageInfo = imageInfo;

			descriptorWrite.push_back(write);
		}
	}

	vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrite.size()), descriptorWrite.data(), 0, nullptr);

	for (const auto imageInfo : imageInfos) {
		delete imageInfo;
	}

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &currentDescriptorSet, 0, nullptr);

	currentUsedUniformStreamBuffersCount++;
	currentUsedDescriptorSetsCount++;
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
	return 0;
}

const Shader::UniformInfo* Shader::getUniformInfo(const std::string& name) const {
	return &uniformInfos.at(name);
}

const Shader::UniformInfo* Shader::getUniformInfo(BuiltinUniform builtin) const {
	return builtinUniformInfo[builtin];
}

void Shader::sendTextures(const UniformInfo* info, graphics::Texture** textures, int count) {
	for (unsigned i = 0; i < count; i++) {
		info->textures[i] = textures[i];
	}
}

void Shader::calculateUniformBufferSizeAligned() {
	gfx = Module::getInstance<Graphics>(Module::ModuleType::M_GRAPHICS);
	auto vgfx = (Graphics*)gfx;
	auto minAlignment = vgfx->getMinUniformBufferOffsetAlignment();
	size_t size = localUniformStagingData.size();
	uniformBufferSizeAligned = 
		static_cast<VkDeviceSize>(
			std::ceil(
				static_cast<float>(size) / static_cast<float>(minAlignment)
			)
		)
		* minAlignment;
}

void Shader::buildLocalUniforms(spirv_cross::Compiler& comp, const spirv_cross::SPIRType& type, size_t baseoff, const std::string& basename) {
	using namespace spirv_cross;

	const auto& membertypes = type.member_types;

	for (size_t uindex = 0; uindex < membertypes.size(); uindex++) {
		const auto& memberType = comp.get_type(membertypes[uindex]);
		size_t memberSize = comp.get_declared_struct_member_size(type, uindex);
		size_t offset = baseoff + comp.type_struct_member_offset(type, uindex);

		std::string name = basename + comp.get_member_name(type.self, uindex);

		switch (memberType.basetype) {
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

		if (memberType.columns == 1) {
			if (memberType.basetype == SPIRType::Int) {
				u.baseType = UNIFORM_INT;
			}
			else if (memberType.basetype == SPIRType::UInt) {
				u.baseType = UNIFORM_UINT;
			}
			else {
				u.baseType = UNIFORM_FLOAT;
			}
			u.components = memberType.vecsize;
		}
		else {
			u.baseType = UNIFORM_MATRIX;
			u.matrix.rows = memberType.vecsize;
			u.matrix.columns = memberType.columns;
		}

		// fixme: initializer values

		uniformInfos[u.name] = u;

		BuiltinUniform builtin = BUILTIN_MAX_ENUM;
		if (getConstant(u.name.c_str(), builtin)) {
			if (builtin == BUILTIN_UNIFORMS_PER_DRAW) {
				builtinUniformDataOffset = offset;
			}
			builtinUniformInfo[builtin] = &uniformInfos[u.name];
		}
	}
}

void Shader::compileShaders() {
	using namespace glslang;
	using namespace spirv_cross;

	std::vector<TShader*> glslangShaders;

	TProgram* program = new TProgram();

	gfx = Module::getInstance<Graphics>(Module::ModuleType::M_GRAPHICS);
	auto vgfx = (Graphics*)gfx;
	device = vgfx->getDevice();

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
		glslangShaders.push_back(tshader);
	}

	if (!program->link(EShMsgDefault)) {
		throw love::Exception("link failed! %s\n", program->getInfoLog());
	}

	if (!program->mapIO()) {
		throw love::Exception("mapIO failed");
	}

	uniformInfos.clear();

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

		spirv_cross::CompilerGLSL comp(spirv);

		// we only care about variables that are actually getting used.
		auto active = comp.get_active_interface_variables();
		auto shaderResources = comp.get_shader_resources(active);
		comp.set_enabled_interface_variables(std::move(active));

		for (const auto& resource : shaderResources.uniform_buffers) {
			if (resource.name == "gl_DefaultUniformBlock") {
				const auto& type = comp.get_type(resource.base_type_id);
				size_t uniformBufferObjectSize = comp.get_declared_struct_size(type);
				auto defaultUniformBlockSize = comp.get_declared_struct_size(type);
				localUniformStagingData.resize(defaultUniformBlockSize);

				memset(localUniformStagingData.data(), 0, defaultUniformBlockSize);

				std::string basename("");
				buildLocalUniforms(comp, type, 0, basename);
			}
			else {
				throw love::Exception("unimplemented: non default uniform blocks.");
			}
		}

		for (const auto& r : shaderResources.sampled_images) {
			const SPIRType& basetype = comp.get_type(r.base_type_id);
			const SPIRType& type = comp.get_type(r.type_id);
			const SPIRType& imagetype = comp.get_type(basetype.image.type);

			graphics::Shader::UniformInfo info;
			info.location = comp.get_decoration(r.id, spv::DecorationBinding);
			info.baseType = UNIFORM_SAMPLER;
			info.name = r.name;
			info.count = type.array.empty() ? 1 : type.array[0];
			info.isDepthSampler = type.image.depth;
			info.components = 1;

			switch (imagetype.basetype) {
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

			switch (basetype.image.dim) {
			case spv::Dim2D:
				info.textureType = basetype.image.arrayed ? TEXTURE_2D_ARRAY : TEXTURE_2D;
				info.textures = new love::graphics::Texture * [info.count];
				break;
			case spv::Dim3D:
				info.textureType = TEXTURE_VOLUME;
				info.textures = new love::graphics::Texture * [info.count];
				break;
			case spv::DimCube:
				if (basetype.image.arrayed) {
					throw love::Exception("cubemap arrays are not currently supported");
				}
				info.textureType = TEXTURE_CUBE;
				info.textures = new love::graphics::Texture * [info.count];
				break;
			case spv::DimBuffer:
				throw love::Exception("dim buffers not implemented yet");
			default:
				throw love::Exception("unknown dim");
			}

			if (info.baseType == UNIFORM_SAMPLER) {
				auto tex = vgfx->getDefaultTexture();
				for (int i = 0; i < info.count; i++) {
					info.textures[i] = tex;
				}
			}
			// fixme
			else if (info.baseType == UNIFORM_TEXELBUFFER) {
				throw love::Exception("texel buffers not supported yet");
			}

			uniformInfos[r.name] = info;
			BuiltinUniform builtin;
			if (getConstant(r.name.c_str(), builtin)) {
				builtinUniformInfo[builtin] = &uniformInfos[info.name];
			}
		}
	}

	delete program;
	for (auto shader : glslangShaders) {
		delete shader;
	}
}

void Shader::createDescriptorSetLayout() {
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	for (auto const& [key, val] : uniformInfos) {
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = val.location;
		layoutBinding.descriptorType = val.baseType == UNIFORM_SAMPLER ? VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBinding.descriptorCount = 1;	// is this correct?
		layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;	// fixme: can we determine in what shader it got used?

		bindings.push_back(layoutBinding);
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
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
	// if the shader doesn't actually use these textures they might get optimized out
	// in that case this function becomes a noop.
	if (builtinUniformInfo[BUILTIN_TEXTURE_VIDEO_Y] != nullptr) {
		builtinUniformInfo[BUILTIN_TEXTURE_VIDEO_Y]->textures[0] = ytexture;
	}
	if (builtinUniformInfo[BUILTIN_TEXTURE_VIDEO_CB] != nullptr) {
		builtinUniformInfo[BUILTIN_TEXTURE_VIDEO_CB]->textures[0] = cbtexture;
	}
	if (builtinUniformInfo[BUILTIN_TEXTURE_VIDEO_CR] != nullptr) {
		builtinUniformInfo[BUILTIN_TEXTURE_VIDEO_CR]->textures[0] = crtexture;
	}
}

bool Shader::hasUniform(const std::string& name) const {
	return uniformInfos.find(name) != uniformInfos.end();
}

void Shader::setUniformData(BuiltinUniformData& data) {
	char* ptr = (char*) builtinUniformInfo[BUILTIN_UNIFORMS_PER_DRAW]->data + builtinUniformDataOffset;
	memcpy(ptr, &data, sizeof(BuiltinUniformData));
}

void Shader::setMainTex(graphics::Texture* texture) {
	// if the shader doesn't actually use the texture it might get optimized out
	// in that case this function becomes a noop.
	if (builtinUniformInfo[BUILTIN_TEXTURE_MAIN] != nullptr) {
		builtinUniformInfo[BUILTIN_TEXTURE_MAIN]->textures[0] = texture;
	}
}

VkDescriptorSet Shader::allocateDescriptorSet() {
	if (currentAllocatedDescriptorSets >= DESCRIPTOR_POOL_SIZE) {
		// fixme: we can optimize this, since sizes should never change for a given shader.
		std::vector<VkDescriptorPoolSize> sizes;

		for (const auto& [key, val] : uniformInfos) {
			VkDescriptorPoolSize size{};
			size.type = Vulkan::getDescriptorType(val.baseType);
			size.descriptorCount = 1;
			sizes.push_back(size);
		}

		VkDescriptorPoolCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		createInfo.maxSets = DESCRIPTOR_POOL_SIZE;
		createInfo.poolSizeCount = static_cast<uint32_t>(sizes.size());
		createInfo.pPoolSizes = sizes.data();

		VkDescriptorPool pool;
		if (vkCreateDescriptorPool(device, &createInfo, nullptr, &pool) != VK_SUCCESS) {
			throw love::Exception("failed to create descriptor pool");
		}
		descriptorPools.push_back(pool);

		currentAllocatedDescriptorSets = 0;
	}

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPools.back();
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSetLayout;

	VkDescriptorSet descriptorSet;
	VkResult result = vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet);
	if (result != VK_SUCCESS) {
		throw love::Exception("failed to allocate descriptor set");
	}

	currentAllocatedDescriptorSets++;

	return descriptorSet;
}
} // vulkan
} // graphics
} // love
