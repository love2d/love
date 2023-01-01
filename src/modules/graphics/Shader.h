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

// LOVE
#include "common/Object.h"
#include "common/StringMap.h"
#include "Texture.h"
#include "ShaderStage.h"
#include "Resource.h"

// STL
#include <string>
#include <map>
#include <vector>
#include <stddef.h>

namespace glslang
{
class TShader;
}

namespace love
{
namespace graphics
{

class Graphics;

// A GLSL shader
class Shader : public Object, public Resource
{
public:

	static love::Type type;

	enum Language
	{
		LANGUAGE_GLSL1,
		LANGUAGE_ESSL1,
		LANGUAGE_GLSL3,
		LANGUAGE_ESSL3,
		LANGUAGE_MAX_ENUM
	};

	// Built-in uniform variables.
	enum BuiltinUniform
	{
		BUILTIN_TEXTURE_MAIN,
		BUILTIN_TEXTURE_VIDEO_Y,
		BUILTIN_TEXTURE_VIDEO_CB,
		BUILTIN_TEXTURE_VIDEO_CR,
		BUILTIN_MATRIX_VIEW_FROM_LOCAL,
		BUILTIN_MATRIX_CLIP_FROM_VIEW,
		BUILTIN_MATRIX_CLIP_FROM_LOCAL,
		BUILTIN_MATRIX_VIEW_NORMAL_FROM_LOCAL,
		BUILTIN_POINT_SIZE,
		BUILTIN_SCREEN_SIZE,
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

	enum StandardShader
	{
		STANDARD_DEFAULT,
		STANDARD_VIDEO,
		STANDARD_ARRAY,
		STANDARD_MAX_ENUM
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
		TextureType textureType;
		bool isDepthSampler;
		std::string name;

		union
		{
			void *data;
			float *floats;
			int *ints;
			unsigned int *uints;
		};

		size_t dataSize;

		Texture **textures;
	};

	// Pointer to currently active Shader.
	static Shader *current;

	// Pointer to the default Shader.
	static Shader *standardShaders[STANDARD_MAX_ENUM];

	Shader(ShaderStage *vertex, ShaderStage *pixel);
	virtual ~Shader();

	/**
	 * Binds this Shader's program to be used when rendering.
	 **/
	virtual void attach() = 0;

	/**
	 * Attach a default shader.
	 **/
	static void attachDefault(StandardShader defaultType);

	/**
	 * Gets whether any of the default shaders are currently active.
	 **/
	static bool isDefaultActive();

	/**
	 * Returns any warnings this Shader may have generated.
	 **/
	virtual std::string getWarnings() const = 0;

	virtual int getVertexAttributeIndex(const std::string &name) = 0;

	virtual const UniformInfo *getUniformInfo(const std::string &name) const = 0;
	virtual const UniformInfo *getUniformInfo(BuiltinUniform builtin) const = 0;

	virtual void updateUniform(const UniformInfo *info, int count) = 0;

	virtual void sendTextures(const UniformInfo *info, Texture **textures, int count) = 0;

	/**
	 * Gets whether a uniform with the specified name exists and is actively
	 * used in the shader.
	 **/
	virtual bool hasUniform(const std::string &name) const = 0;

	/**
	 * Sets the textures used when rendering a video. For internal use only.
	 **/
	virtual void setVideoTextures(Texture *ytexture, Texture *cbtexture, Texture *crtexture) = 0;

	TextureType getMainTextureType() const;
	void checkMainTextureType(TextureType textype, bool isDepthSampler) const;
	void checkMainTexture(Texture *texture) const;

	static bool validate(ShaderStage *vertex, ShaderStage *pixel, std::string &err);

	static bool initialize();
	static void deinitialize();

	static bool getConstant(const char *in, Language &out);
	static bool getConstant(Language in, const char *&out);

	static bool getConstant(const char *in, BuiltinUniform &out);
	static bool getConstant(BuiltinUniform in, const char *&out);

protected:

	StrongRef<ShaderStage> stages[ShaderStage::STAGE_MAX_ENUM];

private:

	static StringMap<Language, LANGUAGE_MAX_ENUM>::Entry languageEntries[];
	static StringMap<Language, LANGUAGE_MAX_ENUM> languages;
	
	// Names for the built-in uniform variables.
	static StringMap<BuiltinUniform, BUILTIN_MAX_ENUM>::Entry builtinNameEntries[];
	static StringMap<BuiltinUniform, BUILTIN_MAX_ENUM> builtinNames;

}; // Shader

} // graphics
} // love
