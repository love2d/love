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
#include "common/Exception.h"

#include "OpenGL.h"
#include "Shader.h"

#include <vector>
#include <algorithm>

namespace love
{
namespace graphics
{
namespace opengl
{

static bool contextInitialized = false;

static int curTextureUnit = 0;
static std::vector<GLuint> textureUnits;

void initializeContext()
{
	if (contextInitialized)
		return;

	contextInitialized = true;

	// initialize multiple texture unit support for shaders, if available
	textureUnits.clear();
	if (Shader::isSupported())
	{
		GLint maxtextureunits;
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxtextureunits);

		textureUnits.resize(maxtextureunits, 0);

		GLenum curgltextureunit;
		glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint *) &curgltextureunit);

		curTextureUnit = curgltextureunit - GL_TEXTURE0;

		// Retrieve currently bound textures for each texture unit
		for (size_t i = 0; i < textureUnits.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint *) &textureUnits[i]);
		}

		glActiveTexture(curgltextureunit);
	}
	else
	{
		// multitexturing not supported, so we only have 1 texture unit
		textureUnits.resize(1, 0);
		curTextureUnit = 0;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint *) &textureUnits[0]);
	}

	// The functionality of the core and ARB VBOs are identical, so we can
	// assign the pointers of the core functions to the names of the ARB
	// functions, if the latter isn't supported but the former is.
	if (GLEE_VERSION_1_5 && !GLEE_ARB_vertex_buffer_object)
	{
		glBindBufferARB = (GLEEPFNGLBINDBUFFERARBPROC) glBindBuffer;
		glBufferDataARB = (GLEEPFNGLBUFFERDATAARBPROC) glBufferData;
		glBufferSubDataARB = (GLEEPFNGLBUFFERSUBDATAARBPROC) glBufferSubData;
		glDeleteBuffersARB = (GLEEPFNGLDELETEBUFFERSARBPROC) glDeleteBuffers;
		glGenBuffersARB = (GLEEPFNGLGENBUFFERSARBPROC) glGenBuffers;
		glGetBufferParameterivARB = (GLEEPFNGLGETBUFFERPARAMETERIVARBPROC) glGetBufferParameteriv;
		glGetBufferPointervARB = (GLEEPFNGLGETBUFFERPOINTERVARBPROC) glGetBufferPointerv;
		glGetBufferSubDataARB = (GLEEPFNGLGETBUFFERSUBDATAARBPROC) glGetBufferSubData;
		glIsBufferARB = (GLEEPFNGLISBUFFERARBPROC) glIsBuffer;
		glMapBufferARB = (GLEEPFNGLMAPBUFFERARBPROC) glMapBuffer;
		glUnmapBufferARB = (GLEEPFNGLUNMAPBUFFERARBPROC) glUnmapBuffer;
	}

	// Set the 'default' texture (id 0) as a repeating white pixel.
	// Otherwise, texture2D inside a shader would return black when drawing graphics primitives,
	// which would create the need to use different "passthrough" shaders for untextured primitives vs images.

	GLuint curtexture = textureUnits[curTextureUnit];
	bindTexture(0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	GLubyte pixel = 255;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8, 1, 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, &pixel);

	bindTexture(curtexture);
}

void uninitializeContext()
{
	contextInitialized = false;
}

void setActiveTextureUnit(int textureunit)
{
	if (textureunit < 0 || (size_t) textureunit >= textureUnits.size())
		throw love::Exception("Invalid texture unit index (%d).", textureunit);

	if (textureunit != curTextureUnit)
	{
		if (textureUnits.size() > 1)
			glActiveTexture(GL_TEXTURE0 + textureunit);
		else
			throw love::Exception("Multitexturing not supported.");
	}

	curTextureUnit = textureunit;
}

void bindTexture(GLuint texture)
{
	if (texture != textureUnits[curTextureUnit])
	{
		textureUnits[curTextureUnit] = texture;
		glBindTexture(GL_TEXTURE_2D, texture);
	}
}

void bindTextureToUnit(GLuint texture, int textureunit, bool restoreprev)
{
	if (textureunit < 0 || (size_t) textureunit >= textureUnits.size())
		throw love::Exception("Invalid texture unit index.");

	if (texture != textureUnits[textureunit])
	{
		int oldtextureunit = curTextureUnit;
		setActiveTextureUnit(textureunit);

		textureUnits[textureunit] = texture;
		glBindTexture(GL_TEXTURE_2D, texture);

		if (restoreprev)
			setActiveTextureUnit(oldtextureunit);
	}
}

void deleteTexture(GLuint texture)
{
	// glDeleteTextures binds texture 0 to all texture units the deleted texture was bound to
	std::vector<GLuint>::iterator it;
	for (it = textureUnits.begin(); it != textureUnits.end(); ++it)
	{
		if (*it == texture)
			*it = 0;
	}

	glDeleteTextures(1, &texture);
}

void setTextureFilter(const graphics::Image::Filter &f)
{
	GLint gmin, gmag;

	if (f.mipmap == Image::FILTER_NONE)
	{
		if (f.min == Image::FILTER_NEAREST)
			gmin = GL_NEAREST;
		else // f.min == Image::FILTER_LINEAR
			gmin = GL_LINEAR;
	}
	else
	{
		if (f.min == Image::FILTER_NEAREST && f.mipmap == Image::FILTER_NEAREST)
			gmin = GL_NEAREST_MIPMAP_NEAREST;
		else if (f.min == Image::FILTER_NEAREST && f.mipmap == Image::FILTER_LINEAR)
			gmin = GL_NEAREST_MIPMAP_LINEAR;
		else if (f.min == Image::FILTER_LINEAR && f.mipmap == Image::FILTER_NEAREST)
			gmin = GL_LINEAR_MIPMAP_NEAREST;
		else if (f.min == Image::FILTER_LINEAR && f.mipmap == Image::FILTER_LINEAR)
			gmin = GL_LINEAR_MIPMAP_LINEAR;
		else
			gmin = GL_LINEAR;
	}


	switch (f.mag)
	{
	case Image::FILTER_NEAREST:
		gmag = GL_NEAREST;
		break;
	case Image::FILTER_LINEAR:
	default:
		gmag = GL_LINEAR;
		break;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gmin);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gmag);
}

graphics::Image::Filter getTextureFilter()
{
	GLint gmin, gmag;
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &gmin);
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, &gmag);

	Image::Filter f;

	switch (gmin)
	{
	case GL_NEAREST:
		f.min = Image::FILTER_NEAREST;
		f.mipmap = Image::FILTER_NONE;
		break;
	case GL_NEAREST_MIPMAP_NEAREST:
		f.min = f.mipmap = Image::FILTER_NEAREST;
		break;
	case GL_NEAREST_MIPMAP_LINEAR:
		f.min = Image::FILTER_NEAREST;
		f.mipmap = Image::FILTER_LINEAR;
		break;
	case GL_LINEAR_MIPMAP_NEAREST:
		f.min = Image::FILTER_LINEAR;
		f.mipmap = Image::FILTER_NEAREST;
		break;
	case GL_LINEAR_MIPMAP_LINEAR:
		f.min = f.mipmap = Image::FILTER_LINEAR;
		break;
	case GL_LINEAR:
	default:
		f.min = Image::FILTER_LINEAR;
		f.mipmap = Image::FILTER_NONE;
		break;
	}

	switch (gmag)
	{
	case GL_NEAREST:
		f.mag = Image::FILTER_NEAREST;
		break;
	case GL_LINEAR:
	default:
		f.mag = Image::FILTER_LINEAR;
		break;
	}

	return f;
}

void setTextureWrap(const graphics::Image::Wrap &w)
{
	GLint gs, gt;

	switch (w.s)
	{
	case Image::WRAP_CLAMP:
		gs = GL_CLAMP_TO_EDGE;
		break;
	case Image::WRAP_REPEAT:
	default:
		gs = GL_REPEAT;
		break;
	}

	switch (w.t)
	{
	case Image::WRAP_CLAMP:
		gt = GL_CLAMP_TO_EDGE;
		break;
	case Image::WRAP_REPEAT:
	default:
		gt = GL_REPEAT;
		break;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gs);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gt);
}

graphics::Image::Wrap getTextureWrap()
{
	GLint gs, gt;

	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &gs);
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, &gt);

	Image::Wrap w;

	switch (gs)
	{
	case GL_CLAMP_TO_EDGE:
		w.s = Image::WRAP_CLAMP;
		break;
	case GL_REPEAT:
	default:
		w.s = Image::WRAP_REPEAT;
		break;
	}

	switch (gt)
	{
	case GL_CLAMP_TO_EDGE:
		w.t = Image::WRAP_CLAMP;
		break;
	case GL_REPEAT:
	default:
		w.t = Image::WRAP_REPEAT;
		break;
	}

	return w;
}

} // opengl
} // graphics
} // love
