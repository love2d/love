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

// LOVE
#include "common/config.h"

#include "Shader.h"
#include "ShaderStage.h"
#include "Graphics.h"
#include "graphics/vertex.h"

// C++
#include <algorithm>
#include <limits>
#include <sstream>

namespace love
{
namespace graphics
{
namespace opengl
{

static bool isBuffer(Shader::UniformType utype)
{
	return utype == Shader::UNIFORM_TEXELBUFFER || utype == Shader::UNIFORM_STORAGEBUFFER;
}

Shader::Shader(StrongRef<love::graphics::ShaderStage> stages[SHADERSTAGE_MAX_ENUM], const CompileOptions &options)
	: love::graphics::Shader(stages, options)
	, program(0)
	, splitUniformsPerDraw(false)
	, builtinUniforms()
	, builtinUniformInfo()
{
	// load shader source and create program object
	loadVolatile();
}

Shader::~Shader()
{
	unloadVolatile();

	for (const auto &p : uniforms)
	{
		// Allocated with malloc().
		if (p.second.data != nullptr)
			free(p.second.data);

		if (p.second.baseType == UNIFORM_SAMPLER || p.second.baseType == UNIFORM_STORAGETEXTURE)
		{
			for (int i = 0; i < p.second.count; i++)
			{
				if (p.second.textures[i] != nullptr)
					p.second.textures[i]->release();
			}

			delete[] p.second.textures;
		}
		else if (isBuffer(p.second.baseType))
		{
			for (int i = 0; i < p.second.count; i++)
			{
				if (p.second.buffers[i] != nullptr)
					p.second.buffers[i]->release();
			}

			delete[] p.second.buffers;
		}
	}
}

void Shader::mapActiveUniforms()
{
	// Built-in uniform locations default to -1 (nonexistent.)
	for (int i = 0; i < int(BUILTIN_MAX_ENUM); i++)
	{
		builtinUniforms[i] = -1;
		builtinUniformInfo[i] = nullptr;
	}

	GLint activeprogram = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &activeprogram);

	gl.useProgram(program);

	GLint numuniforms;
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numuniforms);

	GLchar cname[256];
	const GLint bufsize = (GLint) (sizeof(cname) / sizeof(GLchar));

	std::map<std::string, UniformInfo> olduniforms = uniforms;
	uniforms.clear();

	auto gfx = Module::getInstance<love::graphics::Graphics>(Module::M_GRAPHICS);

	for (int uindex = 0; uindex < numuniforms; uindex++)
	{
		GLsizei namelen = 0;
		GLenum gltype = 0;
		UniformInfo u = {};

		glGetActiveUniform(program, (GLuint) uindex, bufsize, &namelen, &u.count, &gltype, cname);

		u.name = std::string(cname, (size_t) namelen);
		u.location = glGetUniformLocation(program, u.name.c_str());
		u.access = ACCESS_READ;
		computeUniformTypeInfo(gltype, u);

		// glGetActiveUniform appends "[0]" to the end of array uniform names...
		if (u.name.length() > 3)
		{
			size_t findpos = u.name.find("[0]");
			if (findpos != std::string::npos && findpos == u.name.length() - 3)
				u.name.erase(u.name.length() - 3);
		}

		// If this is a built-in (LOVE-created) uniform, store the location.
		BuiltinUniform builtin = BUILTIN_MAX_ENUM;
		if (getConstant(u.name.c_str(), builtin))
			builtinUniforms[int(builtin)] = u.location;

		if (u.location == -1)
			continue;

		if (!fillUniformReflectionData(u))
			continue;

		if ((u.baseType == UNIFORM_SAMPLER && builtin != BUILTIN_TEXTURE_MAIN) || u.baseType == UNIFORM_TEXELBUFFER)
		{
			TextureUnit unit;
			unit.type = u.textureType;
			unit.active = true;

			if (u.baseType == UNIFORM_TEXELBUFFER)
			{
				unit.isTexelBuffer = true;
				unit.texture = gl.getDefaultTexelBuffer();
			}
			else
			{
				unit.isTexelBuffer = false;
				unit.texture = 0; // Handled below.
			}

			for (int i = 0; i < u.count; i++)
				textureUnits.push_back(unit);
		}
		else if (u.baseType == UNIFORM_STORAGETEXTURE)
		{
			StorageTextureBinding binding = {};
			binding.gltexture = 0; // Handled below.
			binding.type = u.textureType;

			if ((u.access & (ACCESS_READ | ACCESS_WRITE)) != 0)
				binding.access = GL_READ_WRITE;
			else if ((u.access & ACCESS_WRITE) != 0)
				binding.access = GL_WRITE_ONLY;
			else if ((u.access & ACCESS_READ) != 0)
				binding.access = GL_READ_ONLY;

			auto fmt = OpenGL::convertPixelFormat(u.storageTextureFormat, false);
			binding.internalFormat = fmt.internalformat;

			for (int i = 0; i < u.count; i++)
				storageTextureBindings.push_back(binding);
		}

		// Make sure previously set uniform data is preserved, and shader-
		// initialized values are retrieved.
		auto oldu = olduniforms.find(u.name);
		if (oldu != olduniforms.end())
		{
			u.data = oldu->second.data;
			u.dataSize = oldu->second.dataSize;
			u.textures = oldu->second.textures;

			updateUniform(&u, u.count, true);
		}
		else
		{
			u.dataSize = 0;

			switch (u.baseType)
			{
			case UNIFORM_FLOAT:
				u.dataSize = sizeof(float) * u.components * u.count;
				u.data = malloc(u.dataSize);
				break;
			case UNIFORM_INT:
			case UNIFORM_BOOL:
			case UNIFORM_SAMPLER:
			case UNIFORM_STORAGETEXTURE:
			case UNIFORM_TEXELBUFFER:
				u.dataSize = sizeof(int) * u.components * u.count;
				u.data = malloc(u.dataSize);
				break;
			case UNIFORM_UINT:
				u.dataSize = sizeof(unsigned int) * u.components * u.count;
				u.data = malloc(u.dataSize);
				break;
			case UNIFORM_MATRIX:
				u.dataSize = sizeof(float) * ((size_t)u.matrix.rows * u.matrix.columns) * u.count;
				u.data = malloc(u.dataSize);
				break;
			default:
				break;
			}

			if (u.dataSize > 0)
			{
				memset(u.data, 0, u.dataSize);

				if (u.baseType == UNIFORM_SAMPLER || u.baseType == UNIFORM_TEXELBUFFER)
				{
					int startunit = (int) textureUnits.size() - u.count;

					if (builtin == BUILTIN_TEXTURE_MAIN)
						startunit = 0;

					for (int i = 0; i < u.count; i++)
						u.ints[i] = startunit + i;

					glUniform1iv(u.location, u.count, u.ints);

					if (u.baseType == UNIFORM_TEXELBUFFER)
					{
						u.buffers = new love::graphics::Buffer*[u.count];
						memset(u.buffers, 0, sizeof(Buffer *) * u.count);
					}
					else
					{
						u.textures = new love::graphics::Texture*[u.count];

						auto *tex = gfx->getDefaultTexture(u.textureType, u.dataBaseType);
						for (int i = 0; i < u.count; i++)
						{
							tex->retain();
							u.textures[i] = tex;
						}
					}
				}
				else if (u.baseType == UNIFORM_STORAGETEXTURE)
				{
					int startbinding = (int) storageTextureBindings.size() - u.count;
					for (int i = 0; i < u.count; i++)
						u.ints[i] = startbinding + i;

					glUniform1iv(u.location, u.count, u.ints);

					u.textures = new love::graphics::Texture*[u.count];

					if ((u.access & ACCESS_WRITE) != 0)
					{
						memset(u.textures, 0, sizeof(Texture *) * u.count);
					}
					else
					{
						auto *tex = gfx->getDefaultTexture(u.textureType, u.dataBaseType);
						for (int i = 0; i < u.count; i++)
						{
							tex->retain();
							u.textures[i] = tex;
						}
					}
				}
			}

			size_t offset = 0;

			// Store any shader-initialized values in our own memory.
			for (int i = 0; i < u.count; i++)
			{
				GLint location = u.location;

				if (u.count > 1)
				{
					std::ostringstream ss;
					ss << i;

					std::string indexname = u.name + "[" + ss.str() + "]";
					location = glGetUniformLocation(program, indexname.c_str());
				}

				if (location == -1)
					continue;

				switch (u.baseType)
				{
				case UNIFORM_FLOAT:
					glGetUniformfv(program, location, &u.floats[offset]);
					offset += u.components;
					break;
				case UNIFORM_INT:
				case UNIFORM_BOOL:
					glGetUniformiv(program, location, &u.ints[offset]);
					offset += u.components;
					break;
				case UNIFORM_UINT:
					glGetUniformuiv(program, location, &u.uints[offset]);
					offset += u.components;
					break;
				case UNIFORM_MATRIX:
					glGetUniformfv(program, location, &u.floats[offset]);
					offset += (size_t)u.matrix.rows * u.matrix.columns;
					break;
				default:
					break;
				}
			}
		}

		uniforms[u.name] = u;

		if (builtin != BUILTIN_MAX_ENUM)
			builtinUniformInfo[(int)builtin] = &uniforms[u.name];

		if (u.baseType == UNIFORM_SAMPLER || u.baseType == UNIFORM_STORAGETEXTURE)
		{
			// Make sure all stored textures have their Volatiles loaded before
			// the sendTextures call, since it calls getHandle().
			for (int i = 0; i < u.count; i++)
			{
				if (u.textures[i] == nullptr)
					continue;
				Volatile *v = dynamic_cast<Volatile *>(u.textures[i]);
				if (v != nullptr)
					v->loadVolatile();
			}

			sendTextures(&u, u.textures, u.count, true);
		}
		else if (u.baseType == UNIFORM_TEXELBUFFER)
		{
			for (int i = 0; i < u.count; i++)
			{
				if (u.buffers[i] == nullptr)
					continue;
				Volatile *v = dynamic_cast<Volatile *>(u.buffers[i]);
				if (v != nullptr)
					v->loadVolatile();
			}

			sendBuffers(&u, u.buffers, u.count, true);
		}
	}

	if (gl.isBufferUsageSupported(BUFFERUSAGE_SHADER_STORAGE))
	{
		GLint numstoragebuffers = 0;
		glGetProgramInterfaceiv(program, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &numstoragebuffers);

		char namebuffer[2048] = { '\0' };
		int nextstoragebufferbinding = 0;

		for (int sindex = 0; sindex < numstoragebuffers; sindex++)
		{
			UniformInfo u = {};
			u.baseType = UNIFORM_STORAGEBUFFER;
			u.access = ACCESS_READ;

			GLsizei namelength = 0;
			glGetProgramResourceName(program, GL_SHADER_STORAGE_BLOCK, sindex, 2048, &namelength, namebuffer);

			u.name = std::string(namebuffer, namelength);
			u.count = 1;

			if (!fillUniformReflectionData(u))
				continue;

			// Make sure previously set uniform data is preserved, and shader-
			// initialized values are retrieved.
			auto oldu = olduniforms.find(u.name);
			if (oldu != olduniforms.end())
			{
				u.data = oldu->second.data;
				u.dataSize = oldu->second.dataSize;
				u.buffers = oldu->second.buffers;
			}
			else
			{
				u.dataSize = sizeof(int) * 1;
				u.data = malloc(u.dataSize);

				u.ints[0] = -1;

				u.buffers = new love::graphics::Buffer * [u.count];
				memset(u.buffers, 0, sizeof(Buffer*)* u.count);
			}

			// Unlike local uniforms and attributes, OpenGL doesn't auto-assign storage
			// block bindings if they're unspecified in the shader. So we overwrite them
			// regardless, here.
			u.ints[0] = nextstoragebufferbinding++;
			glShaderStorageBlockBinding(program, sindex, u.ints[0]);

			BufferBinding binding;
			binding.bindingindex = u.ints[0];
			binding.buffer = gl.getDefaultStorageBuffer();

			if (binding.bindingindex >= 0)
			{
				int activeindex = (int)activeStorageBufferBindings.size();
				activeStorageBufferBindings.push_back(binding);

				auto p = std::make_pair(activeindex, -1);

				if (u.access & ACCESS_WRITE)
				{
					p.second = (int)activeWritableStorageBuffers.size();
					activeWritableStorageBuffers.push_back(u.buffers[0]);
				}

				storageBufferBindingIndexToActiveBinding[binding.bindingindex] = p;
			}

			uniforms[u.name] = u;

			for (int i = 0; i < u.count; i++)
			{
				if (u.buffers[i] == nullptr)
					continue;
				Volatile* v = dynamic_cast<Volatile*>(u.buffers[i]);
				if (v != nullptr)
					v->loadVolatile();
			}

			sendBuffers(&u, u.buffers, u.count, true);
		}
	}

	// Make sure uniforms that existed before but don't exist anymore are
	// cleaned up. This theoretically shouldn't happen, but...
	for (const auto &p : olduniforms)
	{
		if (uniforms.find(p.first) == uniforms.end())
		{
			if (p.second.data != nullptr)
				free(p.second.data);

			if (p.second.baseType == UNIFORM_SAMPLER || p.second.baseType == UNIFORM_STORAGETEXTURE)
			{
				for (int i = 0; i < p.second.count; i++)
				{
					if (p.second.textures[i] != nullptr)
						p.second.textures[i]->release();
				}

				delete[] p.second.textures;
			}
			else if (isBuffer(p.second.baseType))
			{
				for (int i = 0; i < p.second.count; i++)
				{
					if (p.second.buffers[i] != nullptr)
						p.second.buffers[i]->release();
				}

				delete[] p.second.buffers;
			}
		}
	}

	gl.useProgram(activeprogram);
}

bool Shader::loadVolatile()
{
	OpenGL::TempDebugGroup debuggroup("Shader load");

	// love::graphics::Shader sets up the shader code-side of this.
	auto gfx = Module::getInstance<love::graphics::Graphics>(Module::M_GRAPHICS);
	if (gfx != nullptr)
		splitUniformsPerDraw = !gfx->getCapabilities().features[Graphics::FEATURE_PIXEL_SHADER_HIGHP];

	// zero out active texture list
	textureUnits.clear();
	textureUnits.push_back(TextureUnit());

	activeStorageBufferBindings.clear();

	storageBufferBindingIndexToActiveBinding.resize(gl.getMaxShaderStorageBufferBindings(), std::make_pair(-1, -1));
	activeStorageBufferBindings.clear();
	activeWritableStorageBuffers.clear();

	for (const auto &stage : stages)
	{
		if (stage.get() != nullptr)
			((ShaderStage*)stage.get())->loadVolatile();
	}

	program = glCreateProgram();

	if (program == 0)
		throw love::Exception("Cannot create shader program object.");

	if (!debugName.empty() && (GLAD_VERSION_4_3 || GLAD_ES_VERSION_3_2))
		glObjectLabel(GL_PROGRAM, program, -1, debugName.c_str());

	for (const auto &stage : stages)
	{
		if (stage.get() != nullptr)
			glAttachShader(program, (GLuint) stage->getHandle());
	}

	// Bind generic vertex attribute indices to names in the shader.
	for (int i = 0; i < int(ATTRIB_MAX_ENUM); i++)
	{
		const char *name = nullptr;
		if (graphics::getConstant((BuiltinVertexAttribute) i, name))
			glBindAttribLocation(program, i, (const GLchar *) name);
	}

	glLinkProgram(program);

	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);

	if (status == GL_FALSE)
	{
		std::string warnings = getProgramWarnings();
		glDeleteProgram(program);
		program = 0;
		throw love::Exception("Cannot link shader program object:\n%s", warnings.c_str());
	}

	// Get all active uniform variables in this shader from OpenGL.
	mapActiveUniforms();

	if (current == this)
	{
		// make sure glUseProgram gets called.
		current = nullptr;
		attach();
	}

	return true;
}

void Shader::unloadVolatile()
{
	if (program != 0)
	{
		if (current == this)
			gl.useProgram(0);

		glDeleteProgram(program);
		program = 0;
	}

	// active texture list is probably invalid, clear it
	textureUnits.clear();
	textureUnits.push_back(TextureUnit());

	attributes.clear();

	// And the locations of any built-in uniform variables.
	for (int i = 0; i < int(BUILTIN_MAX_ENUM); i++)
		builtinUniforms[i] = -1;
}

std::string Shader::getProgramWarnings() const
{
	GLint strsize, nullpos;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &strsize);

	if (strsize == 0)
		return "";

	char *tempstr = new char[strsize];
	// be extra sure that the error string will be 0-terminated
	memset(tempstr, '\0', strsize);
	glGetProgramInfoLog(program, strsize, &nullpos, tempstr);
	tempstr[nullpos] = '\0';

	std::string warnings(tempstr);
	delete[] tempstr;

	return warnings;
}

std::string Shader::getWarnings() const
{
	std::string warnings;
	const char *stagestr;

	for (const auto &stage : stages)
	{
		if (stage.get() == nullptr)
			continue;

		const std::string &stagewarnings = stage->getWarnings();

		if (ShaderStage::getConstant(stage->getStageType(), stagestr))
			warnings += std::string(stagestr) + std::string(" shader:\n") + stagewarnings;
	}

	warnings += getProgramWarnings();

	return warnings;
}

void Shader::attach()
{
	if (current != this)
	{
		Graphics::flushBatchedDrawsGlobal();

		gl.useProgram(program);
		current = this;
		// retain/release happens in Graphics::setShader.

		// Make sure all textures are bound to their respective texture units.
		for (int i = 0; i < (int) textureUnits.size(); i++)
		{
			const TextureUnit &unit = textureUnits[i];
			if (unit.active)
			{
				if (unit.isTexelBuffer)
					gl.bindBufferTextureToUnit(unit.texture, i, false, false);
				else
					gl.bindTextureToUnit(unit.type, unit.texture, i, false, false);
			}
		}

		for (size_t i = 0; i < storageTextureBindings.size(); i++)
		{
			const auto &binding = storageTextureBindings[i];
			glBindImageTexture((GLuint) i, binding.gltexture, 0, GL_TRUE, 0, binding.access, binding.internalFormat);
		}

		for (auto bufferbinding : activeStorageBufferBindings)
			gl.bindIndexedBuffer(bufferbinding.buffer, BUFFERUSAGE_SHADER_STORAGE, bufferbinding.bindingindex);

		// send any pending uniforms to the shader program.
		for (const auto &p : pendingUniformUpdates)
			updateUniform(p.first, p.second, true);

		pendingUniformUpdates.clear();
	}
}

const Shader::UniformInfo *Shader::getUniformInfo(const std::string &name) const
{
	const auto it = uniforms.find(name);

	if (it == uniforms.end())
		return nullptr;

	return &(it->second);
}

const Shader::UniformInfo *Shader::getUniformInfo(BuiltinUniform builtin) const
{
	return builtinUniformInfo[(int)builtin];
}

void Shader::updateUniform(const UniformInfo *info, int count)
{
	updateUniform(info, count, false);
}

void Shader::updateUniform(const UniformInfo *info, int count, bool internalupdate)
{
	if (current != this && !internalupdate)
	{
		pendingUniformUpdates.push_back(std::make_pair(info, count));
		return;
	}

	if (!internalupdate)
		flushBatchedDraws();

	int location = info->location;
	UniformType type = info->baseType;

	if (type == UNIFORM_FLOAT)
	{
		switch (info->components)
		{
		case 1:
			glUniform1fv(location, count, info->floats);
			break;
		case 2:
			glUniform2fv(location, count, info->floats);
			break;
		case 3:
			glUniform3fv(location, count, info->floats);
			break;
		case 4:
			glUniform4fv(location, count, info->floats);
			break;
		}
	}
	else if (type == UNIFORM_INT || type == UNIFORM_BOOL || type == UNIFORM_SAMPLER || type == UNIFORM_STORAGETEXTURE || type == UNIFORM_TEXELBUFFER)
	{
		switch (info->components)
		{
		case 1:
			glUniform1iv(location, count, info->ints);
			break;
		case 2:
			glUniform2iv(location, count, info->ints);
			break;
		case 3:
			glUniform3iv(location, count, info->ints);
			break;
		case 4:
			glUniform4iv(location, count, info->ints);
			break;
		}
	}
	else if (type == UNIFORM_UINT)
	{
		switch (info->components)
		{
		case 1:
			glUniform1uiv(location, count, info->uints);
			break;
		case 2:
			glUniform2uiv(location, count, info->uints);
			break;
		case 3:
			glUniform3uiv(location, count, info->uints);
			break;
		case 4:
			glUniform4uiv(location, count, info->uints);
			break;
		}
	}
	else if (type == UNIFORM_MATRIX)
	{
		int columns = info->matrix.columns;
		int rows = info->matrix.rows;

		if (columns == 2 && rows == 2)
			glUniformMatrix2fv(location, count, GL_FALSE, info->floats);
		else if (columns == 3 && rows == 3)
			glUniformMatrix3fv(location, count, GL_FALSE, info->floats);
		else if (columns == 4 && rows == 4)
			glUniformMatrix4fv(location, count, GL_FALSE, info->floats);
		else if (columns == 2 && rows == 3)
			glUniformMatrix2x3fv(location, count, GL_FALSE, info->floats);
		else if (columns == 2 && rows == 4)
			glUniformMatrix2x4fv(location, count, GL_FALSE, info->floats);
		else if (columns == 3 && rows == 2)
			glUniformMatrix3x2fv(location, count, GL_FALSE, info->floats);
		else if (columns == 3 && rows == 4)
			glUniformMatrix3x4fv(location, count, GL_FALSE, info->floats);
		else if (columns == 4 && rows == 2)
			glUniformMatrix4x2fv(location, count, GL_FALSE, info->floats);
		else if (columns == 4 && rows == 3)
			glUniformMatrix4x3fv(location, count, GL_FALSE, info->floats);
	}
}

void Shader::sendTextures(const UniformInfo *info, love::graphics::Texture **textures, int count)
{
	Shader::sendTextures(info, textures, count, false);
}

void Shader::sendTextures(const UniformInfo *info, love::graphics::Texture **textures, int count, bool internalUpdate)
{
	bool issampler = info->baseType == UNIFORM_SAMPLER;
	bool isstoragetex = info->baseType == UNIFORM_STORAGETEXTURE;

	if (!issampler && !isstoragetex)
		return;

	bool shaderactive = current == this;

	if (!internalUpdate && shaderactive)
		flushBatchedDraws();

	count = std::min(count, info->count);

	// Bind the textures to the texture units.
	for (int i = 0; i < count; i++)
	{
		love::graphics::Texture *tex = textures[i];

		if (tex != nullptr)
		{
			if (!validateTexture(info, tex, internalUpdate))
				continue;
		}
		else
		{
			auto gfx = Module::getInstance<love::graphics::Graphics>(Module::M_GRAPHICS);
			tex = gfx->getDefaultTexture(info->textureType, info->dataBaseType);
		}

		tex->retain();

		if (info->textures[i] != nullptr)
			info->textures[i]->release();

		info->textures[i] = tex;

		if (isstoragetex)
		{
			GLuint gltex = (GLuint) tex->getHandle();

			int bindingindex = info->ints[i];
			auto &binding = storageTextureBindings[bindingindex];

			binding.texture = tex;
			binding.gltexture = gltex;

			if (shaderactive)
				glBindImageTexture(bindingindex, binding.gltexture, 0, GL_TRUE, 0, binding.access, binding.internalFormat);
		}
		else
		{
			GLuint gltex = (GLuint) tex->getHandle();

			int texunit = info->ints[i];

			if (shaderactive)
				gl.bindTextureToUnit(info->textureType, gltex, texunit, false, false);

			// Store texture id so it can be re-bound to the texture unit later.
			textureUnits[texunit].texture = gltex;
		}
	}
}

void Shader::sendBuffers(const UniformInfo *info, love::graphics::Buffer **buffers, int count)
{
	Shader::sendBuffers(info, buffers, count, false);
}

void Shader::sendBuffers(const UniformInfo *info, love::graphics::Buffer **buffers, int count, bool internalUpdate)
{
	bool texelbinding = info->baseType == UNIFORM_TEXELBUFFER;
	bool storagebinding = info->baseType == UNIFORM_STORAGEBUFFER;

	if (!texelbinding && !storagebinding)
		return;

	bool shaderactive = current == this;

	if (!internalUpdate && shaderactive)
		flushBatchedDraws();

	count = std::min(count, info->count);

	// Bind the textures to the texture units.
	for (int i = 0; i < count; i++)
	{
		love::graphics::Buffer *buffer = buffers[i];

		if (buffer != nullptr)
		{
			if (!validateBuffer(info, buffer, internalUpdate))
				continue;
			buffer->retain();
		}

		if (info->buffers[i] != nullptr)
			info->buffers[i]->release();

		info->buffers[i] = buffer;

		if (texelbinding)
		{
			GLuint gltex = 0;
			if (buffer != nullptr)
				gltex = (GLuint) buffer->getTexelBufferHandle();
			else
				gltex = gl.getDefaultTexelBuffer();

			int texunit = info->ints[i];

			if (shaderactive)
				gl.bindBufferTextureToUnit(gltex, texunit, false, false);

			// Store texture id so it can be re-bound to the texture unit later.
			textureUnits[texunit].texture = gltex;
		}
		else if (storagebinding)
		{
			int bindingindex = info->ints[i];

			GLuint glbuffer = 0;
			if (buffer != nullptr)
				glbuffer = (GLuint) buffer->getHandle();
			else
				glbuffer = gl.getDefaultStorageBuffer();

			if (shaderactive)
				gl.bindIndexedBuffer(glbuffer, BUFFERUSAGE_SHADER_STORAGE, bindingindex);

			auto activeindex = storageBufferBindingIndexToActiveBinding[bindingindex];

			if (activeindex.first >= 0)
				activeStorageBufferBindings[activeindex.first].buffer = glbuffer;

			if (activeindex.second >= 0)
				activeWritableStorageBuffers[activeindex.second] = buffer;
		}
	}
}

void Shader::flushBatchedDraws() const
{
	if (current == this)
		Graphics::flushBatchedDrawsGlobal();
}

bool Shader::hasUniform(const std::string &name) const
{
	return uniforms.find(name) != uniforms.end();
}

ptrdiff_t Shader::getHandle() const
{
	return program;
}

int Shader::getVertexAttributeIndex(const std::string &name)
{
	auto it = attributes.find(name);
	if (it != attributes.end())
		return it->second;

	GLint location = glGetAttribLocation(program, name.c_str());

	attributes[name] = location;
	return location;
}

void Shader::setVideoTextures(love::graphics::Texture *ytexture, love::graphics::Texture *cbtexture, love::graphics::Texture *crtexture)
{
	const BuiltinUniform builtins[3] = {
		BUILTIN_TEXTURE_VIDEO_Y,
		BUILTIN_TEXTURE_VIDEO_CB,
		BUILTIN_TEXTURE_VIDEO_CR,
	};

	love::graphics::Texture *textures[3] = {ytexture, cbtexture, crtexture};

	for (int i = 0; i < 3; i++)
	{
		const UniformInfo *info = builtinUniformInfo[builtins[i]];

		if (info != nullptr)
			sendTextures(info, &textures[i], 1, true);
	}
}

void Shader::updateBuiltinUniforms(love::graphics::Graphics *gfx, int viewportW, int viewportH)
{
	if (current != this)
		return;

	BuiltinUniformData data;

	data.transformMatrix = gfx->getTransform();
	data.projectionMatrix = gfx->getDeviceProjection();

	// The normal matrix is the transpose of the inverse of the rotation portion
	// (top-left 3x3) of the transform matrix.
	{
		Matrix3 normalmatrix = Matrix3(data.transformMatrix).transposedInverse();
		const float *e = normalmatrix.getElements();
		for (int i = 0; i < 3; i++)
		{
			data.normalMatrix[i].x = e[i * 3 + 0];
			data.normalMatrix[i].y = e[i * 3 + 1];
			data.normalMatrix[i].z = e[i * 3 + 2];
			data.normalMatrix[i].w = 0.0f;
		}
	}

	// Store DPI scale in an unused component of another vector.
	data.normalMatrix[0].w = (float) gfx->getCurrentDPIScale();

	// Same with point size.
	data.normalMatrix[1].w = gfx->getPointSize();

	data.screenSizeParams.x = viewportW;
	data.screenSizeParams.y = viewportH;

	// The shader does pixcoord.y = gl_FragCoord.y * params.z + params.w.
	// This lets us flip pixcoord.y when needed, to be consistent (drawing
	// with no RT active makes the pixel coordinates y-flipped.)
	if (gfx->isRenderTargetActive())
	{
		// No flipping: pixcoord.y = gl_FragCoord.y * 1.0 + 0.0.
		data.screenSizeParams.z = 1.0f;
		data.screenSizeParams.w = 0.0f;
	}
	else
	{
		// gl_FragCoord.y is flipped when drawing to the screen, so we
		// un-flip: pixcoord.y = gl_FragCoord.y * -1.0 + height.
		data.screenSizeParams.z = -1.0f;
		data.screenSizeParams.w = viewportH;
	}

	data.constantColor = gfx->getColor();
	gammaCorrectColor(data.constantColor);

	// This branch is to avoid always declaring the whole array as highp in the
	// vertex shader and mediump in the pixel shader for love's default shaders,
	// on systems that don't support highp in pixel shaders. The default shaders
	// use the transform matrices in vertex shaders and screen size params in
	// pixel shaders. If there's a single array containing both and each shader
	// stage declares a different precision, that's a compile error.
	if (splitUniformsPerDraw)
	{
		GLint location = builtinUniforms[BUILTIN_UNIFORMS_PER_DRAW];
		if (location >= 0)
			glUniform4fv(location, 12, (const GLfloat *) &data);
		GLint location2 = builtinUniforms[BUILTIN_UNIFORMS_PER_DRAW_2];
		if (location2 >= 0)
			glUniform4fv(location2, 1, (const GLfloat *) &data.screenSizeParams);
	}
	else
	{
		GLint location = builtinUniforms[BUILTIN_UNIFORMS_PER_DRAW];
		if (location >= 0)
			glUniform4fv(location, 13, (const GLfloat *) &data);
	}
}

int Shader::getUniformTypeComponents(GLenum type) const
{
	switch (type)
	{
	case GL_INT:
	case GL_UNSIGNED_INT:
	case GL_FLOAT:
	case GL_BOOL:
		return 1;
	case GL_INT_VEC2:
	case GL_UNSIGNED_INT_VEC2:
	case GL_FLOAT_VEC2:
	case GL_FLOAT_MAT2:
	case GL_BOOL_VEC2:
		return 2;
	case GL_INT_VEC3:
	case GL_UNSIGNED_INT_VEC3:
	case GL_FLOAT_VEC3:
	case GL_FLOAT_MAT3:
	case GL_BOOL_VEC3:
		return 3;
	case GL_INT_VEC4:
	case GL_UNSIGNED_INT_VEC4:
	case GL_FLOAT_VEC4:
	case GL_FLOAT_MAT4:
	case GL_BOOL_VEC4:
		return 4;
	default:
		return 1;
	}
}

Shader::MatrixSize Shader::getMatrixSize(GLenum type) const
{
	MatrixSize m;

	switch (type)
	{
	case GL_FLOAT_MAT2:
		m.columns = m.rows = 2;
		break;
	case GL_FLOAT_MAT3:
		m.columns = m.rows = 3;
		break;
	case GL_FLOAT_MAT4:
		m.columns = m.rows = 4;
		break;
	case GL_FLOAT_MAT2x3:
		m.columns = 2;
		m.rows = 3;
		break;
	case GL_FLOAT_MAT2x4:
		m.columns = 2;
		m.rows = 4;
		break;
	case GL_FLOAT_MAT3x2:
		m.columns = 3;
		m.rows = 2;
		break;
	case GL_FLOAT_MAT3x4:
		m.columns = 3;
		m.rows = 4;
		break;
	case GL_FLOAT_MAT4x2:
		m.columns = 4;
		m.rows = 2;
		break;
	case GL_FLOAT_MAT4x3:
		m.columns = 4;
		m.rows = 3;
		break;
	default:
		m.columns = m.rows = 0;
		break;
	}

	return m;
}

void Shader::computeUniformTypeInfo(GLenum type, UniformInfo &u)
{
	u.isDepthSampler = false;
	u.components = getUniformTypeComponents(type);
	u.baseType = UNIFORM_UNKNOWN;

	switch (type)
	{
	case GL_INT:
	case GL_INT_VEC2:
	case GL_INT_VEC3:
	case GL_INT_VEC4:
		u.baseType = UNIFORM_INT;
		u.dataBaseType = DATA_BASETYPE_INT;
		break;
	case GL_UNSIGNED_INT:
	case GL_UNSIGNED_INT_VEC2:
	case GL_UNSIGNED_INT_VEC3:
	case GL_UNSIGNED_INT_VEC4:
		u.baseType = UNIFORM_UINT;
		u.dataBaseType = DATA_BASETYPE_UINT;
		break;
	case GL_FLOAT:
	case GL_FLOAT_VEC2:
	case GL_FLOAT_VEC3:
	case GL_FLOAT_VEC4:
		u.baseType = UNIFORM_FLOAT;
		u.dataBaseType = DATA_BASETYPE_FLOAT;
		break;
	case GL_FLOAT_MAT2:
	case GL_FLOAT_MAT3:
	case GL_FLOAT_MAT4:
	case GL_FLOAT_MAT2x3:
	case GL_FLOAT_MAT2x4:
	case GL_FLOAT_MAT3x2:
	case GL_FLOAT_MAT3x4:
	case GL_FLOAT_MAT4x2:
	case GL_FLOAT_MAT4x3:
		u.baseType = UNIFORM_MATRIX;
		u.dataBaseType = DATA_BASETYPE_FLOAT;
		u.matrix = getMatrixSize(type);
		break;
	case GL_BOOL:
	case GL_BOOL_VEC2:
	case GL_BOOL_VEC3:
	case GL_BOOL_VEC4:
		u.baseType = UNIFORM_BOOL;
		u.dataBaseType = DATA_BASETYPE_BOOL;
		break;

	case GL_SAMPLER_2D:
		u.baseType = UNIFORM_SAMPLER;
		u.dataBaseType = DATA_BASETYPE_FLOAT;
		u.textureType = TEXTURE_2D;
		break;
	case GL_SAMPLER_2D_SHADOW:
		u.baseType = UNIFORM_SAMPLER;
		u.dataBaseType = DATA_BASETYPE_FLOAT;
		u.textureType = TEXTURE_2D;
		u.isDepthSampler = true;
		break;
	case GL_INT_SAMPLER_2D:
		u.baseType = UNIFORM_SAMPLER;
		u.dataBaseType = DATA_BASETYPE_INT;
		u.textureType = TEXTURE_2D;
		break;
	case GL_UNSIGNED_INT_SAMPLER_2D:
		u.baseType = UNIFORM_SAMPLER;
		u.dataBaseType = DATA_BASETYPE_UINT;
		u.textureType = TEXTURE_2D;
		break;
	case GL_SAMPLER_2D_ARRAY:
		u.baseType = UNIFORM_SAMPLER;
		u.dataBaseType = DATA_BASETYPE_FLOAT;
		u.textureType = TEXTURE_2D_ARRAY;
		break;
	case GL_SAMPLER_2D_ARRAY_SHADOW:
		u.baseType = UNIFORM_SAMPLER;
		u.dataBaseType = DATA_BASETYPE_FLOAT;
		u.textureType = TEXTURE_2D_ARRAY;
		u.isDepthSampler = true;
		break;
	case GL_INT_SAMPLER_2D_ARRAY:
		u.baseType = UNIFORM_SAMPLER;
		u.dataBaseType = DATA_BASETYPE_INT;
		u.textureType = TEXTURE_2D_ARRAY;
		break;
	case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
		u.baseType = UNIFORM_SAMPLER;
		u.dataBaseType = DATA_BASETYPE_UINT;
		u.textureType = TEXTURE_2D_ARRAY;
		break;
	case GL_SAMPLER_3D:
		u.baseType = UNIFORM_SAMPLER;
		u.dataBaseType = DATA_BASETYPE_FLOAT;
		u.textureType = TEXTURE_VOLUME;
		break;
	case GL_INT_SAMPLER_3D:
		u.baseType = UNIFORM_SAMPLER;
		u.dataBaseType = DATA_BASETYPE_INT;
		u.textureType = TEXTURE_VOLUME;
		break;
	case GL_UNSIGNED_INT_SAMPLER_3D:
		u.baseType = UNIFORM_SAMPLER;
		u.dataBaseType = DATA_BASETYPE_UINT;
		u.textureType = TEXTURE_VOLUME;
		break;
	case GL_SAMPLER_CUBE:
		u.baseType = UNIFORM_SAMPLER;
		u.dataBaseType = DATA_BASETYPE_FLOAT;
		u.textureType = TEXTURE_CUBE;
		break;
	case GL_SAMPLER_CUBE_SHADOW:
		u.baseType = UNIFORM_SAMPLER;
		u.dataBaseType = DATA_BASETYPE_FLOAT;
		u.textureType = TEXTURE_CUBE;
		u.isDepthSampler = true;
		break;
	case GL_INT_SAMPLER_CUBE:
		u.baseType = UNIFORM_SAMPLER;
		u.dataBaseType = DATA_BASETYPE_INT;
		u.textureType = TEXTURE_CUBE;
		break;
	case GL_UNSIGNED_INT_SAMPLER_CUBE:
		u.baseType = UNIFORM_SAMPLER;
		u.dataBaseType = DATA_BASETYPE_UINT;
		u.textureType = TEXTURE_CUBE;
		break;

	case GL_SAMPLER_BUFFER:
		u.baseType = UNIFORM_TEXELBUFFER;
		u.dataBaseType = DATA_BASETYPE_FLOAT;
		break;
	case GL_INT_SAMPLER_BUFFER:
		u.baseType = UNIFORM_TEXELBUFFER;
		u.dataBaseType = DATA_BASETYPE_INT;
		break;
	case GL_UNSIGNED_INT_SAMPLER_BUFFER:
		u.baseType = UNIFORM_TEXELBUFFER;
		u.dataBaseType = DATA_BASETYPE_UINT;
		break;

	case GL_IMAGE_2D:
		u.baseType = UNIFORM_STORAGETEXTURE;
		u.dataBaseType = DATA_BASETYPE_FLOAT;
		u.textureType = TEXTURE_2D;
		break;
	case GL_INT_IMAGE_2D:
		u.baseType = UNIFORM_STORAGETEXTURE;
		u.dataBaseType = DATA_BASETYPE_INT;
		u.textureType = TEXTURE_2D;
		break;
	case GL_UNSIGNED_INT_IMAGE_2D:
		u.baseType = UNIFORM_STORAGETEXTURE;
		u.dataBaseType = DATA_BASETYPE_UINT;
		u.textureType = TEXTURE_2D;
		break;
	case GL_IMAGE_2D_ARRAY:
		u.baseType = UNIFORM_STORAGETEXTURE;
		u.dataBaseType = DATA_BASETYPE_FLOAT;
		u.textureType = TEXTURE_2D_ARRAY;
		break;
	case GL_INT_IMAGE_2D_ARRAY:
		u.baseType = UNIFORM_STORAGETEXTURE;
		u.dataBaseType = DATA_BASETYPE_INT;
		u.textureType = TEXTURE_2D_ARRAY;
		break;
	case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:
		u.baseType = UNIFORM_STORAGETEXTURE;
		u.dataBaseType = DATA_BASETYPE_UINT;
		u.textureType = TEXTURE_2D_ARRAY;
		break;
	case GL_IMAGE_3D:
		u.baseType = UNIFORM_STORAGETEXTURE;
		u.dataBaseType = DATA_BASETYPE_FLOAT;
		u.textureType = TEXTURE_VOLUME;
		break;
	case GL_INT_IMAGE_3D:
		u.baseType = UNIFORM_STORAGETEXTURE;
		u.dataBaseType = DATA_BASETYPE_INT;
		u.textureType = TEXTURE_VOLUME;
		break;
	case GL_UNSIGNED_INT_IMAGE_3D:
		u.baseType = UNIFORM_STORAGETEXTURE;
		u.dataBaseType = DATA_BASETYPE_UINT;
		u.textureType = TEXTURE_VOLUME;
		break;
	case GL_IMAGE_CUBE:
		u.baseType = UNIFORM_STORAGETEXTURE;
		u.dataBaseType = DATA_BASETYPE_FLOAT;
		u.textureType = TEXTURE_CUBE;
		break;
	case GL_INT_IMAGE_CUBE:
		u.baseType = UNIFORM_STORAGETEXTURE;
		u.dataBaseType = DATA_BASETYPE_INT;
		u.textureType = TEXTURE_CUBE;
		break;
	case GL_UNSIGNED_INT_IMAGE_CUBE:
		u.baseType = UNIFORM_STORAGETEXTURE;
		u.dataBaseType = DATA_BASETYPE_UINT;
		u.textureType = TEXTURE_CUBE;
		break;

	default:
		break;
	}
}

} // opengl
} // graphics
} // love
