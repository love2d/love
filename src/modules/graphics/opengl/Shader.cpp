/**
 * Copyright (c) 2006-2014 LOVE Development Team
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
#include "Canvas.h"

// C++
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
			if (prevShader != nullptr)
				prevShader->attach();
			else
				curShader->detach();
		}

		Shader *curShader;
		Shader *prevShader;
	};
} // anonymous namespace


Shader *Shader::current = nullptr;

GLint Shader::maxTexUnits = 0;
std::vector<int> Shader::textureCounters;

Shader::Shader(const ShaderSources &sources)
	: shaderSources(sources)
	, program(0)
	, builtinUniforms()
	, vertexAttributes()
	, lastCanvas((Canvas *) -1)
	, lastViewport()
{
	if (shaderSources.empty())
		throw love::Exception("Cannot create shader: no source code!");

	if (maxTexUnits <= 0)
	{
		GLint maxtexunits;
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxtexunits);
		maxTexUnits = std::max(maxtexunits - 1, 0);
	}

	// initialize global texture id counters if needed
	if (textureCounters.size() < (size_t) maxTexUnits)
		textureCounters.resize(maxTexUnits, 0);

	// load shader source and create program object
	loadVolatile();
}

Shader::~Shader()
{
	if (current == this)
		detach();

	for (auto it = boundRetainables.begin(); it != boundRetainables.end(); ++it)
	{
		it->second->release();
		boundRetainables.erase(it);
	}

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
	glGetShaderInfoLog(shaderid, infologlen, nullptr, infolog);

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

	// Bind generic vertex attribute indices to names in the shader.
	for (int i = 0; i < int(OpenGL::ATTRIB_MAX_ENUM); i++)
	{
		const char *name = nullptr;
		if (attribNames.find((OpenGL::VertexAttrib) i, name))
			glBindAttribLocation(program, i, (const GLchar *) name);
	}

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

		// If this is a built-in (LOVE-created) uniform, store the location.
		BuiltinExtern builtin;
		if (builtinNames.find(u.name.c_str(), builtin))
			builtinUniforms[int(builtin)] = u.location;

		if (u.location != -1)
			uniforms[u.name] = u;
	}
}

bool Shader::loadVolatile()
{
	// zero out active texture list
	activeTexUnits.clear();
	activeTexUnits.insert(activeTexUnits.begin(), maxTexUnits, 0);

	// Built-in uniform locations default to -1 (nonexistant.)
	for (int i = 0; i < int(BUILTIN_MAX_ENUM); i++)
		builtinUniforms[i] = -1;

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

	for (int i = 0; i < int(OpenGL::ATTRIB_MAX_ENUM); i++)
	{
		const char *name = nullptr;
		if (attribNames.find(OpenGL::VertexAttrib(i), name))
			vertexAttributes[i] = glGetAttribLocation(program, name);
		else
			vertexAttributes[i] = -1;
	}

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
	if (current == this)
		glUseProgram(0);

	if (program != 0)
	{
		glDeleteProgram(program);
		program = 0;
	}

	// decrement global texture id counters for texture units which had textures bound from this shader
	for (size_t i = 0; i < activeTexUnits.size(); ++i)
	{
		if (activeTexUnits[i] > 0)
			textureCounters[i] = std::max(textureCounters[i] - 1, 0);
	}

	// active texture list is probably invalid, clear it
	activeTexUnits.clear();
	activeTexUnits.insert(activeTexUnits.begin(), maxTexUnits, 0);

	// same with uniform location list
	uniforms.clear();

	// And the locations of any built-in uniform variables.
	for (int i = 0; i < int(BUILTIN_MAX_ENUM); i++)
		builtinUniforms[i] = -1;

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
		if (current != nullptr)
			current->release();

		glUseProgram(program);
		current = this;

		current->retain();
	}

	if (!temporary)
	{
		// make sure all sent textures are properly bound to their respective texture units
		// note: list potentially contains texture ids of deleted/invalid textures!
		for (size_t i = 0; i < activeTexUnits.size(); ++i)
		{
			if (activeTexUnits[i] > 0)
				gl.bindTextureToUnit(activeTexUnits[i], i + 1, false);
		}

		// We always want to use texture unit 0 for everyhing else.
		gl.setTextureUnit(0);
	}
}

void Shader::detach()
{
	if (current != nullptr)
		glUseProgram(0);

	current = nullptr;
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

void Shader::sendTexture(const std::string &name, Texture *texture)
{
	GLuint gltex = texture->getGLTexture();

	TemporaryAttacher attacher(this);

	int texunit = getTextureUnit(name);

	const Uniform &u = getUniform(name);
	checkSetUniformError(u, 1, 1, UNIFORM_SAMPLER);

	// bind texture to assigned texture unit and send uniform to shader program
	gl.bindTextureToUnit(gltex, texunit, false);

	glUniform1i(u.location, texunit);

	// reset texture unit
	gl.setTextureUnit(0);

	// increment global shader texture id counter for this texture unit, if we haven't already
	if (activeTexUnits[texunit-1] == 0)
		++textureCounters[texunit-1];

	// store texture id so it can be re-bound to the proper texture unit later
	activeTexUnits[texunit-1] = gltex;

	retainObject(name, texture);
}

void Shader::retainObject(const std::string &name, Object *object)
{
	auto it = boundRetainables.find(name);
	if (it != boundRetainables.end())
		it->second->release();

	object->retain();
	boundRetainables[name] = object;
}

int Shader::getTextureUnit(const std::string &name)
{
	auto it = texUnitPool.find(name);

	if (it != texUnitPool.end())
		return it->second;

	int texunit = 1;

	// prefer texture units which are unused by all other shaders
	auto freeunit_it = std::find(textureCounters.begin(), textureCounters.end(), 0);

	if (freeunit_it != textureCounters.end())
	{
		// we don't want to use unit 0
		texunit = std::distance(textureCounters.begin(), freeunit_it) + 1;
	}
	else
	{
		// no completely unused texture units exist, try to use next free slot in our own list
		auto nextunit_it = std::find(activeTexUnits.begin(), activeTexUnits.end(), 0);

		if (nextunit_it == activeTexUnits.end())
			throw love::Exception("No more texture units available for shader.");

		// we don't want to use unit 0
		texunit = std::distance(activeTexUnits.begin(), nextunit_it) + 1;
	}

	texUnitPool[name] = texunit;
	return texunit;
}

bool Shader::hasVertexAttrib(OpenGL::VertexAttrib attrib) const
{
	return vertexAttributes[int(attrib)] != -1;
}

bool Shader::hasBuiltinExtern(BuiltinExtern builtin) const
{
	return builtinUniforms[int(builtin)] != -1;
}

bool Shader::sendBuiltinFloat(BuiltinExtern builtin, int size, const GLfloat *vec, int count)
{
	if (!hasBuiltinExtern(builtin))
		return false;

	GLint location = builtinUniforms[int(builtin)];

	TemporaryAttacher attacher(this);

	switch (size)
	{
	case 1:
		glUniform1fv(location, count, vec);
		break;
	case 2:
		glUniform2fv(location, count, vec);
		break;
	case 3:
		glUniform3fv(location, count, vec);
		break;
	case 4:
		glUniform4fv(location, count, vec);
		break;
	default:
		return false;
	}
	
	return true;
}

void Shader::checkSetScreenParams()
{
	OpenGL::Viewport view = gl.getViewport();

	if (view == lastViewport && lastCanvas == Canvas::current)
		return;

	// In the shader, we do pixcoord.y = gl_FragCoord.y * params.z + params.w.
	// This lets us flip pixcoord.y when needed, to be consistent (Canvases
	// have flipped y-values for pixel coordinates.)
	GLfloat params[] = {
		(GLfloat) view.w, (GLfloat) view.h,
		0.0f, 0.0f,
	};

	if (Canvas::current != nullptr)
	{
		// gl_FragCoord.y is flipped in Canvases, so we un-flip:
		// pixcoord.y = gl_FragCoord.y * -1.0 + height.
		params[2] = -1.0f;
		params[3] = (GLfloat) view.h;
	}
	else
	{
		// No flipping: pixcoord.y = gl_FragCoord.y * 1.0 + 0.0.
		params[2] = 1.0f;
		params[3] = 0.0f;
	}

	sendBuiltinFloat(BUILTIN_SCREEN_SIZE, 4, params, 1);

	lastCanvas = Canvas::current;
	lastViewport = view;
}

const std::map<std::string, Object *> &Shader::getBoundRetainables() const
{
	return boundRetainables;
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
	return getGLSLVersion() >= "1.2";
}

StringMap<Shader::ShaderType, Shader::TYPE_MAX_ENUM>::Entry Shader::typeNameEntries[] =
{
	{"vertex", Shader::TYPE_VERTEX},
	{"pixel", Shader::TYPE_PIXEL},
};

StringMap<Shader::ShaderType, Shader::TYPE_MAX_ENUM> Shader::typeNames(Shader::typeNameEntries, sizeof(Shader::typeNameEntries));

StringMap<OpenGL::VertexAttrib, OpenGL::ATTRIB_MAX_ENUM>::Entry Shader::attribNameEntries[] =
{
	{"love_PseudoInstanceID", OpenGL::ATTRIB_PSEUDO_INSTANCE_ID},
};

StringMap<OpenGL::VertexAttrib, OpenGL::ATTRIB_MAX_ENUM> Shader::attribNames(Shader::attribNameEntries, sizeof(Shader::attribNameEntries));

StringMap<Shader::BuiltinExtern, Shader::BUILTIN_MAX_ENUM>::Entry Shader::builtinNameEntries[] =
{
	{"love_ScreenSize", Shader::BUILTIN_SCREEN_SIZE},
};

StringMap<Shader::BuiltinExtern, Shader::BUILTIN_MAX_ENUM> Shader::builtinNames(Shader::builtinNameEntries, sizeof(Shader::builtinNameEntries));

} // opengl
} // graphics
} // love
