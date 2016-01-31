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

class Canvas;

// A GLSL shader
class Shader : public Object, public Volatile
{
public:

	enum ShaderStage
	{
		STAGE_VERTEX,
		STAGE_PIXEL,
		STAGE_MAX_ENUM
	};

	// Built-in uniform (extern) variables.
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

	// Types of potential uniform (extern) variables used in love's shaders.
	enum UniformType
	{
		UNIFORM_FLOAT,
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

	// Pointer to currently active Shader.
	static Shader *current;

	// Pointer to the default Shader.
	static Shader *defaultShader;
	static Shader *defaultVideoShader;

	// Default shader code (a shader is always required internally.)
	static ShaderSource defaultCode[Graphics::RENDERER_MAX_ENUM];
	static ShaderSource defaultVideoCode[Graphics::RENDERER_MAX_ENUM];

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

	/**
	 * Send at least one integer or int-vector value to this Shader as a uniform.
	 *
	 * @param name The name of the uniform variable in the source code.
	 * @param size Number of elements in each vector to send.
	 *             A value of 1 indicates a single-component vector (an int).
	 * @param vec Pointer to the integer or int-vector values.
	 * @param count Number of integer or int-vector values.
	 **/
	void sendInt(const std::string &name, int size, const GLint *vec, int count);

	/**
	 * Send at least one float or vector value to this Shader as a uniform.
	 *
	 * @param name The name of the uniform variable in the source code.
	 * @param size Number of elements in each vector to send.
	 *             A value of 1 indicates a single-component vector (a float).
	 * @param vec Pointer to the float or float-vector values.
	 * @param count Number of float or float-vector values.
	 **/
	void sendFloat(const std::string &name, int size, const GLfloat *vec, int count);

	/**
	 * Send at least one matrix to this Shader as a uniform.
	 *
	 * @param name The name of the uniform variable in the source code.
	 * @param size Number of rows/columns in the matrix.
	 * @param m Pointer to the first element of the first matrix.
	 * @param count Number of matrices to send.
	 **/
	void sendMatrix(const std::string &name, int size, const GLfloat *m, int count);

	/**
	 * Send a texture to this Shader as a uniform.
	 *
	 * @param name The name of the uniform variable in the source code.
	 **/
	void sendTexture(const std::string &name, Texture *texture);

	/**
	 * Gets the type, number of components, and number of array elements of
	 * an active 'extern' (uniform) variable in the shader. If a uniform
	 * variable with the specified name doesn't exist, returns UNIFORM_UNKNOWN
	 * and sets the 'components' and 'count' values to 0.
	 *
	 * @param name The name of the uniform variable in the source code.
	 * @param[out] components Number of components of the variable (2 for vec2.)
	 * @param[out] count Number of array elements, if the variable is an array.
	 * @return The base type of the uniform variable.
	 **/
	UniformType getExternVariable(const std::string &name, int &components, int &count);

	GLint getAttribLocation(const std::string &name);

	/**
	 * Internal use only.
	 **/
	bool hasVertexAttrib(VertexAttribID attrib) const;

	void setVideoTextures(GLuint ytexture, GLuint cbtexture, GLuint crtexture);
	void checkSetScreenParams();
	void checkSetPointSize(float size);
	void checkSetBuiltinUniforms();

	const std::map<std::string, Object *> &getBoundRetainables() const;

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

	// Represents a single uniform/extern shader variable.
	struct Uniform
	{
		GLint location;
		GLint count;
		GLenum type;
		UniformType baseType;
		std::string name;
	};

	// Map active uniform names to their locations.
	void mapActiveUniforms();

	const Uniform &getUniform(const std::string &name) const;

	int getUniformTypeSize(GLenum type) const;
	UniformType getUniformBaseType(GLenum type) const;
	void checkSetUniformError(const Uniform &u, int size, int count, UniformType sendtype) const;

	GLuint compileCode(ShaderStage stage, const std::string &code);

	int getTextureUnit(const std::string &name);

	void retainObject(const std::string &name, Object *object);

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
	std::map<std::string, Uniform> uniforms;

	// Texture unit pool for setting images
	std::map<std::string, GLint> texUnitPool; // texUnitPool[name] = textureunit
	std::vector<GLuint> activeTexUnits; // activeTexUnits[textureunit-1] = textureid

	// Uniform name to retainable objects
	std::map<std::string, Object*> boundRetainables;

	// Pointer to the active Canvas when the screen params were last checked.
	Canvas *lastCanvas;
	OpenGL::Viewport lastViewport;

	float lastPointSize;

	Matrix4 lastTransformMatrix;
	Matrix4 lastProjectionMatrix;

	GLuint videoTextureUnits[3];

	// Counts total number of textures bound to each texture unit in all shaders
	static std::vector<int> textureCounters;

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
