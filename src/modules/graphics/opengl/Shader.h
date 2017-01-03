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
#include "graphics/Shader.h"
#include "graphics/Graphics.h"
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
class Shader final : public love::graphics::Shader, public Volatile
{
public:

	/**
	 * Creates a new Shader using a list of source codes.
	 * Source must contain either vertex or pixel shader code, or both.
	 **/
	Shader(const ShaderSource &source);

	virtual ~Shader();

	// Implements Volatile
	bool loadVolatile() override;
	void unloadVolatile() override;

	// Implements Shader.
	void attach(bool temporary = false) override;
	std::string getWarnings() const override;
	const UniformInfo *getUniformInfo(const std::string &name) const override;
	void updateUniform(const UniformInfo *info, int count, bool internalUpdate = false) override;
	void sendTextures(const UniformInfo *info, Texture **textures, int count, bool internalUpdate = false) override;
	bool hasUniform(const std::string &name) const override;
	void setVideoTextures(ptrdiff_t ytexture, ptrdiff_t cbtexture, ptrdiff_t crtexture) override;

	GLint getAttribLocation(const std::string &name);

	void updateScreenParams();
	void updatePointSize(float size);
	void updateBuiltinUniforms();

	GLuint getProgram() const
	{
		return program;
	}

	static std::string getGLSLVersion();
	static bool isSupported();

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

}; // Shader

} // opengl
} // graphics
} // love
