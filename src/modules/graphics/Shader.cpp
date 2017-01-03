/**
 * Copyright (c) 2006-2017 LOVE Development Team
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

namespace love
{
namespace graphics
{

love::Type Shader::type("Shader", &Object::type);
Shader *Shader::current = nullptr;
Shader *Shader::defaultShader = nullptr;
Shader *Shader::defaultVideoShader = nullptr;

Shader::~Shader()
{
	if (defaultShader == this)
		defaultShader = nullptr;

	if (defaultVideoShader == this)
		defaultVideoShader = nullptr;

	if (current == this)
		attachDefault();
}

void Shader::attachDefault()
{
	if (defaultShader)
	{
		if (current != defaultShader)
			defaultShader->attach();

		return;
	}

	current = nullptr;
}

bool Shader::getConstant(const char *in, ShaderStage &out)
{
	return stageNames.find(in, out);
}

bool Shader::getConstant(ShaderStage in, const char *&out)
{
	return stageNames.find(in, out);
}

bool Shader::getConstant(const char *in, UniformType &out)
{
	return uniformTypes.find(in, out);
}

bool Shader::getConstant(UniformType in, const char *&out)
{
	return uniformTypes.find(in, out);
}

bool Shader::getConstant(const char *in, VertexAttribID &out)
{
	return attribNames.find(in, out);
}

bool Shader::getConstant(VertexAttribID in, const char *&out)
{
	return attribNames.find(in, out);
}

bool Shader::getConstant(const char *in, BuiltinUniform &out)
{
	return builtinNames.find(in, out);
}

bool Shader::getConstant(BuiltinUniform in, const char *&out)
{
	return builtinNames.find(in, out);
}

StringMap<Shader::ShaderStage, Shader::STAGE_MAX_ENUM>::Entry Shader::stageNameEntries[] =
{
	{"vertex", Shader::STAGE_VERTEX},
	{"pixel", Shader::STAGE_PIXEL},
};

StringMap<Shader::ShaderStage, Shader::STAGE_MAX_ENUM> Shader::stageNames(Shader::stageNameEntries, sizeof(Shader::stageNameEntries));

StringMap<Shader::UniformType, Shader::UNIFORM_MAX_ENUM>::Entry Shader::uniformTypeEntries[] =
{
	{"float", Shader::UNIFORM_FLOAT},
	{"matrix", Shader::UNIFORM_MATRIX},
	{"int", Shader::UNIFORM_INT},
	{"uint", Shader::UNIFORM_UINT},
	{"bool", Shader::UNIFORM_BOOL},
	{"image", Shader::UNIFORM_SAMPLER},
	{"unknown", Shader::UNIFORM_UNKNOWN},
};

StringMap<Shader::UniformType, Shader::UNIFORM_MAX_ENUM> Shader::uniformTypes(Shader::uniformTypeEntries, sizeof(Shader::uniformTypeEntries));

StringMap<VertexAttribID, ATTRIB_MAX_ENUM>::Entry Shader::attribNameEntries[] =
{
	{"VertexPosition", ATTRIB_POS},
	{"VertexTexCoord", ATTRIB_TEXCOORD},
	{"VertexColor", ATTRIB_COLOR},
	{"ConstantColor", ATTRIB_CONSTANTCOLOR},
};

StringMap<VertexAttribID, ATTRIB_MAX_ENUM> Shader::attribNames(Shader::attribNameEntries, sizeof(Shader::attribNameEntries));

StringMap<Shader::BuiltinUniform, Shader::BUILTIN_MAX_ENUM>::Entry Shader::builtinNameEntries[] =
{
	{"TransformMatrix", Shader::BUILTIN_TRANSFORM_MATRIX},
	{"ProjectionMatrix", Shader::BUILTIN_PROJECTION_MATRIX},
	{"TransformProjectionMatrix", Shader::BUILTIN_TRANSFORM_PROJECTION_MATRIX},
	{"NormalMatrix", Shader::BUILTIN_NORMAL_MATRIX},
	{"love_PointSize", Shader::BUILTIN_POINT_SIZE},
	{"love_ScreenSize", Shader::BUILTIN_SCREEN_SIZE},
	{"love_VideoYChannel", Shader::BUILTIN_VIDEO_Y_CHANNEL},
	{"love_VideoCbChannel", Shader::BUILTIN_VIDEO_CB_CHANNEL},
	{"love_VideoCrChannel", Shader::BUILTIN_VIDEO_CR_CHANNEL},
};

StringMap<Shader::BuiltinUniform, Shader::BUILTIN_MAX_ENUM> Shader::builtinNames(Shader::builtinNameEntries, sizeof(Shader::builtinNameEntries));

} // graphics
} // love
