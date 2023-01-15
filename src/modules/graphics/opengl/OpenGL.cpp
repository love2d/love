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

// LOVE
#include "common/config.h"
#include "OpenGL.h"

#include "Shader.h"
#include "Canvas.h"
#include "common/Exception.h"

#include "graphics/Graphics.h"
#include "graphics/Buffer.h"

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

OpenGL::TempDebugGroup::TempDebugGroup(const char *name)
{
	if (isDebugEnabled())
	{
		if (GLAD_VERSION_4_3 || (GLAD_KHR_debug && !GLAD_ES_VERSION_2_0))
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 0, (const GLchar *) name);
		else if (GLAD_ES_VERSION_2_0 && GLAD_KHR_debug)
			glPushDebugGroupKHR(GL_DEBUG_SOURCE_APPLICATION, 0, 0, (const GLchar *) name);
		else if (GLAD_EXT_debug_marker)
			glPushGroupMarkerEXT(0, (const GLchar *) name);
	}
}

OpenGL::TempDebugGroup::~TempDebugGroup()
{
	if (isDebugEnabled())
	{
		if (GLAD_VERSION_4_3 || (GLAD_KHR_debug && !GLAD_ES_VERSION_2_0))
			glPopDebugGroup();
		else if (GLAD_ES_VERSION_2_0 && GLAD_KHR_debug)
			glPopDebugGroupKHR();
		else if (GLAD_EXT_debug_marker)
			glPopGroupMarkerEXT();
	}
}

OpenGL::OpenGL()
	: stats()
	, contextInitialized(false)
	, pixelShaderHighpSupported(false)
	, baseVertexSupported(false)
	, maxAnisotropy(1.0f)
	, max2DTextureSize(0)
	, max3DTextureSize(0)
	, maxCubeTextureSize(0)
	, maxTextureArrayLayers(0)
	, maxRenderTargets(1)
	, maxRenderbufferSamples(0)
	, maxTextureUnits(1)
	, maxPointSize(1)
	, coreProfile(false)
	, vendor(VENDOR_UNKNOWN)
	, state()
{
	state.constantColor = Colorf(1.0f, 1.0f, 1.0f, 1.0f);

	float nan = std::numeric_limits<float>::quiet_NaN();
	state.lastConstantColor = Colorf(nan, nan, nan, nan);
}

bool OpenGL::initContext()
{
	if (contextInitialized)
		return true;

	if (!gladLoadGLLoader(LOVEGetProcAddress))
		return false;

	initVendor();

	bugs = {};

	if (GLAD_ES_VERSION_3_0 && !GLAD_ES_VERSION_3_1)
	{
		const char *device = (const char *) glGetString(GL_RENDERER);
		if (getVendor() == VENDOR_VIVANTE && strstr(device, "Vivante GC7000UL"))
			bugs.brokenGLES3 = true;
	}

	if (bugs.brokenGLES3)
		GLAD_ES_VERSION_3_0 = false;

	if (GLAD_VERSION_3_2)
	{
		GLint profileMask = 0;
		glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profileMask);
		coreProfile = (profileMask & GL_CONTEXT_CORE_PROFILE_BIT);
	}
	else
		coreProfile = false;

	initOpenGLFunctions();

#if defined(LOVE_WINDOWS) || defined(LOVE_LINUX)
	// See the comments in OpenGL.h.
	if (getVendor() == VENDOR_AMD)
	{
		bugs.clearRequiresDriverTextureStateUpdate = true;
		if (!gl.isCoreProfile() && !GLAD_ES_VERSION_2_0)
			bugs.generateMipmapsRequiresTexture2DEnable = true;
	}
#endif

#ifdef LOVE_WINDOWS
	if (getVendor() == VENDOR_INTEL && gl.isCoreProfile())
	{
		const char *device = (const char *) glGetString(GL_RENDERER);
		if (strstr(device, "HD Graphics 4000") || strstr(device, "HD Graphics 2500"))
			bugs.clientWaitSyncStalls = true;
	}

	if (getVendor() == VENDOR_INTEL)
	{
		const char *device = (const char *) glGetString(GL_RENDERER);
		if (strstr(device, "HD Graphics 3000") || strstr(device, "HD Graphics 2000")
			|| !strcmp(device, "Intel(R) HD Graphics") || !strcmp(device, "Intel(R) HD Graphics Family"))
		{
			bugs.brokenSRGB = true;
		}
	}
#endif

#ifdef LOVE_WINDOWS
	if (getVendor() == VENDOR_AMD)
	{
		// Radeon drivers switched from "ATI Radeon" to "AMD Radeon" around
		// the 7000 series. We'll assume this bug doesn't affect those newer
		// GPUs / drivers.
		const char *device = (const char *) glGetString(GL_RENDERER);
		if (strstr(device, "ATI Radeon") || strstr(device, "ATI Mobility Radeon"))
			bugs.texStorageBreaksSubImage = true;
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
	state.instancedAttribArrays = 0;

	setVertexAttributes(vertex::Attributes(), vertex::BufferBindings());

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

	for (int i = 0; i < 2; i++)
		state.boundFramebuffers[i] = std::numeric_limits<GLuint>::max();
	bindFramebuffer(FRAMEBUFFER_ALL, getDefaultFBO());

	setEnableState(ENABLE_DEPTH_TEST, state.enableState[ENABLE_DEPTH_TEST]);
	setEnableState(ENABLE_STENCIL_TEST, state.enableState[ENABLE_STENCIL_TEST]);
	setEnableState(ENABLE_SCISSOR_TEST, state.enableState[ENABLE_SCISSOR_TEST]);
	setEnableState(ENABLE_FACE_CULL, state.enableState[ENABLE_FACE_CULL]);

	if (!bugs.brokenSRGB && (GLAD_VERSION_3_0 || GLAD_ARB_framebuffer_sRGB
		|| GLAD_EXT_framebuffer_sRGB || GLAD_EXT_sRGB_write_control))
	{
		setEnableState(ENABLE_FRAMEBUFFER_SRGB, state.enableState[ENABLE_FRAMEBUFFER_SRGB]);
	}
	else
		state.enableState[ENABLE_FRAMEBUFFER_SRGB] = false;

	GLint faceCull = GL_BACK;
	glGetIntegerv(GL_CULL_FACE_MODE, &faceCull);
	state.faceCullMode = faceCull;

	for (int i = 0; i < (int) BUFFER_MAX_ENUM; i++)
	{
		state.boundBuffers[i] = 0;
		glBindBuffer(getGLBufferType((BufferType) i), 0);
	}

	// Initialize multiple texture unit support for shaders.
	for (int i = 0; i < TEXTURE_MAX_ENUM; i++)
	{
		state.boundTextures[i].clear();
		state.boundTextures[i].resize(maxTextureUnits, 0);
	}

	for (int i = 0; i < maxTextureUnits; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);

		for (int j = 0; j < TEXTURE_MAX_ENUM; j++)
		{
			TextureType textype = (TextureType) j;

			if (isTextureTypeSupported(textype))
				glBindTexture(getGLTextureType(textype), 0);
		}
	}

	glActiveTexture(GL_TEXTURE0);
	state.curTextureUnit = 0;

	setDepthWrites(state.depthWritesEnabled);

	createDefaultTexture();

	contextInitialized = true;

#ifdef LOVE_ANDROID
	// This can't be done in initContext with the rest of the bug checks because
	// Canvas::isFormatSupported relies on state initialized here / after init.
	if (GLAD_ES_VERSION_3_0 && !Canvas::isFormatSupported(PIXELFORMAT_R8))
		bugs.brokenR8PixelFormat = true;
#endif
}

void OpenGL::deInitContext()
{
	if (!contextInitialized)
		return;

	for (int i = 0; i < TEXTURE_MAX_ENUM; i++)
	{
		if (state.defaultTexture[i] != 0)
		{
			gl.deleteTexture(state.defaultTexture[i]);
			state.defaultTexture[i] = 0;
		}
	}

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
	// https://opengl.gpuinfo.org/displaycapability.php?name=GL_VENDOR
	if (strstr(vstr, "ATI Technologies") || strstr(vstr, "AMD") || strstr(vstr, "Advanced Micro Devices"))
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
			fp_glFramebufferTexture3D = fp_glFramebufferTexture3DEXT;
			fp_glFramebufferRenderbuffer = fp_glFramebufferRenderbufferEXT;
			fp_glGetFramebufferAttachmentParameteriv = fp_glGetFramebufferAttachmentParameterivEXT;
			fp_glGenerateMipmap = fp_glGenerateMipmapEXT;
		}

		if (GLAD_VERSION_1_0 && GLAD_EXT_texture_array)
			fp_glFramebufferTextureLayer = fp_glFramebufferTextureLayerEXT;

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

	if (isInstancingSupported() && !(GLAD_VERSION_3_3 || GLAD_ES_VERSION_3_0))
	{
		if (GLAD_ARB_instanced_arrays)
		{
			fp_glDrawArraysInstanced = fp_glDrawArraysInstancedARB;
			fp_glDrawElementsInstanced = fp_glDrawElementsInstancedARB;
			fp_glVertexAttribDivisor = fp_glVertexAttribDivisorARB;
		}
		else if (GLAD_EXT_instanced_arrays)
		{
			fp_glDrawArraysInstanced = fp_glDrawArraysInstancedEXT;
			fp_glDrawElementsInstanced = fp_glDrawElementsInstancedEXT;
			fp_glVertexAttribDivisor = fp_glVertexAttribDivisorEXT;
		}
		else if (GLAD_ANGLE_instanced_arrays)
		{
			fp_glDrawArraysInstanced = fp_glDrawArraysInstancedANGLE;
			fp_glDrawElementsInstanced = fp_glDrawElementsInstancedANGLE;
			fp_glVertexAttribDivisor = fp_glVertexAttribDivisorANGLE;
		}
	}

	if (GLAD_ES_VERSION_2_0 && !GLAD_ES_VERSION_3_0)
	{
		// The Nvidia Tegra 3 driver (used by Ouya) claims to support GL_EXT_texture_array but
		// segfaults if you actually try to use it. OpenGL ES 2.0 devices should use OES_texture_3D.
		// GL_EXT_texture_array is for desktops.
		GLAD_EXT_texture_array = false;

		if (GLAD_OES_texture_3D)
		{
			// Function signatures don't match, we'll have to conditionally call it
			//fp_glTexImage3D = fp_glTexImage3DOES;
			fp_glTexSubImage3D = fp_glTexSubImage3DOES;
			fp_glCopyTexSubImage3D = fp_glCopyTexSubImage3DOES;
			fp_glCompressedTexImage3D = fp_glCompressedTexImage3DOES;
			fp_glCompressedTexSubImage3D = fp_glCompressedTexSubImage3DOES;
			fp_glFramebufferTexture3D = fp_glFramebufferTexture3DOES;
		}
	}

	if (!GLAD_VERSION_3_2 && !GLAD_ES_VERSION_3_2 && !GLAD_ARB_draw_elements_base_vertex)
	{
		if (GLAD_OES_draw_elements_base_vertex)
		{
			fp_glDrawElementsBaseVertex = fp_glDrawElementsBaseVertexOES;

			if (GLAD_ES_VERSION_3_0)
			{
				fp_glDrawRangeElementsBaseVertex = fp_glDrawRangeElementsBaseVertexOES;
				fp_glDrawElementsInstancedBaseVertex = fp_glDrawElementsInstancedBaseVertexOES;
			}

		}
		else if (GLAD_EXT_draw_elements_base_vertex)
		{
			fp_glDrawElementsBaseVertex = fp_glDrawElementsBaseVertexEXT;

			if (GLAD_ES_VERSION_3_0)
			{
				fp_glDrawRangeElementsBaseVertex = fp_glDrawRangeElementsBaseVertexEXT;
				fp_glDrawElementsInstancedBaseVertex = fp_glDrawElementsInstancedBaseVertexEXT;
			}

		}
	}
}

void OpenGL::initMaxValues()
{
	if (GLAD_ES_VERSION_2_0 && !GLAD_ES_VERSION_3_0)
	{
		GLint range = 0;
		GLint precision = 0;
		glGetShaderPrecisionFormat(GL_FRAGMENT_SHADER, GL_HIGH_FLOAT, &range, &precision);
		pixelShaderHighpSupported = range > 0;
	}
	else
		pixelShaderHighpSupported = true;

	baseVertexSupported = GLAD_VERSION_3_2 || GLAD_ES_VERSION_3_2 || GLAD_ARB_draw_elements_base_vertex
		|| GLAD_OES_draw_elements_base_vertex || GLAD_EXT_draw_elements_base_vertex;

	// We'll need this value to clamp anisotropy.
	if (GLAD_EXT_texture_filter_anisotropic)
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
	else
		maxAnisotropy = 1.0f;

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max2DTextureSize);
	glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &maxCubeTextureSize);

	if (isTextureTypeSupported(TEXTURE_VOLUME))
		glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &max3DTextureSize);
	else
		max3DTextureSize = 0;

	if (isTextureTypeSupported(TEXTURE_2D_ARRAY))
		glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &maxTextureArrayLayers);
	else
		maxTextureArrayLayers = 0;

	int maxattachments = 1;
	int maxdrawbuffers = 1;

	if (GLAD_ES_VERSION_3_0 || GLAD_VERSION_2_0)
	{
		glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxattachments);
		glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxdrawbuffers);
	}

	maxRenderTargets = std::max(std::min(maxattachments, maxdrawbuffers), 1);

	if (GLAD_ES_VERSION_3_0 || GLAD_VERSION_3_0 || GLAD_ARB_framebuffer_object
		|| GLAD_EXT_framebuffer_multisample || GLAD_APPLE_framebuffer_multisample
		|| GLAD_ANGLE_framebuffer_multisample)
	{
		glGetIntegerv(GL_MAX_SAMPLES, &maxRenderbufferSamples);
	}
	else
		maxRenderbufferSamples = 0;

	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTextureUnits);

	GLfloat limits[2];
	if (GLAD_VERSION_3_0)
		glGetFloatv(GL_POINT_SIZE_RANGE, limits);
	else
		glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, limits);
	maxPointSize = limits[1];

	if (isSamplerLODBiasSupported())
		glGetFloatv(GL_MAX_TEXTURE_LOD_BIAS, &maxLODBias);
	else
		maxLODBias = 0.0f;
}

void OpenGL::createDefaultTexture()
{
	// Set the 'default' texture as a repeating white pixel. Otherwise, texture
	// calls inside a shader would return black when drawing graphics primitives
	// which would create the need to use different "passthrough" shaders for
	// untextured primitives vs images.
	const GLubyte pix[] = {255, 255, 255, 255};

	Texture::Filter filter;
	filter.min = filter.mag = Texture::FILTER_NEAREST;

	Texture::Wrap wrap;
	wrap.s = wrap.t = wrap.r = Texture::WRAP_CLAMP;

	for (int i = 0; i < TEXTURE_MAX_ENUM; i++)
	{
		state.defaultTexture[i] = 0;

		TextureType type = (TextureType) i;

		if (!isTextureTypeSupported(type))
			continue;

		GLuint curtexture = state.boundTextures[type][0];

		glGenTextures(1, &state.defaultTexture[type]);
		bindTextureToUnit(type, state.defaultTexture[type], 0, false);

		setTextureWrap(type, wrap);
		setTextureFilter(type, filter);

		bool isSRGB = false;
		rawTexStorage(type, 1, PIXELFORMAT_RGBA8, isSRGB, 1, 1);

		TextureFormat fmt = convertPixelFormat(PIXELFORMAT_RGBA8, false, isSRGB);

		int slices = type == TEXTURE_CUBE ? 6 : 1;

		for (int slice = 0; slice < slices; slice++)
		{
			GLenum gltarget = getGLTextureType(type);

			if (type == TEXTURE_CUBE)
				gltarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + slice;

			if (type == TEXTURE_2D || type == TEXTURE_CUBE)
				glTexSubImage2D(gltarget, 0, 0, 0, 1, 1, fmt.externalformat, fmt.type, pix);
			else if (type == TEXTURE_2D_ARRAY || type == TEXTURE_VOLUME)
				glTexSubImage3D(gltarget, 0, 0, 0, slice, 1, 1, 1, fmt.externalformat, fmt.type, pix);
		}

		bindTextureToUnit(type, curtexture, 0, false);
	}
}

void OpenGL::prepareDraw()
{
	TempDebugGroup debuggroup("Prepare OpenGL draw");

	// Make sure the active shader's love-provided uniforms are up to date.
	if (Shader::current != nullptr)
		((Shader *)Shader::current)->updateBuiltinUniforms();

	if (state.constantColor != state.lastConstantColor)
	{
		state.lastConstantColor = state.constantColor;
		Colorf c = state.constantColor;
		gammaCorrectColor(c);
		glVertexAttrib4f(ATTRIB_CONSTANTCOLOR, c.r, c.g, c.b, c.a);
	}
}

GLenum OpenGL::getGLPrimitiveType(PrimitiveType type)
{
	switch (type)
	{
	case PRIMITIVE_TRIANGLES:
		return GL_TRIANGLES;
	case PRIMITIVE_TRIANGLE_STRIP:
		return GL_TRIANGLE_STRIP;
	case PRIMITIVE_TRIANGLE_FAN:
		return GL_TRIANGLE_FAN;
	case PRIMITIVE_POINTS:
		return GL_POINTS;
	case PRIMITIVE_MAX_ENUM:
		return GL_ZERO;
	}

	return GL_ZERO;
}

GLenum OpenGL::getGLBufferType(BufferType type)
{
	switch (type)
	{
	case BUFFER_VERTEX:
		return GL_ARRAY_BUFFER;
	case BUFFER_INDEX:
		return GL_ELEMENT_ARRAY_BUFFER;
	case BUFFER_MAX_ENUM:
		return GL_ZERO;
	}

	return GL_ZERO;
}

GLenum OpenGL::getGLTextureType(TextureType type)
{
	switch (type)
	{
	case TEXTURE_2D:
		return GL_TEXTURE_2D;
	case TEXTURE_VOLUME:
		return GL_TEXTURE_3D;
	case TEXTURE_2D_ARRAY:
		return GL_TEXTURE_2D_ARRAY;
	case TEXTURE_CUBE:
		return GL_TEXTURE_CUBE_MAP;
	case TEXTURE_MAX_ENUM:
		return GL_ZERO;
	}

	return GL_ZERO;
}

GLenum OpenGL::getGLIndexDataType(IndexDataType type)
{
	switch (type)
	{
	case INDEX_UINT16:
		return GL_UNSIGNED_SHORT;
	case INDEX_UINT32:
		return GL_UNSIGNED_INT;
	default:
		return GL_ZERO;
	}
}

GLenum OpenGL::getGLVertexDataType(vertex::DataType type, GLboolean &normalized)
{
	normalized = GL_FALSE;

	switch (type)
	{
	case vertex::DATA_UNORM8:
		normalized = GL_TRUE;
		return GL_UNSIGNED_BYTE;
	case vertex::DATA_UNORM16:
		normalized = GL_TRUE;
		return GL_UNSIGNED_SHORT;
	case vertex::DATA_FLOAT:
		normalized = GL_FALSE;
		return GL_FLOAT;
	case vertex::DATA_MAX_ENUM:
		return GL_ZERO;
	}

	return GL_ZERO;
}

GLenum OpenGL::getGLBufferUsage(vertex::Usage usage)
{
	switch (usage)
	{
	case vertex::USAGE_STREAM:
		return GL_STREAM_DRAW;
	case vertex::USAGE_DYNAMIC:
		return GL_DYNAMIC_DRAW;
	case vertex::USAGE_STATIC:
		return GL_STATIC_DRAW;
	default:
		return 0;
	}
}

void OpenGL::bindBuffer(BufferType type, GLuint buffer)
{
	if (state.boundBuffers[type] != buffer)
	{
		glBindBuffer(getGLBufferType(type), buffer);
		state.boundBuffers[type] = buffer;
	}
}

void OpenGL::deleteBuffer(GLuint buffer)
{
	glDeleteBuffers(1, &buffer);

	for (int i = 0; i < (int) BUFFER_MAX_ENUM; i++)
	{
		if (state.boundBuffers[i] == buffer)
			state.boundBuffers[i] = 0;
	}
}

void OpenGL::setVertexAttributes(const vertex::Attributes &attributes, const vertex::BufferBindings &buffers)
{
	uint32 enablediff = attributes.enableBits ^ state.enabledAttribArrays;
	uint32 instanceattribbits = 0;
	uint32 allbits = attributes.enableBits | state.enabledAttribArrays;

	uint32 i = 0;
	while (allbits)
	{
		uint32 bit = 1u << i;

		if (enablediff & bit)
		{
			if (attributes.enableBits & bit)
				glEnableVertexAttribArray(i);
			else
				glDisableVertexAttribArray(i);
		}

		if (attributes.enableBits & bit)
		{
			const auto &attrib = attributes.attribs[i];
			const auto &layout = attributes.bufferLayouts[attrib.bufferIndex];
			const auto &bufferinfo = buffers.info[attrib.bufferIndex];

			uint32 bufferbit = 1u << attrib.bufferIndex;
			uint32 divisor = (attributes.instanceBits & bufferbit) != 0 ? 1 : 0;
			uint32 divisorbit = divisor << i;
			instanceattribbits |= divisorbit;

			if ((state.instancedAttribArrays & bit) ^ divisorbit)
				glVertexAttribDivisor(i, divisor);

			GLboolean normalized = GL_FALSE;
			GLenum gltype = getGLVertexDataType(attrib.type, normalized);

			const void *offsetpointer = reinterpret_cast<void*>(bufferinfo.offset + attrib.offsetFromVertex);

			bindBuffer(BUFFER_VERTEX, (GLuint) bufferinfo.buffer->getHandle());
			glVertexAttribPointer(i, attrib.components, gltype, normalized, layout.stride, offsetpointer);
		}

		i++;
		allbits >>= 1;
	}

	state.enabledAttribArrays = attributes.enableBits;
	state.instancedAttribArrays = instanceattribbits | (state.instancedAttribArrays & (~attributes.enableBits));

	// glDisableVertexAttribArray will make the constant value for a vertex
	// attribute undefined. We rely on the per-vertex color attribute being
	// white when no per-vertex color is used, so we set it here.
	// FIXME: Is there a better place to do this?
	if ((enablediff & ATTRIBFLAG_COLOR) && !(attributes.enableBits & ATTRIBFLAG_COLOR))
		glVertexAttrib4f(ATTRIB_COLOR, 1.0f, 1.0f, 1.0f, 1.0f);
}

void OpenGL::setCullMode(CullMode mode)
{
	bool enabled = mode != CULL_NONE;

	if (enabled != isStateEnabled(ENABLE_FACE_CULL))
		setEnableState(ENABLE_FACE_CULL, enabled);

	if (enabled)
	{
		GLenum glmode = mode == CULL_BACK ? GL_BACK : GL_FRONT;
		if (glmode != state.faceCullMode)
		{
			glCullFace(glmode);
			state.faceCullMode = glmode;
		}
	}
}

void OpenGL::clearDepth(double value)
{
	if (GLAD_ES_VERSION_2_0)
		glClearDepthf((GLfloat) value);
	else
		glClearDepth(value);
}

void OpenGL::setViewport(const Rect &v)
{
	glViewport(v.x, v.y, v.w, v.h);
	state.viewport = v;
}

Rect OpenGL::getViewport() const
{
	return state.viewport;
}

void OpenGL::setScissor(const Rect &v, bool canvasActive)
{
	if (canvasActive)
		glScissor(v.x, v.y, v.w, v.h);
	else
	{
		// With no Canvas active, we need to compensate for glScissor starting
		// from the lower left of the viewport instead of the top left.
		glScissor(v.x, state.viewport.h - (v.y + v.h), v.w, v.h);
	}

	state.scissor = v;
}

void OpenGL::setConstantColor(const Colorf &color)
{
	state.constantColor = color;
}

const Colorf &OpenGL::getConstantColor() const
{
	return state.constantColor;
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

void OpenGL::setEnableState(EnableState enablestate, bool enable)
{
	GLenum glstate = GL_NONE;

	switch (enablestate)
	{
	case ENABLE_DEPTH_TEST:
		glstate = GL_DEPTH_TEST;
		break;
	case ENABLE_STENCIL_TEST:
		glstate = GL_STENCIL_TEST;
		break;
	case ENABLE_SCISSOR_TEST:
		glstate = GL_SCISSOR_TEST;
		break;
	case ENABLE_FACE_CULL:
		glstate = GL_CULL_FACE;
		break;
	case ENABLE_FRAMEBUFFER_SRGB:
		glstate = GL_FRAMEBUFFER_SRGB;
		break;
	case ENABLE_MAX_ENUM:
		break;
	}

	if (enable)
		glEnable(glstate);
	else
		glDisable(glstate);

	state.enableState[enablestate] = enable;
}

bool OpenGL::isStateEnabled(EnableState enablestate) const
{
	return state.enableState[enablestate];
}

void OpenGL::bindFramebuffer(FramebufferTarget target, GLuint framebuffer)
{
	bool bindingmodified = false;

	if ((target & FRAMEBUFFER_DRAW) && state.boundFramebuffers[0] != framebuffer)
	{
		bindingmodified = true;
		state.boundFramebuffers[0] = framebuffer;
	}

	if ((target & FRAMEBUFFER_READ) && state.boundFramebuffers[1] != framebuffer)
	{
		bindingmodified = true;
		state.boundFramebuffers[1] = framebuffer;
	}

	if (bindingmodified)
	{
		GLenum gltarget = GL_FRAMEBUFFER;
		if (target == FRAMEBUFFER_DRAW)
			gltarget = GL_DRAW_FRAMEBUFFER;
		else if (target == FRAMEBUFFER_READ)
			gltarget = GL_READ_FRAMEBUFFER;

		glBindFramebuffer(gltarget, framebuffer);
	}
}

GLenum OpenGL::getFramebuffer(FramebufferTarget target) const
{
	if (target & FRAMEBUFFER_DRAW)
		return state.boundFramebuffers[0];
	else if (target & FRAMEBUFFER_READ)
		return state.boundFramebuffers[1];
	else
		return 0;
}

void OpenGL::deleteFramebuffer(GLuint framebuffer)
{
	glDeleteFramebuffers(1, &framebuffer);

	for (int i = 0; i < 2; i++)
	{
		if (state.boundFramebuffers[i] == framebuffer)
			state.boundFramebuffers[i] = 0;
	}
}

void OpenGL::framebufferTexture(GLenum attachment, TextureType texType, GLuint texture, int level, int layer, int face)
{
	GLenum textarget = getGLTextureType(texType);

	switch (texType)
	{
	case TEXTURE_2D:
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, textarget, texture, level);
		break;
	case TEXTURE_VOLUME:
		glFramebufferTexture3D(GL_FRAMEBUFFER, attachment, textarget, texture, level, layer);
		break;
	case TEXTURE_2D_ARRAY:
		glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, texture, level, layer);
		break;
	case TEXTURE_CUBE:
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, texture, level);
		break;
	default:
		break;
	}
}

void OpenGL::setDepthWrites(bool enable)
{
	glDepthMask(enable ? GL_TRUE : GL_FALSE);
	state.depthWritesEnabled = enable;
}

bool OpenGL::hasDepthWrites() const
{
	return state.depthWritesEnabled;
}

void OpenGL::useProgram(GLuint program)
{
	glUseProgram(program);
	++stats.shaderSwitches;
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

GLuint OpenGL::getDefaultTexture(TextureType type) const
{
	return state.defaultTexture[type];
}

void OpenGL::setTextureUnit(int textureunit)
{
	if (textureunit != state.curTextureUnit)
		glActiveTexture(GL_TEXTURE0 + textureunit);

	state.curTextureUnit = textureunit;
}

void OpenGL::bindTextureToUnit(TextureType target, GLuint texture, int textureunit, bool restoreprev, bool bindforedit)
{
	if (texture != state.boundTextures[target][textureunit])
	{
		int oldtextureunit = state.curTextureUnit;
		if (oldtextureunit != textureunit)
			glActiveTexture(GL_TEXTURE0 + textureunit);

		state.boundTextures[target][textureunit] = texture;
		glBindTexture(getGLTextureType(target), texture);

		if (restoreprev && oldtextureunit != textureunit)
			glActiveTexture(GL_TEXTURE0 + oldtextureunit);
		else
			state.curTextureUnit = textureunit;
	}
	else if (bindforedit && !restoreprev && textureunit != state.curTextureUnit)
	{
		glActiveTexture(GL_TEXTURE0 + textureunit);
		state.curTextureUnit = textureunit;
	}
}

void OpenGL::bindTextureToUnit(Texture *texture, int textureunit, bool restoreprev, bool bindforedit)
{
	TextureType textype = TEXTURE_2D;
	GLuint handle = 0;

	if (texture != nullptr)
	{
		textype = texture->getTextureType();
		handle = (GLuint) texture->getHandle();
	}
	else
	{
		if (textureunit == 0 && Shader::current != nullptr)
		{
			TextureType shadertex = Shader::current->getMainTextureType();
			if (shadertex != TEXTURE_MAX_ENUM)
				textype = shadertex;
		}

		handle = getDefaultTexture(textype);
	}

	bindTextureToUnit(textype, handle, textureunit, restoreprev, bindforedit);
}

void OpenGL::deleteTexture(GLuint texture)
{
	// glDeleteTextures binds texture 0 to all texture units the deleted texture
	// was bound to before deletion.
	for (int i = 0; i < TEXTURE_MAX_ENUM; i++)
	{
		for (GLuint &texid : state.boundTextures[i])
		{
			if (texid == texture)
				texid = 0;
		}
	}

	glDeleteTextures(1, &texture);
}

void OpenGL::setTextureFilter(TextureType target, graphics::Texture::Filter &f)
{
	GLint gmin = f.min == Texture::FILTER_NEAREST ? GL_NEAREST : GL_LINEAR;
	GLint gmag = f.mag == Texture::FILTER_NEAREST ? GL_NEAREST : GL_LINEAR;

	if (f.mipmap != Texture::FILTER_NONE)
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

	GLenum gltarget = getGLTextureType(target);

	glTexParameteri(gltarget, GL_TEXTURE_MIN_FILTER, gmin);
	glTexParameteri(gltarget, GL_TEXTURE_MAG_FILTER, gmag);

	if (GLAD_EXT_texture_filter_anisotropic)
	{
		f.anisotropy = std::min(std::max(f.anisotropy, 1.0f), maxAnisotropy);
		glTexParameterf(gltarget, GL_TEXTURE_MAX_ANISOTROPY_EXT, f.anisotropy);
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

GLint OpenGL::getGLCompareMode(CompareMode mode)
{
	switch (mode)
	{
	case COMPARE_LESS:
		return GL_LESS;
	case COMPARE_LEQUAL:
		return GL_LEQUAL;
	case COMPARE_EQUAL:
		return GL_EQUAL;
	case COMPARE_GEQUAL:
		return GL_GEQUAL;
	case COMPARE_GREATER:
		return GL_GREATER;
	case COMPARE_NOTEQUAL:
		return GL_NOTEQUAL;
	case COMPARE_ALWAYS:
		return GL_ALWAYS;
	case COMPARE_NEVER:
		return GL_NEVER;
	default:
		return GL_NEVER;
	}
}

void OpenGL::setTextureWrap(TextureType target, const graphics::Texture::Wrap &w)
{
	glTexParameteri(getGLTextureType(target), GL_TEXTURE_WRAP_S, getGLWrapMode(w.s));
	glTexParameteri(getGLTextureType(target), GL_TEXTURE_WRAP_T, getGLWrapMode(w.t));

	if (target == TEXTURE_VOLUME)
		glTexParameteri(getGLTextureType(target), GL_TEXTURE_WRAP_R, getGLWrapMode(w.r));
}

bool OpenGL::rawTexStorage(TextureType target, int levels, PixelFormat pixelformat, bool &isSRGB, int width, int height, int depth)
{
	GLenum gltarget = getGLTextureType(target);
	TextureFormat fmt = convertPixelFormat(pixelformat, false, isSRGB);

	if (fmt.swizzled)
	{
		glTexParameteri(gltarget, GL_TEXTURE_SWIZZLE_R, fmt.swizzle[0]);
		glTexParameteri(gltarget, GL_TEXTURE_SWIZZLE_G, fmt.swizzle[1]);
		glTexParameteri(gltarget, GL_TEXTURE_SWIZZLE_B, fmt.swizzle[2]);
		glTexParameteri(gltarget, GL_TEXTURE_SWIZZLE_A, fmt.swizzle[3]);
	}

	if (isTexStorageSupported())
	{
		if (target == TEXTURE_2D || target == TEXTURE_CUBE)
			glTexStorage2D(gltarget, levels, fmt.internalformat, width, height);
		else if (target == TEXTURE_VOLUME || target == TEXTURE_2D_ARRAY)
			glTexStorage3D(gltarget, levels, fmt.internalformat, width, height, depth);
	}
	else
	{
		int w = width;
		int h = height;
		int d = depth;

		for (int level = 0; level < levels; level++)
		{
			if (target == TEXTURE_2D || target == TEXTURE_CUBE)
			{
				int faces = target == TEXTURE_CUBE ? 6 : 1;
				for (int face = 0; face < faces; face++)
				{
					if (target == TEXTURE_CUBE)
						gltarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face;

					glTexImage2D(gltarget, level, fmt.internalformat, w, h, 0,
					             fmt.externalformat, fmt.type, nullptr);
				}
			}
			else if (target == TEXTURE_2D_ARRAY || target == TEXTURE_VOLUME)
			{
				if (target == TEXTURE_VOLUME && GLAD_ES_VERSION_2_0 && GLAD_OES_texture_3D && !GLAD_ES_VERSION_3_0)
				{
					glTexImage3DOES(gltarget, level, fmt.internalformat, w, h,
					                d, 0, fmt.externalformat, fmt.type, nullptr);
				}
				else
				{
					glTexImage3D(gltarget, level, fmt.internalformat, w, h, d,
					             0, fmt.externalformat, fmt.type, nullptr);
				}
			}

			w = std::max(w / 2, 1);
			h = std::max(h / 2, 1);

			if (target == TEXTURE_VOLUME)
				d = std::max(d / 2, 1);
		}
	}

	return gltarget != GL_ZERO;
}

bool OpenGL::isTexStorageSupported()
{
	bool supportsTexStorage = GLAD_VERSION_4_2 || GLAD_ARB_texture_storage;

	// Apparently there are bugs with glTexStorage on some Android drivers. I'd
	// rather not find out the hard way, so we'll avoid it for now...
#ifndef LOVE_ANDROID
	if (GLAD_ES_VERSION_3_0)
		supportsTexStorage = true;
#endif

	if (gl.bugs.texStorageBreaksSubImage)
		supportsTexStorage = false;

	return supportsTexStorage;
}

bool OpenGL::isTextureTypeSupported(TextureType type) const
{
	switch (type)
	{
	case TEXTURE_2D:
		return true;
	case TEXTURE_VOLUME:
		return GLAD_VERSION_1_1 || GLAD_ES_VERSION_3_0 || GLAD_OES_texture_3D;
	case TEXTURE_2D_ARRAY:
		return GLAD_VERSION_3_0 || GLAD_ES_VERSION_3_0 || GLAD_EXT_texture_array;
	case TEXTURE_CUBE:
		return GLAD_VERSION_1_3 || GLAD_ES_VERSION_2_0;
	default:
		return false;
	}
}

bool OpenGL::isClampZeroTextureWrapSupported() const
{
	return GLAD_VERSION_1_3 || GLAD_EXT_texture_border_clamp || GLAD_NV_texture_border_clamp;
}

bool OpenGL::isPixelShaderHighpSupported() const
{
	return pixelShaderHighpSupported;
}

bool OpenGL::isInstancingSupported() const
{
	return GLAD_ES_VERSION_3_0 || GLAD_VERSION_3_3
		|| GLAD_ARB_instanced_arrays || GLAD_EXT_instanced_arrays || GLAD_ANGLE_instanced_arrays;
}

bool OpenGL::isDepthCompareSampleSupported() const
{
	// Our official API only supports this in GLSL3 shaders, but unofficially
	// the requirements are more lax.
	return GLAD_VERSION_2_0 || GLAD_ES_VERSION_3_0 || GLAD_EXT_shadow_samplers;
}

bool OpenGL::isSamplerLODBiasSupported() const
{
	return GLAD_VERSION_1_4;
}

bool OpenGL::isBaseVertexSupported() const
{
	return baseVertexSupported;
}

int OpenGL::getMax2DTextureSize() const
{
	return std::max(max2DTextureSize, 1);
}

int OpenGL::getMax3DTextureSize() const
{
	return std::max(max3DTextureSize, 1);
}

int OpenGL::getMaxCubeTextureSize() const
{
	return std::max(maxCubeTextureSize, 1);
}

int OpenGL::getMaxTextureLayers() const
{
	return std::max(maxTextureArrayLayers, 1);
}

int OpenGL::getMaxRenderTargets() const
{
	return std::min(maxRenderTargets, MAX_COLOR_RENDER_TARGETS);
}

int OpenGL::getMaxRenderbufferSamples() const
{
	return maxRenderbufferSamples;
}

int OpenGL::getMaxTextureUnits() const
{
	return maxTextureUnits;
}

float OpenGL::getMaxPointSize() const
{
	return maxPointSize;
}

float OpenGL::getMaxAnisotropy() const
{
	return maxAnisotropy;
}

float OpenGL::getMaxLODBias() const
{
	return maxLODBias;
}

bool OpenGL::isCoreProfile() const
{
	return coreProfile;
}

OpenGL::Vendor OpenGL::getVendor() const
{
	return vendor;
}

OpenGL::TextureFormat OpenGL::convertPixelFormat(PixelFormat pixelformat, bool renderbuffer, bool &isSRGB)
{
	TextureFormat f;

	f.framebufferAttachments[0] = GL_COLOR_ATTACHMENT0;
	f.framebufferAttachments[1] = GL_NONE;

	if (pixelformat == PIXELFORMAT_RGBA8 && isSRGB)
		pixelformat = PIXELFORMAT_sRGBA8;
	else if (pixelformat == PIXELFORMAT_ETC1)
	{
		// The ETC2 format can load ETC1 textures.
		if (GLAD_ES_VERSION_3_0 || GLAD_VERSION_4_3 || GLAD_ARB_ES3_compatibility)
			pixelformat = PIXELFORMAT_ETC2_RGB;
	}

	switch (pixelformat)
	{
	case PIXELFORMAT_R8:
		if ((GLAD_VERSION_3_0 || GLAD_ES_VERSION_3_0 || GLAD_ARB_texture_rg || GLAD_EXT_texture_rg)
			&& !gl.bugs.brokenR8PixelFormat)
		{
			f.internalformat = GL_R8;
			f.externalformat = GL_RED;
		}
		else
		{
			f.internalformat = GL_LUMINANCE8;
			f.externalformat = GL_LUMINANCE;
		}
		f.type = GL_UNSIGNED_BYTE;
		break;
	case PIXELFORMAT_RG8:
		f.internalformat = GL_RG8;
		f.externalformat = GL_RG;
		f.type = GL_UNSIGNED_BYTE;
		break;
	case PIXELFORMAT_RGBA8:
		f.internalformat = GL_RGBA8;
		f.externalformat = GL_RGBA;
		f.type = GL_UNSIGNED_BYTE;
		break;
	case PIXELFORMAT_sRGBA8:
		f.internalformat = GL_SRGB8_ALPHA8;
		f.type = GL_UNSIGNED_BYTE;
		if (GLAD_ES_VERSION_2_0 && !GLAD_ES_VERSION_3_0)
			f.externalformat = GL_SRGB_ALPHA;
		else
			f.externalformat = GL_RGBA;
		break;
	case PIXELFORMAT_R16:
		f.internalformat = GL_R16;
		f.externalformat = GL_RED;
		f.type = GL_UNSIGNED_SHORT;
		break;
	case PIXELFORMAT_RG16:
		f.internalformat = GL_RG16;
		f.externalformat = GL_RG;
		f.type = GL_UNSIGNED_SHORT;
		break;
	case PIXELFORMAT_RGBA16:
		f.internalformat = GL_RGBA16;
		f.externalformat = GL_RGBA;
		f.type = GL_UNSIGNED_SHORT;
		break;
	case PIXELFORMAT_R16F:
		f.internalformat = GL_R16F;
		f.externalformat = GL_RED;
		if (GLAD_OES_texture_half_float)
			f.type = GL_HALF_FLOAT_OES;
		else
			f.type = GL_HALF_FLOAT;
		break;
	case PIXELFORMAT_RG16F:
		f.internalformat = GL_RG16F;
		f.externalformat = GL_RG;
		if (GLAD_OES_texture_half_float)
			f.type = GL_HALF_FLOAT_OES;
		else
			f.type = GL_HALF_FLOAT;
		break;
	case PIXELFORMAT_RGBA16F:
		f.internalformat = GL_RGBA16F;
		f.externalformat = GL_RGBA;
		if (GLAD_OES_texture_half_float)
			f.type = GL_HALF_FLOAT_OES;
		else
			f.type = GL_HALF_FLOAT;
		break;
	case PIXELFORMAT_R32F:
		f.internalformat = GL_R32F;
		f.externalformat = GL_RED;
		f.type = GL_FLOAT;
		break;
	case PIXELFORMAT_RG32F:
		f.internalformat = GL_RG32F;
		f.externalformat = GL_RG;
		f.type = GL_FLOAT;
		break;
	case PIXELFORMAT_RGBA32F:
		f.internalformat = GL_RGBA32F;
		f.externalformat = GL_RGBA;
		f.type = GL_FLOAT;
		break;

	case PIXELFORMAT_LA8:
		if (gl.isCoreProfile() || GLAD_ES_VERSION_3_0)
		{
			f.internalformat = GL_RG8;
			f.externalformat = GL_RG;
			f.type = GL_UNSIGNED_BYTE;
			f.swizzled = true;
			f.swizzle[0] = f.swizzle[1] = f.swizzle[2] = GL_RED;
			f.swizzle[3] = GL_GREEN;
		}
		else
		{
			f.internalformat = GL_LUMINANCE8_ALPHA8;
			f.externalformat = GL_LUMINANCE_ALPHA;
			f.type = GL_UNSIGNED_BYTE;
		}
		break;

	case PIXELFORMAT_RGBA4:
		f.internalformat = GL_RGBA4;
		f.externalformat = GL_RGBA;
		f.type = GL_UNSIGNED_SHORT_4_4_4_4;
		break;
	case PIXELFORMAT_RGB5A1:
		f.internalformat = GL_RGB5_A1;
		f.externalformat = GL_RGBA;
		f.type = GL_UNSIGNED_SHORT_5_5_5_1;
		break;
	case PIXELFORMAT_RGB565:
		f.internalformat = GL_RGB565;
		f.externalformat = GL_RGB;
		f.type = GL_UNSIGNED_SHORT_5_6_5;
		break;
	case PIXELFORMAT_RGB10A2:
		f.internalformat = GL_RGB10_A2;
		f.externalformat = GL_RGBA;
		f.type = GL_UNSIGNED_INT_2_10_10_10_REV;
		break;
	case PIXELFORMAT_RG11B10F:
		f.internalformat = GL_R11F_G11F_B10F;
		f.externalformat = GL_RGB;
		f.type = GL_UNSIGNED_INT_10F_11F_11F_REV;
		break;

	case PIXELFORMAT_STENCIL8:
		// Prefer a combined depth/stencil buffer due to driver issues.
		if (GLAD_ES_VERSION_3_0 || GLAD_VERSION_3_0 || GLAD_ARB_framebuffer_object)
		{
			f.internalformat = GL_DEPTH24_STENCIL8;
			f.externalformat = GL_DEPTH_STENCIL;
			f.type = GL_UNSIGNED_INT_24_8;
			f.framebufferAttachments[0] = GL_DEPTH_STENCIL_ATTACHMENT;
		}
		else if (GLAD_EXT_packed_depth_stencil || GLAD_OES_packed_depth_stencil)
		{
			f.internalformat = GL_DEPTH24_STENCIL8;
			f.externalformat = GL_DEPTH_STENCIL;
			f.type = GL_UNSIGNED_INT_24_8;
			f.framebufferAttachments[0] = GL_DEPTH_ATTACHMENT;
			f.framebufferAttachments[1] = GL_STENCIL_ATTACHMENT;
		}
		else
		{
			f.internalformat = GL_STENCIL_INDEX8;
			f.externalformat = GL_STENCIL;
			f.type = GL_UNSIGNED_BYTE;
			f.framebufferAttachments[0] = GL_STENCIL_ATTACHMENT;
		}
		break;

	case PIXELFORMAT_DEPTH16:
		f.internalformat = GL_DEPTH_COMPONENT16;
		f.externalformat = GL_DEPTH_COMPONENT;
		f.type = GL_UNSIGNED_SHORT;
		f.framebufferAttachments[0] = GL_DEPTH_ATTACHMENT;
		break;

	case PIXELFORMAT_DEPTH24:
		if (GLAD_ES_VERSION_2_0 && !GLAD_ES_VERSION_3_0 && !GLAD_OES_depth24 && GLAD_OES_packed_depth_stencil)
		{
			f.internalformat = GL_DEPTH24_STENCIL8;
			f.externalformat = GL_DEPTH_STENCIL;
			f.type = GL_UNSIGNED_INT_24_8;
			f.framebufferAttachments[0] = GL_DEPTH_ATTACHMENT;
			f.framebufferAttachments[1] = GL_STENCIL_ATTACHMENT;
		}
		else
		{
			f.internalformat = GL_DEPTH_COMPONENT24;
			f.externalformat = GL_DEPTH_COMPONENT;
			f.type = GL_UNSIGNED_INT;
			f.framebufferAttachments[0] = GL_DEPTH_ATTACHMENT;
		}
		break;

	case PIXELFORMAT_DEPTH32F:
		f.internalformat = GL_DEPTH_COMPONENT32F;
		f.externalformat = GL_DEPTH_COMPONENT;
		f.type = GL_FLOAT;
		f.framebufferAttachments[0] = GL_DEPTH_ATTACHMENT;
		break;

	case PIXELFORMAT_DEPTH24_STENCIL8:
		f.internalformat = GL_DEPTH24_STENCIL8;
		f.externalformat = GL_DEPTH_STENCIL;
		f.type = GL_UNSIGNED_INT_24_8;
		if (GLAD_ES_VERSION_3_0 || GLAD_VERSION_3_0 || GLAD_ARB_framebuffer_object)
		{
			f.framebufferAttachments[0] = GL_DEPTH_STENCIL_ATTACHMENT;
		}
		else if (GLAD_EXT_packed_depth_stencil || GLAD_OES_packed_depth_stencil)
		{
			f.framebufferAttachments[0] = GL_DEPTH_ATTACHMENT;
			f.framebufferAttachments[1] = GL_STENCIL_ATTACHMENT;
		}
		break;

	case PIXELFORMAT_DEPTH32F_STENCIL8:
		f.internalformat = GL_DEPTH32F_STENCIL8;
		f.externalformat = GL_DEPTH_STENCIL;
		f.type = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
		f.framebufferAttachments[0] = GL_DEPTH_STENCIL_ATTACHMENT;
		break;

	case PIXELFORMAT_DXT1:
		f.internalformat = isSRGB ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT : GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		break;
	case PIXELFORMAT_DXT3:
		f.internalformat = isSRGB ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT : GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		break;
	case PIXELFORMAT_DXT5:
		f.internalformat = isSRGB ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT : GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		break;
	case PIXELFORMAT_BC4:
		isSRGB = false;
		f.internalformat = GL_COMPRESSED_RED_RGTC1;
		break;
	case PIXELFORMAT_BC4s:
		isSRGB = false;
		f.internalformat = GL_COMPRESSED_SIGNED_RED_RGTC1;
		break;
	case PIXELFORMAT_BC5:
		isSRGB = false;
		f.internalformat = GL_COMPRESSED_RG_RGTC2;
		break;
	case PIXELFORMAT_BC5s:
		isSRGB = false;
		f.internalformat = GL_COMPRESSED_SIGNED_RG_RGTC2;
		break;
	case PIXELFORMAT_BC6H:
		isSRGB = false;
		f.internalformat = GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT;
		break;
	case PIXELFORMAT_BC6Hs:
		isSRGB = false;
		f.internalformat = GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT;
		break;
	case PIXELFORMAT_BC7:
		f.internalformat = isSRGB ? GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM : GL_COMPRESSED_RGBA_BPTC_UNORM;
		break;
	case PIXELFORMAT_PVR1_RGB2:
		f.internalformat = isSRGB ? GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT : GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
		break;
	case PIXELFORMAT_PVR1_RGB4:
		f.internalformat = isSRGB ? GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT : GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
		break;
	case PIXELFORMAT_PVR1_RGBA2:
		f.internalformat = isSRGB ? GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT : GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
		break;
	case PIXELFORMAT_PVR1_RGBA4:
		f.internalformat = isSRGB ? GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT : GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
		break;
	case PIXELFORMAT_ETC1:
		isSRGB = false;
		f.internalformat = GL_ETC1_RGB8_OES;
		break;
	case PIXELFORMAT_ETC2_RGB:
		f.internalformat = isSRGB ? GL_COMPRESSED_SRGB8_ETC2 : GL_COMPRESSED_RGB8_ETC2;
		break;
	case PIXELFORMAT_ETC2_RGBA:
		f.internalformat = isSRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC : GL_COMPRESSED_RGBA8_ETC2_EAC;
		break;
	case PIXELFORMAT_ETC2_RGBA1:
		f.internalformat = isSRGB ? GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 : GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
		break;
	case PIXELFORMAT_EAC_R:
		isSRGB = false;
		f.internalformat = GL_COMPRESSED_R11_EAC;
		break;
	case PIXELFORMAT_EAC_Rs:
		isSRGB = false;
		f.internalformat = GL_COMPRESSED_SIGNED_R11_EAC;
		break;
	case PIXELFORMAT_EAC_RG:
		isSRGB = false;
		f.internalformat = GL_COMPRESSED_RG11_EAC;
		break;
	case PIXELFORMAT_EAC_RGs:
		isSRGB = false;
		f.internalformat = GL_COMPRESSED_SIGNED_RG11_EAC;
		break;
	case PIXELFORMAT_ASTC_4x4:
		f.internalformat = isSRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR : GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
		break;
	case PIXELFORMAT_ASTC_5x4:
		f.internalformat = isSRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR : GL_COMPRESSED_RGBA_ASTC_5x4_KHR;
		break;
	case PIXELFORMAT_ASTC_5x5:
		f.internalformat = isSRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR : GL_COMPRESSED_RGBA_ASTC_5x5_KHR;
		break;
	case PIXELFORMAT_ASTC_6x5:
		f.internalformat = isSRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR : GL_COMPRESSED_RGBA_ASTC_6x5_KHR;
		break;
	case PIXELFORMAT_ASTC_6x6:
		f.internalformat = isSRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR : GL_COMPRESSED_RGBA_ASTC_6x6_KHR;
		break;
	case PIXELFORMAT_ASTC_8x5:
		f.internalformat = isSRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR : GL_COMPRESSED_RGBA_ASTC_8x5_KHR;
		break;
	case PIXELFORMAT_ASTC_8x6:
		f.internalformat = isSRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR : GL_COMPRESSED_RGBA_ASTC_8x6_KHR;
		break;
	case PIXELFORMAT_ASTC_8x8:
		f.internalformat = isSRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR : GL_COMPRESSED_RGBA_ASTC_8x8_KHR;
		break;
	case PIXELFORMAT_ASTC_10x5:
		f.internalformat = isSRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR : GL_COMPRESSED_RGBA_ASTC_10x5_KHR;
		break;
	case PIXELFORMAT_ASTC_10x6:
		f.internalformat = isSRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR : GL_COMPRESSED_RGBA_ASTC_10x6_KHR;
		break;
	case PIXELFORMAT_ASTC_10x8:
		f.internalformat = isSRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR : GL_COMPRESSED_RGBA_ASTC_10x8_KHR;
		break;
	case PIXELFORMAT_ASTC_10x10:
		f.internalformat = isSRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR : GL_COMPRESSED_RGBA_ASTC_10x10_KHR;
		break;
	case PIXELFORMAT_ASTC_12x10:
		f.internalformat = isSRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR : GL_COMPRESSED_RGBA_ASTC_12x10_KHR;
		break;
	case PIXELFORMAT_ASTC_12x12:
		f.internalformat = isSRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR : GL_COMPRESSED_RGBA_ASTC_12x12_KHR;
		break;

	default:
		printf("Unhandled pixel format %d when converting to OpenGL enums!", pixelformat);
		break;
	}

	if (!isPixelFormatCompressed(pixelformat))
	{
		// glTexImage in OpenGL ES 2 only accepts internal format enums that
		// match the external format. GLES3 doesn't have that restriction - 
		// except for GL_LUMINANCE_ALPHA which doesn't have a sized version in
		// ES3. However we always use RG8 for PIXELFORMAT_LA8 on GLES3 so it
		// doesn't matter there.
		// Also note that GLES2+extension sRGB format enums are different from
		// desktop GL and GLES3+ (this is handled above).
		if (GLAD_ES_VERSION_2_0 && !GLAD_ES_VERSION_3_0
			&& !renderbuffer && !isTexStorageSupported())
		{
			f.internalformat = f.externalformat;
		}

		if (pixelformat != PIXELFORMAT_sRGBA8)
			isSRGB = false;
	}

	return f;
}

bool OpenGL::isPixelFormatSupported(PixelFormat pixelformat, bool rendertarget, bool readable, bool isSRGB)
{
	if (rendertarget && isPixelFormatCompressed(pixelformat))
		return false;

	if (pixelformat == PIXELFORMAT_RGBA8 && isSRGB)
		pixelformat = PIXELFORMAT_sRGBA8;

	switch (pixelformat)
	{
	case PIXELFORMAT_R8:
	case PIXELFORMAT_RG8:
		if (GLAD_VERSION_3_0 || GLAD_ES_VERSION_3_0 || GLAD_ARB_texture_rg || GLAD_EXT_texture_rg)
			return true;
		else if (pixelformat == PIXELFORMAT_R8 && !rendertarget && (GLAD_ES_VERSION_2_0 || GLAD_VERSION_1_1))
			return true; // We'll use OpenGL's luminance format internally.
		else
			return false;
	case PIXELFORMAT_RGBA8:
		if (rendertarget)
			return GLAD_VERSION_1_0 || GLAD_ES_VERSION_3_0 || GLAD_OES_rgb8_rgba8 || GLAD_ARM_rgba8;
		else
			return true;
	case PIXELFORMAT_sRGBA8:
		if (gl.bugs.brokenSRGB)
			return false;
		if (rendertarget)
		{
			if (GLAD_VERSION_1_0)
			{
				return GLAD_VERSION_3_0 || ((GLAD_ARB_framebuffer_sRGB || GLAD_EXT_framebuffer_sRGB)
					   && (GLAD_VERSION_2_1 || GLAD_EXT_texture_sRGB));
			}
			else
				return GLAD_ES_VERSION_3_0 || GLAD_EXT_sRGB;
		}
		else
			return GLAD_ES_VERSION_3_0 || GLAD_EXT_sRGB || GLAD_VERSION_2_1 || GLAD_EXT_texture_sRGB;
	case PIXELFORMAT_R16:
	case PIXELFORMAT_RG16:
		return GLAD_VERSION_3_0
			|| (GLAD_VERSION_1_1 && GLAD_ARB_texture_rg)
			|| (GLAD_EXT_texture_norm16 && (GLAD_ES_VERSION_3_0 || GLAD_EXT_texture_rg));
	case PIXELFORMAT_RGBA16:
		return GLAD_VERSION_1_1 || GLAD_EXT_texture_norm16;
	case PIXELFORMAT_R16F:
	case PIXELFORMAT_RG16F:
		if (GLAD_VERSION_1_0)
			return GLAD_VERSION_3_0 || (GLAD_ARB_texture_float && GLAD_ARB_half_float_pixel && GLAD_ARB_texture_rg);
		else if (rendertarget && !GLAD_EXT_color_buffer_half_float)
			return false;
		else
			return GLAD_ES_VERSION_3_0 || (GLAD_OES_texture_half_float && GLAD_EXT_texture_rg);
	case PIXELFORMAT_RGBA16F:
		if (GLAD_VERSION_1_0)
			return GLAD_VERSION_3_0 || (GLAD_ARB_texture_float && GLAD_ARB_half_float_pixel);
		else if (rendertarget && !GLAD_EXT_color_buffer_half_float)
			return false;
		else
			return GLAD_ES_VERSION_3_0 || GLAD_OES_texture_half_float;
	case PIXELFORMAT_R32F:
	case PIXELFORMAT_RG32F:
		if (GLAD_VERSION_1_0)
			return GLAD_VERSION_3_0 || (GLAD_ARB_texture_float && GLAD_ARB_texture_rg);
		else if (!rendertarget)
			return GLAD_ES_VERSION_3_0 || (GLAD_OES_texture_float && GLAD_EXT_texture_rg);
		else
			return false;
	case PIXELFORMAT_RGBA32F:
		if (GLAD_VERSION_1_0)
			return GLAD_VERSION_3_0 || GLAD_ARB_texture_float;
		else if (!rendertarget)
			return GLAD_ES_VERSION_3_0 || GLAD_OES_texture_float;
		else
			return false;

	case PIXELFORMAT_LA8:
		return !rendertarget;

	case PIXELFORMAT_RGBA4:
	case PIXELFORMAT_RGB5A1:
		return true;
	case PIXELFORMAT_RGB565:
		return GLAD_ES_VERSION_2_0 || GLAD_VERSION_4_2 || GLAD_ARB_ES2_compatibility;
	case PIXELFORMAT_RGB10A2:
		return GLAD_ES_VERSION_3_0 || GLAD_VERSION_1_0;
	case PIXELFORMAT_RG11B10F:
		if (rendertarget)
			return GLAD_VERSION_3_0 || GLAD_EXT_packed_float || GLAD_APPLE_color_buffer_packed_float;
		else
			return GLAD_VERSION_3_0 || GLAD_EXT_packed_float || GLAD_APPLE_texture_packed_float;

	case PIXELFORMAT_STENCIL8:
		return rendertarget && !readable;

	case PIXELFORMAT_DEPTH16:
		if (!rendertarget)
			return false;
		else if (readable)
			return GLAD_VERSION_2_0 || GLAD_ES_VERSION_3_0 || GLAD_OES_depth_texture;
		else
			return true;

	case PIXELFORMAT_DEPTH24:
		if (!rendertarget)
			return false;
		else if (readable)
			return GLAD_VERSION_2_0 || GLAD_ES_VERSION_3_0 || (GLAD_OES_depth_texture && (GLAD_OES_depth24 || GLAD_OES_depth_texture));
		else
			return GLAD_VERSION_2_0 || GLAD_ES_VERSION_3_0 || GLAD_OES_depth24 || GLAD_OES_depth_texture;

	case PIXELFORMAT_DEPTH24_STENCIL8:
		if (!rendertarget)
			return false;
		else if (readable)
			return GLAD_VERSION_3_0 || GLAD_ES_VERSION_3_0 || GLAD_EXT_packed_depth_stencil || (GLAD_OES_depth_texture && GLAD_OES_packed_depth_stencil);
		else
			return GLAD_VERSION_3_0 || GLAD_ES_VERSION_3_0 || GLAD_EXT_packed_depth_stencil || GLAD_OES_packed_depth_stencil;

	case PIXELFORMAT_DEPTH32F:
	case PIXELFORMAT_DEPTH32F_STENCIL8:
		return rendertarget && (GLAD_VERSION_3_0 || GLAD_ES_VERSION_3_0 || GLAD_ARB_depth_buffer_float);

	case PIXELFORMAT_DXT1:
		return GLAD_EXT_texture_compression_s3tc || GLAD_EXT_texture_compression_dxt1;
	case PIXELFORMAT_DXT3:
		return GLAD_EXT_texture_compression_s3tc || GLAD_ANGLE_texture_compression_dxt3;
	case PIXELFORMAT_DXT5:
		return GLAD_EXT_texture_compression_s3tc || GLAD_ANGLE_texture_compression_dxt5;
	case PIXELFORMAT_BC4:
	case PIXELFORMAT_BC4s:
	case PIXELFORMAT_BC5:
	case PIXELFORMAT_BC5s:
		return (GLAD_VERSION_3_0 || GLAD_ARB_texture_compression_rgtc || GLAD_EXT_texture_compression_rgtc);
	case PIXELFORMAT_BC6H:
	case PIXELFORMAT_BC6Hs:
	case PIXELFORMAT_BC7:
		return GLAD_VERSION_4_2 || GLAD_ARB_texture_compression_bptc;
	case PIXELFORMAT_PVR1_RGB2:
	case PIXELFORMAT_PVR1_RGB4:
	case PIXELFORMAT_PVR1_RGBA2:
	case PIXELFORMAT_PVR1_RGBA4:
		return isSRGB ? GLAD_EXT_pvrtc_sRGB : GLAD_IMG_texture_compression_pvrtc;
	case PIXELFORMAT_ETC1:
		// ETC2 support guarantees ETC1 support as well.
		return GLAD_ES_VERSION_3_0 || GLAD_VERSION_4_3 || GLAD_ARB_ES3_compatibility || GLAD_OES_compressed_ETC1_RGB8_texture;
	case PIXELFORMAT_ETC2_RGB:
	case PIXELFORMAT_ETC2_RGBA:
	case PIXELFORMAT_ETC2_RGBA1:
	case PIXELFORMAT_EAC_R:
	case PIXELFORMAT_EAC_Rs:
	case PIXELFORMAT_EAC_RG:
	case PIXELFORMAT_EAC_RGs:
		return GLAD_ES_VERSION_3_0 || GLAD_VERSION_4_3 || GLAD_ARB_ES3_compatibility;
	case PIXELFORMAT_ASTC_4x4:
	case PIXELFORMAT_ASTC_5x4:
	case PIXELFORMAT_ASTC_5x5:
	case PIXELFORMAT_ASTC_6x5:
	case PIXELFORMAT_ASTC_6x6:
	case PIXELFORMAT_ASTC_8x5:
	case PIXELFORMAT_ASTC_8x6:
	case PIXELFORMAT_ASTC_8x8:
	case PIXELFORMAT_ASTC_10x5:
	case PIXELFORMAT_ASTC_10x6:
	case PIXELFORMAT_ASTC_10x8:
	case PIXELFORMAT_ASTC_10x10:
	case PIXELFORMAT_ASTC_12x10:
	case PIXELFORMAT_ASTC_12x12:
		return GLAD_ES_VERSION_3_2 || GLAD_KHR_texture_compression_astc_ldr;

	default:
		return false;
	}
}

bool OpenGL::hasTextureFilteringSupport(PixelFormat pixelformat)
{
	switch (pixelformat)
	{
	case PIXELFORMAT_R16F:
	case PIXELFORMAT_RG16F:
	case PIXELFORMAT_RGBA16F:
		return GLAD_VERSION_1_1 || GLAD_ES_VERSION_3_0 || GLAD_OES_texture_half_float_linear;
	case PIXELFORMAT_R32F:
	case PIXELFORMAT_RG32F:
	case PIXELFORMAT_RGBA32F:
		return GLAD_VERSION_1_1 || GLAD_OES_texture_float_linear;
	default:
		return true;
	}
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

const char *OpenGL::framebufferStatusString(GLenum status)
{
	switch (status)
	{
	case GL_FRAMEBUFFER_COMPLETE:
		return "complete (success)";
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		return "Texture format cannot be rendered to on this system.";
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		return "Error in graphics driver (missing render texture attachment)";
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		return "Error in graphics driver (incomplete draw buffer)";
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		return "Error in graphics driver (incomplete read buffer)";
	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		return "Canvas with the specified MSAA count cannot be rendered to on this system.";
	case GL_FRAMEBUFFER_UNSUPPORTED:
		return "Renderable textures are unsupported";
	default:
		break;
	}

	static char text[64] = {};

	memset(text, 0, sizeof(text));
	sprintf(text, "0x%x", status);

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
