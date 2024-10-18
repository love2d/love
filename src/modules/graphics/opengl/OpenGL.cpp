/**
 * Copyright (c) 2006-2024 LOVE Development Team
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

#include "graphics/Graphics.h"
#include "graphics/Buffer.h"

// C++
#include <algorithm>
#include <limits>

// C
#include <cstring>
#include <cstdio>

// For SDL_GL_GetProcAddress.
#include <SDL3/SDL_video.h>

#ifdef LOVE_IOS
#include <SDL3/SDL_video.h>
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

	return (void *) SDL_GL_GetProcAddress(name);
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

OpenGL::CleanClearState::CleanClearState(GLbitfield clearFlags)
	: clearFlags(clearFlags)
	, colorWriteMask(gl.getColorWriteMask())
	, stencilWriteMask(gl.getStencilWriteMask())
	, depthWrites(gl.hasDepthWrites())
	, scissor(gl.isStateEnabled(ENABLE_SCISSOR_TEST))
{
	if ((clearFlags & GL_COLOR_BUFFER_BIT) != 0 && colorWriteMask != LOVE_UINT32_MAX)
		gl.setColorWriteMask(LOVE_UINT32_MAX);

	if ((clearFlags & GL_DEPTH_BUFFER_BIT) != 0 && !depthWrites)
		gl.setDepthWrites(true);

	if ((clearFlags & GL_STENCIL_BUFFER_BIT) != 0 && (stencilWriteMask & 0xFF) != 0xFF)
		gl.setStencilWriteMask(LOVE_UINT32_MAX);

	if (clearFlags != 0 && scissor)
		gl.setEnableState(ENABLE_SCISSOR_TEST, false);
}

OpenGL::CleanClearState::~CleanClearState()
{
	if ((clearFlags & GL_COLOR_BUFFER_BIT) != 0 && colorWriteMask != LOVE_UINT32_MAX)
		gl.setColorWriteMask(colorWriteMask);

	if ((clearFlags & GL_DEPTH_BUFFER_BIT) != 0 && !depthWrites)
		gl.setDepthWrites(depthWrites);

	if ((clearFlags & GL_STENCIL_BUFFER_BIT) != 0 && (stencilWriteMask & 0xFF) != 0xFF)
		gl.setStencilWriteMask(stencilWriteMask);

	if (clearFlags != 0 && scissor)
		gl.setEnableState(ENABLE_SCISSOR_TEST, scissor);
}

OpenGL::OpenGL()
	: stats()
	, bugs()
	, contextInitialized(false)
	, baseVertexSupported(false)
	, maxAnisotropy(1.0f)
	, maxLODBias(0.0f)
	, max2DTextureSize(0)
	, max3DTextureSize(0)
	, maxCubeTextureSize(0)
	, maxTextureArrayLayers(0)
	, maxTexelBufferSize(0)
	, maxShaderStorageBufferSize(0)
	, maxComputeWorkGroupsX(0)
	, maxComputeWorkGroupsY(0)
	, maxComputeWorkGroupsZ(0)
	, maxRenderTargets(1)
	, maxSamples(1)
	, maxTextureUnits(1)
	, maxShaderStorageBufferBindings(0)
	, maxPointSize(1)
	, coreProfile(false)
	, vendor(VENDOR_UNKNOWN)
	, state()
{
}

bool OpenGL::initContext()
{
	if (contextInitialized)
		return true;

	if (!gladLoadGLLoader(LOVEGetProcAddress))
		return false;

	initVendor();

	bugs = {};

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

	GLint maxvertexattribs = 1;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxvertexattribs);

	state.enabledAttribArrays = (uint32) ((1ull << uint32(maxvertexattribs)) - 1);
	state.instancedAttribArrays = 0;

	setVertexAttributes(VertexAttributes(), BufferBindings());

	// Get the current viewport.
	glGetIntegerv(GL_VIEWPORT, (GLint *) &state.viewport.x);

	// And the current scissor - but we need to compensate for GL scissors
	// starting at the bottom left instead of top left.
	glGetIntegerv(GL_SCISSOR_BOX, (GLint *) &state.scissor.x);
	state.scissor.y = state.viewport.h - (state.scissor.y + state.scissor.h);

	for (int i = 0; i < 2; i++)
		state.boundFramebuffers[i] = std::numeric_limits<GLuint>::max();
	bindFramebuffer(FRAMEBUFFER_ALL, getDefaultFBO());

	setEnableState(ENABLE_BLEND, state.enableState[ENABLE_BLEND]);
	setEnableState(ENABLE_DEPTH_TEST, state.enableState[ENABLE_DEPTH_TEST]);
	setEnableState(ENABLE_STENCIL_TEST, state.enableState[ENABLE_STENCIL_TEST]);
	setEnableState(ENABLE_SCISSOR_TEST, state.enableState[ENABLE_SCISSOR_TEST]);
	setEnableState(ENABLE_FACE_CULL, state.enableState[ENABLE_FACE_CULL]);

	if (!bugs.brokenSRGB)
		setEnableState(ENABLE_FRAMEBUFFER_SRGB, state.enableState[ENABLE_FRAMEBUFFER_SRGB]);
	else
		state.enableState[ENABLE_FRAMEBUFFER_SRGB] = false;

	GLint faceCull = GL_BACK;
	glGetIntegerv(GL_CULL_FACE_MODE, &faceCull);
	state.faceCullMode = faceCull;

	for (int i = 0; i < (int) BUFFERUSAGE_MAX_ENUM; i++)
	{
		state.boundBuffers[i] = 0;
		if (isBufferUsageSupported((BufferUsage) i))
			glBindBuffer(getGLBufferType((BufferUsage) i), 0);
	}

	if (isBufferUsageSupported(BUFFERUSAGE_SHADER_STORAGE))
		state.boundIndexedBuffers[BUFFERUSAGE_SHADER_STORAGE].resize(maxShaderStorageBufferBindings, 0);

	// Initialize multiple texture unit support for shaders.
	for (int i = 0; i < TEXTURE_MAX_ENUM + 1; i++)
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
			glBindTexture(getGLTextureType(textype), 0);
		}
	}

	glActiveTexture(GL_TEXTURE0);
	state.curTextureUnit = 0;

	setDepthWrites(state.depthWritesEnabled);
	setStencilWriteMask(state.stencilWriteMask);
	setColorWriteMask(state.colorWriteMask);

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
	baseVertexSupported = GLAD_VERSION_3_2 || GLAD_ES_VERSION_3_2 || GLAD_ARB_draw_elements_base_vertex
		|| GLAD_OES_draw_elements_base_vertex || GLAD_EXT_draw_elements_base_vertex;

	// We'll need this value to clamp anisotropy.
	if (GLAD_EXT_texture_filter_anisotropic)
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
	else
		maxAnisotropy = 1.0f;

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max2DTextureSize);
	glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &maxCubeTextureSize);
	glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &max3DTextureSize);
	glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &maxTextureArrayLayers);

	if (isBufferUsageSupported(BUFFERUSAGE_TEXEL))
		glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &maxTexelBufferSize);
	else
		maxTexelBufferSize = 0;

	if (isBufferUsageSupported(BUFFERUSAGE_SHADER_STORAGE))
	{
		glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &maxShaderStorageBufferSize);
		glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &maxShaderStorageBufferBindings);
	}
	else
	{
		maxShaderStorageBufferSize = 0;
		maxShaderStorageBufferBindings = 0;
	}

	if (GLAD_ES_VERSION_3_1 || GLAD_VERSION_4_3)
	{
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxComputeWorkGroupsX);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &maxComputeWorkGroupsY);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &maxComputeWorkGroupsZ);
	}
	else
	{
		maxComputeWorkGroupsX = 0;
		maxComputeWorkGroupsY = 0;
		maxComputeWorkGroupsZ = 0;
	}

	int maxattachments = 1;
	int maxdrawbuffers = 1;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxattachments);
	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxdrawbuffers);

	maxRenderTargets = std::max(std::min(maxattachments, maxdrawbuffers), 1);

	glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);

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

void OpenGL::prepareDraw(love::graphics::Graphics *gfx)
{
	TempDebugGroup debuggroup("Prepare OpenGL draw");

	// Make sure the active shader's love-provided uniforms are up to date.
	if (Shader::current != nullptr)
	{
		Rect viewport = getViewport();
		((Shader *)Shader::current)->updateBuiltinUniforms(gfx, viewport.w, viewport.h);
	}
}

GLenum OpenGL::getGLPrimitiveType(PrimitiveType type)
{
	switch (type)
	{
		case PRIMITIVE_TRIANGLES: return GL_TRIANGLES;
		case PRIMITIVE_TRIANGLE_STRIP: return GL_TRIANGLE_STRIP;
		case PRIMITIVE_TRIANGLE_FAN: return GL_TRIANGLE_FAN;
		case PRIMITIVE_POINTS: return GL_POINTS;
		case PRIMITIVE_MAX_ENUM: return GL_ZERO;
	}

	return GL_ZERO;
}

GLenum OpenGL::getGLBufferType(BufferUsage usage)
{
	switch (usage)
	{
		case BUFFERUSAGE_VERTEX: return GL_ARRAY_BUFFER;
		case BUFFERUSAGE_INDEX: return GL_ELEMENT_ARRAY_BUFFER;
		case BUFFERUSAGE_TEXEL: return GL_TEXTURE_BUFFER;
		case BUFFERUSAGE_UNIFORM: return GL_UNIFORM_BUFFER;
		case BUFFERUSAGE_SHADER_STORAGE: return GL_SHADER_STORAGE_BUFFER;
		case BUFFERUSAGE_INDIRECT_ARGUMENTS: return GL_DRAW_INDIRECT_BUFFER;
		case BUFFERUSAGE_MAX_ENUM: return GL_ZERO;
	}

	return GL_ZERO;
}

GLenum OpenGL::getGLTextureType(TextureType type)
{
	switch (type)
	{
		case TEXTURE_2D: return GL_TEXTURE_2D;
		case TEXTURE_VOLUME: return GL_TEXTURE_3D;
		case TEXTURE_2D_ARRAY: return GL_TEXTURE_2D_ARRAY;
		case TEXTURE_CUBE: return GL_TEXTURE_CUBE_MAP;
		case TEXTURE_MAX_ENUM: return GL_TEXTURE_BUFFER; // Hack
	}

	return GL_ZERO;
}

GLenum OpenGL::getGLIndexDataType(IndexDataType type)
{
	switch (type)
	{
		case INDEX_UINT16: return GL_UNSIGNED_SHORT;
		case INDEX_UINT32: return GL_UNSIGNED_INT;
		default: return GL_ZERO;
	}
}

GLenum OpenGL::getGLVertexDataType(DataFormat format, int &components, GLboolean &normalized, bool &intformat)
{
	normalized = GL_FALSE;
	intformat = false;
	components = 1;

	switch (format)
	{
	case DATAFORMAT_FLOAT:
		components = 1;
		return GL_FLOAT;
	case DATAFORMAT_FLOAT_VEC2:
		components = 2;
		return GL_FLOAT;
	case DATAFORMAT_FLOAT_VEC3:
		components = 3;
		return GL_FLOAT;
	case DATAFORMAT_FLOAT_VEC4:
		components = 4;
		return GL_FLOAT;

	case DATAFORMAT_FLOAT_MAT2X2:
	case DATAFORMAT_FLOAT_MAT2X3:
	case DATAFORMAT_FLOAT_MAT2X4:
	case DATAFORMAT_FLOAT_MAT3X2:
	case DATAFORMAT_FLOAT_MAT3X3:
	case DATAFORMAT_FLOAT_MAT3X4:
	case DATAFORMAT_FLOAT_MAT4X2:
	case DATAFORMAT_FLOAT_MAT4X3:
	case DATAFORMAT_FLOAT_MAT4X4:
		return GL_ZERO;

	case DATAFORMAT_INT32:
		components = 1;
		intformat = true;
		return GL_INT;
	case DATAFORMAT_INT32_VEC2:
		components = 2;
		intformat = true;
		return GL_INT;
	case DATAFORMAT_INT32_VEC3:
		components = 3;
		intformat = true;
		return GL_INT;
	case DATAFORMAT_INT32_VEC4:
		components = 4;
		intformat = true;
		return GL_INT;

	case DATAFORMAT_UINT32:
		components = 1;
		intformat = true;
		return GL_UNSIGNED_INT;
	case DATAFORMAT_UINT32_VEC2:
		components = 2;
		intformat = true;
		return GL_UNSIGNED_INT;
	case DATAFORMAT_UINT32_VEC3:
		components = 3;
		intformat = true;
		return GL_UNSIGNED_INT;
	case DATAFORMAT_UINT32_VEC4:
		components = 4;
		intformat = true;
		return GL_UNSIGNED_INT;

	case DATAFORMAT_SNORM8_VEC4:
		components = 4;
		normalized = GL_TRUE;
		return GL_BYTE;

	case DATAFORMAT_UNORM8_VEC4:
		components = 4;
		normalized = GL_TRUE;
		return GL_UNSIGNED_BYTE;

	case DATAFORMAT_INT8_VEC4:
		components = 4;
		intformat = true;
		return GL_BYTE;

	case DATAFORMAT_UINT8_VEC4:
		components = 4;
		intformat = true;
		return GL_UNSIGNED_BYTE;

	case DATAFORMAT_SNORM16_VEC2:
		components = 2;
		normalized = GL_TRUE;
		return GL_BYTE;
	case DATAFORMAT_SNORM16_VEC4:
		components = 4;
		normalized = GL_TRUE;
		return GL_BYTE;

	case DATAFORMAT_UNORM16_VEC2:
		components = 2;
		normalized = GL_TRUE;
		return GL_UNSIGNED_SHORT;
	case DATAFORMAT_UNORM16_VEC4:
		components = 4;
		normalized = GL_TRUE;
		return GL_UNSIGNED_SHORT;

	case DATAFORMAT_INT16_VEC2:
		components = 2;
		intformat = true;
		return GL_SHORT;
	case DATAFORMAT_INT16_VEC4:
		components = 4;
		intformat = true;
		return GL_SHORT;

	case DATAFORMAT_UINT16:
		components = 1;
		intformat = true;
		return GL_UNSIGNED_SHORT;
	case DATAFORMAT_UINT16_VEC2:
		components = 2;
		intformat = true;
		return GL_UNSIGNED_SHORT;
	case DATAFORMAT_UINT16_VEC4:
		components = 4;
		intformat = true;
		return GL_UNSIGNED_SHORT;

	case DATAFORMAT_BOOL:
	case DATAFORMAT_BOOL_VEC2:
	case DATAFORMAT_BOOL_VEC3:
	case DATAFORMAT_BOOL_VEC4:
		return GL_ZERO;

	case DATAFORMAT_MAX_ENUM:
		return GL_ZERO;
	}

	return GL_ZERO;
}

GLenum OpenGL::getGLBufferDataUsage(BufferDataUsage usage)
{
	switch (usage)
	{
		case BUFFERDATAUSAGE_STREAM: return GL_STREAM_DRAW;
		case BUFFERDATAUSAGE_DYNAMIC: return GL_DYNAMIC_DRAW;
		case BUFFERDATAUSAGE_STATIC: return GL_STATIC_DRAW;
		case BUFFERDATAUSAGE_READBACK: return GL_STREAM_READ;
		default: return 0;
	}
}

void OpenGL::bindBuffer(BufferUsage type, GLuint buffer)
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

	for (int i = 0; i < (int) BUFFERUSAGE_MAX_ENUM; i++)
	{
		if (state.boundBuffers[i] == buffer)
			state.boundBuffers[i] = 0;

		for (GLuint &bufferid : state.boundIndexedBuffers[i])
		{
			if (bufferid == buffer)
				bufferid = 0;
		}
	}
}

void OpenGL::setVertexAttributes(const VertexAttributes &attributes, const BufferBindings &buffers)
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

			int components = 0;
			GLboolean normalized = GL_FALSE;
			bool intformat = false;
			GLenum gltype = getGLVertexDataType(attrib.getFormat(), components, normalized, intformat);

			const void *offsetpointer = reinterpret_cast<void*>(bufferinfo.offset + attrib.offsetFromVertex);

			bindBuffer(BUFFERUSAGE_VERTEX, (GLuint) bufferinfo.buffer->getHandle());

			if (intformat)
				glVertexAttribIPointer(i, components, gltype, layout.stride, offsetpointer);
			else
				glVertexAttribPointer(i, components, gltype, normalized, layout.stride, offsetpointer);
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

void OpenGL::setScissor(const Rect &v, bool rtActive)
{
	if (rtActive)
		glScissor(v.x, v.y, v.w, v.h);
	else
	{
		// With no RT active, we need to compensate for glScissor starting
		// from the lower left of the viewport instead of the top left.
		glScissor(v.x, state.viewport.h - (v.y + v.h), v.w, v.h);
	}

	state.scissor = v;
}

void OpenGL::setEnableState(EnableState enablestate, bool enable)
{
	GLenum glstate = GL_NONE;

	switch (enablestate)
	{
	case ENABLE_BLEND:
		glstate = GL_BLEND;
		break;
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

void OpenGL::setStencilWriteMask(uint32 mask)
{
	glStencilMask(mask);
	state.stencilWriteMask = mask;
}

uint32 OpenGL::getStencilWriteMask() const
{
	return state.stencilWriteMask;
}

void OpenGL::setColorWriteMask(uint32 mask)
{
	glColorMask(mask & (1 << 0), mask & (1 << 1), mask & (1 << 2), mask & (1 << 3));
	state.colorWriteMask = mask;
}

uint32 OpenGL::getColorWriteMask() const
{
	return state.colorWriteMask;
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
	SDL_PropertiesID props = SDL_GetWindowProperties(SDL_GL_GetCurrentWindow());
	return (GLuint)SDL_GetNumberProperty(props, SDL_PROP_WINDOW_UIKIT_OPENGL_FRAMEBUFFER_NUMBER, 0);
#else
	return 0;
#endif
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

void OpenGL::bindBufferTextureToUnit(GLuint texture, int textureunit, bool restoreprev, bool bindforedit)
{
	bindTextureToUnit(TEXTURE_MAX_ENUM, texture, textureunit, restoreprev, bindforedit);
}

void OpenGL::bindTextureToUnit(Texture *texture, int textureunit, bool restoreprev, bool bindforedit)
{
	TextureType textype = texture->getTextureType();
	GLuint handle = (GLuint) texture->getHandle();
	bindTextureToUnit(textype, handle, textureunit, restoreprev, bindforedit);
}

void OpenGL::bindIndexedBuffer(GLuint buffer, BufferUsage type, int index)
{
	auto &bindings = state.boundIndexedBuffers[type];
	if (bindings.size() > (size_t) index && buffer != bindings[index])
	{
		bindings[index] = buffer;
		glBindBufferBase(getGLBufferType(type), index, buffer);

		// glBindBufferBase affects glBindBuffer as well... for some reason.
		state.boundBuffers[type] = buffer;
	}
}

void OpenGL::deleteTexture(GLuint texture)
{
	// glDeleteTextures binds texture 0 to all texture units the deleted texture
	// was bound to before deletion.
	for (int i = 0; i < TEXTURE_MAX_ENUM + 1; i++)
	{
		for (GLuint &texid : state.boundTextures[i])
		{
			if (texid == texture)
				texid = 0;
		}
	}

	glDeleteTextures(1, &texture);
}

GLint OpenGL::getGLWrapMode(SamplerState::WrapMode wmode)
{
	switch (wmode)
	{
	case SamplerState::WRAP_CLAMP:
	default:
		return GL_CLAMP_TO_EDGE;
	case SamplerState::WRAP_CLAMP_ZERO:
	case SamplerState::WRAP_CLAMP_ONE:
		return GL_CLAMP_TO_BORDER;
	case SamplerState::WRAP_REPEAT:
		return GL_REPEAT;
	case SamplerState::WRAP_MIRRORED_REPEAT:
		return GL_MIRRORED_REPEAT;
	}
}

GLint OpenGL::getGLCompareMode(CompareMode mode)
{
	switch (mode)
	{
		case COMPARE_LESS: return GL_LESS;
		case COMPARE_LEQUAL: return GL_LEQUAL;
		case COMPARE_EQUAL: return GL_EQUAL;
		case COMPARE_GEQUAL: return GL_GEQUAL;
		case COMPARE_GREATER: return GL_GREATER;
		case COMPARE_NOTEQUAL: return GL_NOTEQUAL;
		case COMPARE_ALWAYS: return GL_ALWAYS;
		case COMPARE_NEVER: return GL_NEVER;
		default: return GL_NEVER;
	}
}

static bool isClampOne(SamplerState::WrapMode mode)
{
	return mode == SamplerState::WRAP_CLAMP_ONE;
}

void OpenGL::setSamplerState(TextureType target, SamplerState &s)
{
	GLenum gltarget = getGLTextureType(target);

	GLint gmin = s.minFilter == SamplerState::FILTER_NEAREST ? GL_NEAREST : GL_LINEAR;
	GLint gmag = s.magFilter == SamplerState::FILTER_NEAREST ? GL_NEAREST : GL_LINEAR;

	if (s.mipmapFilter != SamplerState::MIPMAP_FILTER_NONE)
	{
		if (s.minFilter == SamplerState::FILTER_NEAREST && s.mipmapFilter == SamplerState::MIPMAP_FILTER_NEAREST)
			gmin = GL_NEAREST_MIPMAP_NEAREST;
		else if (s.minFilter == SamplerState::FILTER_NEAREST && s.mipmapFilter == SamplerState::MIPMAP_FILTER_LINEAR)
			gmin = GL_NEAREST_MIPMAP_LINEAR;
		else if (s.minFilter == SamplerState::FILTER_LINEAR && s.mipmapFilter == SamplerState::MIPMAP_FILTER_NEAREST)
			gmin = GL_LINEAR_MIPMAP_NEAREST;
		else if (s.minFilter == SamplerState::FILTER_LINEAR && s.mipmapFilter == SamplerState::MIPMAP_FILTER_LINEAR)
			gmin = GL_LINEAR_MIPMAP_LINEAR;
	}

	glTexParameteri(gltarget, GL_TEXTURE_MIN_FILTER, gmin);
	glTexParameteri(gltarget, GL_TEXTURE_MAG_FILTER, gmag);

	if (!isClampZeroOneTextureWrapSupported())
	{
		if (SamplerState::isClampZeroOrOne(s.wrapU)) s.wrapU = SamplerState::WRAP_CLAMP;
		if (SamplerState::isClampZeroOrOne(s.wrapV)) s.wrapV = SamplerState::WRAP_CLAMP;
		if (SamplerState::isClampZeroOrOne(s.wrapW)) s.wrapW = SamplerState::WRAP_CLAMP;
	}

	if (SamplerState::isClampZeroOrOne(s.wrapU) || SamplerState::isClampZeroOrOne(s.wrapV) || SamplerState::isClampZeroOrOne(s.wrapW))
	{
		GLfloat c[] = {0.0f, 0.0f, 0.0f, 0.0f};
		if (isClampOne(s.wrapU) || isClampOne(s.wrapU) || isClampOne(s.wrapV))
			c[0] = c[1] = c[2] = c[3] = 1.0f;

		glTexParameterfv(gltarget, GL_TEXTURE_BORDER_COLOR, c);
	}

	glTexParameteri(gltarget, GL_TEXTURE_WRAP_S, getGLWrapMode(s.wrapU));
	glTexParameteri(gltarget, GL_TEXTURE_WRAP_T, getGLWrapMode(s.wrapV));

	if (target == TEXTURE_VOLUME)
		glTexParameteri(gltarget, GL_TEXTURE_WRAP_R, getGLWrapMode(s.wrapW));

	if (isSamplerLODBiasSupported())
	{
		float maxbias = getMaxLODBias();
		if (maxbias > 0.01f)
			maxbias -= 0.01f;

		s.lodBias = std::min(std::max(s.lodBias, -maxbias), maxbias);

		glTexParameterf(gltarget, GL_TEXTURE_LOD_BIAS, s.lodBias);
	}
	else
	{
		s.lodBias = 0.0f;
	}

	if (GLAD_EXT_texture_filter_anisotropic)
	{
		uint8 maxAniso = (uint8) std::min(maxAnisotropy, (float)LOVE_UINT8_MAX);
		s.maxAnisotropy = std::min(std::max(s.maxAnisotropy, (uint8)1), maxAniso);
		glTexParameteri(gltarget, GL_TEXTURE_MAX_ANISOTROPY_EXT, s.maxAnisotropy);
	}
	else
	{
		s.maxAnisotropy = 1;
	}

	glTexParameterf(gltarget, GL_TEXTURE_MIN_LOD, (float)s.minLod);
	glTexParameterf(gltarget, GL_TEXTURE_MAX_LOD, (float)s.maxLod);

	if (s.depthSampleMode.hasValue)
	{
		// See the comment in renderstate.h
		GLenum glmode = getGLCompareMode(getReversedCompareMode(s.depthSampleMode.value));

		glTexParameteri(gltarget, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(gltarget, GL_TEXTURE_COMPARE_FUNC, glmode);
	}
	else
	{
		glTexParameteri(gltarget, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	}
}

bool OpenGL::rawTexStorage(TextureType target, int levels, PixelFormat pixelformat, int width, int height, int depth)
{
	GLenum gltarget = getGLTextureType(target);
	TextureFormat fmt = convertPixelFormat(pixelformat);

	// This shouldn't be needed for glTexStorage, but some drivers don't follow
	// the spec apparently.
	// https://stackoverflow.com/questions/13859061/does-an-immutable-texture-need-a-gl-texture-max-level
	if (GLAD_VERSION_1_2 || GLAD_ES_VERSION_3_0)
		glTexParameteri(gltarget, GL_TEXTURE_MAX_LEVEL, levels - 1);

	if (fmt.swizzled)
	{
		glTexParameteri(gltarget, GL_TEXTURE_SWIZZLE_R, fmt.swizzle[0]);
		glTexParameteri(gltarget, GL_TEXTURE_SWIZZLE_G, fmt.swizzle[1]);
		glTexParameteri(gltarget, GL_TEXTURE_SWIZZLE_B, fmt.swizzle[2]);
		glTexParameteri(gltarget, GL_TEXTURE_SWIZZLE_A, fmt.swizzle[3]);
	}

	bool usetexstorage = isTexStorageSupported();

	// The fallback for bugs.brokenR8PixelFormat is GL_LUMINANCE, which doesn't have a sized
	// version in ES3 so it can't be used with glTexStorage.
	if (pixelformat == PIXELFORMAT_R8_UNORM && bugs.brokenR8PixelFormat && GLAD_ES_VERSION_3_0)
	{
		usetexstorage = false;
		fmt.internalformat = fmt.externalformat;
	}

	if (usetexstorage)
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
				glTexImage3D(gltarget, level, fmt.internalformat, w, h, d,
				             0, fmt.externalformat, fmt.type, nullptr);
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
	if (gl.bugs.texStorageBreaksSubImage)
		return false;

	return GLAD_ES_VERSION_3_0 || GLAD_VERSION_4_2 || GLAD_ARB_texture_storage;
}

bool OpenGL::isBufferUsageSupported(BufferUsage usage) const
{
	switch (usage)
	{
	case BUFFERUSAGE_VERTEX:
	case BUFFERUSAGE_INDEX:
		return true;
	case BUFFERUSAGE_TEXEL:
		return GLAD_VERSION_3_1 || GLAD_ES_VERSION_3_2;
	case BUFFERUSAGE_UNIFORM:
		return true;
	case BUFFERUSAGE_SHADER_STORAGE:
		return (GLAD_VERSION_4_3 && isCoreProfile()) || GLAD_ES_VERSION_3_1;
	case BUFFERUSAGE_INDIRECT_ARGUMENTS:
		return (GLAD_VERSION_4_0 && isCoreProfile()) || GLAD_ES_VERSION_3_1;
	case BUFFERUSAGE_MAX_ENUM:
		return false;
	}
	return false;
}

bool OpenGL::isClampZeroOneTextureWrapSupported() const
{
	return GLAD_VERSION_1_3 || GLAD_EXT_texture_border_clamp || GLAD_NV_texture_border_clamp;
}

bool OpenGL::isSamplerLODBiasSupported() const
{
	return GLAD_VERSION_1_4;
}

bool OpenGL::isBaseVertexSupported() const
{
	return baseVertexSupported;
}

bool OpenGL::isCopyTextureToBufferSupported() const
{
	// Requires glGetTextureSubImage support.
	return GLAD_VERSION_4_5 || GLAD_ARB_get_texture_sub_image;
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

int OpenGL::getMaxTexelBufferSize() const
{
	return maxTexelBufferSize;
}

int OpenGL::getMaxShaderStorageBufferSize() const
{
	return maxShaderStorageBufferSize;
}

int OpenGL::getMaxComputeWorkGroupsX() const
{
	return maxComputeWorkGroupsX;
}

int OpenGL::getMaxComputeWorkGroupsY() const
{
	return maxComputeWorkGroupsY;
}

int OpenGL::getMaxComputeWorkGroupsZ() const
{
	return maxComputeWorkGroupsZ;
}

int OpenGL::getMaxRenderTargets() const
{
	return std::min(maxRenderTargets, MAX_COLOR_RENDER_TARGETS);
}

int OpenGL::getMaxSamples() const
{
	return maxSamples;
}

int OpenGL::getMaxTextureUnits() const
{
	return maxTextureUnits;
}

int OpenGL::getMaxShaderStorageBufferBindings() const
{
	return maxShaderStorageBufferBindings;
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

OpenGL::TextureFormat OpenGL::convertPixelFormat(PixelFormat pixelformat)
{
	TextureFormat f;

	f.framebufferAttachments[0] = GL_COLOR_ATTACHMENT0;
	f.framebufferAttachments[1] = GL_NONE;

	if (pixelformat == PIXELFORMAT_ETC1_UNORM)
	{
		// The ETC2 format can load ETC1 textures.
		if (GLAD_ES_VERSION_3_0 || GLAD_VERSION_4_3 || GLAD_ARB_ES3_compatibility)
			pixelformat = PIXELFORMAT_ETC2_RGB_UNORM;
	}

	switch (pixelformat)
	{
	case PIXELFORMAT_R8_UNORM:
		if (!gl.bugs.brokenR8PixelFormat)
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
	case PIXELFORMAT_RG8_UNORM:
		f.internalformat = GL_RG8;
		f.externalformat = GL_RG;
		f.type = GL_UNSIGNED_BYTE;
		break;
	case PIXELFORMAT_RGBA8_UNORM:
		f.internalformat = GL_RGBA8;
		f.externalformat = GL_RGBA;
		f.type = GL_UNSIGNED_BYTE;
		break;
	case PIXELFORMAT_RGBA8_sRGB:
		f.internalformat = GL_SRGB8_ALPHA8;
		f.type = GL_UNSIGNED_BYTE;
		f.externalformat = GL_RGBA;
		break;
	case PIXELFORMAT_BGRA8_UNORM:
	case PIXELFORMAT_BGRA8_sRGB:
		// Not supported right now.
		break;
	case PIXELFORMAT_R16_UNORM:
		f.internalformat = GL_R16;
		f.externalformat = GL_RED;
		f.type = GL_UNSIGNED_SHORT;
		break;
	case PIXELFORMAT_RG16_UNORM:
		f.internalformat = GL_RG16;
		f.externalformat = GL_RG;
		f.type = GL_UNSIGNED_SHORT;
		break;
	case PIXELFORMAT_RGBA16_UNORM:
		f.internalformat = GL_RGBA16;
		f.externalformat = GL_RGBA;
		f.type = GL_UNSIGNED_SHORT;
		break;

	case PIXELFORMAT_R16_FLOAT:
		f.internalformat = GL_R16F;
		f.externalformat = GL_RED;
		if (GLAD_OES_texture_half_float)
			f.type = GL_HALF_FLOAT_OES;
		else
			f.type = GL_HALF_FLOAT;
		break;
	case PIXELFORMAT_RG16_FLOAT:
		f.internalformat = GL_RG16F;
		f.externalformat = GL_RG;
		if (GLAD_OES_texture_half_float)
			f.type = GL_HALF_FLOAT_OES;
		else
			f.type = GL_HALF_FLOAT;
		break;
	case PIXELFORMAT_RGBA16_FLOAT:
		f.internalformat = GL_RGBA16F;
		f.externalformat = GL_RGBA;
		if (GLAD_OES_texture_half_float)
			f.type = GL_HALF_FLOAT_OES;
		else
			f.type = GL_HALF_FLOAT;
		break;
	case PIXELFORMAT_R32_FLOAT:
		f.internalformat = GL_R32F;
		f.externalformat = GL_RED;
		f.type = GL_FLOAT;
		break;
	case PIXELFORMAT_RG32_FLOAT:
		f.internalformat = GL_RG32F;
		f.externalformat = GL_RG;
		f.type = GL_FLOAT;
		break;
	case PIXELFORMAT_RGBA32_FLOAT:
		f.internalformat = GL_RGBA32F;
		f.externalformat = GL_RGBA;
		f.type = GL_FLOAT;
		break;

	case PIXELFORMAT_R8_INT:
		f.internalformat = GL_R8I;
		f.externalformat = GL_RED_INTEGER;
		f.type = GL_BYTE;
		break;
	case PIXELFORMAT_R8_UINT:
		f.internalformat = GL_R8UI;
		f.externalformat = GL_RED_INTEGER;
		f.type = GL_UNSIGNED_BYTE;
		break;
	case PIXELFORMAT_RG8_INT:
		f.internalformat = GL_RG8I;
		f.externalformat = GL_RG_INTEGER;
		f.type = GL_BYTE;
		break;
	case PIXELFORMAT_RG8_UINT:
		f.internalformat = GL_RG8UI;
		f.externalformat = GL_RG_INTEGER;
		f.type = GL_UNSIGNED_BYTE;
		break;
	case PIXELFORMAT_RGBA8_INT:
		f.internalformat = GL_RGBA8I;
		f.externalformat = GL_RGBA_INTEGER;
		f.type = GL_BYTE;
		break;
	case PIXELFORMAT_RGBA8_UINT:
		f.internalformat = GL_RGBA8UI;
		f.externalformat = GL_RGBA_INTEGER;
		f.type = GL_UNSIGNED_BYTE;
		break;
	case PIXELFORMAT_R16_INT:
		f.internalformat = GL_R16I;
		f.externalformat = GL_RED_INTEGER;
		f.type = GL_SHORT;
		break;
	case PIXELFORMAT_R16_UINT:
		f.internalformat = GL_R16UI;
		f.externalformat = GL_RED_INTEGER;
		f.type = GL_UNSIGNED_SHORT;
		break;
	case PIXELFORMAT_RG16_INT:
		f.internalformat = GL_RG16I;
		f.externalformat = GL_RG_INTEGER;
		f.type = GL_SHORT;
		break;
	case PIXELFORMAT_RG16_UINT:
		f.internalformat = GL_RG16UI;
		f.externalformat = GL_RG_INTEGER;
		f.type = GL_UNSIGNED_SHORT;
		break;
	case PIXELFORMAT_RGBA16_INT:
		f.internalformat = GL_RGBA16I;
		f.externalformat = GL_RGBA_INTEGER;
		f.type = GL_SHORT;
		break;
	case PIXELFORMAT_RGBA16_UINT:
		f.internalformat = GL_RGBA16UI;
		f.externalformat = GL_RGBA_INTEGER;
		f.type = GL_UNSIGNED_SHORT;
		break;
	case PIXELFORMAT_R32_INT:
		f.internalformat = GL_R32I;
		f.externalformat = GL_RED_INTEGER;
		f.type = GL_INT;
		break;
	case PIXELFORMAT_R32_UINT:
		f.internalformat = GL_R32UI;
		f.externalformat = GL_RED_INTEGER;
		f.type = GL_UNSIGNED_INT;
		break;
	case PIXELFORMAT_RG32_INT:
		f.internalformat = GL_RG32I;
		f.externalformat = GL_RG_INTEGER;
		f.type = GL_INT;
		break;
	case PIXELFORMAT_RG32_UINT:
		f.internalformat = GL_RG32UI;
		f.externalformat = GL_RG_INTEGER;
		f.type = GL_UNSIGNED_INT;
		break;
	case PIXELFORMAT_RGBA32_INT:
		f.internalformat = GL_RGBA32I;
		f.externalformat = GL_RGBA_INTEGER;
		f.type = GL_INT;
		break;
	case PIXELFORMAT_RGBA32_UINT:
		f.internalformat = GL_RGBA32UI;
		f.externalformat = GL_RGBA_INTEGER;
		f.type = GL_UNSIGNED_INT;
		break;

	case PIXELFORMAT_LA8_UNORM:
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

	case PIXELFORMAT_RGBA4_UNORM:
		f.internalformat = GL_RGBA4;
		f.externalformat = GL_RGBA;
		f.type = GL_UNSIGNED_SHORT_4_4_4_4;
		break;
	case PIXELFORMAT_RGB5A1_UNORM:
		f.internalformat = GL_RGB5_A1;
		f.externalformat = GL_RGBA;
		f.type = GL_UNSIGNED_SHORT_5_5_5_1;
		break;
	case PIXELFORMAT_RGB565_UNORM:
		f.internalformat = GL_RGB565;
		f.externalformat = GL_RGB;
		f.type = GL_UNSIGNED_SHORT_5_6_5;
		break;
	case PIXELFORMAT_RGB10A2_UNORM:
		f.internalformat = GL_RGB10_A2;
		f.externalformat = GL_RGBA;
		f.type = GL_UNSIGNED_INT_2_10_10_10_REV;
		break;
	case PIXELFORMAT_RG11B10_FLOAT:
		f.internalformat = GL_R11F_G11F_B10F;
		f.externalformat = GL_RGB;
		f.type = GL_UNSIGNED_INT_10F_11F_11F_REV;
		break;

	case PIXELFORMAT_STENCIL8:
		// Prefer a combined depth/stencil buffer due to driver issues.
		f.internalformat = GL_DEPTH24_STENCIL8;
		f.externalformat = GL_DEPTH_STENCIL;
		f.type = GL_UNSIGNED_INT_24_8;
		f.framebufferAttachments[0] = GL_DEPTH_STENCIL_ATTACHMENT;
		break;

	case PIXELFORMAT_DEPTH16_UNORM:
		f.internalformat = GL_DEPTH_COMPONENT16;
		f.externalformat = GL_DEPTH_COMPONENT;
		f.type = GL_UNSIGNED_SHORT;
		f.framebufferAttachments[0] = GL_DEPTH_ATTACHMENT;
		break;

	case PIXELFORMAT_DEPTH24_UNORM:
		f.internalformat = GL_DEPTH_COMPONENT24;
		f.externalformat = GL_DEPTH_COMPONENT;
		f.type = GL_UNSIGNED_INT;
		f.framebufferAttachments[0] = GL_DEPTH_ATTACHMENT;
		break;

	case PIXELFORMAT_DEPTH32_FLOAT:
		f.internalformat = GL_DEPTH_COMPONENT32F;
		f.externalformat = GL_DEPTH_COMPONENT;
		f.type = GL_FLOAT;
		f.framebufferAttachments[0] = GL_DEPTH_ATTACHMENT;
		break;

	case PIXELFORMAT_DEPTH24_UNORM_STENCIL8:
		f.internalformat = GL_DEPTH24_STENCIL8;
		f.externalformat = GL_DEPTH_STENCIL;
		f.type = GL_UNSIGNED_INT_24_8;
		f.framebufferAttachments[0] = GL_DEPTH_STENCIL_ATTACHMENT;
		break;

	case PIXELFORMAT_DEPTH32_FLOAT_STENCIL8:
		f.internalformat = GL_DEPTH32F_STENCIL8;
		f.externalformat = GL_DEPTH_STENCIL;
		f.type = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
		f.framebufferAttachments[0] = GL_DEPTH_STENCIL_ATTACHMENT;
		break;

	case PIXELFORMAT_DXT1_UNORM:
		f.internalformat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		break;
	case PIXELFORMAT_DXT1_sRGB:
		f.internalformat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
		break;
	case PIXELFORMAT_DXT3_UNORM:
		f.internalformat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		break;
	case PIXELFORMAT_DXT3_sRGB:
		f.internalformat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
		break;
	case PIXELFORMAT_DXT5_UNORM:
		f.internalformat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		break;
	case PIXELFORMAT_DXT5_sRGB:
		f.internalformat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
		break;
	case PIXELFORMAT_BC4_UNORM:
		f.internalformat = GL_COMPRESSED_RED_RGTC1;
		break;
	case PIXELFORMAT_BC4_SNORM:
		f.internalformat = GL_COMPRESSED_SIGNED_RED_RGTC1;
		break;
	case PIXELFORMAT_BC5_UNORM:
		f.internalformat = GL_COMPRESSED_RG_RGTC2;
		break;
	case PIXELFORMAT_BC5_SNORM:
		f.internalformat = GL_COMPRESSED_SIGNED_RG_RGTC2;
		break;
	case PIXELFORMAT_BC6H_UFLOAT:
		f.internalformat = GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT;
		break;
	case PIXELFORMAT_BC6H_FLOAT:
		f.internalformat = GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT;
		break;
	case PIXELFORMAT_BC7_UNORM:
		f.internalformat = GL_COMPRESSED_RGBA_BPTC_UNORM;
		break;
	case PIXELFORMAT_BC7_sRGB:
		f.internalformat = GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM;
		break;

	case PIXELFORMAT_PVR1_RGB2_UNORM:
		f.internalformat = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
		break;
	case PIXELFORMAT_PVR1_RGB2_sRGB:
		f.internalformat = GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT;
		break;
	case PIXELFORMAT_PVR1_RGB4_UNORM:
		f.internalformat = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
		break;
	case PIXELFORMAT_PVR1_RGB4_sRGB:
		f.internalformat = GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT;
		break;
	case PIXELFORMAT_PVR1_RGBA2_UNORM:
		f.internalformat = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
		break;
	case PIXELFORMAT_PVR1_RGBA2_sRGB:
		f.internalformat = GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT;
		break;
	case PIXELFORMAT_PVR1_RGBA4_UNORM:
		f.internalformat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
		break;
	case PIXELFORMAT_PVR1_RGBA4_sRGB:
		f.internalformat = GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT;
		break;

	case PIXELFORMAT_ETC1_UNORM:
		f.internalformat = GL_ETC1_RGB8_OES;
		break;
	case PIXELFORMAT_ETC2_RGB_UNORM:
		f.internalformat = GL_COMPRESSED_RGB8_ETC2;
		break;
	case PIXELFORMAT_ETC2_RGB_sRGB:
		f.internalformat = GL_COMPRESSED_SRGB8_ETC2;
		break;
	case PIXELFORMAT_ETC2_RGBA_UNORM:
		f.internalformat = GL_COMPRESSED_RGBA8_ETC2_EAC;
		break;
	case PIXELFORMAT_ETC2_RGBA_sRGB:
		f.internalformat = GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC;
		break;
	case PIXELFORMAT_ETC2_RGBA1_UNORM:
		f.internalformat = GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
		break;
	case PIXELFORMAT_ETC2_RGBA1_sRGB:
		f.internalformat = GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2;
		break;
	case PIXELFORMAT_EAC_R_UNORM:
		f.internalformat = GL_COMPRESSED_R11_EAC;
		break;
	case PIXELFORMAT_EAC_R_SNORM:
		f.internalformat = GL_COMPRESSED_SIGNED_R11_EAC;
		break;
	case PIXELFORMAT_EAC_RG_UNORM:
		f.internalformat = GL_COMPRESSED_RG11_EAC;
		break;
	case PIXELFORMAT_EAC_RG_SNORM:
		f.internalformat = GL_COMPRESSED_SIGNED_RG11_EAC;
		break;

	case PIXELFORMAT_ASTC_4x4_UNORM:
		f.internalformat = GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
		break;
	case PIXELFORMAT_ASTC_4x4_sRGB:
		f.internalformat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR;
		break;
	case PIXELFORMAT_ASTC_5x4_UNORM:
		f.internalformat = GL_COMPRESSED_RGBA_ASTC_5x4_KHR;
		break;
	case PIXELFORMAT_ASTC_5x4_sRGB:
		f.internalformat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR;
		break;
	case PIXELFORMAT_ASTC_5x5_UNORM:
		f.internalformat = GL_COMPRESSED_RGBA_ASTC_5x5_KHR;
		break;
	case PIXELFORMAT_ASTC_5x5_sRGB:
		f.internalformat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR;
		break;
	case PIXELFORMAT_ASTC_6x5_UNORM:
		f.internalformat = GL_COMPRESSED_RGBA_ASTC_6x5_KHR;
		break;
	case PIXELFORMAT_ASTC_6x5_sRGB:
		f.internalformat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR;
		break;
	case PIXELFORMAT_ASTC_6x6_UNORM:
		f.internalformat = GL_COMPRESSED_RGBA_ASTC_6x6_KHR;
		break;
	case PIXELFORMAT_ASTC_6x6_sRGB:
		f.internalformat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR;
		break;
	case PIXELFORMAT_ASTC_8x5_UNORM:
		f.internalformat = GL_COMPRESSED_RGBA_ASTC_8x5_KHR;
		break;
	case PIXELFORMAT_ASTC_8x5_sRGB:
		f.internalformat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR;
		break;
	case PIXELFORMAT_ASTC_8x6_UNORM:
		f.internalformat = GL_COMPRESSED_RGBA_ASTC_8x6_KHR;
		break;
	case PIXELFORMAT_ASTC_8x6_sRGB:
		f.internalformat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR;
		break;
	case PIXELFORMAT_ASTC_8x8_UNORM:
		f.internalformat = GL_COMPRESSED_RGBA_ASTC_8x8_KHR;
		break;
	case PIXELFORMAT_ASTC_8x8_sRGB:
		f.internalformat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR;
		break;
	case PIXELFORMAT_ASTC_10x5_UNORM:
		f.internalformat = GL_COMPRESSED_RGBA_ASTC_10x5_KHR;
		break;
	case PIXELFORMAT_ASTC_10x5_sRGB:
		f.internalformat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR;
		break;
	case PIXELFORMAT_ASTC_10x6_UNORM:
		f.internalformat = GL_COMPRESSED_RGBA_ASTC_10x6_KHR;
		break;
	case PIXELFORMAT_ASTC_10x6_sRGB:
		f.internalformat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR;
		break;
	case PIXELFORMAT_ASTC_10x8_UNORM:
		f.internalformat = GL_COMPRESSED_RGBA_ASTC_10x8_KHR;
		break;
	case PIXELFORMAT_ASTC_10x8_sRGB:
		f.internalformat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR;
		break;
	case PIXELFORMAT_ASTC_10x10_UNORM:
		f.internalformat = GL_COMPRESSED_RGBA_ASTC_10x10_KHR;
		break;
	case PIXELFORMAT_ASTC_10x10_sRGB:
		f.internalformat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR;
		break;
	case PIXELFORMAT_ASTC_12x10_UNORM:
		f.internalformat = GL_COMPRESSED_RGBA_ASTC_12x10_KHR;
		break;
	case PIXELFORMAT_ASTC_12x10_sRGB:
		f.internalformat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR;
		break;
	case PIXELFORMAT_ASTC_12x12_UNORM:
		f.internalformat = GL_COMPRESSED_RGBA_ASTC_12x12_KHR;
		break;
	case PIXELFORMAT_ASTC_12x12_sRGB:
		f.internalformat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR;
		break;

	default:
		printf("Unhandled pixel format %d when converting to OpenGL enums!", pixelformat);
		break;
	}

	return f;
}

uint32 OpenGL::getPixelFormatUsageFlags(PixelFormat pixelformat)
{
	const uint32 commonsample = PIXELFORMATUSAGEFLAGS_SAMPLE | PIXELFORMATUSAGEFLAGS_LINEAR;
	const uint32 commonrender = PIXELFORMATUSAGEFLAGS_RENDERTARGET | PIXELFORMATUSAGEFLAGS_BLEND | PIXELFORMATUSAGEFLAGS_MSAA;
	const uint32 computewrite = PIXELFORMATUSAGEFLAGS_COMPUTEWRITE;

	uint32 flags = PIXELFORMATUSAGEFLAGS_NONE;

	switch (pixelformat)
	{
	case PIXELFORMAT_R8_UNORM:
	case PIXELFORMAT_RG8_UNORM:
		flags |= commonsample | commonrender;
		if (GLAD_VERSION_4_3)
			flags |= computewrite;
		break;
	case PIXELFORMAT_RGBA8_UNORM:
		flags |= commonsample | commonrender;
		if (GLAD_VERSION_4_3 || GLAD_ES_VERSION_3_1)
			flags |= computewrite;
		break;
	case PIXELFORMAT_RGBA8_sRGB:
		if (gl.bugs.brokenSRGB)
			break;
		flags |= commonsample | commonrender;
		break;
	case PIXELFORMAT_BGRA8_UNORM:
	case PIXELFORMAT_BGRA8_sRGB:
		// Not supported right now.
		break;
	case PIXELFORMAT_R16_UNORM:
	case PIXELFORMAT_RG16_UNORM:
		if (GLAD_VERSION_3_0 || (GLAD_EXT_texture_norm16 && GLAD_ES_VERSION_3_0))
			flags |= commonsample | commonrender;
		if (GLAD_VERSION_4_3)
			flags |= computewrite;
		break;
	case PIXELFORMAT_RGBA16_UNORM:
		if (GLAD_VERSION_1_1 || GLAD_EXT_texture_norm16)
			flags |= commonsample | commonrender;
		if (GLAD_VERSION_4_3)
			flags |= computewrite;
		break;
	case PIXELFORMAT_R16_FLOAT:
	case PIXELFORMAT_RG16_FLOAT:
		flags |= commonsample;
		if (GLAD_VERSION_3_0)
			flags |= commonrender;
		if ((GLAD_EXT_color_buffer_half_float || GLAD_EXT_color_buffer_float) && GLAD_ES_VERSION_3_0)
			flags |= commonrender;
		if (GLAD_VERSION_4_3)
			flags |= computewrite;
		break;
	case PIXELFORMAT_RGBA16_FLOAT:
		flags |= commonsample;
		if (GLAD_VERSION_3_0)
			flags |= commonrender;
		if (GLAD_EXT_color_buffer_half_float || GLAD_EXT_color_buffer_float)
			flags |= commonrender;
		if (GLAD_VERSION_4_3 || GLAD_ES_VERSION_3_1)
			flags |= computewrite;
		break;
	case PIXELFORMAT_R32_FLOAT:
		if (GLAD_ES_VERSION_3_1)
			flags |= computewrite;
		// Fallthrough.
	case PIXELFORMAT_RG32_FLOAT:
		flags |= commonsample;
		if (GLAD_VERSION_3_0)
			flags |= commonrender;
		if (GLAD_EXT_color_buffer_float)
			flags |= commonrender;
		if (!(GLAD_VERSION_1_1 || GLAD_OES_texture_float_linear))
			flags &= ~PIXELFORMATUSAGEFLAGS_LINEAR;
		if (GLAD_VERSION_4_3)
			flags |= computewrite;
		break;
	case PIXELFORMAT_RGBA32_FLOAT:
		flags |= commonsample;
		if (GLAD_VERSION_3_0)
			flags |= commonrender;
		if (GLAD_EXT_color_buffer_float)
			flags |= commonrender;
		if (!(GLAD_VERSION_1_1 || GLAD_OES_texture_float_linear))
			flags &= ~PIXELFORMATUSAGEFLAGS_LINEAR;
		if (GLAD_VERSION_4_3 || GLAD_ES_VERSION_3_1)
			flags |= computewrite;
		break;

	case PIXELFORMAT_R8_INT:
	case PIXELFORMAT_R8_UINT:
	case PIXELFORMAT_RG8_INT:
	case PIXELFORMAT_RG8_UINT:
	case PIXELFORMAT_RGBA8_INT:
	case PIXELFORMAT_RGBA8_UINT:
	case PIXELFORMAT_R16_INT:
	case PIXELFORMAT_R16_UINT:
	case PIXELFORMAT_RG16_INT:
	case PIXELFORMAT_RG16_UINT:
	case PIXELFORMAT_RGBA16_INT:
	case PIXELFORMAT_RGBA16_UINT:
	case PIXELFORMAT_R32_INT:
	case PIXELFORMAT_R32_UINT:
	case PIXELFORMAT_RG32_INT:
	case PIXELFORMAT_RG32_UINT:
	case PIXELFORMAT_RGBA32_INT:
	case PIXELFORMAT_RGBA32_UINT:
		flags |= PIXELFORMATUSAGEFLAGS_SAMPLE | PIXELFORMATUSAGEFLAGS_RENDERTARGET;
		if (GLAD_VERSION_4_3)
			flags |= computewrite;
		if (GLAD_ES_VERSION_3_1)
		{
			switch (pixelformat)
			{
			case PIXELFORMAT_RGBA8_INT:
			case PIXELFORMAT_RGBA8_UINT:
			case PIXELFORMAT_RGBA16_INT:
			case PIXELFORMAT_RGBA16_UINT:
			case PIXELFORMAT_R32_INT:
			case PIXELFORMAT_R32_UINT:
			case PIXELFORMAT_RGBA32_INT:
			case PIXELFORMAT_RGBA32_UINT:
				flags |= computewrite;
				break;
			default:
				break;
			}
		}
		break;

	case PIXELFORMAT_LA8_UNORM:
		flags |= commonsample;
		break;

	case PIXELFORMAT_RGBA4_UNORM:
	case PIXELFORMAT_RGB5A1_UNORM:
		flags |= commonsample | commonrender;
		break;
	case PIXELFORMAT_RGB565_UNORM:
		if (GLAD_ES_VERSION_2_0 || GLAD_VERSION_4_2 || GLAD_ARB_ES2_compatibility)
			flags |= commonsample | commonrender;
		break;
	case PIXELFORMAT_RGB10A2_UNORM:
		flags |= commonsample | commonrender;
		if (GLAD_VERSION_4_3)
			flags |= computewrite;
		break;
	case PIXELFORMAT_RG11B10_FLOAT:
		if (GLAD_ES_VERSION_3_1 || GLAD_VERSION_3_0 || GLAD_EXT_packed_float || GLAD_APPLE_texture_packed_float)
			flags |= commonsample;
		if (GLAD_VERSION_3_0 || GLAD_EXT_packed_float || GLAD_APPLE_color_buffer_packed_float)
			flags |= commonrender;
		if (GLAD_EXT_color_buffer_float)
			flags |= commonrender;
		if (GLAD_VERSION_4_3)
			flags |= computewrite;
		break;

	case PIXELFORMAT_STENCIL8:
		flags |= PIXELFORMATUSAGEFLAGS_RENDERTARGET | PIXELFORMATUSAGEFLAGS_MSAA;
		break;

	case PIXELFORMAT_DEPTH16_UNORM:
		flags |= commonsample | PIXELFORMATUSAGEFLAGS_RENDERTARGET | PIXELFORMATUSAGEFLAGS_MSAA;
		break;

	case PIXELFORMAT_DEPTH24_UNORM:
		flags |= commonsample | PIXELFORMATUSAGEFLAGS_RENDERTARGET | PIXELFORMATUSAGEFLAGS_MSAA;
		break;

	case PIXELFORMAT_DEPTH24_UNORM_STENCIL8:
		flags |= commonsample | PIXELFORMATUSAGEFLAGS_RENDERTARGET | PIXELFORMATUSAGEFLAGS_MSAA;
		break;

	case PIXELFORMAT_DEPTH32_FLOAT:
	case PIXELFORMAT_DEPTH32_FLOAT_STENCIL8:
		flags |= commonsample | PIXELFORMATUSAGEFLAGS_RENDERTARGET | PIXELFORMATUSAGEFLAGS_MSAA;
		break;

	case PIXELFORMAT_DXT1_UNORM:
	case PIXELFORMAT_DXT1_sRGB:
		if (GLAD_EXT_texture_compression_s3tc || GLAD_EXT_texture_compression_dxt1)
			flags |= commonsample;
		break;
	case PIXELFORMAT_DXT3_UNORM:
	case PIXELFORMAT_DXT3_sRGB:
		if (GLAD_EXT_texture_compression_s3tc || GLAD_ANGLE_texture_compression_dxt3)
			flags |= commonsample;
		break;
	case PIXELFORMAT_DXT5_UNORM:
	case PIXELFORMAT_DXT5_sRGB:
		if (GLAD_EXT_texture_compression_s3tc || GLAD_ANGLE_texture_compression_dxt5)
			flags |= commonsample;
		break;
	case PIXELFORMAT_BC4_UNORM:
	case PIXELFORMAT_BC4_SNORM:
	case PIXELFORMAT_BC5_UNORM:
	case PIXELFORMAT_BC5_SNORM:
		if (GLAD_VERSION_3_0 || GLAD_ARB_texture_compression_rgtc || GLAD_EXT_texture_compression_rgtc)
			flags |= commonsample;
		break;
	case PIXELFORMAT_BC6H_UFLOAT:
	case PIXELFORMAT_BC6H_FLOAT:
	case PIXELFORMAT_BC7_UNORM:
	case PIXELFORMAT_BC7_sRGB:
		if (GLAD_VERSION_4_2 || GLAD_ARB_texture_compression_bptc)
			flags |= commonsample;
		break;
	case PIXELFORMAT_PVR1_RGB2_UNORM:
	case PIXELFORMAT_PVR1_RGB4_UNORM:
	case PIXELFORMAT_PVR1_RGBA2_UNORM:
	case PIXELFORMAT_PVR1_RGBA4_UNORM:
		if (GLAD_IMG_texture_compression_pvrtc)
			flags |= commonsample;
		break;
	case PIXELFORMAT_PVR1_RGB2_sRGB:
	case PIXELFORMAT_PVR1_RGB4_sRGB:
	case PIXELFORMAT_PVR1_RGBA2_sRGB:
	case PIXELFORMAT_PVR1_RGBA4_sRGB:
		if (GLAD_EXT_pvrtc_sRGB)
			flags |= commonsample;
		break;
	case PIXELFORMAT_ETC1_UNORM:
		// ETC2 support guarantees ETC1 support as well.
		if (GLAD_ES_VERSION_3_0 || GLAD_VERSION_4_3 || GLAD_ARB_ES3_compatibility)
			flags |= commonsample;
		break;
	case PIXELFORMAT_ETC2_RGB_UNORM:
	case PIXELFORMAT_ETC2_RGB_sRGB:
	case PIXELFORMAT_ETC2_RGBA_UNORM:
	case PIXELFORMAT_ETC2_RGBA_sRGB:
	case PIXELFORMAT_ETC2_RGBA1_UNORM:
	case PIXELFORMAT_ETC2_RGBA1_sRGB:
	case PIXELFORMAT_EAC_R_UNORM:
	case PIXELFORMAT_EAC_R_SNORM:
	case PIXELFORMAT_EAC_RG_UNORM:
	case PIXELFORMAT_EAC_RG_SNORM:
		if (GLAD_ES_VERSION_3_0 || GLAD_VERSION_4_3 || GLAD_ARB_ES3_compatibility)
			flags |= commonsample;
		break;
	case PIXELFORMAT_ASTC_4x4_UNORM:
	case PIXELFORMAT_ASTC_5x4_UNORM:
	case PIXELFORMAT_ASTC_5x5_UNORM:
	case PIXELFORMAT_ASTC_6x5_UNORM:
	case PIXELFORMAT_ASTC_6x6_UNORM:
	case PIXELFORMAT_ASTC_8x5_UNORM:
	case PIXELFORMAT_ASTC_8x6_UNORM:
	case PIXELFORMAT_ASTC_8x8_UNORM:
	case PIXELFORMAT_ASTC_10x5_UNORM:
	case PIXELFORMAT_ASTC_10x6_UNORM:
	case PIXELFORMAT_ASTC_10x8_UNORM:
	case PIXELFORMAT_ASTC_10x10_UNORM:
	case PIXELFORMAT_ASTC_12x10_UNORM:
	case PIXELFORMAT_ASTC_12x12_UNORM:
	case PIXELFORMAT_ASTC_4x4_sRGB:
	case PIXELFORMAT_ASTC_5x4_sRGB:
	case PIXELFORMAT_ASTC_5x5_sRGB:
	case PIXELFORMAT_ASTC_6x5_sRGB:
	case PIXELFORMAT_ASTC_6x6_sRGB:
	case PIXELFORMAT_ASTC_8x5_sRGB:
	case PIXELFORMAT_ASTC_8x6_sRGB:
	case PIXELFORMAT_ASTC_8x8_sRGB:
	case PIXELFORMAT_ASTC_10x5_sRGB:
	case PIXELFORMAT_ASTC_10x6_sRGB:
	case PIXELFORMAT_ASTC_10x8_sRGB:
	case PIXELFORMAT_ASTC_10x10_sRGB:
	case PIXELFORMAT_ASTC_12x10_sRGB:
	case PIXELFORMAT_ASTC_12x12_sRGB:
		if (GLAD_ES_VERSION_3_2 || GLAD_KHR_texture_compression_astc_ldr)
			flags |= commonsample;
		break;

	case PIXELFORMAT_UNKNOWN:
	case PIXELFORMAT_NORMAL:
	case PIXELFORMAT_HDR:
	case PIXELFORMAT_MAX_ENUM:
		break;
	}

	return flags;
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

	snprintf(text, sizeof(text), "0x%x", errorcode);

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
		return "Texture with the specified MSAA count cannot be rendered to on this system.";
	case GL_FRAMEBUFFER_UNSUPPORTED:
		return "Renderable textures are unsupported";
	default:
		break;
	}

	static char text[64] = {};

	snprintf(text, sizeof(text), "0x%x", status);

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
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		return "notification";
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
