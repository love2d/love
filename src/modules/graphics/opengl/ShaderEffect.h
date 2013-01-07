/**
 * Copyright (c) 2006-2012 LOVE Development Team
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

#ifndef LOVE_GRAPHICS_EFFECT_H
#define LOVE_GRAPHICS_EFFECT_H

#include "common/Object.h"
#include <string>
#include <map>
#include <vector>
#include "OpenGL.h"
#include "Image.h"
#include "Canvas.h"

namespace love
{
namespace graphics
{
namespace opengl
{
// A GLSL shader
class ShaderEffect : public Object, public Volatile
{
public:
	
	// Different types of shaders.
	// Only vertex and fragment shaders have guaranteed support in all ShaderEffects.
	enum ShaderType
	{
		TYPE_VERTEX,
		TYPE_TESSCONTROL,
		TYPE_TESSEVAL,
		TYPE_GEOMETRY,
		TYPE_FRAGMENT,
		TYPE_MAX_ENUM
	};

	// thin wrapper for GLSL source code.
	struct ShaderSource
	{
		std::string code;
		ShaderType type;
	};

	/**
	 * Creates a new ShaderEffect using a list of source codes.
	 * Must contain at least one vertex or fragment shader source.
	 * 
	 * @param shadersources Vector of shader source codes.
	 **/
	ShaderEffect(const std::vector<ShaderSource> &shadersources);

	virtual ~ShaderEffect();

	// Implements Volatile
	virtual bool loadVolatile();
	virtual void unloadVolatile();

	/**
	 * Binds this ShaderEffect's program to be used when rendering.
	 * 
	 * @param temporary True if we just want to send values to the shader with no intention of rendering.
	 **/
	void attach(bool temporary = false);

	/**
	 * Detach the currently bound ShaderEffect.
	 * Causes OpenGL to use fixed functionality in place of shader programs.
	 **/
	static void detach();

	/**
	 * Returns any warnings this ShaderEffect may have generated.
	 **/
	std::string getWarnings() const;

	/**
	 * Returns the maximum GLSL version supported on this system.
	 **/
	static std::string getGLSLVersion();

	/**
	 * Returns whether ShaderEffects are supported on this system.
	 **/
	static bool isSupported();

	/**
	 * Send at least one float or vector value to this ShaderEffect as a uniform.
	 * 
	 * @param name The name of the uniform variable in the source code.
	 * @param size Number of elements in each vector to send.
	 *             A value of 1 indicates a single-component vector, AKA a float.
	 * @param vec Pointer to the float or vector values.
	 * @param count Number of float or vector values.
	 **/
	void sendFloat(const std::string &name, int size, const GLfloat *vec, int count);

	/**
	 * Send at least one matrix to this ShaderEffect as a uniform.
	 * 
	 * @param name The name of the uniform variable in the source code.
	 * @param size Number of rows/columns in the matrix.
	 * @param m Pointer to the first element of the first matrix.
	 * @param count Number of matrices to send.
	 **/
	void sendMatrix(const std::string &name, int size, const GLfloat *m, int count);

	/**
	 * Send an image to this ShaderEffect as a uniform.
	 * 
	 * @param name The name of the uniform variable in the source code.
	 * @param image The image to send.
	 **/
	void sendImage(const std::string &name, const Image &image);

	/**
	 * Send a canvas to this ShaderEffect as a uniform.
	 *
	 * @param name The name of the uniform variable in the source code.
	 * @param canvas The canvas to send.
	 **/
	void sendCanvas(const std::string &name, const Canvas &canvas);

	// pointer to currently active ShaderEffect.
	static ShaderEffect *current;

private:

	GLint getUniformLocation(const std::string &name);
	void checkSetUniformError();
	GLuint createShader(const ShaderSource &source);
	void createProgram(const std::vector<GLuint> &shaderids);

	// list of all shader code attached to this ShaderEffect
	std::vector<ShaderSource> _shadersources;

	GLuint _program; // volatile

	// uniform location buffer map
	std::map<std::string, GLint> _uniforms;

	// total max GPU texture units for shaders
	static GLint _max_texture_units;

	// counts total number of textures bound to each texture unit in all shaders
	static std::vector<int> _texture_id_counters;

	// texture unit pool for setting images
	std::map<std::string, GLint> _texture_unit_pool;
	std::vector<GLuint> _texture_id_list;
	GLint getTextureUnit(const std::string &name);

	void sendTexture(const std::string &name, GLuint texture);
};

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_EFFECT_H
