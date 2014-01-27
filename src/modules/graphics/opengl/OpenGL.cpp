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
#include "OpenGL.h"

#include "Shader.h"
#include "Canvas.h"
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
	, maxRenderTargets(0)
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

	state.lastPseudoInstanceID = -1;

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

	if (Canvas::isSupported() && (GLEE_VERSION_2_0 || GLEE_ARB_draw_buffers))
	{
		int maxattachments = 0;
		glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxattachments);

		int maxdrawbuffers = 0;
		glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxdrawbuffers);

		maxRenderTargets = std::min(maxattachments, maxdrawbuffers);
	}
	else
		maxRenderTargets = 0;
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

void OpenGL::prepareDraw()
{
	Shader *shader = Shader::current;
	if (shader != nullptr)
	{
		// Make sure the active shader has the correct values for its
		// love-provided uniforms.
		shader->checkSetScreenParams();

		// Make sure the Instance ID variable is up-to-date when
		// pseudo-instancing is used.
		if (state.lastPseudoInstanceID != 0 && shader->hasVertexAttrib(ATTRIB_PSEUDO_INSTANCE_ID))
		{
			glVertexAttrib1f((GLuint) ATTRIB_PSEUDO_INSTANCE_ID, 0.0f);
			state.lastPseudoInstanceID = 0;
		}

		// We need to make sure antialiased Canvases are properly resolved
		// before sampling from their textures in a shader.
		// This is kind of a big hack. :(
		const std::map<std::string, Object *> &r = shader->getBoundRetainables();
		for (auto it = r.begin(); it != r.end(); ++it)
		{
			// Even bigger hack! D:
			Canvas *canvas = dynamic_cast<Canvas *>(it->second);
			if (canvas != nullptr)
				canvas->resolveMSAA();
		}
	}
}

void OpenGL::drawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei primcount)
{
	Shader *shader = Shader::current;

	if (GLEE_ARB_draw_instanced)
		glDrawArraysInstancedARB(mode, first, count, primcount);
	else
	{
		bool shaderHasID = shader && shader->hasVertexAttrib(ATTRIB_PSEUDO_INSTANCE_ID);

		// Pseudo-instancing fallback.
		for (int i = 0; i < primcount; i++)
		{
			if (shaderHasID)
				glVertexAttrib1f((GLuint) ATTRIB_PSEUDO_INSTANCE_ID, (GLfloat) i);

			glDrawArrays(mode, first, count);
		}

		if (shaderHasID)
			state.lastPseudoInstanceID = primcount - 1;
	}
}

void OpenGL::drawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount)
{
	Shader *shader = Shader::current;

	if (GLEE_ARB_draw_instanced)
		glDrawElementsInstancedARB(mode, count, type, indices, primcount);
	else
	{
		bool shaderHasID = shader && shader->hasVertexAttrib(ATTRIB_PSEUDO_INSTANCE_ID);

		// Pseudo-instancing fallback.
		for (int i = 0; i < primcount; i++)
		{
			if (shaderHasID)
				glVertexAttrib1f((GLuint) ATTRIB_PSEUDO_INSTANCE_ID, (GLfloat) i);

			glDrawElements(mode, count, type, indices);
		}

		if (shaderHasID)
			state.lastPseudoInstanceID = primcount - 1;
	}
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
	setScissor(state.scissor);
}

OpenGL::Viewport OpenGL::getViewport() const
{
	return state.viewport;
}

void OpenGL::setScissor(const OpenGL::Viewport &v)
{
	if (Canvas::current)
		glScissor(v.x, v.y, v.w, v.h);
	else
	{
		// With no Canvas active, we need to compensate for glScissor starting
		// from the lower left of the viewport instead of the top left.
		glScissor(v.x, state.viewport.h - (v.y + v.h), v.w, v.h);
	}

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
			throw love::Exception("Multitexturing is not supported.");
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

float OpenGL::setTextureFilter(graphics::Texture::Filter &f)
{
	GLint gmin, gmag;

	if (f.mipmap == Texture::FILTER_NONE)
	{
		if (f.min == Texture::FILTER_NEAREST)
			gmin = GL_NEAREST;
		else // f.min == Texture::FILTER_LINEAR
			gmin = GL_LINEAR;
	}
	else
	{
		if (f.min == Texture::FILTER_NEAREST && f.mipmap == Texture::FILTER_NEAREST)
			gmin = GL_NEAREST_MIPMAP_NEAREST;
		else if (f.min == Texture::FILTER_NEAREST && f.mipmap == Texture::FILTER_LINEAR)
			gmin = GL_NEAREST_MIPMAP_LINEAR;
		else if (f.min == Texture::FILTER_LINEAR && f.mipmap == Texture::FILTER_NEAREST)
			gmin = GL_LINEAR_MIPMAP_NEAREST;
		else if (f.min == Texture::FILTER_LINEAR && f.mipmap == Texture::FILTER_LINEAR)
			gmin = GL_LINEAR_MIPMAP_LINEAR;
		else
			gmin = GL_LINEAR;
	}


	switch (f.mag)
	{
	case Texture::FILTER_NEAREST:
		gmag = GL_NEAREST;
		break;
	case Texture::FILTER_LINEAR:
	default:
		gmag = GL_LINEAR;
		break;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gmin);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gmag);

	if (GLEE_EXT_texture_filter_anisotropic)
	{
		f.anisotropy = std::min(std::max(f.anisotropy, 1.0f), maxAnisotropy);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, f.anisotropy);
	}

	return f.anisotropy;
}

graphics::Texture::Filter OpenGL::getTextureFilter()
{
	GLint gmin, gmag;
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &gmin);
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, &gmag);

	Texture::Filter f;

	switch (gmin)
	{
	case GL_NEAREST:
		f.min = Texture::FILTER_NEAREST;
		f.mipmap = Texture::FILTER_NONE;
		break;
	case GL_NEAREST_MIPMAP_NEAREST:
		f.min = f.mipmap = Texture::FILTER_NEAREST;
		break;
	case GL_NEAREST_MIPMAP_LINEAR:
		f.min = Texture::FILTER_NEAREST;
		f.mipmap = Texture::FILTER_LINEAR;
		break;
	case GL_LINEAR_MIPMAP_NEAREST:
		f.min = Texture::FILTER_LINEAR;
		f.mipmap = Texture::FILTER_NEAREST;
		break;
	case GL_LINEAR_MIPMAP_LINEAR:
		f.min = f.mipmap = Texture::FILTER_LINEAR;
		break;
	case GL_LINEAR:
	default:
		f.min = Texture::FILTER_LINEAR;
		f.mipmap = Texture::FILTER_NONE;
		break;
	}

	switch (gmag)
	{
	case GL_NEAREST:
		f.mag = Texture::FILTER_NEAREST;
		break;
	case GL_LINEAR:
	default:
		f.mag = Texture::FILTER_LINEAR;
		break;
	}

	if (GLEE_EXT_texture_filter_anisotropic)
		glGetTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, &f.anisotropy);

	return f;
}

void OpenGL::setTextureWrap(const graphics::Texture::Wrap &w)
{
	GLint gs, gt;

	switch (w.s)
	{
	case Texture::WRAP_CLAMP:
		gs = GL_CLAMP_TO_EDGE;
		break;
	case Texture::WRAP_REPEAT:
	default:
		gs = GL_REPEAT;
		break;
	}

	switch (w.t)
	{
	case Texture::WRAP_CLAMP:
		gt = GL_CLAMP_TO_EDGE;
		break;
	case Texture::WRAP_REPEAT:
	default:
		gt = GL_REPEAT;
		break;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gs);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gt);
}

graphics::Texture::Wrap OpenGL::getTextureWrap()
{
	GLint gs, gt;

	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &gs);
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, &gt);

	Texture::Wrap w;

	switch (gs)
	{
	case GL_CLAMP_TO_EDGE:
		w.s = Texture::WRAP_CLAMP;
		break;
	case GL_REPEAT:
	default:
		w.s = Texture::WRAP_REPEAT;
		break;
	}

	switch (gt)
	{
	case GL_CLAMP_TO_EDGE:
		w.t = Texture::WRAP_CLAMP;
		break;
	case GL_REPEAT:
	default:
		w.t = Texture::WRAP_REPEAT;
		break;
	}

	return w;
}

int OpenGL::getMaxTextureSize() const
{
	return maxTextureSize;
}

int OpenGL::getMaxRenderTargets() const
{
	return maxRenderTargets;
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
