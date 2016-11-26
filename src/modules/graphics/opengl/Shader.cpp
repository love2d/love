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

namespace
{
	// temporarily attaches a shader program (for setting uniforms, etc)
	// reattaches the originally active program when destroyed
	struct TemporaryAttacher
	{
		TemporaryAttacher(Shader *shader, bool attachNow)
		: curShader(shader)
		, prevShader(Shader::current)
		{
			if (attachNow)
				attach();
		}

		~TemporaryAttacher()
		{
			if (prevShader != nullptr)
				prevShader->attach();
			else
				curShader->detach();
		}

		void attach()
		{
			curShader->attach(true);
		}

		Shader *curShader;
		Shader *prevShader;
	};
} // anonymous namespace


Shader *Shader::current = nullptr;
Shader *Shader::defaultShader = nullptr;
Shader *Shader::defaultVideoShader = nullptr;

Shader::ShaderSource Shader::defaultCode[Graphics::RENDERER_MAX_ENUM][2];
Shader::ShaderSource Shader::defaultVideoCode[Graphics::RENDERER_MAX_ENUM][2];

Shader::Shader(const ShaderSource &source)
	: shaderSource(source)
	, program(0)
	, builtinUniforms()
	, builtinAttributes()
	, canvasWasActive(false)
	, lastViewport()
	, lastPointSize(0.0f)
	, videoTextureUnits()
{
	if (source.vertex.empty() && source.pixel.empty())
		throw love::Exception("Cannot create shader: no source code!");

	// load shader source and create program object
	loadVolatile();
}

Shader::~Shader()
{
	if (current == this)
		detach();

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

GLuint Shader::compileCode(ShaderStage stage, const std::string &code)
{
	GLenum glstage;
	const char *typestr;

	if (!stageNames.find(stage, typestr))
		typestr = "";

	switch (stage)
	{
	case STAGE_VERTEX:
		glstage = GL_VERTEX_SHADER;
		break;
	case STAGE_PIXEL:
		glstage = GL_FRAGMENT_SHADER;
		break;
	default:
		throw love::Exception("Cannot create shader object: unknown shader type.");
		break;
	}

	GLuint shaderid = glCreateShader(glstage);

	if (shaderid == 0)
	{
		if (glGetError() == GL_INVALID_ENUM)
			throw love::Exception("Cannot create %s shader object: %s shaders not supported.", typestr, typestr);
		else
			throw love::Exception("Cannot create %s shader object.", typestr);
	}

	const char *src = code.c_str();
	GLint srclen = (GLint) code.length();
	glShaderSource(shaderid, 1, (const GLchar **)&src, &srclen);

	glCompileShader(shaderid);

	GLint infologlen;
	glGetShaderiv(shaderid, GL_INFO_LOG_LENGTH, &infologlen);

	// Get any warnings the shader compiler may have produced.
	if (infologlen > 0)
	{
		GLchar *infolog = new GLchar[infologlen];
		glGetShaderInfoLog(shaderid, infologlen, nullptr, infolog);

		// Save any warnings for later querying.
		shaderWarnings[stage] = infolog;

		delete[] infolog;
	}

	GLint status;
	glGetShaderiv(shaderid, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE)
	{
		glDeleteShader(shaderid);
		throw love::Exception("Cannot compile %s shader code:\n%s",
		                      typestr, shaderWarnings[stage].c_str());
	}

	return shaderid;
}

void Shader::mapActiveUniforms()
{
	// Built-in uniform locations default to -1 (nonexistant.)
	for (int i = 0; i < int(BUILTIN_MAX_ENUM); i++)
		builtinUniforms[i] = -1;

	GLint activeprogram = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &activeprogram);

	gl.useProgram(program);

	GLint numuniforms;
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numuniforms);

	GLchar cname[256];
	const GLint bufsize = (GLint) (sizeof(cname) / sizeof(GLchar));

	std::map<std::string, UniformInfo> olduniforms = uniforms;
	uniforms.clear();

	for (int i = 0; i < numuniforms; i++)
	{
		GLsizei namelen = 0;
		GLenum gltype = 0;
		UniformInfo u = {};

		glGetActiveUniform(program, (GLuint) i, bufsize, &namelen, &u.count, &gltype, cname);

		u.name = std::string(cname, (size_t) namelen);
		u.location = glGetUniformLocation(program, u.name.c_str());
		u.baseType = getUniformBaseType(gltype);

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
		BuiltinUniform builtin;
		if (builtinNames.find(u.name.c_str(), builtin))
			builtinUniforms[int(builtin)] = u.location;

		if (u.location == -1)
			continue;

		if (u.baseType == UNIFORM_SAMPLER)
		{
			// Initialize all samplers to 0. Both GLSL and GLSL ES are supposed
			// to do this themselves, but some Android devices (galaxy tab 3 and
			// 4) don't seem to do it...
			// NOTE: We also restore previously set texture units below.
			glUniform1iv(u.location, u.count, u.ints);
		}

		// Make sure previously set uniform data is preserved, and shader-
		// initialized values are retrieved.
		auto oldu = olduniforms.find(u.name);
		if (oldu != olduniforms.end())
		{
			u.data = oldu->second.data;
			u.textures = oldu->second.textures;

			updateUniform(&u, u.count, true);

			if (u.baseType == UNIFORM_SAMPLER)
			{
				// Make sure all stored textures have their Volatiles loaded
				// before the sendTextures call, since it calls getHandle().
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
		else
		{
			size_t datasize = 0;

			switch (u.baseType)
			{
			case UNIFORM_FLOAT:
				datasize = sizeof(float) * u.components * u.count;
				u.data = malloc(datasize);
				break;
			case UNIFORM_INT:
			case UNIFORM_BOOL:
			case UNIFORM_SAMPLER:
				datasize = sizeof(int) * u.components * u.count;
				u.data = malloc(datasize);
				break;
			case UNIFORM_MATRIX:
				datasize = sizeof(float) * (u.matrix.rows * u.matrix.columns) * u.count;
				u.data = malloc(datasize);
				break;
			default:
				break;
			}

			if (datasize > 0)
			{
				memset(u.data, 0, datasize);

				if (u.baseType == UNIFORM_SAMPLER)
				{
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
    lastViewport = OpenGL::Viewport();

	lastPointSize = -1.0f;

	// Invalidate the cached matrices by setting some elements to NaN.
	float nan = std::numeric_limits<float>::quiet_NaN();
	lastProjectionMatrix.setTranslation(nan, nan);
	lastTransformMatrix.setTranslation(nan, nan);

	for (int i = 0; i < 3; i++)
		videoTextureUnits[i] = 0;

	// zero out active texture list
	textureUnits.clear();
	textureUnits.resize(gl.getMaxTextureUnits(), TextureUnit());

	std::vector<GLuint> shaderids;

	bool gammacorrect = graphics::isGammaCorrect();
	const ShaderSource *defaults = &defaultCode[Graphics::RENDERER_OPENGL][gammacorrect ? 1 : 0];
	if (GLAD_ES_VERSION_2_0)
		defaults = &defaultCode[Graphics::RENDERER_OPENGLES][gammacorrect ? 1 : 0];

	// The shader program must have both vertex and pixel shader stages.
	const std::string &vertexcode = shaderSource.vertex.empty() ? defaults->vertex : shaderSource.vertex;
	const std::string &pixelcode = shaderSource.pixel.empty() ? defaults->pixel : shaderSource.pixel;

	try
	{
		shaderids.push_back(compileCode(STAGE_VERTEX, vertexcode));
		shaderids.push_back(compileCode(STAGE_PIXEL, pixelcode));
	}
	catch (love::Exception &)
	{
		for (GLuint id : shaderids)
			glDeleteShader(id);
		throw;
	}

	program = glCreateProgram();

	if (program == 0)
	{
		for (GLuint id : shaderids)
			glDeleteShader(id);
		throw love::Exception("Cannot create shader program object.");
	}

	for (GLuint id : shaderids)
		glAttachShader(program, id);

	// Bind generic vertex attribute indices to names in the shader.
	for (int i = 0; i < int(ATTRIB_MAX_ENUM); i++)
	{
		const char *name = nullptr;
		if (attribNames.find((VertexAttribID) i, name))
			glBindAttribLocation(program, i, (const GLchar *) name);
	}

	glLinkProgram(program);

	// Flag shaders for auto-deletion when the program object is deleted.
	for (GLuint id : shaderids)
		glDeleteShader(id);

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
		if (attribNames.find(VertexAttribID(i), name))
			builtinAttributes[i] = glGetAttribLocation(program, name);
		else
			builtinAttributes[i] = -1;
	}

	if (current == this)
	{
		// make sure glUseProgram gets called.
		current = nullptr;
		attach();
		checkSetBuiltinUniforms();
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
	textureUnits.resize(gl.getMaxTextureUnits(), TextureUnit());

	attributes.clear();

	// And the locations of any built-in uniform variables.
	for (int i = 0; i < int(BUILTIN_MAX_ENUM); i++)
		builtinUniforms[i] = -1;

	shaderWarnings.clear();
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

	// Get the individual shader stage warnings
	for (const auto &warning : shaderWarnings)
	{
		if (stageNames.find(warning.first, stagestr))
			warnings += std::string(stagestr) + std::string(" shader:\n") + warning.second;
	}

	warnings += getProgramWarnings();

	return warnings;
}

void Shader::attach(bool temporary)
{
	if (current != this)
	{
		gl.useProgram(program);
		current = this;
		// retain/release happens in Graphics::setShader.

		if (!temporary)
		{
			// make sure all sent textures are properly bound to their respective texture units
			// note: list potentially contains texture ids of deleted/invalid textures!
			for (int i = 1; i < (int) textureUnits.size(); ++i)
			{
				if (textureUnits[i].active)
					gl.bindTextureToUnit(textureUnits[i].texture, i, false);
			}
		}
	}
}

void Shader::detach()
{
	if (defaultShader)
	{
		if (current != defaultShader)
			defaultShader->attach();

		return;
	}

	if (current != nullptr)
		gl.useProgram(0);

	current = nullptr;
}

const Shader::UniformInfo *Shader::getUniformInfo(const std::string &name) const
{
	const auto it = uniforms.find(name);

	if (it == uniforms.end())
		return nullptr;

	return &(it->second);
}

void Shader::updateUniform(const UniformInfo *info, int count, bool internalUpdate)
{
	TemporaryAttacher attacher(this, !internalUpdate);

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

int Shader::getFreeTextureUnits(int count)
{
	int startunit = -1;

	// Ignore the first texture unit for Shader-local texture bindings.
	for (int i = 1; i < (int) textureUnits.size(); i++)
	{
		if (!textureUnits[i].active && i + count <= (int) textureUnits.size())
		{
			startunit = i;
			break;
		}
	}

	if (startunit == -1)
		throw love::Exception("No more texture units available for shader.");

	return startunit;
}

void Shader::sendTextures(const UniformInfo *info, Texture **textures, int count, bool internalUpdate)
{
	if (info->baseType != UNIFORM_SAMPLER)
		return;

	count = std::min(count, info->count);
	bool updateuniform = internalUpdate;

	// Make sure the shader's samplers are associated with texture units.
	for (int i = 0; i < count; i++)
	{
		if (info->ints[i] == 0 && textures[i] != nullptr)
		{
			int texunit = getFreeTextureUnits(1);
			textureUnits[texunit].active = true;

			info->ints[i] = texunit;
			updateuniform = true;
		}
	}

	if (updateuniform)
		updateUniform(info, count, internalUpdate);

	// Bind the textures to the texture units.
	for (int i = 0; i < count; i++)
	{
		if (textures[i] != nullptr)
			textures[i]->retain();

		if (info->textures[i] != nullptr)
			info->textures[i]->release();

		info->textures[i] = textures[i];

		int texunit = info->ints[i];

		if (textures[i] != nullptr)
		{
			GLuint gltex = *(GLuint *) textures[i]->getHandle();

			gl.bindTextureToUnit(gltex, texunit, false);

			// store texture id so it can be re-bound to the proper texture unit later
			textureUnits[texunit].texture = gltex;
		}
		else
		{
			gl.bindTextureToUnit(0, texunit, false);
			textureUnits[texunit].texture = 0;
			textureUnits[texunit].active = false;
		}
	}
}

bool Shader::hasUniform(const std::string &name) const
{
	return uniforms.find(name) != uniforms.end();
}

GLint Shader::getAttribLocation(const std::string &name)
{
	auto it = attributes.find(name);
	if (it != attributes.end())
		return it->second;

	GLint location = glGetAttribLocation(program, name.c_str());

	attributes[name] = location;
	return location;
}

bool Shader::hasVertexAttrib(VertexAttribID attrib) const
{
	return builtinAttributes[int(attrib)] != -1;
}

void Shader::setVideoTextures(GLuint ytexture, GLuint cbtexture, GLuint crtexture)
{
	// Set up the texture units that will be used by the shader to sample from
	// the textures, if they haven't been set up yet.
	if (videoTextureUnits[0] == 0)
	{
		TemporaryAttacher attacher(this, true);

		const GLint locs[3] = {
			builtinUniforms[BUILTIN_VIDEO_Y_CHANNEL],
			builtinUniforms[BUILTIN_VIDEO_CB_CHANNEL],
			builtinUniforms[BUILTIN_VIDEO_CR_CHANNEL]
		};

		const char *names[3] = {nullptr, nullptr, nullptr};
		builtinNames.find(BUILTIN_VIDEO_Y_CHANNEL,  names[0]);
		builtinNames.find(BUILTIN_VIDEO_CB_CHANNEL, names[1]);
		builtinNames.find(BUILTIN_VIDEO_CR_CHANNEL, names[2]);

		for (int i = 0; i < 3; i++)
		{
			if (locs[i] >= 0 && names[i] != nullptr)
			{
				const UniformInfo *info = getUniformInfo(names[i]);
				if (info != nullptr)
				{
					videoTextureUnits[i] = getFreeTextureUnits(1);
					textureUnits[videoTextureUnits[i]].active = true;

					info->ints[0] = videoTextureUnits[i];
					updateUniform(info, 1);
				}
			}
		}
	}

	const GLuint textures[3] = {ytexture, cbtexture, crtexture};

	// Bind the textures to their respective texture units.
	for (int i = 0; i < 3; i++)
	{
		if (videoTextureUnits[i] != 0)
		{
			// Store texture id so it can be re-bound later.
			textureUnits[videoTextureUnits[i]].texture = textures[i];
			gl.bindTextureToUnit(textures[i], videoTextureUnits[i], false);
		}
	}
}

void Shader::checkSetScreenParams()
{
	OpenGL::Viewport view = gl.getViewport();

	auto gfx = Module::getInstance<Graphics>(Module::M_GRAPHICS);
	bool canvasActive = gfx->getActivePass().colorAttachmentCount > 0;

	if (view == lastViewport && canvasWasActive == canvasActive)
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
	{
		TemporaryAttacher attacher(this, true);
		glUniform4fv(location, 1, params);
	}

	canvasWasActive = canvasActive;
	lastViewport = view;
}

void Shader::checkSetPointSize(float size)
{
	if (size == lastPointSize)
		return;

	GLint location = builtinUniforms[BUILTIN_POINT_SIZE];

	if (location >= 0)
	{
		TemporaryAttacher attacher(this, true);
		glUniform1f(location, size);
	}

	lastPointSize = size;
}

void Shader::checkSetBuiltinUniforms()
{
	checkSetScreenParams();

	// We use a more efficient method for sending transformation matrices to
	// the GPU on desktop GL.
	if (GLAD_ES_VERSION_2_0)
	{
		checkSetPointSize(gl.getPointSize());

		const Matrix4 &curxform = gl.matrices.transform.back();
		const Matrix4 &curproj = gl.matrices.projection;

		TemporaryAttacher attacher(this, true);

		bool tpmatrixneedsupdate = false;

		// Only upload the matrices if they've changed.
		if (memcmp(curxform.getElements(), lastTransformMatrix.getElements(), sizeof(float) * 16) != 0)
		{
			GLint location = builtinUniforms[BUILTIN_TRANSFORM_MATRIX];
			if (location >= 0)
				glUniformMatrix4fv(location, 1, GL_FALSE, curxform.getElements());

			// Also upload the re-calculated normal matrix, if possible. The
			// normal matrix is the transpose of the inverse of the rotation
			// portion (top-left 3x3) of the transform matrix.
			location = builtinUniforms[BUILTIN_NORMAL_MATRIX];
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
			GLint location = builtinUniforms[BUILTIN_PROJECTION_MATRIX];
			if (location >= 0)
				glUniformMatrix4fv(location, 1, GL_FALSE, curproj.getElements());

			tpmatrixneedsupdate = true;
			lastProjectionMatrix = curproj;
		}

		if (tpmatrixneedsupdate)
		{
			GLint location = builtinUniforms[BUILTIN_TRANSFORM_PROJECTION_MATRIX];
			if (location >= 0)
			{
				Matrix4 tp_matrix(curproj * curxform);
				glUniformMatrix4fv(location, 1, GL_FALSE, tp_matrix.getElements());
			}
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
	switch (type)
	{
	case GL_INT:
	case GL_FLOAT:
	case GL_BOOL:
	case GL_SAMPLER_1D:
	case GL_SAMPLER_2D:
	case GL_SAMPLER_3D:
		return 1;
	case GL_INT_VEC2:
	case GL_FLOAT_VEC2:
	case GL_FLOAT_MAT2:
	case GL_BOOL_VEC2:
		return 2;
	case GL_INT_VEC3:
	case GL_FLOAT_VEC3:
	case GL_FLOAT_MAT3:
	case GL_BOOL_VEC3:
		return 3;
	case GL_INT_VEC4:
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

bool Shader::getConstant(const char *in, UniformType &out)
{
	return uniformTypes.find(in, out);
}

bool Shader::getConstant(UniformType in, const char *&out)
{
	return uniformTypes.find(in, out);
}

bool Shader::getConstant(const char *in, VertexAttribID &out)
{
	return attribNames.find(in, out);
}

bool Shader::getConstant(VertexAttribID in, const char *&out)
{
	return attribNames.find(in, out);
}

StringMap<Shader::ShaderStage, Shader::STAGE_MAX_ENUM>::Entry Shader::stageNameEntries[] =
{
	{"vertex", Shader::STAGE_VERTEX},
	{"pixel", Shader::STAGE_PIXEL},
};

StringMap<Shader::ShaderStage, Shader::STAGE_MAX_ENUM> Shader::stageNames(Shader::stageNameEntries, sizeof(Shader::stageNameEntries));

StringMap<Shader::UniformType, Shader::UNIFORM_MAX_ENUM>::Entry Shader::uniformTypeEntries[] =
{
	{"float", Shader::UNIFORM_FLOAT},
	{"matrix", Shader::UNIFORM_MATRIX},
	{"int", Shader::UNIFORM_INT},
	{"bool", Shader::UNIFORM_BOOL},
	{"image", Shader::UNIFORM_SAMPLER},
	{"unknown", Shader::UNIFORM_UNKNOWN},
};

StringMap<Shader::UniformType, Shader::UNIFORM_MAX_ENUM> Shader::uniformTypes(Shader::uniformTypeEntries, sizeof(Shader::uniformTypeEntries));

StringMap<VertexAttribID, ATTRIB_MAX_ENUM>::Entry Shader::attribNameEntries[] =
{
	{"VertexPosition", ATTRIB_POS},
	{"VertexTexCoord", ATTRIB_TEXCOORD},
	{"VertexColor", ATTRIB_COLOR},
	{"ConstantColor", ATTRIB_CONSTANTCOLOR},
};

StringMap<VertexAttribID, ATTRIB_MAX_ENUM> Shader::attribNames(Shader::attribNameEntries, sizeof(Shader::attribNameEntries));

StringMap<Shader::BuiltinUniform, Shader::BUILTIN_MAX_ENUM>::Entry Shader::builtinNameEntries[] =
{
	{"TransformMatrix", Shader::BUILTIN_TRANSFORM_MATRIX},
	{"ProjectionMatrix", Shader::BUILTIN_PROJECTION_MATRIX},
	{"TransformProjectionMatrix", Shader::BUILTIN_TRANSFORM_PROJECTION_MATRIX},
	{"NormalMatrix", Shader::BUILTIN_NORMAL_MATRIX},
	{"love_PointSize", Shader::BUILTIN_POINT_SIZE},
	{"love_ScreenSize", Shader::BUILTIN_SCREEN_SIZE},
	{"love_VideoYChannel", Shader::BUILTIN_VIDEO_Y_CHANNEL},
	{"love_VideoCbChannel", Shader::BUILTIN_VIDEO_CB_CHANNEL},
	{"love_VideoCrChannel", Shader::BUILTIN_VIDEO_CR_CHANNEL},
};

StringMap<Shader::BuiltinUniform, Shader::BUILTIN_MAX_ENUM> Shader::builtinNames(Shader::builtinNameEntries, sizeof(Shader::builtinNameEntries));

} // opengl
} // graphics
} // love
