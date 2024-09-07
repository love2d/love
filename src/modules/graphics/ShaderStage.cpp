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

#include "ShaderStage.h"
#include "common/Exception.h"
#include "Graphics.h"

#include "libraries/glslang/glslang/Public/ShaderLang.h"
#include "libraries/glslang/glslang/Public/ResourceLimits.h"

namespace love
{
namespace graphics
{

ShaderStage::ShaderStage(Graphics */*gfx*/, ShaderStageType stage, const std::string &glsl, bool gles, const std::string &cachekey)
	: stageType(stage)
	, source(glsl)
	, cacheKey(cachekey)
	, glslangValidationShader(nullptr)
{
	EShLanguage glslangStage = EShLangCount;
	if (stage == SHADERSTAGE_VERTEX)
		glslangStage = EShLangVertex;
	else if (stage == SHADERSTAGE_PIXEL)
		glslangStage = EShLangFragment;
	else if (stage == SHADERSTAGE_COMPUTE)
		glslangStage = EShLangCompute;
	else
		throw love::Exception("Cannot compile shader stage: unknown stage type.");

	auto glslangShader = new glslang::TShader(glslangStage);

	int defaultversion = gles ? 300 : 330;
	EProfile defaultprofile = gles ? EEsProfile : ECoreProfile;

	const char *csrc = glsl.c_str();
	int srclen = (int) glsl.length();
	glslangShader->setStringsWithLengths(&csrc, &srclen, 1);

	bool forcedefault = false;
	bool forwardcompat = true;

	if (!glslangShader->parse(GetResources(), defaultversion, defaultprofile, forcedefault, forwardcompat, (EShMessages)(EShMsgSuppressWarnings | EshMsgOverlappingLocations)))
	{
		const char *stagename = "unknown";
		getConstant(stage, stagename);

		std::string err = "Error validating " + std::string(stagename) + " shader:\n\n"
			+ std::string(glslangShader->getInfoLog()) + "\n"
			+ std::string(glslangShader->getInfoDebugLog());

		delete glslangShader;
		throw love::Exception("%s", err.c_str());
	}

	glslangValidationShader = glslangShader;
}

ShaderStage::~ShaderStage()
{
	if (!cacheKey.empty())
	{
		auto gfx = Module::getInstance<Graphics>(Module::M_GRAPHICS);
		if (gfx != nullptr)
			gfx->cleanupCachedShaderStage(stageType, cacheKey);
	}

	delete glslangValidationShader;
}

bool ShaderStage::getConstant(const char *in, ShaderStageType &out)
{
	return stageNames.find(in, out);
}

bool ShaderStage::getConstant(ShaderStageType in, const char *&out)
{
	return stageNames.find(in, out);
}

const char *ShaderStage::getConstant(ShaderStageType in)
{
	const char *name = nullptr;
	getConstant(in, name);
	return name;
}

StringMap<ShaderStageType, SHADERSTAGE_MAX_ENUM>::Entry ShaderStage::stageNameEntries[] =
{
	{ "vertex",  SHADERSTAGE_VERTEX  },
	{ "pixel",   SHADERSTAGE_PIXEL   },
	{ "compute", SHADERSTAGE_COMPUTE },
};

StringMap<ShaderStageType, SHADERSTAGE_MAX_ENUM> ShaderStage::stageNames(ShaderStage::stageNameEntries, sizeof(ShaderStage::stageNameEntries));

} // graphics
} // love
