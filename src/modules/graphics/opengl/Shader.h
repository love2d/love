/**
 * Copyright (c) 2006-2013 LOVE Development Team
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
#include "OpenGL.h"
#include "Texture.h"

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

	// Pointer to currently active Shader.
	static Shader *current;

	enum ShaderType
	{
		TYPE_VERTEX,
		TYPE_PIXEL,
		TYPE_MAX_ENUM
	};

	// Built-in extern (uniform) variables.
	enum BuiltinExtern
	{
		BUILTIN_SCREEN_PARAMS,
		BUILTIN_MAX_ENUM
	};

	// Type for a list of shader source codes in the form of sources[shadertype] = code
	typedef std::map<ShaderType, std::string> ShaderSources;

	/**
	 * Creates a new Shader using a list of source codes.
	 * Sources must contain either vertex or pixel shader code, or both.
	 **/
	Shader(const ShaderSources &sources);

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
	 * Internal use only.
	 **/
	bool hasBuiltinExtern(BuiltinExtern builtin) const;
	bool sendBuiltinFloat(BuiltinExtern builtin, int size, const GLfloat *m, int count);
	void checkSetScreenParams();

	static std::string getGLSLVersion();
	static bool isSupported();

private:

	// Types of potential uniform variables used in love's shaders.
	enum UniformType
	{
		UNIFORM_FLOAT,
		UNIFORM_INT,
		UNIFORM_BOOL,
		UNIFORM_SAMPLER,
		UNIFORM_UNKNOWN
	};

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

	GLuint compileCode(ShaderType type, const std::string &code);
	void createProgram(const std::vector<GLuint> &shaderids);

	int getTextureUnit(const std::string &name);

	void retainObject(const std::string &name, Object *object);

	// Get any warnings or errors generated only by the shader program object.
	std::string getProgramWarnings() const;

	// List of all shader code attached to this Shader
	ShaderSources shaderSources;

	// Shader compiler warning strings for individual shader stages.
	std::map<ShaderType, std::string> shaderWarnings;

	// volatile
	GLuint program;

	// Location values for any built-in uniform variables.
	GLint builtinUniforms[BUILTIN_MAX_ENUM];

	// Uniform location buffer map
	std::map<std::string, Uniform> uniforms;

	// Texture unit pool for setting images
	std::map<std::string, GLint> texUnitPool; // texUnitPool[name] = textureunit
	std::vector<GLuint> activeTexUnits; // activeTexUnits[textureunit-1] = textureid

	// Uniform name to retainable objects
	std::map<std::string, Object*> boundRetainables;

	// Pointer to the active Canvas when the screen params were last checked.
	Canvas *lastCanvas;

	// Max GPU texture units available for sent images
	static GLint maxTexUnits;

	// Counts total number of textures bound to each texture unit in all shaders
	static std::vector<int> textureCounters;

	static StringMap<ShaderType, TYPE_MAX_ENUM>::Entry typeNameEntries[];
	static StringMap<ShaderType, TYPE_MAX_ENUM> typeNames;

	// Names for the built-in uniform variables.
	static StringMap<BuiltinExtern, BUILTIN_MAX_ENUM>::Entry builtinNameEntries[];
	static StringMap<BuiltinExtern, BUILTIN_MAX_ENUM> builtinNames;
};

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_SHADER_H
