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

// LOVE
#include "Shader.h"
#include "Graphics.h"
#include "math/MathModule.h"

// glslang
#include "libraries/glslang/glslang/Public/ShaderLang.h"

// C++
#include <string>

namespace love
{
namespace graphics
{

love::Type Shader::type("Shader", &Object::type);

Shader *Shader::current = nullptr;
Shader *Shader::standardShaders[Shader::STANDARD_MAX_ENUM] = {nullptr};

Shader::Shader(ShaderStage *vertex, ShaderStage *pixel)
	: stages()
{
	std::string err;
	if (!validate(vertex, pixel, err))
		throw love::Exception("%s", err.c_str());

	stages[ShaderStage::STAGE_VERTEX] = vertex;
	stages[ShaderStage::STAGE_PIXEL] = pixel;
}

Shader::~Shader()
{
	for (int i = 0; i < STANDARD_MAX_ENUM; i++)
	{
		if (this == standardShaders[i])
			standardShaders[i] = nullptr;
	}

	if (current == this)
		attachDefault(STANDARD_DEFAULT);
}

void Shader::attachDefault(StandardShader defaultType)
{
	Shader *defaultshader = standardShaders[defaultType];

	if (defaultshader == nullptr)
	{
		current = nullptr;
		return;
	}

	if (current != defaultshader)
		defaultshader->attach();
}

bool Shader::isDefaultActive()
{
	for (int i = 0; i < STANDARD_MAX_ENUM; i++)
	{
		if (current == standardShaders[i])
			return true;
	}

	return false;
}

TextureType Shader::getMainTextureType() const
{
	const UniformInfo *info = getUniformInfo(BUILTIN_TEXTURE_MAIN);
	return info != nullptr ? info->textureType : TEXTURE_MAX_ENUM;
}

void Shader::checkMainTextureType(TextureType textype, bool isDepthSampler) const
{
	const UniformInfo *info = getUniformInfo(BUILTIN_TEXTURE_MAIN);

	if (info == nullptr)
		return;

	if (info->textureType != TEXTURE_MAX_ENUM && info->textureType != textype)
	{
		const char *textypestr = "unknown";
		const char *shadertextypestr = "unknown";
		Texture::getConstant(textype, textypestr);
		Texture::getConstant(info->textureType, shadertextypestr);
		throw love::Exception("Texture's type (%s) must match the type of the shader's main texture type (%s).", textypestr, shadertextypestr);
	}

	if (info->isDepthSampler != isDepthSampler)
	{
		if (info->isDepthSampler)
			throw love::Exception("Depth comparison samplers in shaders can only be used with depth textures which have depth comparison set.");
		else
			throw love::Exception("Depth textures which have depth comparison set can only be used with depth/shadow samplers in shaders.");
	}
}

void Shader::checkMainTexture(Texture *tex) const
{
	if (!tex->isReadable())
		throw love::Exception("Textures with non-readable formats cannot be sampled from in a shader.");

	checkMainTextureType(tex->getTextureType(), tex->getDepthSampleMode().hasValue);
}

bool Shader::validate(ShaderStage *vertex, ShaderStage *pixel, std::string &err)
{
	glslang::TProgram program;

	if (vertex != nullptr)
		program.addShader(vertex->getGLSLangShader());

	if (pixel != nullptr)
		program.addShader(pixel->getGLSLangShader());

	if (!program.link(EShMsgDefault))
	{
		err = "Cannot compile shader:\n\n" + std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog());
		return false;
	}

	return true;
}

bool Shader::initialize()
{
	return glslang::InitializeProcess();
}

void Shader::deinitialize()
{
	glslang::FinalizeProcess();
}

bool Shader::getConstant(const char *in, Language &out)
{
	return languages.find(in, out);
}

bool Shader::getConstant(Language in, const char *&out)
{
	return languages.find(in, out);
}

bool Shader::getConstant(const char *in, BuiltinUniform &out)
{
	return builtinNames.find(in, out);
}

bool Shader::getConstant(BuiltinUniform in, const char *&out)
{
	return builtinNames.find(in, out);
}

StringMap<Shader::Language, Shader::LANGUAGE_MAX_ENUM>::Entry Shader::languageEntries[] =
{
	{ "glsl1", LANGUAGE_GLSL1 },
	{ "essl1", LANGUAGE_ESSL1 },
	{ "glsl3", LANGUAGE_GLSL3 },
	{ "essl3", LANGUAGE_ESSL3 },
};

StringMap<Shader::Language, Shader::LANGUAGE_MAX_ENUM> Shader::languages(Shader::languageEntries, sizeof(Shader::languageEntries));

StringMap<Shader::BuiltinUniform, Shader::BUILTIN_MAX_ENUM>::Entry Shader::builtinNameEntries[] =
{
	{ "MainTex",             BUILTIN_TEXTURE_MAIN                  },
	{ "love_VideoYChannel",  BUILTIN_TEXTURE_VIDEO_Y               },
	{ "love_VideoCbChannel", BUILTIN_TEXTURE_VIDEO_CB              },
	{ "love_VideoCrChannel", BUILTIN_TEXTURE_VIDEO_CR              },
	{ "ViewSpaceFromLocal",  BUILTIN_MATRIX_VIEW_FROM_LOCAL        },
	{ "ClipSpaceFromView",   BUILTIN_MATRIX_CLIP_FROM_VIEW         },
	{ "ClipSpaceFromLocal",  BUILTIN_MATRIX_CLIP_FROM_LOCAL        },
	{ "ViewNormalFromLocal", BUILTIN_MATRIX_VIEW_NORMAL_FROM_LOCAL },
	{ "love_PointSize",      BUILTIN_POINT_SIZE                    },
	{ "love_ScreenSize",     BUILTIN_SCREEN_SIZE                   },
};

StringMap<Shader::BuiltinUniform, Shader::BUILTIN_MAX_ENUM> Shader::builtinNames(Shader::builtinNameEntries, sizeof(Shader::builtinNameEntries));

} // graphics
} // love
