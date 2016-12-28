/**
 * Copyright (c) 2006-2016 LOVE Development Team
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

#ifndef LOVE_GRAPHICS_SHADER_H
#define LOVE_GRAPHICS_SHADER_H

// LOVE
#include "common/Object.h"
#include "common/StringMap.h"
#include "graphics/Graphics.h"
#include "graphics/Texture.h"
#include "graphics/Volatile.h"
#include "OpenGL.h"

// STL
#include <string>
#include <map>
#include <vector>

namespace love
{
namespace graphics
{
namespace opengl
{

// A GLSL shader
class Shader : public Object, public Volatile
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
		};

		Texture **textures;
	};

	// Pointer to currently active Shader.
	static Shader *current;

	// Pointer to the default Shader.
	static Shader *defaultShader;
	static Shader *defaultVideoShader;

	// Default shader code (a shader is always required internally.)
	static ShaderSource defaultCode[Graphics::RENDERER_MAX_ENUM][2];
	static ShaderSource defaultVideoCode[Graphics::RENDERER_MAX_ENUM][2];

	/**
	 * Creates a new Shader using a list of source codes.
	 * Source must contain either vertex or pixel shader code, or both.
	 **/
	Shader(const ShaderSource &source);

	virtual ~Shader();

	// Implements Volatile
	virtual bool loadVolatile();
	virtual void unloadVolatile();

	/**
	 * Binds this Shader's program to be used when rendering.
	 *
	 * @param temporary True if we just want to send values to the shader with no intention of rendering.
	 **/
	void attach(bool temporary = false);

	/**
	 * Detach the currently bound Shader.
	 * Causes the GPU rendering pipeline to use fixed functionality in place of shader programs.
	 **/
	static void detach();

	/**
	 * Returns any warnings this Shader may have generated.
	 **/
	std::string getWarnings() const;

	const UniformInfo *getUniformInfo(const std::string &name) const;
	void updateUniform(const UniformInfo *info, int count, bool internalUpdate = false);

	void sendTextures(const UniformInfo *info, Texture **textures, int count, bool internalUpdate = false);

	/**
	 * Gets whether a uniform with the specified name exists and is actively
	 * used in the shader.
	 **/
	bool hasUniform(const std::string &name) const;

	GLint getAttribLocation(const std::string &name);

	/**
	 * Internal use only.
	 **/
	bool hasVertexAttrib(VertexAttribID attrib) const;

	void setVideoTextures(GLuint ytexture, GLuint cbtexture, GLuint crtexture);
	void checkSetScreenParams();
	void checkSetPointSize(float size);
	void checkSetBuiltinUniforms();

	GLuint getProgram() const
	{
		return program;
	}

	static std::string getGLSLVersion();
	static bool isSupported();

	static bool getConstant(const char *in, UniformType &out);
	static bool getConstant(UniformType in, const char *&out);

	static bool getConstant(const char *in, VertexAttribID &out);
	static bool getConstant(VertexAttribID in, const char *&out);

private:

	struct TextureUnit
	{
		GLuint texture = 0;
		bool active = false;
	};

	// Map active uniform names to their locations.
	void mapActiveUniforms();

	int getUniformTypeComponents(GLenum type) const;
	MatrixSize getMatrixSize(GLenum type) const;
	UniformType getUniformBaseType(GLenum type) const;

	GLuint compileCode(ShaderStage stage, const std::string &code);

	int getFreeTextureUnits(int count);

	void flushStreamDraws() const;

	// Get any warnings or errors generated only by the shader program object.
	std::string getProgramWarnings() const;

	// Source code used for this Shader.
	ShaderSource shaderSource;

	// Shader compiler warning strings for individual shader stages.
	std::map<ShaderStage, std::string> shaderWarnings;

	// volatile
	GLuint program;

	// Location values for any built-in uniform variables.
	GLint builtinUniforms[BUILTIN_MAX_ENUM];

	// Location values for any generic vertex attribute variables.
	GLint builtinAttributes[ATTRIB_MAX_ENUM];

	std::map<std::string, GLint> attributes;

	// Uniform location buffer map
	std::map<std::string, UniformInfo> uniforms;

	// Texture unit pool for setting images
	std::vector<TextureUnit> textureUnits;

	std::vector<std::pair<const UniformInfo *, int>> pendingUniformUpdates;

	bool canvasWasActive;
	Rect lastViewport;

	float lastPointSize;

	Matrix4 lastTransformMatrix;
	Matrix4 lastProjectionMatrix;

	GLuint videoTextureUnits[3];

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
};

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_SHADER_H
