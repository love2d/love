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

#include "common/config.h"

#include "Shader.h"
#include "Graphics.h"

#include <algorithm>

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
		TemporaryAttacher(Shader *shader)
		: curShader(shader)
		, prevShader(Shader::current)
		{
			curShader->attach(true);
		}

		~TemporaryAttacher()
		{
			if (prevShader != NULL)
				prevShader->attach();
			else
				curShader->detach();
		}

		Shader *curShader;
		Shader *prevShader;
	};
} // anonymous namespace


Shader *Shader::current = NULL;

GLint Shader::maxTextureUnits = 0;
std::vector<int> Shader::textureCounters;

Shader::Shader(const ShaderSources &sources)
	: shaderSources(sources)
	, program(0)
{
	if (shaderSources.empty())
		throw love::Exception("Cannot create shader: no source code!");

	if (maxTextureUnits <= 0)
	{
		GLint maxtexunits;
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxtexunits);
		maxTextureUnits = std::max(maxtexunits - 1, 0);
	}

	// initialize global texture id counters if needed
	if (textureCounters.size() < (size_t) maxTextureUnits)
		textureCounters.resize(maxTextureUnits, 0);

	// load shader source and create program object
	loadVolatile();
}

Shader::~Shader()
{
	if (current == this)
		detach();

	unloadVolatile();
}

GLuint Shader::compileCode(ShaderType type, const std::string &code)
{
	GLenum glshadertype;
	const char *typestr;

	if (!typeNames.find(type, typestr))
		typestr = "";

	switch (type)
	{
	case TYPE_VERTEX:
		glshadertype = GL_VERTEX_SHADER;
		break;
	case TYPE_PIXEL:
		glshadertype = GL_FRAGMENT_SHADER;
		break;
	default:
		throw love::Exception("Cannot create shader object: unknown shader type.");
		break;
	}

	// clear existing errors
	while (glGetError() != GL_NO_ERROR);

	GLuint shaderid = glCreateShader(glshadertype);

	if (shaderid == 0) // oh no!
	{
		GLenum err = glGetError();

		if (err == GL_INVALID_ENUM)
			throw love::Exception("Cannot create %s shader object: %s shaders not supported.", typestr, typestr);
		else
			throw love::Exception("Cannot create %s shader object.", typestr);
	}

	const char *src = code.c_str();
	size_t srclen = code.length();
	glShaderSource(shaderid, 1, (const GLchar **)&src, (GLint *)&srclen);

	glCompileShader(shaderid);

	// Get any warnings the shader compiler may have produced.
	GLint infologlen;
	glGetShaderiv(shaderid, GL_INFO_LOG_LENGTH, &infologlen);

	GLchar *infolog = new GLchar[infologlen + 1];
	glGetShaderInfoLog(shaderid, infologlen, NULL, infolog);

	// Save any warnings for later querying.
	if (infologlen > 0)
		shaderWarnings[type] = infolog;

	delete[] infolog;

	GLint status;
	glGetShaderiv(shaderid, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE)
	{
		throw love::Exception("Cannot compile %s shader code:\n%s",
		                      typestr, shaderWarnings[type].c_str());
	}

	return shaderid;
}

void Shader::createProgram(const std::vector<GLuint> &shaderids)
{
	program = glCreateProgram();
	if (program == 0)
		throw love::Exception("Cannot create shader program object.");

	std::vector<GLuint>::const_iterator it;
	for (it = shaderids.begin(); it != shaderids.end(); ++it)
		glAttachShader(program, *it);

	glLinkProgram(program);

	// flag shaders for auto-deletion when the program object is deleted.
	for (it = shaderids.begin(); it != shaderids.end(); ++it)
		glDeleteShader(*it);

	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);

	if (status == GL_FALSE)
	{
		std::string warnings = getProgramWarnings();
		glDeleteProgram(program);
		program = 0;

		throw love::Exception("Cannot link shader program object:\n%s", warnings.c_str());
	}
}

void Shader::mapActiveUniforms()
{
	uniforms.clear();

	GLint numuniforms;
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numuniforms);

	GLsizei bufsize;
	glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, (GLint *) &bufsize);

	if (bufsize <= 0)
		return;

	for (int i = 0; i < numuniforms; i++)
	{
		GLchar *cname = new GLchar[bufsize];
		GLsizei namelength;

		Uniform u;

		glGetActiveUniform(program, (GLuint) i, bufsize, &namelength, &u.count, &u.type, cname);

		u.name = std::string(cname, (size_t) namelength);
		u.location = glGetUniformLocation(program, u.name.c_str());
		u.baseType = getUniformBaseType(u.type);

		delete[] cname;

		// glGetActiveUniform appends "[0]" to the end of array uniform names...
		if (u.name.length() > 3)
		{
			size_t findpos = u.name.find("[0]");
			if (findpos != std::string::npos && findpos == u.name.length() - 3)
				u.name.erase(u.name.length() - 3);
		}

		if (u.location != -1)
			uniforms[u.name] = u;
	}
}

bool Shader::loadVolatile()
{
	// zero out active texture list
	activeTextureUnits.clear();
	activeTextureUnits.insert(activeTextureUnits.begin(), maxTextureUnits, 0);

	std::vector<GLuint> shaderids;

	ShaderSources::const_iterator source;
	for (source = shaderSources.begin(); source != shaderSources.end(); ++source)
	{
		GLuint shaderid = compileCode(source->first, source->second);
		shaderids.push_back(shaderid);
	}

	if (shaderids.empty())
		throw love::Exception("Cannot create shader: no valid source code!");

	createProgram(shaderids);

	// Retreive all active uniform variables in this shader from OpenGL.
	mapActiveUniforms();

	if (current == this)
	{
		// make sure glUseProgram gets called.
		current = NULL;
		attach();
	}

	return true;
}

void Shader::unloadVolatile()
{
	if (current == this)
		glUseProgram(0);

	if (program != 0)
	{
		glDeleteProgram(program);
		program = 0;
	}

	// decrement global texture id counters for texture units which had textures bound from this shader
	for (size_t i = 0; i < activeTextureUnits.size(); ++i)
	{
		if (activeTextureUnits[i] > 0)
			textureCounters[i] = std::max(textureCounters[i] - 1, 0);
	}

	// active texture list is probably invalid, clear it
	activeTextureUnits.clear();
	activeTextureUnits.insert(activeTextureUnits.begin(), maxTextureUnits, 0);

	// same with uniform location list
	uniforms.clear();

	shaderWarnings.clear();
}

std::string Shader::getProgramWarnings() const
{
	GLint strlen, nullpos;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &strlen);

	char *tempstr = new char[strlen+1];
	// be extra sure that the error string will be 0-terminated
	memset(tempstr, '\0', strlen+1);
	glGetProgramInfoLog(program, strlen, &nullpos, tempstr);
	tempstr[nullpos] = '\0';

	std::string warnings(tempstr);
	delete[] tempstr;

	return warnings;
}

std::string Shader::getWarnings() const
{
	std::string warnings;
	const char *typestr;

	// Get the individual shader stage warnings
	std::map<ShaderType, std::string>::const_iterator it;
	for (it = shaderWarnings.begin(); it != shaderWarnings.end(); ++it)
	{
		if (typeNames.find(it->first, typestr))
			warnings += std::string(typestr) + std::string(" shader:\n") + it->second;
	}

	warnings += getProgramWarnings();

	return warnings;
}

void Shader::attach(bool temporary)
{
	if (current != this)
	{
		glUseProgram(program);
		current = this;
	}

	if (!temporary)
	{
		// make sure all sent textures are properly bound to their respective texture units
		// note: list potentially contains texture ids of deleted/invalid textures!
		for (size_t i = 0; i < activeTextureUnits.size(); ++i)
		{
			if (activeTextureUnits[i] > 0)
				gl.bindTextureToUnit(activeTextureUnits[i], i + 1, false);
		}

		// We always want to use texture unit 0 for everyhing else.
		gl.setActiveTextureUnit(0);
	}
}

void Shader::detach()
{
	if (current != NULL)
		glUseProgram(0);

	current = NULL;
}

const Shader::Uniform &Shader::getUniform(const std::string &name) const
{
	std::map<std::string, Uniform>::const_iterator it = uniforms.find(name);

	if (it == uniforms.end())
		throw love::Exception("Variable '%s' does not exist.\n"
		                      "A common error is to define but not use the variable.", name.c_str());

	return it->second;
}

int Shader::getUniformTypeSize(GLenum type) const
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
		break;
	}

	return 1;
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
	case GL_FLOAT_MAT2:
	case GL_FLOAT_MAT3:
	case GL_FLOAT_MAT4:
		return UNIFORM_FLOAT;
	case GL_BOOL:
	case GL_BOOL_VEC2:
	case GL_BOOL_VEC3:
	case GL_BOOL_VEC4:
		return UNIFORM_BOOL;
	case GL_SAMPLER_1D:
	case GL_SAMPLER_2D:
	case GL_SAMPLER_3D:
		return UNIFORM_SAMPLER;
	default:
		break;
	}

	return UNIFORM_UNKNOWN;
}

void Shader::checkSetUniformError(const Uniform &u, int size, int count, UniformType sendtype) const
{
	if (!program)
		throw love::Exception("No active shader program.");

	int realsize = getUniformTypeSize(u.type);

	if (size != realsize)
		throw love::Exception("Value size of %d does not match variable size of %d.", size, realsize);

	if ((u.count == 1 && count > 1) || count < 0)
		throw love::Exception("Invalid number of values (expected %d, got %d).", u.count, count);

	if (u.baseType == UNIFORM_SAMPLER && sendtype != u.baseType)
		throw love::Exception("Cannot send a value of this type to an Image variable.");

	if ((sendtype == UNIFORM_FLOAT && u.baseType == UNIFORM_INT) || (sendtype == UNIFORM_INT && u.baseType == UNIFORM_FLOAT))
		throw love::Exception("Cannot convert between float and int.");
}

void Shader::sendInt(const std::string &name, int size, const GLint *vec, int count)
{
	TemporaryAttacher attacher(this);

	const Uniform &u = getUniform(name);
	checkSetUniformError(u, size, count, UNIFORM_INT);

	switch (size)
	{
	case 4:
		glUniform4iv(u.location, count, vec);
		break;
	case 3:
		glUniform3iv(u.location, count, vec);
		break;
	case 2:
		glUniform2iv(u.location, count, vec);
		break;
	case 1:
	default:
		glUniform1iv(u.location, count, vec);
		break;
	}
}

void Shader::sendFloat(const std::string &name, int size, const GLfloat *vec, int count)
{
	TemporaryAttacher attacher(this);

	const Uniform &u = getUniform(name);
	checkSetUniformError(u, size, count, UNIFORM_FLOAT);

	switch (size)
	{
	case 4:
		glUniform4fv(u.location, count, vec);
		break;
	case 3:
		glUniform3fv(u.location, count, vec);
		break;
	case 2:
		glUniform2fv(u.location, count, vec);
		break;
	case 1:
	default:
		glUniform1fv(u.location, count, vec);
		break;
	}
}

void Shader::sendMatrix(const std::string &name, int size, const GLfloat *m, int count)
{
	TemporaryAttacher attacher(this);

	if (size < 2 || size > 4)
	{
		throw love::Exception("Invalid matrix size: %dx%d "
							  "(can only set 2x2, 3x3 or 4x4 matrices.)", size,size);
	}

	const Uniform &u = getUniform(name);
	checkSetUniformError(u, size, count, UNIFORM_FLOAT);

	switch (size)
	{
	case 4:
		glUniformMatrix4fv(u.location, count, GL_FALSE, m);
		break;
	case 3:
		glUniformMatrix3fv(u.location, count, GL_FALSE, m);
		break;
	case 2:
	default:
		glUniformMatrix2fv(u.location, count, GL_FALSE, m);
		break;
	}
}

void Shader::sendTexture(const std::string &name, GLuint texture)
{
	TemporaryAttacher attacher(this);

	int textureunit = getTextureUnit(name);

	const Uniform &u = getUniform(name);
	checkSetUniformError(u, 1, 1, UNIFORM_SAMPLER);

	// bind texture to assigned texture unit and send uniform to shader program
	gl.bindTextureToUnit(texture, textureunit, false);

	glUniform1i(u.location, textureunit);

	// reset texture unit
	gl.setActiveTextureUnit(0);

	// increment global shader texture id counter for this texture unit, if we haven't already
	if (activeTextureUnits[textureunit-1] == 0)
		++textureCounters[textureunit-1];

	// store texture id so it can be re-bound to the proper texture unit when necessary
	activeTextureUnits[textureunit-1] = texture;
}

void Shader::sendImage(const std::string &name, const Image &image)
{
	sendTexture(name, image.getTextureName());
}

void Shader::sendCanvas(const std::string &name, const Canvas &canvas)
{
	sendTexture(name, canvas.getTextureName());
}

int Shader::getTextureUnit(const std::string &name)
{
	std::map<std::string, GLint>::const_iterator it = textureUnitPool.find(name);

	if (it != textureUnitPool.end())
		return it->second;

	int textureunit = 1;

	// prefer texture units which are unused by all other shaders
	std::vector<int>::iterator nextfreeunit = std::find(textureCounters.begin(), textureCounters.end(), 0);

	if (nextfreeunit != textureCounters.end())
		textureunit = std::distance(textureCounters.begin(), nextfreeunit) + 1; // we don't want to use unit 0
	else
	{
		// no completely unused texture units exist, try to use next free slot in our own list
		std::vector<GLuint>::iterator nexttexunit = std::find(activeTextureUnits.begin(), activeTextureUnits.end(), 0);

		if (nexttexunit == activeTextureUnits.end())
			throw love::Exception("No more texture units available for shader.");

		textureunit = std::distance(activeTextureUnits.begin(), nexttexunit) + 1; // we don't want to use unit 0
	}

	textureUnitPool[name] = textureunit;
	return textureunit;
}

std::string Shader::getGLSLVersion()
{
	const char *tmp = 0;

	// GL_SHADING_LANGUAGE_VERSION isn't available in OpenGL < 2.0.
	if (GL_VERSION_2_0 || GLEE_ARB_shading_language_100)
		tmp = (const char *) glGetString(GL_SHADING_LANGUAGE_VERSION);

	if (tmp == 0)
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
	return GLEE_VERSION_2_0 && getGLSLVersion() >= "1.2";
}

StringMap<Shader::ShaderType, Shader::TYPE_MAX_ENUM>::Entry Shader::typeNameEntries[] =
{
	{"vertex", Shader::TYPE_VERTEX},
	{"pixel", Shader::TYPE_PIXEL},
};

StringMap<Shader::ShaderType, Shader::TYPE_MAX_ENUM> Shader::typeNames(Shader::typeNameEntries, sizeof(Shader::typeNameEntries));

} // opengl
} // graphics
} // love
