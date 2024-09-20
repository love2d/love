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

	struct StorageTextureBinding
	{
		love::graphics::Texture *texture = nullptr;
		GLuint gltexture = 0;
		TextureType type = TEXTURE_2D;
		GLenum access = GL_READ_ONLY;
		GLenum internalFormat;
	};

	Shader(StrongRef<love::graphics::ShaderStage> stages[SHADERSTAGE_MAX_ENUM], const CompileOptions &options);
	virtual ~Shader();

	// Implements Volatile
	bool loadVolatile() override;
	void unloadVolatile() override;

	// Implements Shader.
	void attach() override;
	std::string getWarnings() const override;
	int getVertexAttributeIndex(const std::string &name) override;
	const UniformInfo *getUniformInfo(BuiltinUniform builtin) const override;
	void updateUniform(const UniformInfo *info, int count) override;
	ptrdiff_t getHandle() const override;

	void updateBuiltinUniforms(love::graphics::Graphics *gfx, int viewportW, int viewportH);

	const std::vector<Buffer *> &getActiveWritableStorageBuffers() const { return activeWritableStorageBuffers; }
	const std::vector<StorageTextureBinding> &getStorageTextureBindings() const { return storageTextureBindings; }

private:

	struct TextureUnit
	{
		GLuint texture = 0;
		TextureType type = TEXTURE_2D;
		bool isTexelBuffer = false;
		bool active = false;
	};

	struct BufferBinding
	{
		int bindingindex = 0;
		GLuint buffer = 0;
	};

	// Map active uniform names to their locations.
	void mapActiveUniforms();

	void updateUniform(const UniformInfo *info, int count, bool internalupdate);

	void applyTexture(const UniformInfo *info, int i, love::graphics::Texture *texture, UniformType basetype, bool isdefault) override;
	void applyBuffer(const UniformInfo *info, int i, love::graphics::Buffer *buffer, UniformType basetype, bool isdefault) override;

	// Get any warnings or errors generated only by the shader program object.
	std::string getProgramWarnings() const;

	// volatile
	GLuint program;

	// Location values for any built-in uniform variables.
	GLint builtinUniforms[BUILTIN_MAX_ENUM];
	UniformInfo *builtinUniformInfo[BUILTIN_MAX_ENUM];

	std::map<std::string, GLint> attributes;

	// Texture unit pool for setting textures
	std::vector<TextureUnit> textureUnits;

	std::vector<StorageTextureBinding> storageTextureBindings;

	std::vector<std::pair<int, int>> storageBufferBindingIndexToActiveBinding;
	std::vector<BufferBinding> activeStorageBufferBindings;

	std::vector<Buffer *> activeWritableStorageBuffers;

	std::vector<std::pair<const UniformInfo *, int>> pendingUniformUpdates;

}; // Shader

} // opengl
} // graphics
} // love
