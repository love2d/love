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

#pragma once

#include "common/Object.h"
#include "common/StringMap.h"
#include "Volatile.h"
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

class ShaderStage : public love::Object, public Volatile, public Resource
{
public:

	enum StageType
	{
		STAGE_VERTEX,
		STAGE_PIXEL,
		STAGE_MAX_ENUM
	};

	ShaderStage(Graphics *gfx, StageType stage, const std::string &glsl, bool gles, const std::string &cachekey);
	virtual ~ShaderStage();

	StageType getStageType() const { return stageType; }
	const std::string &getSource() const { return source; }
	const std::string &getWarnings() const { return warnings; }
	glslang::TShader *getGLSLangShader() const { return glslangShader; }

	static bool getConstant(const char *in, StageType &out);
	static bool getConstant(StageType in, const char *&out);

protected:

	std::string warnings;

private:

	StageType stageType;
	std::string source;
	std::string cacheKey;
	glslang::TShader *glslangShader;

	static StringMap<StageType, STAGE_MAX_ENUM>::Entry stageNameEntries[];
	static StringMap<StageType, STAGE_MAX_ENUM> stageNames;

}; // ShaderStage

class ShaderStageForValidation final : public ShaderStage
{
public:

	ShaderStageForValidation(Graphics *gfx, StageType stage, const std::string &glsl, bool gles)
		: ShaderStage(gfx, stage, glsl, gles, "")
	{}

	virtual ~ShaderStageForValidation() {}

	ptrdiff_t getHandle() const override { return 0; }
	bool loadVolatile() override { return true; }
	void unloadVolatile() override { }

}; // ShaderStageForValidation

} // graphics
} // love
