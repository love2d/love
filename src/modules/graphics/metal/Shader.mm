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

static MTLVertexFormat getMTLVertexFormat(vertex::DataType type, int components)
{
	switch (type)
	{
	case vertex::DATA_UNORM8:
		if (components == 1)
			return MTLVertexFormatUCharNormalized;
		else if (components == 2)
			return MTLVertexFormatUChar2Normalized;
		else if (components == 3)
			return MTLVertexFormatUChar3Normalized;
		else
			return MTLVertexFormatUChar4Normalized;
	case vertex::DATA_UNORM16:
		if (components == 1)
			return MTLVertexFormatUShortNormalized;
		else if (components == 2)
			return MTLVertexFormatUShort2Normalized;
		else if (components == 3)
			return MTLVertexFormatUShort3Normalized;
		else
			return MTLVertexFormatUShort4Normalized;
	case vertex::DATA_FLOAT:
		if (components == 1)
			return MTLVertexFormatFloat;
		else if (components == 2)
			return MTLVertexFormatFloat2;
		else if (components == 3)
			return MTLVertexFormatFloat3;
		else
			return MTLVertexFormatFloat4;
	}

	// TODO
	return MTLVertexFormatFloat4;
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

Shader::Shader(love::graphics::ShaderStage *vertex, love::graphics::ShaderStage *pixel)
	: love::graphics::Shader(vertex, pixel)
	, functions()
{ @autoreleasepool {
	auto gfx = Graphics::getInstance();

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

			CompilerMSL::Options options;

#ifdef LOVE_IOS
			options.platform = CompilerMSL::Options::iOS;
#else
			options.platform = CompilerMSL::Options::macOS;
#endif

			msl.set_msl_options(options);

			std::string source = msl.compile();
//			printf("MSL SOURCE:\n\n%s\n\n", source.c_str());

			NSString *nssource = [[NSString alloc] initWithBytes:source.c_str()
														  length:source.length()
														encoding:NSUTF8StringEncoding];

			NSError *err = nil;
			id<MTLLibrary> library = [gfx->device newLibraryWithSource:nssource options:nil error:&err];
			if (library == nil && err != nil)
			{
				NSLog(@"errors: %@", err);
				throw love::Exception("Error compiling converted Metal shader code");
			}

			functions[i] = [library newFunctionWithName:library.functionNames[0]];
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
}}

void Shader::attach()
{
	if (current != this)
	{
		Graphics::flushBatchedDrawsGlobal();
		current = this;
	}
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
		MTLPixelFormat metalformat = Metal::convertPixelFormat(format, isSRGB);

		attachment.pixelFormat = metalformat;

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

	{
		MTLVertexDescriptor *vertdesc = [MTLVertexDescriptor vertexDescriptor];

		const auto &attributes = key.vertexAttributes;
		uint32 allbits = attributes.enableBits;
		uint32 i = 0;
		while (allbits)
		{
			uint32 bit = 1u << i;

			if (attributes.enableBits & bit)
			{
				const auto &attrib = attributes.attribs[i];

				vertdesc.attributes[i].format = getMTLVertexFormat(attrib.type, attrib.components);
				vertdesc.attributes[i].offset = attrib.offsetFromVertex;
				vertdesc.attributes[i].bufferIndex = attrib.bufferIndex;

				const auto &layout = attributes.bufferLayouts[attrib.bufferIndex];

				bool instanced = attributes.instanceBits & (1u << attrib.bufferIndex);
				auto step = instanced ? MTLVertexStepFunctionPerInstance : MTLVertexStepFunctionPerVertex;

				vertdesc.layouts[attrib.bufferIndex].stride = layout.stride;
				vertdesc.layouts[attrib.bufferIndex].stepFunction = step;
			}

			i++;
			allbits >>= 1;
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

} // metal
} // graphics
} // love
