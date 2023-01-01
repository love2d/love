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

#include "ShaderStage.h"

namespace love
{
namespace graphics
{
namespace opengl
{

ShaderStage::ShaderStage(love::graphics::Graphics *gfx, StageType stage, const std::string &source, bool gles, const std::string &cachekey)
	: love::graphics::ShaderStage(gfx, stage, source, gles, cachekey)
	, glShader(0)
{
	loadVolatile();
}

ShaderStage::~ShaderStage()
{
	unloadVolatile();
}

bool ShaderStage::loadVolatile()
{
	if (glShader != 0)
		return true;

	StageType stage = getStageType();
	const char *typestr = "unknown";
	getConstant(stage, typestr);

	GLenum glstage = 0;
	if (stage == STAGE_VERTEX)
		glstage = GL_VERTEX_SHADER;
	else if (stage == STAGE_PIXEL)
		glstage = GL_FRAGMENT_SHADER;
	else
		throw love::Exception("%s shader stage is not handled in OpenGL backend code.", typestr);

	glShader = glCreateShader(glstage);

	if (glShader == 0)
		throw love::Exception("Cannot create OpenGL %s shader object.", typestr);

	const std::string &sourcestring = getSource();
	const char *src = sourcestring.c_str();
	GLint srclen = (GLint) sourcestring.length();

	glShaderSource(glShader, 1, (const GLchar **)&src, &srclen);
	glCompileShader(glShader);

	GLint infologlen;
	glGetShaderiv(glShader, GL_INFO_LOG_LENGTH, &infologlen);

	if (infologlen > 0)
	{
		GLchar *infolog = new GLchar[infologlen];
		glGetShaderInfoLog(glShader, infologlen, nullptr, infolog);

		warnings = infolog;
		delete[] infolog;
	}

	GLint status = GL_FALSE;
	glGetShaderiv(glShader, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE)
	{
		glDeleteShader(glShader);
		throw love::Exception("Cannot compile %s shader code:\n%s", typestr, warnings.c_str());
	}

	return true;
}

void ShaderStage::unloadVolatile()
{
	if (glShader != 0)
		glDeleteShader(glShader);

	glShader = 0;
}

} // opengl
} // graphics
} // love
