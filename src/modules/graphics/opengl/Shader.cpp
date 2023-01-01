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
#include "Graphics.h"

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

Shader::Shader(love::graphics::ShaderStage *vertex, love::graphics::ShaderStage *pixel)
	: love::graphics::Shader(vertex, pixel)
	, program(0)
	, builtinUniforms()
	, builtinUniformInfo()
	, builtinAttributes()
	, canvasWasActive(false)
	, lastViewport()
	, lastPointSize(0.0f)
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

		if (p.second.baseType == UNIFORM_SAMPLER)
		{
			for (int i = 0; i < p.second.count; i++)
			{
				if (p.second.textures[i] != nullptr)
					p.second.textures[i]->release();
			}

			delete[] p.second.textures;
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

	for (int uindex = 0; uindex < numuniforms; uindex++)
	{
		GLsizei namelen = 0;
		GLenum gltype = 0;
		UniformInfo u = {};

		glGetActiveUniform(program, (GLuint) uindex, bufsize, &namelen, &u.count, &gltype, cname);

		u.name = std::string(cname, (size_t) namelen);
		u.location = glGetUniformLocation(program, u.name.c_str());
		u.baseType = getUniformBaseType(gltype);
		u.textureType = getUniformTextureType(gltype);
		u.isDepthSampler = isDepthTextureType(gltype);

		if (u.baseType == UNIFORM_MATRIX)
			u.matrix = getMatrixSize(gltype);
		else
			u.components = getUniformTypeComponents(gltype);

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

		if (u.baseType == UNIFORM_SAMPLER && builtin != BUILTIN_TEXTURE_MAIN)
		{
			TextureUnit unit;
			unit.type = u.textureType;
			unit.active = true;
			unit.texture = gl.getDefaultTexture(u.textureType);

			for (int i = 0; i < u.count; i++)
				textureUnits.push_back(unit);
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
				u.dataSize = sizeof(int) * u.components * u.count;
				u.data = malloc(u.dataSize);
				break;
			case UNIFORM_UINT:
				u.dataSize = sizeof(unsigned int) * u.components * u.count;
				u.data = malloc(u.dataSize);
				break;
			case UNIFORM_MATRIX:
				u.dataSize = sizeof(float) * (u.matrix.rows * u.matrix.columns) * u.count;
				u.data = malloc(u.dataSize);
				break;
			default:
				break;
			}

			if (u.dataSize > 0)
			{
				memset(u.data, 0, u.dataSize);

				if (u.baseType == UNIFORM_SAMPLER)
				{
					int startunit = (int) textureUnits.size() - u.count;

					if (builtin == BUILTIN_TEXTURE_MAIN)
						startunit = 0;

					for (int i = 0; i < u.count; i++)
						u.ints[i] = startunit + i;

					glUniform1iv(u.location, u.count, u.ints);

					u.textures = new Texture*[u.count];
					memset(u.textures, 0, sizeof(Texture *) * u.count);
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
					offset += u.matrix.rows * u.matrix.columns;
					break;
				default:
					break;
				}
			}
		}

		uniforms[u.name] = u;

		if (builtin != BUILTIN_MAX_ENUM)
			builtinUniformInfo[(int)builtin] = &uniforms[u.name];

		if (u.baseType == UNIFORM_SAMPLER)
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
	}

	// Make sure uniforms that existed before but don't exist anymore are
	// cleaned up. This theoretically shouldn't happen, but...
	for (const auto &p : olduniforms)
	{
		if (uniforms.find(p.first) == uniforms.end())
		{
			free(p.second.data);

			if (p.second.baseType != UNIFORM_SAMPLER)
				continue;

			for (int i = 0; i < p.second.count; i++)
			{
				if (p.second.textures[i] != nullptr)
					p.second.textures[i]->release();
			}

			delete[] p.second.textures;
		}
	}

	gl.useProgram(activeprogram);
}

bool Shader::loadVolatile()
{
	OpenGL::TempDebugGroup debuggroup("Shader load");

    // Recreating the shader program will invalidate uniforms that rely on these.
	canvasWasActive = false;
    lastViewport = Rect();

	lastPointSize = -1.0f;

	// Invalidate the cached matrices by setting some elements to NaN.
	float nan = std::numeric_limits<float>::quiet_NaN();
	lastProjectionMatrix.setTranslation(nan, nan);
	lastTransformMatrix.setTranslation(nan, nan);

	// zero out active texture list
	textureUnits.clear();
	textureUnits.push_back(TextureUnit());

	for (const auto &stage : stages)
	{
		if (stage.get() != nullptr)
			stage->loadVolatile();
	}

	program = glCreateProgram();

	if (program == 0)
		throw love::Exception("Cannot create shader program object.");

	for (const auto &stage : stages)
	{
		if (stage.get() != nullptr)
			glAttachShader(program, (GLuint) stage->getHandle());
	}

	// Bind generic vertex attribute indices to names in the shader.
	for (int i = 0; i < int(ATTRIB_MAX_ENUM); i++)
	{
		const char *name = nullptr;
		if (vertex::getConstant((BuiltinVertexAttribute) i, name))
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

	for (int i = 0; i < int(ATTRIB_MAX_ENUM); i++)
	{
		const char *name = nullptr;
		if (vertex::getConstant(BuiltinVertexAttribute(i), name))
			builtinAttributes[i] = glGetAttribLocation(program, name);
		else
			builtinAttributes[i] = -1;
	}

	if (current == this)
	{
		// make sure glUseProgram gets called.
		current = nullptr;
		attach();
		updateBuiltinUniforms();
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
		Graphics::flushStreamDrawsGlobal();

		gl.useProgram(program);
		current = this;
		// retain/release happens in Graphics::setShader.

		// Make sure all textures are bound to their respective texture units.
		for (int i = 0; i < (int) textureUnits.size(); ++i)
		{
			const TextureUnit &unit = textureUnits[i];
			if (unit.active)
				gl.bindTextureToUnit(unit.type, unit.texture, i, false, false);
		}

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
		flushStreamDraws();

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
	else if (type == UNIFORM_INT || type == UNIFORM_BOOL || type == UNIFORM_SAMPLER)
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

void Shader::sendTextures(const UniformInfo *info, Texture **textures, int count)
{
	Shader::sendTextures(info, textures, count, false);
}

void Shader::sendTextures(const UniformInfo *info, Texture **textures, int count, bool internalUpdate)
{
	if (info->baseType != UNIFORM_SAMPLER)
		return;

	bool shaderactive = current == this;

	if (!internalUpdate && shaderactive)
		flushStreamDraws();

	count = std::min(count, info->count);

	// Bind the textures to the texture units.
	for (int i = 0; i < count; i++)
	{
		Texture *tex = textures[i];

		if (tex != nullptr)
		{
			if (!tex->isReadable())
			{
				if (internalUpdate)
					continue;
				else
					throw love::Exception("Textures with non-readable formats cannot be sampled from in a shader.");
			}
			else if (info->isDepthSampler != tex->getDepthSampleMode().hasValue)
			{
				if (internalUpdate)
					continue;
				else if (info->isDepthSampler)
					throw love::Exception("Depth comparison samplers in shaders can only be used with depth textures which have depth comparison set.");
				else
					throw love::Exception("Depth textures which have depth comparison set can only be used with depth/shadow samplers in shaders.");
			}
			else if (tex->getTextureType() != info->textureType)
			{
				if (internalUpdate)
					continue;
				else
				{
					const char *textypestr = "unknown";
					const char *shadertextypestr = "unknown";
					Texture::getConstant(tex->getTextureType(), textypestr);
					Texture::getConstant(info->textureType, shadertextypestr);
					throw love::Exception("Texture's type (%s) must match the type of %s (%s).", textypestr, info->name.c_str(), shadertextypestr);
				}
			}

			tex->retain();
		}

		if (info->textures[i] != nullptr)
			info->textures[i]->release();

		info->textures[i] = tex;

		GLuint gltex = 0;
		if (textures[i] != nullptr)
			gltex = (GLuint) tex->getHandle();
		else
			gltex = gl.getDefaultTexture(info->textureType);

		int texunit = info->ints[i];

		if (shaderactive)
			gl.bindTextureToUnit(info->textureType, gltex, texunit, false, false);

		// Store texture id so it can be re-bound to the texture unit later.
		textureUnits[texunit].texture = gltex;
	}
}

void Shader::flushStreamDraws() const
{
	if (current == this)
		Graphics::flushStreamDrawsGlobal();
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

void Shader::setVideoTextures(Texture *ytexture, Texture *cbtexture, Texture *crtexture)
{
	const BuiltinUniform builtins[3] = {
		BUILTIN_TEXTURE_VIDEO_Y,
		BUILTIN_TEXTURE_VIDEO_CB,
		BUILTIN_TEXTURE_VIDEO_CR,
	};

	Texture *textures[3] = {ytexture, cbtexture, crtexture};

	for (int i = 0; i < 3; i++)
	{
		const UniformInfo *info = builtinUniformInfo[builtins[i]];

		if (info != nullptr)
			sendTextures(info, &textures[i], 1, true);
	}
}

void Shader::updateScreenParams()
{
	Rect view = gl.getViewport();

	auto gfx = Module::getInstance<Graphics>(Module::M_GRAPHICS);
	bool canvasActive = gfx->isCanvasActive();

	if ((view == lastViewport && canvasWasActive == canvasActive) || current != this)
		return;

	// In the shader, we do pixcoord.y = gl_FragCoord.y * params.z + params.w.
	// This lets us flip pixcoord.y when needed, to be consistent (drawing with
	// no Canvas active makes the y-values for pixel coordinates flipped.)
	GLfloat params[] = {
		(GLfloat) view.w, (GLfloat) view.h,
		0.0f, 0.0f,
	};

	if (canvasActive)
	{
		// No flipping: pixcoord.y = gl_FragCoord.y * 1.0 + 0.0.
		params[2] = 1.0f;
		params[3] = 0.0f;
	}
	else
	{
		// gl_FragCoord.y is flipped when drawing to the screen, so we un-flip:
		// pixcoord.y = gl_FragCoord.y * -1.0 + height.
		params[2] = -1.0f;
		params[3] = (GLfloat) view.h;
	}

	GLint location = builtinUniforms[BUILTIN_SCREEN_SIZE];
	if (location >= 0)
		glUniform4fv(location, 1, params);

	canvasWasActive = canvasActive;
	lastViewport = view;
}

void Shader::updatePointSize(float size)
{
	if (size == lastPointSize || current != this)
		return;

	GLint location = builtinUniforms[BUILTIN_POINT_SIZE];
	if (location >= 0)
		glUniform1f(location, size);

	lastPointSize = size;
}

void Shader::updateBuiltinUniforms()
{
	if (current != this)
		return;

	updateScreenParams();

	if (GLAD_ES_VERSION_2_0)
		updatePointSize(gl.getPointSize());

	auto gfx = Module::getInstance<graphics::Graphics>(Module::M_GRAPHICS);

	const Matrix4 &curproj = gfx->getProjection();
	const Matrix4 &curxform = gfx->getTransform();

	bool tpmatrixneedsupdate = false;

	// Only upload the matrices if they've changed.
	if (memcmp(curxform.getElements(), lastTransformMatrix.getElements(), sizeof(float) * 16) != 0)
	{
		GLint location = builtinUniforms[BUILTIN_MATRIX_VIEW_FROM_LOCAL];
		if (location >= 0)
			glUniformMatrix4fv(location, 1, GL_FALSE, curxform.getElements());

		// Also upload the re-calculated normal matrix, if possible. The normal
		// matrix is the transpose of the inverse of the rotation portion
		// (top-left 3x3) of the transform matrix.
		location = builtinUniforms[BUILTIN_MATRIX_VIEW_NORMAL_FROM_LOCAL];
		if (location >= 0)
		{
			Matrix3 normalmatrix = Matrix3(curxform).transposedInverse();
			glUniformMatrix3fv(location, 1, GL_FALSE, normalmatrix.getElements());
		}

		tpmatrixneedsupdate = true;
		lastTransformMatrix = curxform;
	}

	if (memcmp(curproj.getElements(), lastProjectionMatrix.getElements(), sizeof(float) * 16) != 0)
	{
		GLint location = builtinUniforms[BUILTIN_MATRIX_CLIP_FROM_VIEW];
		if (location >= 0)
			glUniformMatrix4fv(location, 1, GL_FALSE, curproj.getElements());

		tpmatrixneedsupdate = true;
		lastProjectionMatrix = curproj;
	}

	if (tpmatrixneedsupdate)
	{
		GLint location = builtinUniforms[BUILTIN_MATRIX_CLIP_FROM_LOCAL];
		if (location >= 0)
		{
			Matrix4 tp_matrix(curproj, curxform);
			glUniformMatrix4fv(location, 1, GL_FALSE, tp_matrix.getElements());
		}
	}
}

std::string Shader::getGLSLVersion()
{
	const char *tmp = (const char *) glGetString(GL_SHADING_LANGUAGE_VERSION);

	if (tmp == nullptr)
		return "0.0";

	// the version string always begins with a version number of the format
	//   major_number.minor_number
	// or
	//   major_number.minor_number.release_number
	// we can keep release_number, since it does not affect the check below.
	std::string versionstring(tmp);
	size_t minorendpos = versionstring.find(' ');
	return versionstring.substr(0, minorendpos);
}

bool Shader::isSupported()
{
	return GLAD_ES_VERSION_2_0 || (getGLSLVersion() >= "1.2");
}

int Shader::getUniformTypeComponents(GLenum type) const
{
	if (getUniformBaseType(type) == UNIFORM_SAMPLER)
		return 1;

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
	}

	return m;
}

Shader::UniformType Shader::getUniformBaseType(GLenum type) const
{
	switch (type)
	{
	case GL_INT:
	case GL_INT_VEC2:
	case GL_INT_VEC3:
	case GL_INT_VEC4:
		return UNIFORM_INT;
	case GL_UNSIGNED_INT:
	case GL_UNSIGNED_INT_VEC2:
	case GL_UNSIGNED_INT_VEC3:
	case GL_UNSIGNED_INT_VEC4:
		return UNIFORM_UINT;
	case GL_FLOAT:
	case GL_FLOAT_VEC2:
	case GL_FLOAT_VEC3:
	case GL_FLOAT_VEC4:
		return UNIFORM_FLOAT;
	case GL_FLOAT_MAT2:
	case GL_FLOAT_MAT3:
	case GL_FLOAT_MAT4:
	case GL_FLOAT_MAT2x3:
	case GL_FLOAT_MAT2x4:
	case GL_FLOAT_MAT3x2:
	case GL_FLOAT_MAT3x4:
	case GL_FLOAT_MAT4x2:
	case GL_FLOAT_MAT4x3:
		return UNIFORM_MATRIX;
	case GL_BOOL:
	case GL_BOOL_VEC2:
	case GL_BOOL_VEC3:
	case GL_BOOL_VEC4:
		return UNIFORM_BOOL;
	case GL_SAMPLER_1D:
	case GL_SAMPLER_1D_SHADOW:
	case GL_SAMPLER_1D_ARRAY:
	case GL_SAMPLER_1D_ARRAY_SHADOW:
	case GL_SAMPLER_2D:
	case GL_SAMPLER_2D_MULTISAMPLE:
	case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
	case GL_SAMPLER_2D_RECT:
	case GL_SAMPLER_2D_RECT_SHADOW:
	case GL_SAMPLER_2D_SHADOW:
	case GL_SAMPLER_2D_ARRAY:
	case GL_SAMPLER_2D_ARRAY_SHADOW:
	case GL_SAMPLER_3D:
	case GL_SAMPLER_CUBE:
	case GL_SAMPLER_CUBE_SHADOW:
	case GL_SAMPLER_CUBE_MAP_ARRAY:
	case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW:
		return UNIFORM_SAMPLER;
	default:
		return UNIFORM_UNKNOWN;
	}
}

TextureType Shader::getUniformTextureType(GLenum type) const
{
	switch (type)
	{
	case GL_SAMPLER_1D:
	case GL_SAMPLER_1D_SHADOW:
	case GL_SAMPLER_1D_ARRAY:
	case GL_SAMPLER_1D_ARRAY_SHADOW:
		// 1D-typed textures are not supported.
		return TEXTURE_MAX_ENUM;
	case GL_SAMPLER_2D:
	case GL_SAMPLER_2D_SHADOW:
		return TEXTURE_2D;
	case GL_SAMPLER_2D_MULTISAMPLE:
	case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
		// Multisample textures are not supported.
		return TEXTURE_MAX_ENUM;
	case GL_SAMPLER_2D_RECT:
	case GL_SAMPLER_2D_RECT_SHADOW:
		// Rectangle textures are not supported.
		return TEXTURE_MAX_ENUM;
	case GL_SAMPLER_2D_ARRAY:
	case GL_SAMPLER_2D_ARRAY_SHADOW:
		return TEXTURE_2D_ARRAY;
	case GL_SAMPLER_3D:
		return TEXTURE_VOLUME;
	case GL_SAMPLER_CUBE:
	case GL_SAMPLER_CUBE_SHADOW:
		return TEXTURE_CUBE;
	case GL_SAMPLER_CUBE_MAP_ARRAY:
	case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW:
		// Cubemap array textures are not supported.
		return TEXTURE_MAX_ENUM;
	default:
		return TEXTURE_MAX_ENUM;
	}
}

bool Shader::isDepthTextureType(GLenum type) const
{
	switch (type)
	{
	case GL_SAMPLER_1D_SHADOW:
	case GL_SAMPLER_1D_ARRAY_SHADOW:
	case GL_SAMPLER_2D_SHADOW:
	case GL_SAMPLER_2D_ARRAY_SHADOW:
	case GL_SAMPLER_CUBE_SHADOW:
	case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW:
		return true;
	default:
		return false;
	}
}

} // opengl
} // graphics
} // love
