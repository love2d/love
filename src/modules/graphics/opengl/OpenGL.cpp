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
#include "OpenGL.h"

#include "Shader.h"
#include "Canvas.h"
#include "common/Exception.h"

// C++
#include <algorithm>
#include <limits>

// C
#include <cstring>
#include <cstdio>

// For SDL_GL_GetProcAddress.
#include <SDL_video.h>

#ifdef LOVE_IOS
#include <SDL_syswm.h>
#endif

#ifdef LOVE_ANDROID
#include <dlfcn.h>
#endif

namespace love
{
namespace graphics
{
namespace opengl
{

static void *LOVEGetProcAddress(const char *name)
{
#ifdef LOVE_ANDROID
	void *proc = dlsym(RTLD_DEFAULT, name);
	if (proc)
		return proc;
#endif

	return SDL_GL_GetProcAddress(name);
}

OpenGL::OpenGL()
	: stats()
	, contextInitialized(false)
	, maxAnisotropy(1.0f)
	, maxTextureSize(0)
	, maxRenderTargets(1)
	, maxRenderbufferSamples(0)
	, maxTextureUnits(1)
	, vendor(VENDOR_UNKNOWN)
	, state()
{
	matrices.transform.reserve(10);
	matrices.projection.reserve(2);
}

bool OpenGL::initContext()
{
	if (contextInitialized)
		return true;

	if (!gladLoadGLLoader(LOVEGetProcAddress))
		return false;

	initOpenGLFunctions();
	initVendor();
	initMatrices();

	bugs = {};

#if defined(LOVE_WINDOWS) || defined(LOVE_LINUX)
	// See the comments in OpenGL.h.
	if (getVendor() == VENDOR_AMD)
	{
		bugs.clearRequiresDriverTextureStateUpdate = true;
		bugs.generateMipmapsRequiresTexture2DEnable = true;
	}
#endif

	contextInitialized = true;

	return true;
}

void OpenGL::setupContext()
{
	if (!contextInitialized)
		return;

	initMaxValues();

	GLfloat glcolor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	glVertexAttrib4fv(ATTRIB_COLOR, glcolor);
	glVertexAttrib4fv(ATTRIB_CONSTANTCOLOR, glcolor);

	GLint maxvertexattribs = 1;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxvertexattribs);
	state.enabledAttribArrays = (uint32) ((1ull << uint32(maxvertexattribs)) - 1);
	useVertexAttribArrays(0);

	// Get the current viewport.
	glGetIntegerv(GL_VIEWPORT, (GLint *) &state.viewport.x);

	// And the current scissor - but we need to compensate for GL scissors
	// starting at the bottom left instead of top left.
	glGetIntegerv(GL_SCISSOR_BOX, (GLint *) &state.scissor.x);
	state.scissor.y = state.viewport.h - (state.scissor.y + state.scissor.h);

	if (GLAD_VERSION_1_0)
		glGetFloatv(GL_POINT_SIZE, &state.pointSize);
	else
		state.pointSize = 1.0f;

	if (GLAD_VERSION_3_0 || GLAD_ARB_framebuffer_sRGB || GLAD_EXT_framebuffer_sRGB
		|| GLAD_EXT_sRGB_write_control)
	{
		state.framebufferSRGBEnabled = (glIsEnabled(GL_FRAMEBUFFER_SRGB) == GL_TRUE);
	}
	else
		state.framebufferSRGBEnabled = false;

	// Initialize multiple texture unit support for shaders.
	state.boundTextures.clear();
	state.boundTextures.resize(maxTextureUnits, 0);

	for (int i = 0; i < (int) state.boundTextures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glActiveTexture(GL_TEXTURE0);
	state.curTextureUnit = 0;

	createDefaultTexture();

	// Invalidate the cached matrices by setting some elements to NaN.
	float nan = std::numeric_limits<float>::quiet_NaN();
	state.lastProjectionMatrix.setTranslation(nan, nan);
	state.lastTransformMatrix.setTranslation(nan, nan);

	if (GLAD_VERSION_1_0)
		glMatrixMode(GL_MODELVIEW);

	contextInitialized = true;
}

void OpenGL::deInitContext()
{
	if (!contextInitialized)
		return;

	glDeleteTextures(1, &state.defaultTexture);
	state.defaultTexture = 0;

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
	// http://stackoverflow.com/questions/2093594/opengl-extensions-available-on-different-android-devices
	if (strstr(vstr, "ATI Technologies"))
		vendor = VENDOR_AMD;
	else if (strstr(vstr, "NVIDIA"))
		vendor = VENDOR_NVIDIA;
	else if (strstr(vstr, "Intel"))
		vendor = VENDOR_INTEL;
	else if (strstr(vstr, "Mesa"))
		vendor = VENDOR_MESA_SOFT;
	else if (strstr(vstr, "Apple Computer") || strstr(vstr, "Apple Inc."))
		vendor = VENDOR_APPLE;
	else if (strstr(vstr, "Microsoft"))
		vendor = VENDOR_MICROSOFT;
	else if (strstr(vstr, "Imagination"))
		vendor = VENDOR_IMGTEC;
	else if (strstr(vstr, "ARM"))
		vendor = VENDOR_ARM;
	else if (strstr(vstr, "Qualcomm"))
		vendor = VENDOR_QUALCOMM;
	else if (strstr(vstr, "Broadcom"))
		vendor = VENDOR_BROADCOM;
	else if (strstr(vstr, "Vivante"))
		vendor = VENDOR_VIVANTE;
	else
		vendor = VENDOR_UNKNOWN;
}

void OpenGL::initOpenGLFunctions()
{
	// Alias extension-suffixed framebuffer functions to core versions since
	// there are so many different-named extensions that do the same things...
	if (!(GLAD_ES_VERSION_3_0 || GLAD_VERSION_3_0 || GLAD_ARB_framebuffer_object))
	{
		if (GLAD_VERSION_1_0 && GLAD_EXT_framebuffer_object)
		{
			fp_glBindRenderbuffer = fp_glBindRenderbufferEXT;
			fp_glDeleteRenderbuffers = fp_glDeleteRenderbuffersEXT;
			fp_glGenRenderbuffers = fp_glGenRenderbuffersEXT;
			fp_glRenderbufferStorage = fp_glRenderbufferStorageEXT;
			fp_glGetRenderbufferParameteriv = fp_glGetRenderbufferParameterivEXT;
			fp_glBindFramebuffer = fp_glBindFramebufferEXT;
			fp_glDeleteFramebuffers = fp_glDeleteFramebuffersEXT;
			fp_glGenFramebuffers = fp_glGenFramebuffersEXT;
			fp_glCheckFramebufferStatus = fp_glCheckFramebufferStatusEXT;
			fp_glFramebufferTexture2D = fp_glFramebufferTexture2DEXT;
			fp_glFramebufferRenderbuffer = fp_glFramebufferRenderbufferEXT;
			fp_glGetFramebufferAttachmentParameteriv = fp_glGetFramebufferAttachmentParameterivEXT;
			fp_glGenerateMipmap = fp_glGenerateMipmapEXT;
		}

		if (GLAD_EXT_framebuffer_blit)
			fp_glBlitFramebuffer = fp_glBlitFramebufferEXT;
		else if (GLAD_ANGLE_framebuffer_blit)
			fp_glBlitFramebuffer = fp_glBlitFramebufferANGLE;
		else if (GLAD_NV_framebuffer_blit)
			fp_glBlitFramebuffer = fp_glBlitFramebufferNV;

		if (GLAD_EXT_framebuffer_multisample)
			fp_glRenderbufferStorageMultisample = fp_glRenderbufferStorageMultisampleEXT;
		else if (GLAD_APPLE_framebuffer_multisample)
			fp_glRenderbufferStorageMultisample = fp_glRenderbufferStorageMultisampleAPPLE;
		else if (GLAD_ANGLE_framebuffer_multisample)
			fp_glRenderbufferStorageMultisample = fp_glRenderbufferStorageMultisampleANGLE;
		else if (GLAD_NV_framebuffer_multisample)
			fp_glRenderbufferStorageMultisample = fp_glRenderbufferStorageMultisampleNV;
	}
}

void OpenGL::initMaxValues()
{
	// We'll need this value to clamp anisotropy.
	if (GLAD_EXT_texture_filter_anisotropic)
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
	else
		maxAnisotropy = 1.0f;

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);

	int maxattachments = 1;
	int maxdrawbuffers = 1;

	if (GLAD_ES_VERSION_3_0 || GLAD_VERSION_2_0)
	{
		glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxattachments);
		glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxdrawbuffers);
	}

	maxRenderTargets = std::min(maxattachments, maxdrawbuffers);

	if (GLAD_ES_VERSION_3_0 || GLAD_VERSION_3_0 || GLAD_ARB_framebuffer_object
		|| GLAD_EXT_framebuffer_multisample || GLAD_APPLE_framebuffer_multisample
		|| GLAD_ANGLE_framebuffer_multisample)
	{
		glGetIntegerv(GL_MAX_SAMPLES, &maxRenderbufferSamples);
	}
	else
		maxRenderbufferSamples = 0;

	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
}

void OpenGL::initMatrices()
{
	matrices.transform.clear();
	matrices.projection.clear();

	matrices.transform.push_back(Matrix4());
	matrices.projection.push_back(Matrix4());
}

void OpenGL::createDefaultTexture()
{
	// Set the 'default' texture (id 0) as a repeating white pixel. Otherwise,
	// texture2D calls inside a shader would return black when drawing graphics
	// primitives, which would create the need to use different "passthrough"
	// shaders for untextured primitives vs images.

	GLuint curtexture = state.boundTextures[state.curTextureUnit];

	glGenTextures(1, &state.defaultTexture);
	bindTexture(state.defaultTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	GLubyte pix[] = {255, 255, 255, 255};
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pix);

	bindTexture(curtexture);
}

void OpenGL::pushTransform()
{
	matrices.transform.push_back(matrices.transform.back());
}

void OpenGL::popTransform()
{
	matrices.transform.pop_back();
}

Matrix4 &OpenGL::getTransform()
{
	return matrices.transform.back();
}

void OpenGL::prepareDraw()
{
	TempDebugGroup debuggroup("Prepare OpenGL draw");

	// Make sure the active shader's love-provided uniforms are up to date.
	if (Shader::current != nullptr)
		Shader::current->checkSetBuiltinUniforms();

	// We use glLoadMatrix rather than uniforms for our matrices when possible,
	// because uniform uploads can be significantly slower than glLoadMatrix.
	if (GLAD_VERSION_1_0)
	{
		const Matrix4 &curproj = matrices.projection.back();
		const Matrix4 &curxform = matrices.transform.back();

		const Matrix4 &lastproj = state.lastProjectionMatrix;
		const Matrix4 &lastxform = state.lastTransformMatrix;

		// We only need to re-upload the projection matrix if it's changed.
		if (memcmp(curproj.getElements(), lastproj.getElements(), sizeof(float) * 16) != 0)
		{
			glMatrixMode(GL_PROJECTION);
			glLoadMatrixf(curproj.getElements());
			glMatrixMode(GL_MODELVIEW);

			state.lastProjectionMatrix = matrices.projection.back();
		}

		// Same with the transform matrix.
		if (memcmp(curxform.getElements(), lastxform.getElements(), sizeof(float) * 16) != 0)
		{
			glLoadMatrixf(curxform.getElements());
			state.lastTransformMatrix = matrices.transform.back();
		}
	}
}

void OpenGL::drawArrays(GLenum mode, GLint first, GLsizei count)
{
	glDrawArrays(mode, first, count);
	++stats.drawCalls;
}

void OpenGL::drawElements(GLenum mode, GLsizei count, GLenum type, const void *indices)
{
	glDrawElements(mode, count, type, indices);
	++stats.drawCalls;
}

void OpenGL::useVertexAttribArrays(uint32 arraybits)
{
	uint32 diff = arraybits ^ state.enabledAttribArrays;

	if (diff == 0)
		return;

	// Max 32 attributes. As of when this was written, no GL driver exposes more
	// than 32. Lets hope that doesn't change...
	for (uint32 i = 0; i < 32; i++)
	{
		uint32 bit = 1 << i;

		if (diff & bit)
		{
			if (arraybits & bit)
				glEnableVertexAttribArray(i);
			else
				glDisableVertexAttribArray(i);
		}
	}

	state.enabledAttribArrays = arraybits;

	// glDisableVertexAttribArray will make the constant value for a vertex
	// attribute undefined. We rely on the per-vertex color attribute being
	// white when no per-vertex color is used, so we set it here.
	// FIXME: Is there a better place to do this?
	if ((diff & ATTRIBFLAG_COLOR) && !(arraybits & ATTRIBFLAG_COLOR))
		glVertexAttrib4f(ATTRIB_COLOR, 1.0f, 1.0f, 1.0f, 1.0f);
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

void OpenGL::setPointSize(float size)
{
	if (GLAD_VERSION_1_0)
		glPointSize(size);

	state.pointSize = size;
}

float OpenGL::getPointSize() const
{
	return state.pointSize;
}

void OpenGL::setFramebufferSRGB(bool enable)
{
	if (enable)
		glEnable(GL_FRAMEBUFFER_SRGB);
	else
		glDisable(GL_FRAMEBUFFER_SRGB);

	state.framebufferSRGBEnabled = enable;
}

bool OpenGL::hasFramebufferSRGB() const
{
	return state.framebufferSRGBEnabled;
}

void OpenGL::bindFramebuffer(GLenum target, GLuint framebuffer)
{
	glBindFramebuffer(target, framebuffer);

	if (target == GL_FRAMEBUFFER)
		++stats.framebufferBinds;
}

GLuint OpenGL::getDefaultFBO() const
{
#ifdef LOVE_IOS
	// Hack: iOS uses a custom FBO.
	SDL_SysWMinfo info = {};
	SDL_VERSION(&info.version);
	SDL_GetWindowWMInfo(SDL_GL_GetCurrentWindow(), &info);
	return info.info.uikit.framebuffer;
#else
	return 0;
#endif
}

GLuint OpenGL::getDefaultTexture() const
{
	return state.defaultTexture;
}

void OpenGL::setTextureUnit(int textureunit)
{
	if (textureunit < 0 || (size_t) textureunit >= state.boundTextures.size())
		throw love::Exception("Invalid texture unit index (%d).", textureunit);

	if (textureunit != state.curTextureUnit)
		glActiveTexture(GL_TEXTURE0 + textureunit);

	state.curTextureUnit = textureunit;
}

void OpenGL::bindTexture(GLuint texture)
{
	if (texture != state.boundTextures[state.curTextureUnit])
	{
		state.boundTextures[state.curTextureUnit] = texture;
		glBindTexture(GL_TEXTURE_2D, texture);
	}
}

void OpenGL::bindTextureToUnit(GLuint texture, int textureunit, bool restoreprev)
{
	if (textureunit < 0 || (size_t) textureunit >= state.boundTextures.size())
		throw love::Exception("Invalid texture unit index.");

	if (texture != state.boundTextures[textureunit])
	{
		int oldtextureunit = state.curTextureUnit;
		setTextureUnit(textureunit);

		state.boundTextures[textureunit] = texture;
		glBindTexture(GL_TEXTURE_2D, texture);

		if (restoreprev)
			setTextureUnit(oldtextureunit);
	}
}

void OpenGL::deleteTexture(GLuint texture)
{
	// glDeleteTextures binds texture 0 to all texture units the deleted texture
	// was bound to before deletion.
	for (GLuint &texid : state.boundTextures)
	{
		if (texid == texture)
			texid = 0;
	}

	glDeleteTextures(1, &texture);
}

void OpenGL::setTextureFilter(graphics::Texture::Filter &f)
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

	if (GLAD_EXT_texture_filter_anisotropic)
	{
		f.anisotropy = std::min(std::max(f.anisotropy, 1.0f), maxAnisotropy);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, f.anisotropy);
	}
	else
		f.anisotropy = 1.0f;
}

GLint OpenGL::getGLWrapMode(Texture::WrapMode wmode)
{
	switch (wmode)
	{
	case Texture::WRAP_CLAMP:
	default:
		return GL_CLAMP_TO_EDGE;
	case Texture::WRAP_CLAMP_ZERO:
		return GL_CLAMP_TO_BORDER;
	case Texture::WRAP_REPEAT:
		return GL_REPEAT;
	case Texture::WRAP_MIRRORED_REPEAT:
		return GL_MIRRORED_REPEAT;
	}

}

void OpenGL::setTextureWrap(const graphics::Texture::Wrap &w)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, getGLWrapMode(w.s));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, getGLWrapMode(w.t));
}

bool OpenGL::isClampZeroTextureWrapSupported() const
{
	return GLAD_VERSION_1_3 || GLAD_EXT_texture_border_clamp || GLAD_NV_texture_border_clamp;
}

int OpenGL::getMaxTextureSize() const
{
	return maxTextureSize;
}

int OpenGL::getMaxRenderTargets() const
{
	return maxRenderTargets;
}

int OpenGL::getMaxRenderbufferSamples() const
{
	return maxRenderbufferSamples;
}

int OpenGL::getMaxTextureUnits() const
{
	return maxTextureUnits;
}

void OpenGL::updateTextureMemorySize(size_t oldsize, size_t newsize)
{
	int64 memsize = (int64) stats.textureMemory + ((int64) newsize - (int64) oldsize);
	stats.textureMemory = (size_t) std::max(memsize, (int64) 0);
}

OpenGL::Vendor OpenGL::getVendor() const
{
	return vendor;
}

const char *OpenGL::errorString(GLenum errorcode)
{
	switch (errorcode)
	{
	case GL_NO_ERROR:
		return "no error";
	case GL_INVALID_ENUM:
		return "invalid enum";
	case GL_INVALID_VALUE:
		return "invalid value";
	case GL_INVALID_OPERATION:
		return "invalid operation";
	case GL_OUT_OF_MEMORY:
		return "out of memory";
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		return "invalid framebuffer operation";
	case GL_CONTEXT_LOST:
		return "OpenGL context has been lost";
	default:
		break;
	}

	static char text[64] = {};

	memset(text, 0, sizeof(text));
	sprintf(text, "0x%x", errorcode);

	return text;
}

const char *OpenGL::debugSeverityString(GLenum severity)
{
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:
		return "high";
	case GL_DEBUG_SEVERITY_MEDIUM:
		return "medium";
	case GL_DEBUG_SEVERITY_LOW:
		return "low";
	default:
		return "unknown";
	}
}

const char *OpenGL::debugSourceString(GLenum source)
{
	switch (source)
	{
	case GL_DEBUG_SOURCE_API:
		return "API";
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		return "window";
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		return "shader";
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		return "external";
	case GL_DEBUG_SOURCE_APPLICATION:
		return "LOVE";
	case GL_DEBUG_SOURCE_OTHER:
		return "other";
	default:
		return "unknown";
	}
}

const char *OpenGL::debugTypeString(GLenum type)
{
	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:
		return "error";
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		return "deprecated behavior";
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		return "undefined behavior";
	case GL_DEBUG_TYPE_PERFORMANCE:
		return "performance";
	case GL_DEBUG_TYPE_PORTABILITY:
		return "portability";
	case GL_DEBUG_TYPE_OTHER:
		return "other";
	default:
		return "unknown";
	}
}


// OpenGL class instance singleton.
OpenGL gl;

} // opengl
} // graphics
} // love
