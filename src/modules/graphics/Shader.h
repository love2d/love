/**
 * Copyright (c) 2006-2021 LOVE Development Team
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

namespace love
{
namespace graphics
{

class Graphics;
class Buffer;

// A GLSL shader
class Shader : public Object, public Resource
{
public:

	static love::Type type;

	enum Language
	{
		LANGUAGE_GLSL1,
		LANGUAGE_GLSL3,
		LANGUAGE_GLSL4,
		LANGUAGE_MAX_ENUM
	};

	// Built-in uniform variables.
	enum BuiltinUniform
	{
		BUILTIN_TEXTURE_MAIN,
		BUILTIN_TEXTURE_VIDEO_Y,
		BUILTIN_TEXTURE_VIDEO_CB,
		BUILTIN_TEXTURE_VIDEO_CR,
		BUILTIN_UNIFORMS_PER_DRAW,
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
		UNIFORM_TEXELBUFFER,
		UNIFORM_STORAGEBUFFER,
		UNIFORM_UNKNOWN,
		UNIFORM_MAX_ENUM
	};

	enum StandardShader
	{
		STANDARD_DEFAULT,
		STANDARD_VIDEO,
		STANDARD_ARRAY,
		STANDARD_POINTS,
		STANDARD_MAX_ENUM
	};

	enum EntryPoint
	{
		ENTRYPOINT_NONE,
		ENTRYPOINT_HIGHLEVEL,
		ENTRYPOINT_CUSTOM,
		ENTRYPOINT_RAW,
	};

	struct SourceInfo
	{
		Language language;
		EntryPoint stages[SHADERSTAGE_MAX_ENUM];
		bool usesMRT;
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
		DataBaseType texelBufferType;
		bool isDepthSampler;
		size_t bufferStride;
		size_t bufferMemberCount;
		std::string name;

		union
		{
			void *data;
			float *floats;
			int *ints;
			unsigned int *uints;
		};

		size_t dataSize;

		union
		{
			Texture **textures;
			Buffer **buffers;
		};
	};

	// The members in here must respect uniform buffer alignment/padding rules.
 	struct BuiltinUniformData
 	{
 		Matrix4 transformMatrix;
 		Matrix4 projectionMatrix;
 		Vector4 normalMatrix[3]; // 3x3 matrix padded to an array of 3 vector4s.
 		Vector4 screenSizeParams;
 		Colorf constantColor;
 	};

	// Pointer to currently active Shader.
	static Shader *current;

	// Pointer to the default Shader.
	static Shader *standardShaders[STANDARD_MAX_ENUM];

	Shader(StrongRef<ShaderStage> stages[]);
	virtual ~Shader();

	/**
	 * Check whether a Shader has a stage.
	 **/
	bool hasStage(ShaderStageType stage);

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
	virtual void sendBuffers(const UniformInfo *info, Buffer **buffers, int count) = 0;

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
	void validateDrawState(PrimitiveType primtype, Texture *maintexture) const;

	static SourceInfo getSourceInfo(const std::string &src);
	static std::string createShaderStageCode(Graphics *gfx, ShaderStageType stage, const std::string &code, const SourceInfo &info);

	static bool validate(StrongRef<ShaderStage> stages[], std::string &err);

	static bool initialize();
	static void deinitialize();

	static const std::string &getDefaultCode(StandardShader shader, ShaderStageType stage);

	static bool getConstant(const char *in, Language &out);
	static bool getConstant(Language in, const char *&out);

	static bool getConstant(const char *in, BuiltinUniform &out);
	static bool getConstant(BuiltinUniform in, const char *&out);

protected:

	struct BufferReflection
	{
		size_t stride;
		size_t memberCount;
	};

	struct ValidationReflection
	{
		std::map<std::string, BufferReflection> storageBuffers;
		bool usesPointSize;
	};

	static bool validateInternal(StrongRef<ShaderStage> stages[], std::string& err, ValidationReflection &reflection);

	StrongRef<ShaderStage> stages[SHADERSTAGE_MAX_ENUM];

	ValidationReflection validationReflection;

}; // Shader

} // graphics
} // love
