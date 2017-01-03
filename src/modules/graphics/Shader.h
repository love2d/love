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

#pragma once

// LOVE
#include "common/Object.h"
#include "common/StringMap.h"
#include "Texture.h"

// STL
#include <string>
#include <map>
#include <vector>
#include <stddef.h>

namespace love
{
namespace graphics
{

// A GLSL shader
class Shader : public Object
{
public:

	static love::Type type;

	enum ShaderStage
	{
		STAGE_VERTEX,
		STAGE_PIXEL,
		STAGE_MAX_ENUM
	};

	// Built-in uniform variables.
	enum BuiltinUniform
	{
		BUILTIN_TRANSFORM_MATRIX = 0,
		BUILTIN_PROJECTION_MATRIX,
		BUILTIN_TRANSFORM_PROJECTION_MATRIX,
		BUILTIN_NORMAL_MATRIX,
		BUILTIN_POINT_SIZE,
		BUILTIN_SCREEN_SIZE,
		BUILTIN_VIDEO_Y_CHANNEL,
		BUILTIN_VIDEO_CB_CHANNEL,
		BUILTIN_VIDEO_CR_CHANNEL,
		BUILTIN_MAX_ENUM
	};

	// Types of potential uniform variables used in love's shaders.
	enum UniformType
	{
		UNIFORM_FLOAT,
		UNIFORM_MATRIX,
		UNIFORM_INT,
		UNIFORM_UINT,
		UNIFORM_BOOL,
		UNIFORM_SAMPLER,
		UNIFORM_UNKNOWN,
		UNIFORM_MAX_ENUM
	};

	struct ShaderSource
	{
		std::string vertex;
		std::string pixel;
	};

	struct MatrixSize
	{
		short columns;
		short rows;
	};

	struct UniformInfo
	{
		int location;
		int count;

		union
		{
			int components;
			MatrixSize matrix;
		};

		UniformType baseType;
		std::string name;

		union
		{
			void *data;
			float *floats;
			int *ints;
			unsigned int *uints;
		};

		Texture **textures;
	};

	// Pointer to currently active Shader.
	static Shader *current;

	// Pointer to the default Shader.
	static Shader *defaultShader;
	static Shader *defaultVideoShader;

	virtual ~Shader();

	/**
	 * Binds this Shader's program to be used when rendering.
	 *
	 * @param temporary True if we just want to send values to the shader with
	 * no intention of rendering.
	 **/
	virtual void attach(bool temporary = false) = 0;

	/**
	 * Attach the default shader.
	 **/
	static void attachDefault();

	/**
	 * Returns any warnings this Shader may have generated.
	 **/
	virtual std::string getWarnings() const = 0;

	virtual const UniformInfo *getUniformInfo(const std::string &name) const = 0;
	virtual void updateUniform(const UniformInfo *info, int count, bool internalUpdate = false) = 0;

	virtual void sendTextures(const UniformInfo *info, Texture **textures, int count, bool internalUpdate = false) = 0;

	/**
	 * Gets whether a uniform with the specified name exists and is actively
	 * used in the shader.
	 **/
	virtual bool hasUniform(const std::string &name) const = 0;

	/**
	 * Sets the textures used when rendering a video. For internal use only.
	 **/
	virtual void setVideoTextures(ptrdiff_t ytexture, ptrdiff_t cbtexture, ptrdiff_t crtexture) = 0;

	static bool getConstant(const char *in, ShaderStage &out);
	static bool getConstant(ShaderStage in, const char *&out);

	static bool getConstant(const char *in, UniformType &out);
	static bool getConstant(UniformType in, const char *&out);

	static bool getConstant(const char *in, VertexAttribID &out);
	static bool getConstant(VertexAttribID in, const char *&out);

	static bool getConstant(const char *in, BuiltinUniform &out);
	static bool getConstant(BuiltinUniform in, const char *&out);

private:

	static StringMap<ShaderStage, STAGE_MAX_ENUM>::Entry stageNameEntries[];
	static StringMap<ShaderStage, STAGE_MAX_ENUM> stageNames;
	
	static StringMap<UniformType, UNIFORM_MAX_ENUM>::Entry uniformTypeEntries[];
	static StringMap<UniformType, UNIFORM_MAX_ENUM> uniformTypes;
	
	// Names for the generic vertex attributes used by love.
	static StringMap<VertexAttribID, ATTRIB_MAX_ENUM>::Entry attribNameEntries[];
	static StringMap<VertexAttribID, ATTRIB_MAX_ENUM> attribNames;
	
	// Names for the built-in uniform variables.
	static StringMap<BuiltinUniform, BUILTIN_MAX_ENUM>::Entry builtinNameEntries[];
	static StringMap<BuiltinUniform, BUILTIN_MAX_ENUM> builtinNames;

}; // Shader

} // graphics
} // love
