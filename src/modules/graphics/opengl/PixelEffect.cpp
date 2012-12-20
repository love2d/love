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

#include "PixelEffect.h"
#include "GLee.h"
#include "Graphics.h"

namespace
{
// temporarily attaches a shader program (for setting uniforms, etc)
// reattaches the originally active program when destroyed
struct TemporaryAttacher
{
	TemporaryAttacher(love::graphics::opengl::PixelEffect *sp) : s(sp)
	{
		glGetIntegerv(GL_CURRENT_PROGRAM, &activeProgram);
		s->attach();
	}
	~TemporaryAttacher()
	{
		glUseProgram(activeProgram);
	}
	love::graphics::opengl::PixelEffect *s;
	GLint activeProgram;
};
} // anonymous namespace

namespace love
{
namespace graphics
{
namespace opengl
{

PixelEffect *PixelEffect::current = NULL;

std::map<std::string, GLint> PixelEffect::_texture_unit_pool;
GLint PixelEffect::_current_texture_unit = 0;
GLint PixelEffect::_max_texture_units = 0;

GLint PixelEffect::getTextureUnit(const std::string &name)
{
	std::map<std::string, GLint>::const_iterator it = _texture_unit_pool.find(name);

	if (it != _texture_unit_pool.end())
		return it->second;

	if (++_current_texture_unit >= _max_texture_units)
		throw love::Exception("No more texture units available");

	_texture_unit_pool[name] = _current_texture_unit;
	return _current_texture_unit;
}

	PixelEffect::PixelEffect(const std::string &vertcode, const std::string &fragcode)
	: _program(0)
	, _vertcode(vertcode)
	, _fragcode(fragcode)
{
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &_max_texture_units);
	loadVolatile();
}

GLuint PixelEffect::createShader(GLenum type, const std::string &code)
{
	const char *shadertypename = NULL;
	switch (type)
	{
	case GL_VERTEX_SHADER:
		shadertypename = "vertex";
		break;
	case GL_GEOMETRY_SHADER_ARB:
		shadertypename = "geometry";
	case GL_FRAGMENT_SHADER:
	default:
		shadertypename = "fragment";
		break;
	}
	
	GLuint shader = glCreateShader(type);
	if (shader == 0) // should only fail when called between glBegin() and glEnd()
		throw love::Exception("Cannot create %s shader object.", shadertypename);
	
	const char *src = code.c_str();
	size_t srclen = code.length();
	glShaderSource(shader, 1, (const GLchar **)&src, (GLint *)&srclen);
	
	glCompileShader(shader);
	
	GLint compile_status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
	if (compile_status == GL_FALSE)
	{
		GLint infologlen;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologlen);
		
		GLchar *errorlog = new GLchar[infologlen + 1];
		glGetShaderInfoLog(shader, infologlen, NULL, errorlog);
		
		std::string tmp(errorlog);
		
		delete[] errorlog;
		glDeleteShader(shader);
		
		throw love::Exception("Cannot compile %s shader:\n%s", shadertypename, tmp.c_str());
	}
	
	return shader;
}

GLuint PixelEffect::createProgram(const std::vector<GLuint> &shaders)
{
	GLuint program = glCreateProgram();
	if (program == 0) // should only fail when called between glBegin() and glEnd()
		throw love::Exception("Cannot create shader program object.");
	
	std::vector<GLuint>::const_iterator it;
	for (it = shaders.begin(); it != shaders.end(); ++it)
		glAttachShader(program, *it);
	
	glLinkProgram(program);
	
	for (it = shaders.begin(); it != shaders.end(); ++it)
		glDetachShader(program, *it);
	
	GLint link_ok;
	glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
	if (link_ok == GL_FALSE)
	{
		GLint strlen, nullpos;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &strlen);
		
		char *temp_str = new char[strlen+1];
		// be extra sure that the error string will be 0-terminated
		memset(temp_str, '\0', strlen+1);
		glGetProgramInfoLog(program, strlen, &nullpos, temp_str);
		temp_str[nullpos] = '\0';
		
		std::string warnings(temp_str);
		delete[] temp_str;

		glDeleteProgram(program);
		
		throw love::Exception("Cannot link shader program object:\n%s", warnings.c_str());
	}
	
	return program;
}

bool PixelEffect::loadVolatile()
{	
	std::vector<GLuint> shaders;
	
	if (_vertcode.length() > 0)
		shaders.push_back(createShader(GL_VERTEX_SHADER, _vertcode));
	
	if (_fragcode.length() > 0)
		shaders.push_back(createShader(GL_FRAGMENT_SHADER, _fragcode));
	
	if (shaders.size() == 0)
		throw love::Exception("Cannot create PixelEffect: no source code!");
	
	try
	{
		_program = createProgram(shaders);
	}
	catch (love::Exception &e)
	{
		std::vector<GLuint>::iterator it;
		for (it = shaders.begin(); it != shaders.end(); ++it)
			glDeleteShader(*it);
		
		throw;
	}
	
	std::vector<GLuint>::iterator it;
	for (it = shaders.begin(); it != shaders.end(); ++it)
		glDeleteShader(*it);

	return true;
}

PixelEffect::~PixelEffect()
{
	unloadVolatile();
}

void PixelEffect::unloadVolatile()
{
	if (_program != 0)
		glDeleteProgram(_program);
}

std::string PixelEffect::getGLSLVersion()
{
	// GL_SHADING_LANGUAGE_VERSION may not be available in OpenGL < 2.0.
	const char *tmp = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
	if (NULL == tmp)
		return "0.0";

	// the version string always begins with a version number of the format
	//   major_number.minor_number
	// or
	//   major_number.minor_number.release_number
	// we can keep release_number, since it does not affect the check below.
	std::string versionString(tmp);
	size_t minorEndPos = versionString.find(' ');
	return versionString.substr(0, minorEndPos);
}


bool PixelEffect::isSupported()
{
	return GLEE_VERSION_2_0 && getGLSLVersion() >= "1.2";
}

std::string PixelEffect::getWarnings() const
{
	GLint strlen, nullpos;
	glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &strlen);
	char *temp_str = new char[strlen+1];
	// be extra sure that the error string will be 0-terminated
	memset(temp_str, '\0', strlen+1);
	glGetProgramInfoLog(_program, strlen, &nullpos, temp_str);
	temp_str[nullpos] = '\0';

	std::string warnings(temp_str);
	delete[] temp_str;
	return warnings;
}

void PixelEffect::attach()
{
	glUseProgram(_program);
	current = this;
}

void PixelEffect::detach()
{
	glUseProgram(0);
	current = NULL;
}

void PixelEffect::sendFloat(const std::string &name, int size, const GLfloat *vec, int count)
{
	TemporaryAttacher attacher(this);
	GLint location = getUniformLocation(name);

	if (size < 1 || size > 4)
	{
		throw love::Exception("Invalid variable size: %d (expected 1-4).", size);
	}

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

void PixelEffect::sendMatrix(const std::string &name, int size, const GLfloat *m, int count)
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

void PixelEffect::sendImage(const std::string &name, const Image &image)
{
	GLint texture_unit = getTextureUnit(name);

	TemporaryAttacher attacher(this);
	GLint location = getUniformLocation(name);

	glActiveTexture(GL_TEXTURE0 + texture_unit);
	bindTexture(image.getTextureName(), true); // guarantee it gets bound
	glUniform1i(location, texture_unit);

	// reset texture unit
	glActiveTexture(GL_TEXTURE0);

	// throw error if needed
	checkSetUniformError();
}

void PixelEffect::sendCanvas(const std::string &name, const Canvas &canvas)
{
	GLint texture_unit = getTextureUnit(name);

	TemporaryAttacher attacher(this);
	GLint location = getUniformLocation(name);

	glActiveTexture(GL_TEXTURE0 + texture_unit);
	bindTexture(canvas.getTextureName(), true); // guarantee it gets bound
	glUniform1i(location, texture_unit);

	// reset texture unit
	glActiveTexture(GL_TEXTURE0);

	// throw error if needed
	checkSetUniformError();
}

GLint PixelEffect::getUniformLocation(const std::string &name)
{
	std::map<std::string, GLint>::const_iterator it = _uniforms.find(name);
	if (it != _uniforms.end())
		return it->second;

	GLint location = glGetUniformLocation(_program, name.c_str());
	if (location == -1)
	{
		throw love::Exception(
			"Cannot get location of shader variable `%s'.\n"
			"A common error is to define but not use the variable.", name.c_str());
	}

	_uniforms[name] = location;
	return location;
}

void PixelEffect::checkSetUniformError()
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

} // opengl
} // graphics
} // love
