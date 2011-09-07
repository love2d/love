#include "PixelEffect.h"
#include "GLee.h"
#include <limits>
#include <sstream>

namespace
{
	// temporarily attaches a shader program (for setting uniforms, etc)
	// reattaches the originally active program when destroyed
	struct TemporaryAttacher
	{
		TemporaryAttacher(love::graphics::opengl::PixelEffect* sp) : s(sp)
		{
			glGetIntegerv(GL_CURRENT_PROGRAM, &activeProgram);
			s->attach();
		}
		~TemporaryAttacher() { glUseProgram(activeProgram); }
		love::graphics::opengl::PixelEffect* s;
		GLint activeProgram;
	};
} // anonymous namespace

namespace love
{
namespace graphics
{
namespace opengl
{
	std::map<std::string, GLint> PixelEffect::_texture_unit_pool;
	GLint PixelEffect::_current_texture_unit = 0;
	GLint PixelEffect::_max_texture_units = 0;

	GLint PixelEffect::getTextureUnit(const std::string& name)
	{
		std::map<std::string, GLint>::const_iterator it = _texture_unit_pool.find(name);

		if (it != _texture_unit_pool.end())
			return it->second;

		if (++_current_texture_unit >= _max_texture_units)
			throw love::Exception("No more texture units available");

		_texture_unit_pool[name] = _current_texture_unit;
		return _current_texture_unit;
	}


	PixelEffect::PixelEffect(const std::string& code)
		: _program(0), _code(code)
	{
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &_max_texture_units);
		loadVolatile();
	}

	bool PixelEffect::loadVolatile()
	{
		_program = glCreateProgram();
		// should only fail if this is called between a glBegin()/glEnd() pair
		if (_program == 0)
			throw love::Exception("Cannot create shader program object.");

		GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);
		// should only fail if this is called between a glBegin()/glEnd() pair
		if (shader == 0) {
			glDeleteProgram(_program);
			throw love::Exception("Cannot create shader object.");
		}

		// compile fragment shader code
		const char* src = _code.c_str();
		GLint strlen = _code.length();
		glShaderSource(shader, 1, (const GLchar**)&src, &strlen);
		glCompileShader(shader);

		GLint compile_ok;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_ok);
		if (GL_FALSE == compile_ok) {
			// get compiler error
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &strlen);
			char *error_str = new char[strlen];
			glGetShaderInfoLog(shader, strlen, NULL, error_str);
			std::string tmp(error_str);

			// cleanup before throw
			delete[] error_str;
			glDeleteShader(shader);
			glDeleteProgram(_program);

			// XXX: errorlog may contain escape sequences.
			throw love::Exception("Cannot compile shader:\n%s", tmp.c_str());
		}

		// link fragment shader
		GLint link_ok;
		glAttachShader(_program, shader);
		glLinkProgram(_program);
		glGetProgramiv(_program, GL_LINK_STATUS, &link_ok);
		if (GL_FALSE == link_ok) {
			// this should not happen if compiling is ok, but one can never be too careful
			// get linker error
			std::string tmp(getWarnings());

			// cleanup before throw
			glDeleteShader(shader);
			glDeleteProgram(_program);
			throw love::Exception("Cannot compile shader:\n%s", tmp.c_str());
		}

		glDeleteShader(shader);
		return true;
	}

	PixelEffect::~PixelEffect()
	{
		unloadVolatile();
	}

	void PixelEffect::unloadVolatile()
	{
		glDeleteProgram(_program);
	}

	bool PixelEffect::isSupported()
	{
		return GLEE_VERSION_2_0 && GLEE_ARB_shader_objects && GLEE_ARB_fragment_shader;
	}

	std::string PixelEffect::getWarnings() const
	{
		GLint strlen;
		glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &strlen);
		char *temp_str = new char[strlen];
		glGetProgramInfoLog(_program, strlen, NULL, temp_str);

		std::string warnings(temp_str);
		delete[] temp_str;
		return warnings;
	}

	void PixelEffect::attach()
	{
		glUseProgram(_program);
	}

	void PixelEffect::detach()
	{
		glUseProgram(0);
	}

	void PixelEffect::sendFloat(const std::string& name, int size, const GLfloat* vec, int count)
	{
		TemporaryAttacher attacher(this);
		GLint location = getUniformLocation(name);

		if (size < 1 || size > 4) {
			throw love::Exception("Invalid variable size: %d (expected 1-4).", size);
		}

		switch (size) {
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

	void PixelEffect::sendMatrix(const std::string& name, int size, const GLfloat* m, int count)
	{
		TemporaryAttacher attacher(this);
		GLint location = getUniformLocation(name);

		if (size < 2 || size > 4) {
			throw love::Exception("Invalid matrix size: %dx%d "
					"(can only set 2x2, 3x3 or 4x4 matrices).", size,size);
		}

		switch (size) {
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

	void PixelEffect::sendImage(const std::string& name, const Image& image)
	{
		GLint texture_unit = getTextureUnit(name);

		TemporaryAttacher attacher(this);
		GLint location = getUniformLocation(name);

		glActiveTexture(GL_TEXTURE0 + texture_unit);
		glBindTexture(GL_TEXTURE_2D, image.getTextureName());
		glUniform1i(location, texture_unit);

		// reset texture unit
		glActiveTexture(GL_TEXTURE0);

		// throw error if needed
		checkSetUniformError();
	}

	void PixelEffect::sendCanvas(const std::string& name, const Canvas& canvas)
	{
		GLint texture_unit = getTextureUnit(name);

		TemporaryAttacher attacher(this);
		GLint location = getUniformLocation(name);

		glActiveTexture(GL_TEXTURE0 + texture_unit);
		glBindTexture(GL_TEXTURE_2D, canvas.getTextureName());
		glUniform1i(location, texture_unit);

		// reset texture unit
		glActiveTexture(GL_TEXTURE0);

		// throw error if needed
		checkSetUniformError();
	}

	GLint PixelEffect::getUniformLocation(const std::string& name)
	{
		GLint location = glGetUniformLocation(_program, name.c_str());
		if (location == -1) {
			throw love::Exception(
					"Cannot get location of shader variable `%s'.\n"
					"A common error is to define but not use the variable.", name.c_str());
		}
		return location;
	}

	void PixelEffect::checkSetUniformError()
	{
		GLenum error_code = glGetError();
		if (GL_INVALID_OPERATION == error_code) {
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
