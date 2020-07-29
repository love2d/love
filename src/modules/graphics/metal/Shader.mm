/**
 * Copyright (c) 2006-2020 LOVE Development Team
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

static EShLanguage getGLSLangStage(ShaderStage::StageType stage)
{
	switch (stage)
	{
		case ShaderStage::STAGE_VERTEX: return EShLangVertex;
		case ShaderStage::STAGE_PIXEL: return EShLangFragment;
		case ShaderStage::STAGE_MAX_ENUM: return EShLangCount;
	}
	return EShLangCount;
}

Shader::Shader(id<MTLDevice> device, love::graphics::ShaderStage *vertex, love::graphics::ShaderStage *pixel)
	: love::graphics::Shader(vertex, pixel)
	, functions()
	, builtinUniformInfo()
	, localUniformBufferData(nullptr)
	, localUniformBufferSize(0)
{ @autoreleasepool {
	using namespace glslang;
	using namespace spirv_cross;

	// TODO: can this be done in ShaderStage (no linking)?

	glslang::TProgram program;

	if (vertex != nullptr)
		program.addShader((TShader *) vertex->getHandle());

	if (pixel != nullptr)
		program.addShader((TShader *) pixel->getHandle());

	if (!program.link(EShMsgDefault))
	{
		//err = "Cannot compile shader:\n\n" + std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog());
		throw love::Exception("link failed!\n");
	}

	std::map<std::string, int> varyings;
	int nextVaryingLocation = 0;

	for (int i = 0; i < ShaderStage::STAGE_MAX_ENUM; i++)
	{
		auto glslangstage = getGLSLangStage((ShaderStage::StageType) i);
		auto intermediate = program.getIntermediate(glslangstage);
		if (intermediate == nullptr)
			continue;

		spv::SpvBuildLogger logger;
		glslang::SpvOptions opt;
		opt.validate = true;

		std::vector<unsigned int> spirv;

		{
//			timer::ScopedTimer("shader stage");
			GlslangToSpv(*intermediate, spirv, &logger, &opt);
		}

		std::string msgs = logger.getAllMessages();
//		printf("spirv length: %ld, messages:\n%s\n", spirv.size(), msgs.c_str());

		// Compile to GLSL, ready to give to GL driver.
		try
		{
//			printf("GLSL INPUT SOURCE:\n\n%s\n\n", pixel->getSource().c_str());

			CompilerMSL msl(std::move(spirv));

			auto interfacevars = msl.get_active_interface_variables();

			msl.set_enabled_interface_variables(interfacevars);

			ShaderResources resources = msl.get_shader_resources();

			for (const auto &resource : resources.sampled_images)
			{
				int binding = msl.get_decoration(resource.id, spv::DecorationBinding);

				BuiltinUniform builtin = BUILTIN_MAX_ENUM;
				if (getConstant(resource.name.c_str(), builtin))
				{
					// TODO
				}

				auto it = uniforms.find(resource.name);
				if (it != uniforms.end())
				{
					if (it->second.ints[0] != binding)
						throw love::Exception("texture binding mismatch for %s: %d vs %d", resource.name.c_str(), it->second.ints[0], binding);
				}
				else
				{
					UniformInfo u = {};
					u.baseType = UNIFORM_SAMPLER;
					u.name = resource.name;
					u.location = 0;
					u.textures = new love::graphics::Texture*[1];
					u.textures[0] = nullptr;
					u.data = malloc(sizeof(int) * 1);
					u.ints[0] = binding;
//					printf("binding for %s: %d\n", u.name.c_str(), binding);

					uniforms[u.name] = u;
				}
			}

			for (const auto &resource : resources.uniform_buffers)
			{
				if (resource.name == "love_UniformsPerDrawBuffer")
				{
					msl.set_decoration(resource.id, spv::DecorationBinding, 0);
				}
			}

			if (i == ShaderStage::STAGE_VERTEX)
			{
				for (const auto &varying : resources.stage_outputs)
				{
//					printf("vertex shader output %s: %d\n", inp.name.c_str(), msl.get_decoration(inp.id, spv::DecorationLocation));
					varyings[varying.name] = nextVaryingLocation;
					msl.set_decoration(varying.id, spv::DecorationLocation, nextVaryingLocation++);
				}
			}
			else if (i == ShaderStage::STAGE_PIXEL)
			{
				for (const auto &varying : resources.stage_inputs)
				{
					const auto it = varyings.find(varying.name);
					if (it != varyings.end())
						msl.set_decoration(varying.id, spv::DecorationLocation, it->second);
				}
			}

			printf("ubos: %d, storage: %d, inputs: %d, outputs: %d, images: %d, samplers: %d, push: %d\n", resources.uniform_buffers.size(), resources.storage_buffers.size(), resources.stage_inputs.size(), resources.stage_outputs.size(), resources.storage_images.size(), resources.sampled_images.size(), resources.push_constant_buffers.size());

			CompilerMSL::Options options;

#ifdef LOVE_IOS
			options.platform = CompilerMSL::Options::iOS;
#else
			options.platform = CompilerMSL::Options::macOS;
#endif

			msl.set_msl_options(options);

			std::string source = msl.compile();
//			printf("MSL SOURCE for stage %d:\n\n%s\n\n", i, source.c_str());

			NSString *nssource = [[NSString alloc] initWithBytes:source.c_str()
														  length:source.length()
														encoding:NSUTF8StringEncoding];

			NSError *err = nil;
			id<MTLLibrary> library = [device newLibraryWithSource:nssource options:nil error:&err];
			if (library == nil && err != nil)
			{
				NSLog(@"errors: %@", err);
				throw love::Exception("Error compiling converted Metal shader code");
			}

			functions[i] = [library newFunctionWithName:library.functionNames[0]];

			for (const auto &var : interfacevars)
			{
				spv::StorageClass storage = msl.get_storage_class(var);
				const std::string &name = msl.get_name(var);

				if (i == ShaderStage::STAGE_VERTEX && storage == spv::StorageClassInput)
					attributes[name] = msl.get_decoration(var, spv::DecorationLocation);
			}
		}
		catch (std::exception &e)
		{
			printf("Error parsing SPIR-V shader source: %s\n", e.what());
		}
	}
}}

Shader::~Shader()
{ @autoreleasepool {
	for (int i = 0; i < ShaderStage::STAGE_MAX_ENUM; i++)
		functions[i] = nil;

	for (const auto &kvp : cachedRenderPipelines)
		CFBridgingRelease(kvp.second);

	cachedRenderPipelines.clear();

	for (const auto &it : uniforms)
	{
		free(it.second.data);
		if (it.second.textures != nullptr)
			delete[] it.second.textures;
	}

	delete[] localUniformBufferData;
}}

void Shader::attach()
{
	if (current != this)
	{
		Graphics::flushBatchedDrawsGlobal();
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

id<MTLRenderPipelineState> Shader::getCachedRenderPipeline(const RenderPipelineKey &key)
{
	auto it = cachedRenderPipelines.find(key);

	if (it != cachedRenderPipelines.end())
		return (__bridge id<MTLRenderPipelineState>) it->second;

	id<MTLDevice> device = Graphics::getInstance()->device;

	MTLRenderPipelineDescriptor *desc = [MTLRenderPipelineDescriptor new];

	desc.vertexFunction = functions[ShaderStage::STAGE_VERTEX];
	desc.fragmentFunction = functions[ShaderStage::STAGE_PIXEL];

	desc.rasterSampleCount = std::max((int) key.msaa, 1);

	for (int i = 0; i < MAX_COLOR_RENDER_TARGETS; i++)
	{
		PixelFormat format = (PixelFormat)((key.colorRenderTargetFormats >> (i * 8)) & 0xFF);
		if (format == PIXELFORMAT_UNKNOWN)
			continue;

		MTLRenderPipelineColorAttachmentDescriptor *attachment = desc.colorAttachments[i];

		bool isSRGB = false;
		auto formatdesc = Metal::convertPixelFormat(format, isSRGB);
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

	// TODO: depth/stencil attachment formats
	

	{
		MTLVertexDescriptor *vertdesc = [MTLVertexDescriptor vertexDescriptor];
		const auto &attributes = key.vertexAttributes;

		for (const auto &pair : this->attributes)
		{
			int i = pair.second;
			uint32 bit = 1u << i;

			if (attributes.enableBits & bit)
			{
				const auto &attrib = attributes.attribs[i];
				int metalBufferIndex = attrib.bufferIndex + VERTEX_BUFFER_BINDING_START;

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
	}

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
