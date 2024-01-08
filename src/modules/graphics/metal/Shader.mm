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
#include "common/int.h"

// glslang
#include "libraries/glslang/glslang/Public/ShaderLang.h"
#include "libraries/glslang/glslang/Public/ResourceLimits.h"
#include "libraries/glslang/SPIRV/GlslangToSpv.h"
#include "libraries/spirv_cross/spirv_msl.hpp"
#include "libraries/spirv_cross/spirv_reflect.hpp"

#include <algorithm>

namespace love
{
namespace graphics
{
namespace metal
{

static_assert(MAX_COLOR_RENDER_TARGETS <= 8, "Metal pipeline cache key only stores 8 render target pixel formats.");

static MTLVertexFormat getMTLVertexFormat(DataFormat format)
{
	switch (format)
	{
		case DATAFORMAT_FLOAT: return MTLVertexFormatFloat;
		case DATAFORMAT_FLOAT_VEC2: return MTLVertexFormatFloat2;
		case DATAFORMAT_FLOAT_VEC3: return MTLVertexFormatFloat3;
		case DATAFORMAT_FLOAT_VEC4: return MTLVertexFormatFloat4;

		case DATAFORMAT_INT32: return MTLVertexFormatInt;
		case DATAFORMAT_INT32_VEC2: return MTLVertexFormatInt2;
		case DATAFORMAT_INT32_VEC3: return MTLVertexFormatInt3;
		case DATAFORMAT_INT32_VEC4: return MTLVertexFormatInt4;

		case DATAFORMAT_UINT32: return MTLVertexFormatUInt;
		case DATAFORMAT_UINT32_VEC2: return MTLVertexFormatUInt2;
		case DATAFORMAT_UINT32_VEC3: return MTLVertexFormatUInt3;
		case DATAFORMAT_UINT32_VEC4: return MTLVertexFormatUInt4;

		case DATAFORMAT_SNORM8_VEC4: return MTLVertexFormatChar4Normalized;
		case DATAFORMAT_UNORM8_VEC4: return MTLVertexFormatUChar4Normalized;
		case DATAFORMAT_INT8_VEC4: return MTLVertexFormatChar4;
		case DATAFORMAT_UINT8_VEC4: return MTLVertexFormatUChar4;

		case DATAFORMAT_SNORM16_VEC2: return MTLVertexFormatShort2Normalized;
		case DATAFORMAT_SNORM16_VEC4: return MTLVertexFormatShort4Normalized;

		case DATAFORMAT_UNORM16_VEC2: return MTLVertexFormatUShort2Normalized;
		case DATAFORMAT_UNORM16_VEC4: return MTLVertexFormatUShort4Normalized;

		case DATAFORMAT_INT16_VEC2: return MTLVertexFormatShort2;
		case DATAFORMAT_INT16_VEC4: return MTLVertexFormatShort4;

		case DATAFORMAT_UINT16: return MTLVertexFormatUShort;
		case DATAFORMAT_UINT16_VEC2: return MTLVertexFormatUShort2;
		case DATAFORMAT_UINT16_VEC4: return MTLVertexFormatUShort4;

		default: return MTLVertexFormatInvalid;
	}
}

static MTLBlendOperation getMTLBlendOperation(BlendOperation op)
{
	switch (op)
	{
		case BLENDOP_ADD: return MTLBlendOperationAdd;
		case BLENDOP_SUBTRACT: return MTLBlendOperationSubtract;
		case BLENDOP_REVERSE_SUBTRACT: return MTLBlendOperationReverseSubtract;
		case BLENDOP_MIN: return MTLBlendOperationMin;
		case BLENDOP_MAX: return MTLBlendOperationMax;
		case BLENDOP_MAX_ENUM: return MTLBlendOperationAdd;
	}
	return MTLBlendOperationAdd;
}

static MTLBlendFactor getMTLBlendFactor(BlendFactor factor)
{
	switch (factor)
	{
		case BLENDFACTOR_ZERO: return MTLBlendFactorZero;
		case BLENDFACTOR_ONE: return MTLBlendFactorOne;
		case BLENDFACTOR_SRC_COLOR: return MTLBlendFactorSourceColor;
		case BLENDFACTOR_ONE_MINUS_SRC_COLOR: return MTLBlendFactorOneMinusSourceColor;
		case BLENDFACTOR_SRC_ALPHA: return MTLBlendFactorSourceAlpha;
		case BLENDFACTOR_ONE_MINUS_SRC_ALPHA: return MTLBlendFactorOneMinusSourceAlpha;
		case BLENDFACTOR_DST_COLOR: return MTLBlendFactorDestinationColor;
		case BLENDFACTOR_ONE_MINUS_DST_COLOR: return MTLBlendFactorOneMinusDestinationColor;
		case BLENDFACTOR_DST_ALPHA: return MTLBlendFactorDestinationAlpha;
		case BLENDFACTOR_ONE_MINUS_DST_ALPHA: return MTLBlendFactorOneMinusDestinationAlpha;
		case BLENDFACTOR_SRC_ALPHA_SATURATED: return MTLBlendFactorSourceAlphaSaturated;
		case BLENDFACTOR_MAX_ENUM: return MTLBlendFactorZero;
	}
	return MTLBlendFactorZero;
}

static inline id<MTLTexture> getMTLTexture(love::graphics::Texture *tex)
{
	return tex ? (__bridge id<MTLTexture>)(void *) tex->getHandle() : nil;
}

static inline id<MTLTexture> getMTLTexture(love::graphics::Buffer *buffer)
{
	return buffer ? (__bridge id<MTLTexture>)(void *) buffer->getTexelBufferHandle() : nil;
}

static inline id<MTLBuffer> getMTLBuffer(love::graphics::Buffer *buffer)
{
	return buffer ? (__bridge id<MTLBuffer>)(void *) buffer->getHandle() : nil;
}

static EShLanguage getGLSLangStage(ShaderStageType stage)
{
	switch (stage)
	{
		case SHADERSTAGE_VERTEX: return EShLangVertex;
		case SHADERSTAGE_PIXEL: return EShLangFragment;
		case SHADERSTAGE_COMPUTE: return EShLangCompute;
		case SHADERSTAGE_MAX_ENUM: return EShLangCount;
	}
	return EShLangCount;
}

Shader::Shader(id<MTLDevice> device, StrongRef<love::graphics::ShaderStage> stages[SHADERSTAGE_MAX_ENUM], const CompileOptions &options)
	: love::graphics::Shader(stages, options)
	, functions()
	, builtinUniformInfo()
	, localUniformStagingData(nullptr)
	, localUniformBufferData(nullptr)
	, localUniformBufferSize(0)
	, builtinUniformDataOffset(0)
	, firstVertexBufferBinding(DEFAULT_VERTEX_BUFFER_BINDING + 1)
{ @autoreleasepool {
	using namespace glslang;

	TShader *glslangShaders[SHADERSTAGE_MAX_ENUM] = {};

	TProgram *program = new TProgram();

	auto cleanup = [&]()
	{
		delete program;
		for (int i = 0; i < SHADERSTAGE_MAX_ENUM; i++)
			delete glslangShaders[i];
	};

	// We can't do this in ShaderStage because the mapIO call modifies the
	// TShader internals in a manner that prevents it from being shared.
	for (int i = 0; i < SHADERSTAGE_MAX_ENUM; i++)
	{
		if (!stages[i])
			continue;

		auto stage = (ShaderStageType) i;
		auto glslangstage = getGLSLangStage(stage);
		auto tshader = new TShader(glslangstage);

		glslangShaders[i] = tshader;

		tshader->setEnvInput(EShSourceGlsl, glslangstage, EShClientVulkan, 450);
		tshader->setEnvClient(EShClientVulkan, EShTargetVulkan_1_2);
		tshader->setEnvTarget(EShTargetSpv, EShTargetSpv_1_5);
		tshader->setAutoMapLocations(true);
		tshader->setAutoMapBindings(true);

		// Needed for local uniforms to work (they will be converted into a
		// uniform block).
		// https://github.com/KhronosGroup/GLSL/blob/master/extensions/ext/GL_EXT_vulkan_glsl_relaxed.txt
		tshader->setEnvInputVulkanRulesRelaxed();
		tshader->setGlobalUniformBinding(0);
		tshader->setGlobalUniformSet(0);

		const std::string &source = stages[i]->getSource();
		const char *csrc = source.c_str();
		int srclen = (int) source.length();
		tshader->setStringsWithLengths(&csrc, &srclen, 1);

		int defaultversion = 450;
		EProfile defaultprofile = ECoreProfile;
		bool forcedefault = false;
		bool forwardcompat = true;

#ifdef LOVE_IOS
		defaultversion = 320;
		defaultprofile = EEsProfile;
		forcedefault = true;
#endif

		if (!tshader->parse(GetDefaultResources(), defaultversion, defaultprofile, forcedefault, forwardcompat, EShMsgSuppressWarnings))
		{
			const char *stagename = "unknown";
			ShaderStage::getConstant(stage, stagename);

			std::string err = "Error parsing " + std::string(stagename) + " shader:\n\n"
				+ std::string(tshader->getInfoLog()) + "\n"
				+ std::string(tshader->getInfoDebugLog());

			cleanup();
			throw love::Exception("%s", err.c_str());
		}

		program->addShader(tshader);
	}

	if (!program->link(EShMsgDefault))
	{
		//err = "Cannot compile shader:\n\n" + std::string(program->getInfoLog()) + "\n" + std::string(program->getInfoDebugLog());
		cleanup();
		throw love::Exception("link failed! %s\n", program->getInfoLog());
	}

	if (!program->mapIO())
	{
		cleanup();
		throw love::Exception("mapIO failed");
	}

	try
	{
		compileFromGLSLang(device, *program);
	}
	catch (love::Exception &)
	{
		cleanup();
		throw;
	}

	cleanup();

	if (functions[SHADERSTAGE_COMPUTE] != nil)
	{
		MTLComputePipelineDescriptor *desc = [MTLComputePipelineDescriptor new];
		desc.computeFunction = functions[SHADERSTAGE_COMPUTE];

		// TODO: threadGroupSizeIsMultipleOfThreadExecutionWidth

		NSError *err = nil;
		computePipeline = [device newComputePipelineStateWithDescriptor:desc
																options:MTLPipelineOptionNone
															 reflection:nil
																  error:&err];
		if (computePipeline == nil)
		{
			if (err != nil)
				throw love::Exception("Error creating compute shader pipeline: %s", err.localizedDescription.UTF8String);
			else
				throw love::Exception("Error creating compute shader pipeline.");
		}
	}
}}

void Shader::buildLocalUniforms(const spirv_cross::CompilerMSL &msl, const spirv_cross::SPIRType &type, size_t baseoffset, const std::string &basename)
{
	using namespace spirv_cross;

	const auto &membertypes = type.member_types;

	for (size_t uindex = 0; uindex < membertypes.size(); uindex++)
	{
		const auto &membertype = msl.get_type(membertypes[uindex]);
		size_t membersize = msl.get_declared_struct_member_size(type, uindex);
		size_t offset = baseoffset + msl.type_struct_member_offset(type, uindex);

		std::string name = basename + msl.get_member_name(type.self, uindex);

		switch (membertype.basetype)
		{
			case SPIRType::Struct:
				name += ".";
				buildLocalUniforms(msl, membertype, offset, name);
				continue;
			case SPIRType::Int:
			case SPIRType::UInt:
			case SPIRType::Float:
				break;
			default:
				continue;
		}

		if (offset + membersize > localUniformBufferSize)
			throw love::Exception("Invalid uniform offset + size for '%s' (offset=%d, size=%d, buffer size=%d)", name.c_str(), (int)offset, (int)membersize, (int)localUniformBufferSize);

		UniformInfo u = {};
		u.name = name;
		u.dataSize = membersize;
		u.count = membertype.array.empty() ? 1 : membertype.array[0];
		u.components = 1;

		u.data = localUniformStagingData + offset;
		if (membertype.columns == 1)
		{
			if (membertype.basetype == SPIRType::Int)
				u.baseType = UNIFORM_INT;
			else if (membertype.basetype == SPIRType::UInt)
				u.baseType = UNIFORM_UINT;
			else
				u.baseType = UNIFORM_FLOAT;
			u.components = membertype.vecsize;
		}
		else
		{
			u.baseType = UNIFORM_MATRIX;
			u.matrix.rows = membertype.vecsize;
			u.matrix.columns = membertype.columns;
		}

		const auto &reflectionit = validationReflection.localUniforms.find(u.name);
		if (reflectionit != validationReflection.localUniforms.end())
		{
			const auto &localuniform = reflectionit->second;
			const auto &values = localuniform.initializerValues;
			if (!values.empty())
				memcpy(u.data, values.data(), std::min(u.dataSize, values.size() * sizeof(LocalUniformValue)));
		}

		uniforms[u.name] = u;

		BuiltinUniform builtin = BUILTIN_MAX_ENUM;
		if (getConstant(u.name.c_str(), builtin))
		{
			if (builtin == BUILTIN_UNIFORMS_PER_DRAW)
				builtinUniformDataOffset = offset;
			builtinUniformInfo[builtin] = &uniforms[u.name];
		}

		updateUniform(&u, u.count);
	}
}

void Shader::addImage(const spirv_cross::CompilerMSL &msl, const spirv_cross::Resource &resource, UniformType baseType)
{
	using namespace spirv_cross;

	const SPIRType &basetype = msl.get_type(resource.base_type_id);
	const SPIRType &type = msl.get_type(resource.type_id);
	const SPIRType &imagetype = msl.get_type(basetype.image.type);

	UniformInfo u = {};
	u.baseType = baseType;
	u.name = resource.name;
	u.count = type.array.empty() ? 1 : type.array[0];
	u.isDepthSampler = type.image.depth;
	u.components = 1;

	auto it = uniforms.find(u.name);
	if (it != uniforms.end())
		return;

	if (!fillUniformReflectionData(u))
		return;

	switch (imagetype.basetype)
	{
	case SPIRType::Float:
		u.dataBaseType = DATA_BASETYPE_FLOAT;
		break;
	case SPIRType::Int:
		u.dataBaseType = DATA_BASETYPE_INT;
		break;
	case SPIRType::UInt:
		u.dataBaseType = DATA_BASETYPE_UINT;
		break;
	default:
		break;
	}

	switch (basetype.image.dim)
	{
	case spv::Dim2D:
		u.textureType = basetype.image.arrayed ? TEXTURE_2D_ARRAY : TEXTURE_2D;
		u.textures = new love::graphics::Texture*[u.count];
		break;
	case spv::Dim3D:
		u.textureType = TEXTURE_VOLUME;
		u.textures = new love::graphics::Texture*[u.count];
		break;
	case spv::DimCube:
		if (basetype.image.arrayed)
			throw love::Exception("Cubemap Arrays are not currently supported.");
		u.textureType = TEXTURE_CUBE;
		u.textures = new love::graphics::Texture*[u.count];
		break;
	case spv::DimBuffer:
		u.baseType = UNIFORM_TEXELBUFFER;
		u.buffers = new love::graphics::Buffer*[u.count];
		break;
	default:
		// TODO: error? continue?
		break;
	}

	u.dataSize = sizeof(int) * u.count;
	u.data = malloc(u.dataSize);
	for (int i = 0; i < u.count; i++)
		u.ints[i] = -1; // Initialized below, after compiling.

	if (u.baseType == UNIFORM_SAMPLER)
	{
		auto tex = Graphics::getInstance()->getDefaultTexture(u.textureType, u.dataBaseType);
		for (int i = 0; i < u.count; i++)
		{
			tex->retain();
			u.textures[i] = tex;
		}
	}
	else if (u.baseType == UNIFORM_TEXELBUFFER)
	{
		for (int i = 0; i < u.count; i++)
			u.buffers[i] = nullptr; // TODO
	}
	else if (u.baseType == UNIFORM_STORAGETEXTURE)
	{
		Texture *tex = nullptr;
		if ((u.access & ACCESS_WRITE) == 0)
			tex = Graphics::getInstance()->getDefaultTexture(u.textureType, u.dataBaseType);
		for (int i = 0; i < u.count; i++)
		{
			if (tex)
				tex->retain();
			u.textures[i] = tex;
		}
	}

	uniforms[u.name] = u;

	BuiltinUniform builtin;
	if (getConstant(resource.name.c_str(), builtin))
		builtinUniformInfo[builtin] = &uniforms[u.name];
}

void Shader::compileFromGLSLang(id<MTLDevice> device, const glslang::TProgram &program)
{
	using namespace glslang;
	using namespace spirv_cross;

	std::map<std::string, int> varyings;
	int nextVaryingLocation = 0;

	int metalBufferIndices[SHADERSTAGE_MAX_ENUM];
	for (int i = 0; i < SHADERSTAGE_MAX_ENUM; i++)
		metalBufferIndices[i] = getUniformBufferBinding() + 1;
	metalBufferIndices[SHADERSTAGE_VERTEX] = DEFAULT_VERTEX_BUFFER_BINDING + 1;

	for (int stageindex = 0; stageindex < SHADERSTAGE_MAX_ENUM; stageindex++)
	{
		auto glslangstage = getGLSLangStage((ShaderStageType) stageindex);
		auto intermediate = program.getIntermediate(glslangstage);
		if (intermediate == nullptr)
			continue;

		spv::SpvBuildLogger logger;
		glslang::SpvOptions opt;
		opt.validate = true;

		std::vector<unsigned int> spirv;
		GlslangToSpv(*intermediate, spirv, &logger, &opt);

		std::string msgs = logger.getAllMessages();
//		printf("spirv length: %ld, messages:\n%s\n", spirv.size(), msgs.c_str());

		try
		{
//			printf("GLSL INPUT SOURCE:\n\n%s\n\n", pixel->getSource().c_str());

			CompilerMSL msl(std::move(spirv));

			auto interfacevars = msl.get_active_interface_variables();

			msl.set_enabled_interface_variables(interfacevars);

			ShaderResources resources = msl.get_shader_resources();

			for (const auto &resource : resources.storage_images)
			{
				addImage(msl, resource, UNIFORM_STORAGETEXTURE);
			}

			for (const auto &resource : resources.sampled_images)
			{
				addImage(msl, resource, UNIFORM_SAMPLER);
			}

			for (const auto &resource : resources.uniform_buffers)
			{
				MSLResourceBinding binding;
				binding.stage = msl.get_execution_model();
				binding.binding = msl.get_decoration(resource.id, spv::DecorationBinding);
				binding.desc_set = msl.get_decoration(resource.id, spv::DecorationDescriptorSet);

				if (resource.name == "gl_DefaultUniformBlock")
				{
					binding.msl_buffer = getUniformBufferBinding();
					msl.add_msl_resource_binding(binding);

					const SPIRType &type = msl.get_type(resource.base_type_id);
					size_t size = msl.get_declared_struct_size(type);

					if (localUniformBufferSize != 0)
					{
						if (localUniformBufferSize != size)
							throw love::Exception("Local uniform buffer size mismatch");
						continue;
					}

					localUniformStagingData = new uint8[size];
					localUniformBufferData = new uint8[size];
					localUniformBufferSize = size;

					memset(localUniformStagingData, 0, size);
					memset(localUniformBufferData, 0, size);

					std::string basename("");
					buildLocalUniforms(msl, type, 0, basename);
				}
				else
				{
					binding.msl_buffer = metalBufferIndices[stageindex]++;
					msl.add_msl_resource_binding(binding);
				}
			}

			for (const auto &resource : resources.storage_buffers)
			{
				MSLResourceBinding binding;
				binding.stage = msl.get_execution_model();
				binding.binding = msl.get_decoration(resource.id, spv::DecorationBinding);
				binding.desc_set = msl.get_decoration(resource.id, spv::DecorationDescriptorSet);
				binding.msl_buffer = metalBufferIndices[stageindex]++;
				msl.add_msl_resource_binding(binding);

				auto it = uniforms.find(resource.name);
				if (it != uniforms.end())
					continue;

				const SPIRType &type = msl.get_type(resource.type_id);

				UniformInfo u = {};
				u.baseType = UNIFORM_STORAGEBUFFER;
				u.components = 1;
				u.name = resource.name;
				u.count = type.array.empty() ? 1 : type.array[0];

				if (!fillUniformReflectionData(u))
					continue;

				u.buffers = new love::graphics::Buffer*[u.count];
				u.dataSize = sizeof(int) * u.count;
				u.data = malloc(u.dataSize);

				for (int i = 0; i < u.count; i++)
				{
					u.ints[i] = -1; // Initialized below, after compiling.
					u.buffers[i] = nullptr; // TODO
				}

				uniforms[u.name] = u;
			}

			if (stageindex == SHADERSTAGE_VERTEX)
			{
				int nextattributeindex = ATTRIB_MAX_ENUM;

				for (const auto &var : interfacevars)
				{
					spv::StorageClass storage = msl.get_storage_class(var);
					const std::string &name = msl.get_name(var);

					if (storage == spv::StorageClassInput)
					{
						int index = 0;

						BuiltinVertexAttribute builtinattribute;
						if (graphics::getConstant(name.c_str(), builtinattribute))
							index = (int) builtinattribute;
						else
							index = nextattributeindex++;

						msl.set_decoration(var, spv::DecorationLocation, index);
						attributes[name] = msl.get_decoration(var, spv::DecorationLocation);
					}
				}

				for (const auto &varying : resources.stage_outputs)
				{
//					printf("vertex shader output %s: %d\n", inp.name.c_str(), msl.get_decoration(inp.id, spv::DecorationLocation));
					varyings[varying.name] = nextVaryingLocation;
					msl.set_decoration(varying.id, spv::DecorationLocation, nextVaryingLocation++);
				}
			}
			else if (stageindex == SHADERSTAGE_PIXEL)
			{
				for (const auto &varying : resources.stage_inputs)
				{
					const auto it = varyings.find(varying.name);
					if (it != varyings.end())
						msl.set_decoration(varying.id, spv::DecorationLocation, it->second);
				}
			}

			CompilerMSL::Options options;
			options.set_msl_version(2, 1);
			options.texture_buffer_native = true;
#ifdef LOVE_IOS
			options.platform = CompilerMSL::Options::iOS;
#else
			options.platform = CompilerMSL::Options::macOS;
#endif

			msl.set_msl_options(options);

			std::string source = msl.compile();
//			printf("// MSL SOURCE for stage %d:\n\n%s\n\n", stageindex, source.c_str());

			NSString *nssource = [[NSString alloc] initWithBytes:source.c_str()
														  length:source.length()
														encoding:NSUTF8StringEncoding];

			MTLCompileOptions *opts = [MTLCompileOptions new];

			// Silences warning. We already only use metal on these OS versions.
			if (@available(macOS 10.14, iOS 12.0, *))
				opts.languageVersion = MTLLanguageVersion2_1;

			NSError *err = nil;
			id<MTLLibrary> library = [device newLibraryWithSource:nssource options:opts error:&err];
			if (library == nil && err != nil)
			{
				NSLog(@"errors: %@", err);
				throw love::Exception("Error compiling converted Metal shader code");
			}

			functions[stageindex] = [library newFunctionWithName:library.functionNames[0]];

			std::string debugname = getShaderStageDebugName((ShaderStageType)stageindex);
			if (!debugname.empty())
				functions[stageindex].label = @(debugname.c_str());

			auto setTextureBinding = [this](CompilerMSL &msl, int stageindex, const spirv_cross::Resource &resource) -> void
			{
				auto it = uniforms.find(resource.name);
				if (it == uniforms.end())
					return;

				UniformInfo &u = it->second;

				uint32 texturebinding = msl.get_automatic_msl_resource_binding(resource.id);
				uint32 samplerbinding = msl.get_automatic_msl_resource_binding_secondary(resource.id);

				if (texturebinding == (uint32)-1)
					return;

				for (int i = 0; i < u.count; i++)
				{
					if (u.ints[i] == -1)
					{
						u.ints[i] = (int)textureBindings.size();
						TextureBinding b = {};
						b.access = u.access;

						if (u.baseType == UNIFORM_TEXELBUFFER)
						{
							// TODO
						}
						else
						{
							b.texture = getMTLTexture(u.textures[i]);
							b.samplerTexture = u.textures[i];

							BuiltinUniform builtin = BUILTIN_MAX_ENUM;
							if (getConstant(u.name.c_str(), builtin) && builtin == BUILTIN_TEXTURE_MAIN)
								b.isMainTexture = true;
						}

						for (uint8 &stagebinding : b.textureStages)
							stagebinding = LOVE_UINT8_MAX;
						for (uint8 &stagebinding : b.samplerStages)
							stagebinding = LOVE_UINT8_MAX;

						textureBindings.push_back(b);
					}

					auto &b = textureBindings[u.ints[i]];
					b.textureStages[stageindex] = (uint8) texturebinding;
					b.samplerStages[stageindex] = (uint8) samplerbinding;
				}
			};

			for (const auto &resource : resources.sampled_images)
			{
				setTextureBinding(msl, stageindex, resource);
			}

			for (const auto &resource : resources.storage_images)
			{
				setTextureBinding(msl, stageindex, resource);
			}

			for (const auto &resource : resources.storage_buffers)
			{
				auto it = uniforms.find(resource.name);
				if (it == uniforms.end())
					continue;

				UniformInfo &u = it->second;

				uint32 bufferbinding = msl.get_automatic_msl_resource_binding(resource.id);
				if (bufferbinding == (uint32)-1)
					continue;

				for (int i = 0; i < u.count; i++)
				{
					if (u.ints[i] == -1)
					{
						u.ints[i] = (int)bufferBindings.size();
						BufferBinding b = {};
						b.access = u.access;

						for (uint8 &stagebinding : b.stages)
							stagebinding = LOVE_UINT8_MAX;

						bufferBindings.push_back(b);
					}

					bufferBindings[u.ints[i]].stages[stageindex] = (uint8) bufferbinding;
				}
			}
		}
		catch (std::exception &e)
		{
			printf("Error parsing SPIR-V shader source: %s\n", e.what());
		}
	}

	firstVertexBufferBinding = metalBufferIndices[SHADERSTAGE_VERTEX];
}

Shader::~Shader()
{ @autoreleasepool {
	for (int i = 0; i < SHADERSTAGE_MAX_ENUM; i++)
		functions[i] = nil;

	for (const auto &kvp : cachedRenderPipelines)
		CFBridgingRelease(kvp.second);

	cachedRenderPipelines.clear();

	for (const auto &it : uniforms)
	{
		const auto &u = it.second;
		if (u.baseType == UNIFORM_SAMPLER || u.baseType == UNIFORM_STORAGETEXTURE)
		{
			free(u.data);
			for (int i = 0; i < u.count; i++)
			{
				if (u.textures[i] != nullptr)
					u.textures[i]->release();
			}
			delete[] u.textures;
		}
		else if (u.baseType == UNIFORM_TEXELBUFFER || u.baseType == UNIFORM_STORAGEBUFFER)
		{
			free(u.data);
			for (int i = 0; i < u.count; i++)
			{
				if (u.buffers[i] != nullptr)
					u.buffers[i]->release();
			}
			delete[] u.buffers;
		}
	}

	delete[] localUniformStagingData;
	delete[] localUniformBufferData;
}}

void Shader::attach()
{
	if (current != this)
	{
		Graphics *gfx = Graphics::getInstance();
		gfx->flushBatchedDraws();
		gfx->setShaderChanged();
		current = this;
	}
}

int Shader::getVertexAttributeIndex(const std::string &name)
{
	const auto it = attributes.find(name);
	return it != attributes.end() ? it->second : -1;
}

const Shader::UniformInfo *Shader::getUniformInfo(const std::string &name) const
{
	const auto it = uniforms.find(name);
	return it != uniforms.end() ? &(it->second) : nullptr;
}

const Shader::UniformInfo *Shader::getUniformInfo(BuiltinUniform builtin) const
{
	return builtinUniformInfo[(int)builtin];
}

void Shader::updateUniform(const UniformInfo *info, int count)
{
	if (current == this)
		Graphics::flushBatchedDrawsGlobal();

	count = std::min(count, info->count);

	// TODO: store some of this in UniformInfo.
	size_t elementsize = info->components * 4;
	if (info->baseType == UNIFORM_MATRIX)
		elementsize = info->matrix.columns * info->matrix.rows * 4;

	size_t offset = (const uint8 *)info->data - localUniformStagingData;

	// Assuming std140 packing rules, the source data can only be direct-copied
	// to the uniform buffer in certain cases because it's tightly packed whereas
	// the buffer's data isn't.
	if (elementsize * info->count == info->dataSize || (count == 1 && info->baseType != UNIFORM_MATRIX))
	{
		memcpy(localUniformBufferData + offset, info->data, elementsize * count);
	}
	else
	{
		int veccount = count;
		int comp = info->components;

		if (info->baseType == UNIFORM_MATRIX)
		{
			veccount *= info->matrix.rows;
			comp = info->matrix.columns;
		}

		const int *src = info->ints;
		int *dst = (int *) (localUniformBufferData + offset);

		for (int i = 0; i < veccount; i++)
		{
			for (int c = 0; c < comp; c++)
				dst[i * 4 + c] = src[i * comp + c];
		}
	}
}

void Shader::sendTextures(const UniformInfo *info, love::graphics::Texture **textures, int count)
{ @autoreleasepool {
	if (info->baseType != UNIFORM_SAMPLER && info->baseType != UNIFORM_STORAGETEXTURE)
		return;

	if (current == this)
		Graphics::flushBatchedDrawsGlobal();

	count = std::min(count, info->count);

	for (int i = 0; i < count; i++)
	{
		love::graphics::Texture *tex = textures[i];

		if (tex != nullptr)
		{
			if (!validateTexture(info, tex, false))
				continue;
		}
		else
		{
			auto gfx = Graphics::getInstance();
			tex = gfx->getDefaultTexture(info->textureType, info->dataBaseType);
		}

		tex->retain();

		if (info->textures[i] != nullptr)
			info->textures[i]->release();

		info->textures[i] = tex;

		textureBindings[info->ints[i]].texture = getMTLTexture(tex);
		textureBindings[info->ints[i]].samplerTexture = tex;
	}
}}

void Shader::sendBuffers(const UniformInfo *info, love::graphics::Buffer **buffers, int count)
{
	bool texelbinding = info->baseType == UNIFORM_TEXELBUFFER;
	bool storagebinding = info->baseType == UNIFORM_STORAGEBUFFER;

	if (!texelbinding && !storagebinding)
		return;

	if (current == this)
		Graphics::flushBatchedDrawsGlobal();

	count = std::min(count, info->count);

	// Bind the textures to the texture units.
	for (int i = 0; i < count; i++)
	{
		love::graphics::Buffer *buffer = buffers[i];

		if (buffer != nullptr)
		{
			if (!validateBuffer(info, buffer, false))
				continue;
			buffer->retain();
		}

		if (info->buffers[i] != nullptr)
			info->buffers[i]->release();

		info->buffers[i] = buffer;

		if (texelbinding)
		{
			textureBindings[info->ints[i]].texture = getMTLTexture(buffer);
		}
		else if (storagebinding)
		{
			// TODO
			bufferBindings[info->ints[i]].buffer = getMTLBuffer(buffer);
		}
	}
}

void Shader::setVideoTextures(love::graphics::Texture *ytexture, love::graphics::Texture *cbtexture, love::graphics::Texture *crtexture)
{
	const BuiltinUniform builtins[3] = {
		BUILTIN_TEXTURE_VIDEO_Y,
		BUILTIN_TEXTURE_VIDEO_CB,
		BUILTIN_TEXTURE_VIDEO_CR,
	};

	love::graphics::Texture *textures[3] = {ytexture, cbtexture, crtexture};

	for (int i = 0; i < 3; i++)
	{
		const UniformInfo *info = builtinUniformInfo[builtins[i]];
		if (info != nullptr)
			sendTextures(info, &textures[i], 1);
	}
}

bool Shader::hasUniform(const std::string &name) const
{
	return uniforms.find(name) != uniforms.end();
}

id<MTLRenderPipelineState> Shader::getCachedRenderPipeline(const RenderPipelineKey &key)
{
	auto it = cachedRenderPipelines.find(key);

	if (it != cachedRenderPipelines.end())
		return (__bridge id<MTLRenderPipelineState>) it->second;

	id<MTLDevice> device = Graphics::getInstance()->device;

	MTLRenderPipelineDescriptor *desc = [MTLRenderPipelineDescriptor new];

	desc.vertexFunction = functions[SHADERSTAGE_VERTEX];
	desc.fragmentFunction = functions[SHADERSTAGE_PIXEL];

	desc.rasterSampleCount = std::max((int) key.msaa, 1);

	for (int i = 0; i < MAX_COLOR_RENDER_TARGETS; i++)
	{
		PixelFormat format = (PixelFormat)((key.colorRenderTargetFormats >> (i * 8)) & 0xFF);
		if (format == PIXELFORMAT_UNKNOWN)
			continue;

		MTLRenderPipelineColorAttachmentDescriptor *attachment = desc.colorAttachments[i];

		auto formatdesc = Metal::convertPixelFormat(device, format);
		attachment.pixelFormat = formatdesc.format;

		if (key.blend.enable)
		{
			attachment.blendingEnabled = YES;
			attachment.sourceRGBBlendFactor = getMTLBlendFactor(key.blend.srcFactorRGB);
			attachment.destinationRGBBlendFactor = getMTLBlendFactor(key.blend.dstFactorRGB);
			attachment.rgbBlendOperation = getMTLBlendOperation(key.blend.operationRGB);
			attachment.sourceAlphaBlendFactor = getMTLBlendFactor(key.blend.srcFactorA);
			attachment.destinationAlphaBlendFactor = getMTLBlendFactor(key.blend.dstFactorA);
			attachment.alphaBlendOperation = getMTLBlendOperation(key.blend.operationA);
		}

		MTLColorWriteMask writeMask = MTLColorWriteMaskNone;
		if (key.colorChannelMask.r)
			writeMask |= MTLColorWriteMaskRed;
		if (key.colorChannelMask.g)
			writeMask |= MTLColorWriteMaskGreen;
		if (key.colorChannelMask.b)
			writeMask |= MTLColorWriteMaskBlue;
		if (key.colorChannelMask.a)
			writeMask |= MTLColorWriteMaskAlpha;

		attachment.writeMask = writeMask;

		desc.colorAttachments[i] = attachment;
	}

	auto dsformat = (PixelFormat) key.depthStencilFormat;
	if (isPixelFormatDepthStencil(dsformat))
	{
		if (@available(macOS 10.15, iOS 13, *))
		{
			// We already don't really support metal on older systems, this just
			// silences a compiler warning about it.
			auto formatdesc = Metal::convertPixelFormat(device, dsformat);
			if (isPixelFormatDepth(dsformat))
				desc.depthAttachmentPixelFormat = formatdesc.format;
			if (isPixelFormatStencil(dsformat))
				desc.stencilAttachmentPixelFormat = formatdesc.format;
		}
	}

	MTLVertexDescriptor *vertdesc = [MTLVertexDescriptor vertexDescriptor];
	const auto &attributes = key.vertexAttributes;

	for (const auto &pair : this->attributes)
	{
		int i = pair.second;
		uint32 bit = 1u << i;

		if (attributes.enableBits & bit)
		{
			const auto &attrib = attributes.attribs[i];
			int metalBufferIndex = firstVertexBufferBinding + attrib.bufferIndex;

			vertdesc.attributes[i].format = getMTLVertexFormat(attrib.format);
			vertdesc.attributes[i].offset = attrib.offsetFromVertex;
			vertdesc.attributes[i].bufferIndex = metalBufferIndex;

			const auto &layout = attributes.bufferLayouts[attrib.bufferIndex];

			bool instanced = attributes.instanceBits & (1u << attrib.bufferIndex);
			auto step = instanced ? MTLVertexStepFunctionPerInstance : MTLVertexStepFunctionPerVertex;

			vertdesc.layouts[metalBufferIndex].stride = layout.stride;
			vertdesc.layouts[metalBufferIndex].stepFunction = step;
		}
		else
		{
			vertdesc.attributes[i].format = MTLVertexFormatFloat4;
			vertdesc.attributes[i].offset = 0;
			vertdesc.attributes[i].bufferIndex = DEFAULT_VERTEX_BUFFER_BINDING;

			vertdesc.layouts[DEFAULT_VERTEX_BUFFER_BINDING].stride = sizeof(float) * 4;
			vertdesc.layouts[DEFAULT_VERTEX_BUFFER_BINDING].stepFunction = MTLVertexStepFunctionConstant;
			vertdesc.layouts[DEFAULT_VERTEX_BUFFER_BINDING].stepRate = 0;
		}
	}

	desc.vertexDescriptor = vertdesc;

	NSError *err = nil;
	id<MTLRenderPipelineState> pipeline = [device newRenderPipelineStateWithDescriptor:desc error:&err];

	if (err != nil)
	{
		NSLog(@"Error creating render pipeline: %@", err);
		return nil;
	}

	cachedRenderPipelines[key] = CFBridgingRetain(pipeline);

	return pipeline;
}

int Shader::getUniformBufferBinding()
{
	return spirv_cross::ResourceBindingPushConstantBinding;
}

} // metal
} // graphics
} // love
