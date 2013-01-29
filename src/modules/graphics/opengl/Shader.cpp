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
				Shader::detach();
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

	GLint maxtexunits;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxtexunits);
	maxTextureUnits = std::max(maxtexunits - 1, 0);

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
	const char *shadertypename = NULL;

	switch (type)
	{
	case TYPE_VERTEX:
		glshadertype = GL_VERTEX_SHADER;
		shadertypename = "vertex";
		break;
	case TYPE_FRAGMENT:
		glshadertype = GL_FRAGMENT_SHADER;
		shadertypename = "fragment";
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

		if (err == GL_INVALID_ENUM) // invalid or unsupported shader type
			throw love::Exception("Cannot create %s shader object: %s shaders not supported.", shadertypename, shadertypename);
		else // other errors should only happen between glBegin() and glEnd()
			throw love::Exception("Cannot create %s shader object.", shadertypename);
	}

	const char *src = code.c_str();
	size_t srclen = code.length();
	glShaderSource(shaderid, 1, (const GLchar **)&src, (GLint *)&srclen);

	glCompileShader(shaderid);

	GLint status;
	glGetShaderiv(shaderid, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE)
	{
		GLint infologlen;
		glGetShaderiv(shaderid, GL_INFO_LOG_LENGTH, &infologlen);

		GLchar *errorlog = new GLchar[infologlen + 1];
		glGetShaderInfoLog(shaderid, infologlen, NULL, errorlog);

		std::string tmp(errorlog);

		delete[] errorlog;
		glDeleteShader(shaderid);

		throw love::Exception("Cannot compile %s shader code:\n%s", shadertypename, tmp.c_str());
	}

	return shaderid;
}

void Shader::createProgram(const std::vector<GLuint> &shaderids)
{
	program = glCreateProgram();
	if (program == 0) // should only fail when called between glBegin() and glEnd()
		throw love::Exception("Cannot create shader program object.");

	std::vector<GLuint>::const_iterator it;
	for (it = shaderids.begin(); it != shaderids.end(); ++it)
		glAttachShader(program, *it);

	glLinkProgram(program);

	for (it = shaderids.begin(); it != shaderids.end(); ++it)
		glDeleteShader(*it); // flag shaders for auto-deletion when program object is deleted

	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);

	if (status == GL_FALSE)
	{
		const std::string warnings = getWarnings();
		glDeleteProgram(program);

		throw love::Exception("Cannot link shader program object:\n%s", warnings.c_str());
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

	if (current == this)
	{
		current = NULL; // make sure glUseProgram gets called
		attach();
	}

	return true;
}

void Shader::unloadVolatile()
{
	if (current == this)
		glUseProgram(0);

	if (program != 0)
		glDeleteProgram(program);

	program = 0;

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
}

std::string Shader::getWarnings() const
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

void Shader::attach(bool temporary)
{
	if (current != this)
		glUseProgram(program);

	current = this;

	if (!temporary)
	{
		// make sure all sent textures are properly bound to their respective texture units
		// note: list potentially contains texture ids of deleted/invalid textures!
		for (size_t i = 0; i < activeTextureUnits.size(); ++i)
		{
			if (activeTextureUnits[i] > 0)
				bindTextureToUnit(activeTextureUnits[i], i + 1, false);
		}
		setActiveTextureUnit(0);
	}
}

void Shader::detach()
{
	if (current != NULL)
		glUseProgram(0);

	current = NULL;
}

void Shader::sendFloat(const std::string &name, int size, const GLfloat *vec, int count)
{
	TemporaryAttacher attacher(this);
	GLint location = getUniformLocation(name);

	if (size < 1 || size > 4)
		throw love::Exception("Invalid variable size: %d (expected 1-4).", size);

	switch (size)
	{
	case 4:
		glUniform4fv(location, count, vec);
		break;
	case 3:
		glUniform3fv(location, count, vec);
		break;
	case 2:
		glUniform2fv(location, count, vec);
		break;
	case 1:
	default:
		glUniform1fv(location, count, vec);
		break;
	}

	// throw error if needed
	checkSetUniformError();
}

void Shader::sendMatrix(const std::string &name, int size, const GLfloat *m, int count)
{
	TemporaryAttacher attacher(this);
	GLint location = getUniformLocation(name);

	if (size < 2 || size > 4)
	{
		throw love::Exception("Invalid matrix size: %dx%d "
							  "(can only set 2x2, 3x3 or 4x4 matrices).", size,size);
	}

	switch (size)
	{
	case 4:
		glUniformMatrix4fv(location, count, GL_FALSE, m);
		break;
	case 3:
		glUniformMatrix3fv(location, count, GL_FALSE, m);
		break;
	case 2:
	default:
		glUniformMatrix2fv(location, count, GL_FALSE, m);
		break;
	}

	// throw error if needed
	checkSetUniformError();
}

void Shader::sendTexture(const std::string &name, GLuint texture)
{
	TemporaryAttacher attacher(this);
	GLint location = getUniformLocation(name);
	int textureunit = getTextureUnit(name);

	// bind texture to assigned texture unit and send uniform to shader program
	bindTextureToUnit(texture, textureunit, false);
	glUniform1i(location, textureunit);

	// reset texture unit
	setActiveTextureUnit(0);

	// throw error if needed
	checkSetUniformError();

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

GLint Shader::getUniformLocation(const std::string &name)
{
	std::map<std::string, GLint>::const_iterator it = uniforms.find(name);
	if (it != uniforms.end())
		return it->second;

	GLint location = glGetUniformLocation(program, name.c_str());
	if (location == -1)
	{
		throw love::Exception(
			"Cannot get location of shader variable `%s'.\n"
			"A common error is to define but not use the variable.", name.c_str());
	}

	uniforms[name] = location;
	return location;
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

void Shader::checkSetUniformError()
{
	GLenum error_code = glGetError();
	if (GL_INVALID_OPERATION == error_code)
	{
		throw love::Exception(
			"Invalid operation:\n"
			"- Trying to send the wrong value type to shader variable, or\n"
			"- Trying to send array values with wrong dimension, or\n"
			"- Invalid variable name.");
	}
}

std::string Shader::getGLSLVersion()
{
	// GL_SHADING_LANGUAGE_VERSION may not be available in OpenGL < 2.0.
	const char *tmp = (const char *) glGetString(GL_SHADING_LANGUAGE_VERSION);
	if (tmp == NULL)
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

} // opengl
} // graphics
} // love
