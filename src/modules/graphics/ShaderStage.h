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

#pragma once

#include "common/Object.h"
#include "common/StringMap.h"
#include "Resource.h"

#include <stddef.h>
#include <string>

namespace glslang
{
class TShader;
}

namespace love
{
namespace graphics
{

class Graphics;

// Order is used for stages array in ShaderStage.cpp
enum ShaderStageType
{
	SHADERSTAGE_VERTEX,
	SHADERSTAGE_PIXEL,
	SHADERSTAGE_COMPUTE,
	SHADERSTAGE_MAX_ENUM
};

enum ShaderStageMask
{
	SHADERSTAGEMASK_NONE = 0,
	SHADERSTAGEMASK_VERTEX = 1 << SHADERSTAGE_VERTEX,
	SHADERSTAGEMASK_PIXEL = 1 << SHADERSTAGE_PIXEL,
	SHADERSTAGEMASK_COMPUTE = 1 << SHADERSTAGE_COMPUTE,
};

class ShaderStage : public love::Object
{
public:

	ShaderStage(Graphics *gfx, ShaderStageType stage, const std::string &glsl, bool gles, const std::string &cachekey);
	virtual ~ShaderStage();

	virtual ptrdiff_t getHandle() const = 0;

	ShaderStageType getStageType() const { return stageType; }
	const std::string &getSource() const { return source; }
	const std::string &getWarnings() const { return warnings; }
	glslang::TShader *getGLSLangValidationShader() const { return glslangValidationShader; }

	static bool getConstant(const char *in, ShaderStageType &out);
	static bool getConstant(ShaderStageType in, const char *&out);
	static const char *getConstant(ShaderStageType in);

protected:

	std::string warnings;

private:

	ShaderStageType stageType;
	std::string source;
	std::string cacheKey;
	glslang::TShader *glslangValidationShader;

	static StringMap<ShaderStageType, SHADERSTAGE_MAX_ENUM>::Entry stageNameEntries[];
	static StringMap<ShaderStageType, SHADERSTAGE_MAX_ENUM> stageNames;

}; // ShaderStage

class ShaderStageForValidation final : public ShaderStage
{
public:

	ShaderStageForValidation(Graphics *gfx, ShaderStageType stage, const std::string &glsl, bool gles)
		: ShaderStage(gfx, stage, glsl, gles, "")
	{}
	virtual ~ShaderStageForValidation() {}
	ptrdiff_t getHandle() const override { return 0; }

}; // ShaderStageForValidation

} // graphics
} // love
