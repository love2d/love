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
#include "Canvas.h"

// C++
#include <algorithm>
#include <limits>

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
Shader *Shader::defaultShader = nullptr;
Shader *Shader::defaultVideoShader = nullptr;

Shader::ShaderSource Shader::defaultCode[Graphics::RENDERER_MAX_ENUM];
Shader::ShaderSource Shader::defaultVideoCode[Graphics::RENDERER_MAX_ENUM];

std::vector<int> Shader::textureCounters;

Shader::Shader(const ShaderSource &source)
	: shaderSource(source)
	, program(0)
	, builtinUniforms()
	, builtinAttributes()
	, lastCanvas((Canvas *) -1)
	, lastViewport()
	, lastPointSize(0.0f)
	, videoTextureUnits()
{
	if (source.vertex.empty() && source.pixel.empty())
		throw love::Exception("Cannot create shader: no source code!");

	// initialize global texture id counters if needed
	if ((int) textureCounters.size() < gl.getMaxTextureUnits() - 1)
		textureCounters.resize(gl.getMaxTextureUnits() - 1, 0);

	// load shader source and create program object
	loadVolatile();
}

Shader::~Shader()
{
	if (current == this)
		detach();

	for (const auto &retainable : boundRetainables)
		retainable.second->release();

	boundRetainables.clear();

	unloadVolatile();
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

	uniforms.clear();

	GLint activeprogram = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &activeprogram);

	glUseProgram(program);

	GLint numuniforms;
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numuniforms);

	GLchar cname[256];
	const GLint bufsize = (GLint) (sizeof(cname) / sizeof(GLchar));

	for (int i = 0; i < numuniforms; i++)
	{
		GLsizei namelen = 0;
		Uniform u = {};

		glGetActiveUniform(program, (GLuint) i, bufsize, &namelen, &u.count, &u.type, cname);

		u.name = std::string(cname, (size_t) namelen);
		u.location = glGetUniformLocation(program, u.name.c_str());
		u.baseType = getUniformBaseType(u.type);

		// Initialize all samplers to 0. Both GLSL and GLSL ES are supposed to
		// do this themselves, but some Android devices (galaxy tab 3 and 4)
		// don't seem to do it...
		if (u.baseType == UNIFORM_SAMPLER)
			glUniform1i(u.location, 0);

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

		if (u.location != -1)
			uniforms[u.name] = u;
	}

	glUseProgram(activeprogram);
}

bool Shader::loadVolatile()
{
	OpenGL::TempDebugGroup debuggroup("Shader load");

    // Recreating the shader program will invalidate uniforms that rely on these.
    lastCanvas = (Canvas *) -1;
    lastViewport = OpenGL::Viewport();

	lastPointSize = -1.0f;

	// Invalidate the cached matrices by setting some elements to NaN.
	float nan = std::numeric_limits<float>::quiet_NaN();
	lastProjectionMatrix.setTranslation(nan, nan);
	lastTransformMatrix.setTranslation(nan, nan);

	for (int i = 0; i < 3; i++)
		videoTextureUnits[i] = 0;

	// zero out active texture list
	activeTexUnits.clear();
	activeTexUnits.insert(activeTexUnits.begin(), gl.getMaxTextureUnits() - 1, 0);

	std::vector<GLuint> shaderids;

	const ShaderSource *defaults = &defaultCode[Graphics::RENDERER_OPENGL];
	if (GLAD_ES_VERSION_2_0)
		defaults = &defaultCode[Graphics::RENDERER_OPENGLES];

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
	activeTexUnits.resize(gl.getMaxTextureUnits() - 1, 0);

	attributes.clear();

	// same with uniform location list
	uniforms.clear();

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
		glUseProgram(program);
		current = this;
		// retain/release happens in Graphics::setShader.
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
	if (defaultShader)
	{
		if (current != defaultShader)
			defaultShader->attach();

		return;
	}

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
	GLuint gltex = *(GLuint *) texture->getHandle();

	TemporaryAttacher attacher(this);

	int texunit = getTextureUnit(name);

	const Uniform &u = getUniform(name);
	checkSetUniformError(u, 1, 1, UNIFORM_SAMPLER);

	// bind texture to assigned texture unit and send uniform to shader program
	gl.bindTextureToUnit(gltex, texunit, true);

	glUniform1i(u.location, texunit);

	// increment global shader texture id counter for this texture unit, if we haven't already
	if (activeTexUnits[texunit-1] == 0)
		++textureCounters[texunit-1];

	// store texture id so it can be re-bound to the proper texture unit later
	activeTexUnits[texunit-1] = gltex;

	retainObject(name, texture);
}

void Shader::retainObject(const std::string &name, Object *object)
{
	object->retain();

	auto it = boundRetainables.find(name);
	if (it != boundRetainables.end())
		it->second->release();

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
		texunit = (int) std::distance(textureCounters.begin(), freeunit_it) + 1;
	}
	else
	{
		// no completely unused texture units exist, try to use next free slot in our own list
		auto nextunit_it = std::find(activeTexUnits.begin(), activeTexUnits.end(), 0);

		if (nextunit_it == activeTexUnits.end())
			throw love::Exception("No more texture units available for shader.");

		// we don't want to use unit 0
		texunit = (int) std::distance(activeTexUnits.begin(), nextunit_it) + 1;
	}

	texUnitPool[name] = texunit;
	return texunit;
}

Shader::UniformType Shader::getExternVariable(const std::string &name, int &components, int &count)
{
	auto it = uniforms.find(name);

	if (it == uniforms.end())
	{
		components = 0;
		count = 0;
		return UNIFORM_UNKNOWN;
	}

	components = getUniformTypeSize(it->second.type);
	count = (int) it->second.count;

	return it->second.baseType;
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
	TemporaryAttacher attacher(this);

	// Set up the texture units that will be used by the shader to sample from
	// the textures, if they haven't been set up yet.
	if (videoTextureUnits[0] == 0)
	{
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
				videoTextureUnits[i] = getTextureUnit(names[i]);

				// Increment global shader texture id counter for this texture
				// unit, if we haven't already.
				if (activeTexUnits[videoTextureUnits[i] - 1] == 0)
					++textureCounters[videoTextureUnits[i] - 1];

				glUniform1i(locs[i], videoTextureUnits[i]);
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
			activeTexUnits[videoTextureUnits[i] - 1] = textures[i];
			gl.bindTextureToUnit(textures[i], videoTextureUnits[i], false);
		}
	}

	gl.setTextureUnit(0);
}

void Shader::checkSetScreenParams()
{
	OpenGL::Viewport view = gl.getViewport();

	if (view == lastViewport && lastCanvas == Canvas::current)
		return;

	// In the shader, we do pixcoord.y = gl_FragCoord.y * params.z + params.w.
	// This lets us flip pixcoord.y when needed, to be consistent (drawing with
	// no Canvas active makes the y-values for pixel coordinates flipped.)
	GLfloat params[] = {
		(GLfloat) view.w, (GLfloat) view.h,
		0.0f, 0.0f,
	};

	if (Canvas::current != nullptr)
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
		TemporaryAttacher attacher(this);
		glUniform4fv(location, 1, params);
	}

	lastCanvas = Canvas::current;
	lastViewport = view;
}

void Shader::checkSetPointSize(float size)
{
	if (size == lastPointSize)
		return;

	GLint location = builtinUniforms[BUILTIN_POINT_SIZE];

	if (location >= 0)
	{
		TemporaryAttacher attacher(this);
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
		const Matrix4 &curproj = gl.matrices.projection.back();

		TemporaryAttacher attacher(this);

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
	return GLAD_ES_VERSION_2_0 || (getGLSLVersion() >= "1.2");
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
		return 1;
	}
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
	case GL_FLOAT_MAT2x3:
	case GL_FLOAT_MAT2x4:
	case GL_FLOAT_MAT3x2:
	case GL_FLOAT_MAT3x4:
	case GL_FLOAT_MAT4x2:
	case GL_FLOAT_MAT4x3:
		return UNIFORM_FLOAT;
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
