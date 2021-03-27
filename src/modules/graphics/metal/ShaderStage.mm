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

#include "ShaderStage.h"

#include "libraries/glslang/glslang/Public/ShaderLang.h"

// TODO: Use love.graphics to determine actual limits?
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

namespace love
{
namespace graphics
{
namespace metal
{

ShaderStage::ShaderStage(love::graphics::Graphics *gfx, StageType stage, const std::string &source, bool gles, const std::string &cachekey)
	: love::graphics::ShaderStage(gfx, stage, source, gles, cachekey)
{
	using namespace glslang;

	EShLanguage glslangStage = EShLangCount;
	if (stage == STAGE_VERTEX)
		glslangStage = EShLangVertex;
	else if (stage == STAGE_PIXEL)
		glslangStage = EShLangFragment;
	else
		throw love::Exception("Cannot compile shader stage: unknown stage type.");

	glslangShader = new TShader(glslangStage);

	// We can't reuse the validation glslang shader object in the base class,
	// because we need these options set (and the language set to >= 300).
	glslangShader->setEnvInput(EShSourceGlsl, glslangStage, EShClientVulkan, 450);
	glslangShader->setEnvClient(EShClientVulkan, EShTargetVulkan_1_2);
	glslangShader->setEnvTarget(EShTargetSpv, EShTargetSpv_1_5);
	glslangShader->setAutoMapLocations(true);
	glslangShader->setAutoMapBindings(true);

	// Needed for local uniforms to work (they will be converted into a uniform
	// block).
	// https://github.com/KhronosGroup/GLSL/blob/master/extensions/ext/GL_EXT_vulkan_glsl_relaxed.txt
	glslangShader->setEnvInputVulkanRulesRelaxed();

	const char *csrc = source.c_str();
	int srclen = (int) source.length();
	glslangShader->setStringsWithLengths(&csrc, &srclen, 1);

	int defaultversion = gles ? 300 : 330;
	EProfile defaultprofile = gles ? EEsProfile : ECoreProfile;
	bool forcedefault = false;
	bool forwardcompat = true;

	if (!glslangShader->parse(&defaultTBuiltInResource, defaultversion, defaultprofile, forcedefault, forwardcompat, EShMsgSuppressWarnings))
	{
		const char *stagename = "unknown";
		getConstant(stage, stagename);

		std::string err = "Error parsing " + std::string(stagename) + " shader:\n\n"
			+ std::string(glslangShader->getInfoLog()) + "\n"
			+ std::string(glslangShader->getInfoDebugLog());

		delete glslangShader;
		throw love::Exception("%s", err.c_str());
	}
}

ShaderStage::~ShaderStage()
{
	delete glslangShader;
}

} // metal
} // graphics
} // love
