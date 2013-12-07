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

// LOVE
#include "common/config.h"
#include "OpenGL.h"

#include "Shader.h"
#include "common/Exception.h"

// C++
#include <algorithm>

// C
#include <cstring>

namespace love
{
namespace graphics
{
namespace opengl
{

OpenGL::OpenGL()
	: contextInitialized(false)
	, maxAnisotropy(1.0f)
	, maxTextureSize(0)
	, vendor(VENDOR_UNKNOWN)
	, state()
{
}

void OpenGL::initContext()
{
	if (contextInitialized)
		return;

	initOpenGLFunctions();
	initVendor();

	// Store the current color so we don't have to get it through GL later.
	GLfloat glcolor[4];
	glGetFloatv(GL_CURRENT_COLOR, glcolor);
	state.color.r = glcolor[0] * 255;
	state.color.g = glcolor[1] * 255;
	state.color.b = glcolor[2] * 255;
	state.color.a = glcolor[3] * 255;

	// Same with the current clear color.
	glGetFloatv(GL_COLOR_CLEAR_VALUE, glcolor);
	state.clearColor.r = glcolor[0] * 255;
	state.clearColor.g = glcolor[1] * 255;
	state.clearColor.b = glcolor[2] * 255;
	state.clearColor.a = glcolor[3] * 255;

	// Get the current viewport.
	glGetIntegerv(GL_VIEWPORT, (GLint *) &state.viewport.x);

	// And the current scissor - but we need to compensate for GL scissors
	// starting at the bottom left instead of top left.
	glGetIntegerv(GL_SCISSOR_BOX, (GLint *) &state.scissor.x);
	state.scissor.y = state.viewport.h - (state.scissor.y + state.scissor.h);

	// Initialize multiple texture unit support for shaders, if available.
	state.textureUnits.clear();
	if (Shader::isSupported())
	{
		GLint maxtextureunits;
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxtextureunits);

		state.textureUnits.resize(maxtextureunits, 0);

		GLenum curgltextureunit;
		glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint *) &curgltextureunit);

		state.curTextureUnit = (int) curgltextureunit - GL_TEXTURE0;

		// Retrieve currently bound textures for each texture unit.
		for (size_t i = 0; i < state.textureUnits.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint *) &state.textureUnits[i]);
		}

		glActiveTexture(curgltextureunit);
	}
	else
	{
		// Multitexturing not supported, so we only have 1 texture unit.
		state.textureUnits.resize(1, 0);
		state.curTextureUnit = 0;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint *) &state.textureUnits[0]);
	}

	initMaxValues();
	createDefaultTexture();

	contextInitialized = true;
}

void OpenGL::deInitContext()
{
	if (!contextInitialized)
		return;

	contextInitialized = false;
}

void OpenGL::initVendor()
{
	const char *vstr = (const char *) glGetString(GL_VENDOR);
	if (!vstr)
	{
		vendor = VENDOR_UNKNOWN;
		return;
	}

	// http://feedback.wildfiregames.com/report/opengl/feature/GL_VENDOR
	if (strstr(vstr, "ATI Technologies"))
		vendor = VENDOR_ATI_AMD;
	else if (strstr(vstr, "NVIDIA"))
		vendor = VENDOR_NVIDIA;
	else if (strstr(vstr, "Intel"))
		vendor = VENDOR_INTEL;
	else if (strstr(vstr, "Mesa"))
		vendor = VENDOR_MESA_SOFT;
	else if (strstr(vstr, "Apple Computer"))
		vendor = VENDOR_APPLE;
	else if (strstr(vstr, "Microsoft"))
		vendor = VENDOR_MICROSOFT;
	else
		vendor = VENDOR_UNKNOWN;
}

void OpenGL::initOpenGLFunctions()
{
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

	// Same deal for compressed textures.
	if (GLEE_VERSION_1_3 && !GLEE_ARB_texture_compression)
	{
		glCompressedTexImage2DARB = (GLEEPFNGLCOMPRESSEDTEXIMAGE2DARBPROC) glCompressedTexImage2D;
		glCompressedTexSubImage2DARB = (GLEEPFNGLCOMPRESSEDTEXSUBIMAGE2DARBPROC) glCompressedTexSubImage2D;
		glGetCompressedTexImageARB = (GLEEPFNGLGETCOMPRESSEDTEXIMAGEARBPROC) glGetCompressedTexImage;
	}
}

void OpenGL::initMaxValues()
{
	// We'll need this value to clamp anisotropy.
	if (GLEE_EXT_texture_filter_anisotropic)
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
	else
		maxAnisotropy = 1.0f;

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
}

void OpenGL::createDefaultTexture()
{
	// Set the 'default' texture (id 0) as a repeating white pixel. Otherwise,
	// texture2D calls inside a shader would return black when drawing graphics
	// primitives, which would create the need to use different "passthrough"
	// shaders for untextured primitives vs images.

	GLuint curtexture = state.textureUnits[state.curTextureUnit];
	bindTexture(0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	GLubyte pix = 255;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8, 1, 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, &pix);

	bindTexture(curtexture);
}

void OpenGL::setColor(const Color &c)
{
	glColor4ubv(&c.r);
	state.color = c;
}

Color OpenGL::getColor() const
{
	return state.color;
}

void OpenGL::setClearColor(const Color &c)
{
	glClearColor(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f);
	state.clearColor = c;
}

Color OpenGL::getClearColor() const
{
	return state.clearColor;
}

void OpenGL::setViewport(const OpenGL::Viewport &v)
{
	glViewport(v.x, v.y, v.w, v.h);
	state.viewport = v;

	// glScissor starts from the lower left, so we compensate when setting the
	// scissor. When the viewport is changed, we need to manually update the
	// scissor again.
	if (v.h != state.viewport.h)
		setScissor(state.scissor);
}

OpenGL::Viewport OpenGL::getViewport() const
{
	return state.viewport;
}

void OpenGL::setScissor(const OpenGL::Viewport &v)
{
	// We need to compensate for glScissor starting from the lower left of the
	// viewport instead of the top left.
	glScissor(v.x,  state.viewport.h - (v.y + v.h), v.w, v.h);
	state.scissor = v;
}

OpenGL::Viewport OpenGL::getScissor() const
{
	return state.scissor;
}

void OpenGL::setTextureUnit(int textureunit)
{
	if (textureunit < 0 || (size_t) textureunit >= state.textureUnits.size())
		throw love::Exception("Invalid texture unit index (%d).", textureunit);

	if (textureunit != state.curTextureUnit)
	{
		if (state.textureUnits.size() > 1)
			glActiveTexture(GL_TEXTURE0 + textureunit);
		else
			throw love::Exception("Multitexturing not supported.");
	}

	state.curTextureUnit = textureunit;
}

void OpenGL::bindTexture(GLuint texture)
{
	if (texture != state.textureUnits[state.curTextureUnit])
	{
		state.textureUnits[state.curTextureUnit] = texture;
		glBindTexture(GL_TEXTURE_2D, texture);
	}
}

void OpenGL::bindTextureToUnit(GLuint texture, int textureunit, bool restoreprev)
{
	if (textureunit < 0 || (size_t) textureunit >= state.textureUnits.size())
		throw love::Exception("Invalid texture unit index.");

	if (texture != state.textureUnits[textureunit])
	{
		int oldtextureunit = state.curTextureUnit;
		setTextureUnit(textureunit);

		state.textureUnits[textureunit] = texture;
		glBindTexture(GL_TEXTURE_2D, texture);

		if (restoreprev)
			setTextureUnit(oldtextureunit);
	}
}

void OpenGL::deleteTexture(GLuint texture)
{
	// glDeleteTextures binds texture 0 to all texture units the deleted texture
	// was bound to before deletion.
	std::vector<GLuint>::iterator it;
	for (it = state.textureUnits.begin(); it != state.textureUnits.end(); ++it)
	{
		if (*it == texture)
			*it = 0;
	}

	glDeleteTextures(1, &texture);
}

float OpenGL::setTextureFilter(const graphics::Image::Filter &f)
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

	float anisotropy = 1.0f;

	if (GLEE_EXT_texture_filter_anisotropic)
	{
		anisotropy = std::min(std::max(f.anisotropy, 1.0f), maxAnisotropy);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
	}

	return anisotropy;
}

graphics::Image::Filter OpenGL::getTextureFilter()
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

	if (GLEE_EXT_texture_filter_anisotropic)
		glGetTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, &f.anisotropy);

	return f;
}

void OpenGL::setTextureWrap(const graphics::Image::Wrap &w)
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

graphics::Image::Wrap OpenGL::getTextureWrap()
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

int OpenGL::getMaxTextureSize() const
{
	return maxTextureSize;
}

OpenGL::Vendor OpenGL::getVendor() const
{
	return vendor;
}

// OpenGL class instance singleton.
OpenGL gl;

} // opengl
} // graphics
} // love
