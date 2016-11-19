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

#include "Canvas.h"
#include "Image.h"
#include "Graphics.h"
#include "common/Matrix.h"

#include <cstring> // For memcpy
#include <algorithm> // For min/max

namespace love
{
namespace graphics
{
namespace opengl
{

static GLenum createFBO(GLuint &framebuffer, GLuint texture)
{
	// get currently bound fbo to reset to it later
	GLuint current_fbo = gl.getFramebuffer(OpenGL::FRAMEBUFFER_ALL);

	glGenFramebuffers(1, &framebuffer);
	gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, framebuffer);

	if (texture != 0)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

		// Initialize the texture to transparent black.
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, current_fbo);
	return status;
}

static bool createMSAABuffer(int width, int height, int &samples, GLenum iformat, GLuint &buffer)
{
	GLuint current_fbo = gl.getFramebuffer(OpenGL::FRAMEBUFFER_ALL);

	// Temporary FBO used to clear the renderbuffer.
	GLuint fbo = 0;
	glGenFramebuffers(1, &fbo);
	gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, fbo);

	glGenRenderbuffers(1, &buffer);
	glBindRenderbuffer(GL_RENDERBUFFER, buffer);

	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, iformat, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, buffer);

	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES, &samples);

	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (status == GL_FRAMEBUFFER_COMPLETE && samples > 1)
	{
		// Initialize the buffer to transparent black.
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	else
	{
		glDeleteRenderbuffers(1, &buffer);
		buffer = 0;
		samples = 0;
	}

	gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, current_fbo);
	gl.deleteFramebuffer(fbo);

	return status == GL_FRAMEBUFFER_COMPLETE && samples > 1;
}

int Canvas::canvasCount = 0;

Canvas::Canvas(int width, int height, Format format, int msaa)
	: fbo(0)
	, texture(0)
    , msaa_buffer(0)
	, format(format)
    , requested_samples(msaa)
	, actual_samples(0)
	, texture_memory(0)
{
	this->width = width;
	this->height = height;

	float w = static_cast<float>(width);
	float h = static_cast<float>(height);

	// Vertices are ordered for use with triangle strips:
	// 0----2
	// |  / |
	// | /  |
	// 1----3
	// world coordinates
	vertices[0].x = 0;
	vertices[0].y = 0;
	vertices[1].x = 0;
	vertices[1].y = h;
	vertices[2].x = w;
	vertices[2].y = 0;
	vertices[3].x = w;
	vertices[3].y = h;

	// texture coordinates
	vertices[0].s = 0;
	vertices[0].t = 0;
	vertices[1].s = 0;
	vertices[1].t = 1;
	vertices[2].s = 1;
	vertices[2].t = 0;
	vertices[3].s = 1;
	vertices[3].t = 1;

	loadVolatile();

	++canvasCount;
}

Canvas::~Canvas()
{
	--canvasCount;
	unloadVolatile();
}

bool Canvas::loadVolatile()
{
	OpenGL::TempDebugGroup debuggroup("Canvas load");

	fbo = texture = 0;
	msaa_buffer = 0;
	status = GL_FRAMEBUFFER_COMPLETE;

	// glTexImage2D is guaranteed to error in this case.
	if (width > gl.getMaxTextureSize() || height > gl.getMaxTextureSize())
	{
		status = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
		return false;
	}

	// getMaxRenderbufferSamples will be 0 on systems that don't support
	// multisampled renderbuffers / don't export FBO multisample extensions.
	requested_samples = std::min(requested_samples, gl.getMaxRenderbufferSamples());
	requested_samples = std::max(requested_samples, 0);

	glGenTextures(1, &texture);
	gl.bindTextureToUnit(texture, 0, false);

	if (GLAD_ANGLE_texture_usage)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_USAGE_ANGLE, GL_FRAMEBUFFER_ATTACHMENT_ANGLE);

	setFilter(filter);
	setWrap(wrap);

	GLenum internalformat = GL_RGBA;
	GLenum externalformat = GL_RGBA;
	GLenum textype = GL_UNSIGNED_BYTE;

	convertFormat(format, internalformat, externalformat, textype);

	// in GLES2, the internalformat and format params of TexImage have to match.
	GLint iformat = (GLint) internalformat;
	if (GLAD_ES_VERSION_2_0 && !GLAD_ES_VERSION_3_0)
		iformat = (GLint) externalformat;

	while (glGetError() != GL_NO_ERROR)
		/* Clear the error buffer. */;

	glTexImage2D(GL_TEXTURE_2D, 0, iformat, width, height, 0, externalformat,
	             textype, nullptr);

	if (glGetError() != GL_NO_ERROR)
	{
        gl.deleteTexture(texture);
        texture = 0;
        status = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
		return false;
	}

	// Create a canvas-local FBO used for glReadPixels as well as MSAA blitting.
	status = createFBO(fbo, texture);

	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		if (fbo != 0)
		{
			gl.deleteFramebuffer(fbo);
			fbo = 0;
		}
		return false;
	}

	actual_samples = requested_samples == 1 ? 0 : requested_samples;

	if (actual_samples > 0 && !createMSAABuffer(width, height, actual_samples, internalformat, msaa_buffer))
		actual_samples = 0;

	size_t prevmemsize = texture_memory;

	texture_memory = ((getFormatBitsPerPixel(format) * width) / 8) * height;
	if (msaa_buffer != 0)
		texture_memory += (texture_memory * actual_samples);

	gl.updateTextureMemorySize(prevmemsize, texture_memory);

	return true;
}

void Canvas::unloadVolatile()
{
	if (fbo != 0)
		gl.deleteFramebuffer(fbo);

	if (msaa_buffer != 0)
		glDeleteRenderbuffers(1, &msaa_buffer);

	if (texture != 0)
		gl.deleteTexture(texture);

	fbo = 0;
	msaa_buffer = 0;
	texture = 0;

	gl.updateTextureMemorySize(texture_memory, 0);
	texture_memory = 0;
}

void Canvas::drawv(const Matrix4 &t, const Vertex *v)
{
	auto gfx = Module::getInstance<Graphics>(Module::M_GRAPHICS);
	if (gfx != nullptr)
	{
		const PassInfo &info = gfx->getActivePass();
		for (const auto &attachment : info.colorAttachments)
		{
			if (attachment.canvas == this)
				throw love::Exception("Cannot render a Canvas to itself!");
		}
	}

	OpenGL::TempDebugGroup debuggroup("Canvas draw");

	OpenGL::TempTransform transform(gl);
	transform.get() *= t;

	gl.bindTextureToUnit(texture, 0, false);

	gl.useVertexAttribArrays(ATTRIBFLAG_POS | ATTRIBFLAG_TEXCOORD);

	gl.bindBuffer(BUFFER_VERTEX, 0);
	glVertexAttribPointer(ATTRIB_POS, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), &v[0].x);
	glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), &v[0].s);

	gl.prepareDraw();
	gl.drawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Canvas::draw(const Matrix4 &m)
{
	drawv(m, vertices);
}

void Canvas::drawq(Quad *quad, const Matrix4 &m)
{
	drawv(m, quad->getVertices());
}

void Canvas::setFilter(const Texture::Filter &f)
{
	if (!validateFilter(f, false))
		throw love::Exception("Invalid texture filter.");

	filter = f;
	gl.bindTextureToUnit(texture, 0, false);
	gl.setTextureFilter(filter);
}

bool Canvas::setWrap(const Texture::Wrap &w)
{
	bool success = true;
	wrap = w;

	if ((GLAD_ES_VERSION_2_0 && !(GLAD_ES_VERSION_3_0 || GLAD_OES_texture_npot))
		&& (width != nextP2(width) || height != nextP2(height)))
	{
		if (wrap.s != WRAP_CLAMP || wrap.t != WRAP_CLAMP)
			success = false;

		// If we only have limited NPOT support then the wrap mode must be CLAMP.
		wrap.s = wrap.t = WRAP_CLAMP;
	}

	if (!gl.isClampZeroTextureWrapSupported())
	{
		if (wrap.s == WRAP_CLAMP_ZERO)
			wrap.s = WRAP_CLAMP;
		if (wrap.t == WRAP_CLAMP_ZERO)
			wrap.t = WRAP_CLAMP;
	}

	gl.bindTextureToUnit(texture, 0, false);
	gl.setTextureWrap(wrap);

	return success;
}

const void *Canvas::getHandle() const
{
	return &texture;
}

Canvas::Format Canvas::getSizedFormat(Canvas::Format format)
{
	switch (format)
	{
	case FORMAT_NORMAL:
		if (isGammaCorrect())
			return FORMAT_SRGB;
		else if (GLAD_ES_VERSION_2_0 && !(GLAD_ES_VERSION_3_0 || GLAD_OES_rgb8_rgba8 || GLAD_ARM_rgba8))
			// 32-bit render targets don't have guaranteed support on GLES2.
			return FORMAT_RGBA4;
		else
			return FORMAT_RGBA8;
	case FORMAT_HDR:
		return FORMAT_RGBA16F;
	default:
		return format;
	}
}

void Canvas::convertFormat(Canvas::Format format, GLenum &internalformat, GLenum &externalformat, GLenum &type)
{
	format = getSizedFormat(format);
	externalformat = GL_RGBA;

	switch (format)
	{
	case FORMAT_RGBA4:
		internalformat = GL_RGBA4;
		type = GL_UNSIGNED_SHORT_4_4_4_4;
		break;
	case FORMAT_RGB5A1:
		internalformat = GL_RGB5_A1;
		type = GL_UNSIGNED_SHORT_5_5_5_1;
		break;
	case FORMAT_RGB565:
		internalformat = GL_RGB565;
		externalformat = GL_RGB;
		type = GL_UNSIGNED_SHORT_5_6_5;
		break;
	case FORMAT_R8:
		internalformat = GL_R8;
		externalformat = GL_RED;
		type = GL_UNSIGNED_BYTE;
		break;
	case FORMAT_RG8:
		internalformat = GL_RG8;
		externalformat = GL_RG;
		type = GL_UNSIGNED_BYTE;
		break;
	case FORMAT_RGBA8:
	default:
		internalformat = GL_RGBA8;
		type = GL_UNSIGNED_BYTE;
		break;
	case FORMAT_RGB10A2:
		internalformat = GL_RGB10_A2;
		type = GL_UNSIGNED_INT_2_10_10_10_REV;
		break;
	case FORMAT_RG11B10F:
		internalformat = GL_R11F_G11F_B10F;
		externalformat = GL_RGB;
		type = GL_UNSIGNED_INT_10F_11F_11F_REV;
		break;
	case FORMAT_R16F:
		internalformat = GL_R16F;
		externalformat = GL_RED;
		if (GLAD_OES_texture_half_float)
			type = GL_HALF_FLOAT_OES;
		else if (GLAD_VERSION_1_0)
			type = GL_FLOAT;
		else
			type = GL_HALF_FLOAT;
		break;
	case FORMAT_RG16F:
		internalformat = GL_RG16F;
		externalformat = GL_RG;
		if (GLAD_OES_texture_half_float)
			type = GL_HALF_FLOAT_OES;
		else if (GLAD_VERSION_1_0)
			type = GL_FLOAT;
		else
			type = GL_HALF_FLOAT;
		break;
	case FORMAT_RGBA16F:
		internalformat = GL_RGBA16F;
		if (GLAD_OES_texture_half_float)
			type = GL_HALF_FLOAT_OES;
		else if (GLAD_VERSION_1_0)
			type = GL_FLOAT;
		else
			type = GL_HALF_FLOAT;
		break;
	case FORMAT_R32F:
		internalformat = GL_R32F;
		externalformat = GL_RED;
		type = GL_FLOAT;
		break;
	case FORMAT_RG32F:
		internalformat = GL_RG32F;
		externalformat = GL_RG;
		type = GL_FLOAT;
		break;
	case FORMAT_RGBA32F:
		internalformat = GL_RGBA32F;
		type = GL_FLOAT;
		break;
	case FORMAT_SRGB:
		internalformat = GL_SRGB8_ALPHA8;
		type = GL_UNSIGNED_BYTE;
		if (GLAD_ES_VERSION_2_0 && !GLAD_ES_VERSION_3_0)
			externalformat = GL_SRGB_ALPHA;
		break;
	}
}

size_t Canvas::getFormatBitsPerPixel(Format format)
{
	switch (getSizedFormat(format))
	{
	case FORMAT_R8:
		return 8;
	case FORMAT_RGBA4:
	case FORMAT_RGB5A1:
	case FORMAT_RGB565:
	case FORMAT_RG8:
	case FORMAT_R16F:
		return 16;
	case FORMAT_RGBA8:
	case FORMAT_RGB10A2:
	case FORMAT_RG11B10F:
	case FORMAT_RG16F:
	case FORMAT_R32F:
	case FORMAT_SRGB:
	default:
		return 32;
	case FORMAT_RGBA16F:
	case FORMAT_RG32F:
		return 64;
	case FORMAT_RGBA32F:
		return 128;
	}
}

bool Canvas::isSupported()
{
	return GLAD_ES_VERSION_2_0 || GLAD_VERSION_3_0 || GLAD_ARB_framebuffer_object || GLAD_EXT_framebuffer_object;
}

bool Canvas::isMultiFormatMultiCanvasSupported()
{
	return gl.getMaxRenderTargets() > 1 && (GLAD_ES_VERSION_3_0 || GLAD_VERSION_3_0 || GLAD_ARB_framebuffer_object);
}

bool Canvas::supportedFormats[] = {false};
bool Canvas::checkedFormats[] = {false};

bool Canvas::isFormatSupported(Canvas::Format format)
{
	if (!isSupported())
		return false;

	bool supported = true;
	format = getSizedFormat(format);

	switch (format)
	{
	case FORMAT_RGBA4:
	case FORMAT_RGB5A1:
		supported = true;
		break;
	case FORMAT_RGB565:
		supported = GLAD_ES_VERSION_2_0 || GLAD_VERSION_4_2 || GLAD_ARB_ES2_compatibility;
		break;
	case FORMAT_R8:
	case FORMAT_RG8:
		if (GLAD_VERSION_1_0)
			supported = GLAD_VERSION_3_0 || GLAD_ARB_texture_rg;
		else if (GLAD_ES_VERSION_2_0)
			supported = GLAD_ES_VERSION_3_0 || GLAD_EXT_texture_rg;
		break;
	case FORMAT_RGBA8:
		supported = GLAD_VERSION_1_0 || GLAD_ES_VERSION_3_0 || GLAD_OES_rgb8_rgba8 || GLAD_ARM_rgba8;
		break;
	case FORMAT_RGB10A2:
		supported = GLAD_ES_VERSION_3_0 || GLAD_VERSION_1_0;
		break;
	case FORMAT_RG11B10F:
		supported = GLAD_VERSION_3_0 || GLAD_EXT_packed_float || GLAD_APPLE_color_buffer_packed_float;
		break;
	case FORMAT_R16F:
	case FORMAT_RG16F:
		if (GLAD_VERSION_1_0)
			supported = GLAD_VERSION_3_0 || (GLAD_ARB_texture_float && GLAD_ARB_texture_rg);
		else
			supported = GLAD_EXT_color_buffer_half_float && (GLAD_ES_VERSION_3_0 || (GLAD_OES_texture_half_float && GLAD_EXT_texture_rg));
		break;
	case FORMAT_RGBA16F:
		if (GLAD_VERSION_1_0)
			supported = GLAD_VERSION_3_0 || GLAD_ARB_texture_float;
		else if (GLAD_ES_VERSION_2_0)
			supported = GLAD_EXT_color_buffer_half_float && (GLAD_ES_VERSION_3_0 || GLAD_OES_texture_half_float);
		break;
	case FORMAT_R32F:
	case FORMAT_RG32F:
		supported = GLAD_VERSION_3_0 || (GLAD_ARB_texture_float && GLAD_ARB_texture_rg);
		break;
	case FORMAT_RGBA32F:
		supported = GLAD_VERSION_3_0 || GLAD_ARB_texture_float;
		break;
	case FORMAT_SRGB:
		if (GLAD_VERSION_1_0)
		{
			supported = GLAD_VERSION_3_0 || ((GLAD_ARB_framebuffer_sRGB || GLAD_EXT_framebuffer_sRGB)
				&& (GLAD_VERSION_2_1 || GLAD_EXT_texture_sRGB));
		}
		else
			supported = GLAD_ES_VERSION_3_0 || GLAD_EXT_sRGB;
		break;
	default:
		supported = false;
		break;
	}

	if (!supported)
		return false;

	if (checkedFormats[format])
		return supportedFormats[format];

	// Even though we might have the necessary OpenGL version or extension,
	// drivers are still allowed to throw FRAMEBUFFER_UNSUPPORTED when attaching
	// a texture to a FBO whose format the driver doesn't like. So we should
	// test with an actual FBO.

	GLenum internalformat = GL_RGBA;
	GLenum externalformat = GL_RGBA;
	GLenum textype = GL_UNSIGNED_BYTE;
	convertFormat(format, internalformat, externalformat, textype);

	// in GLES2, the internalformat and format params of TexImage have to match.
	if (GLAD_ES_VERSION_2_0 && !GLAD_ES_VERSION_3_0)
		internalformat = externalformat;

	GLuint texture = 0;
	glGenTextures(1, &texture);
	gl.bindTextureToUnit(texture, 0, false);

	Texture::Filter f;
	f.min = f.mag = Texture::FILTER_NEAREST;
	gl.setTextureFilter(f);

	Texture::Wrap w;
	gl.setTextureWrap(w);

	glTexImage2D(GL_TEXTURE_2D, 0, internalformat, 2, 2, 0, externalformat, textype, nullptr);

	GLuint fbo = 0;
	supported = (createFBO(fbo, texture) == GL_FRAMEBUFFER_COMPLETE);
	gl.deleteFramebuffer(fbo);

	gl.deleteTexture(texture);

	// Cache the result so we don't do this for every isFormatSupported call.
	checkedFormats[format] = true;
	supportedFormats[format] = supported;

	return supported;
}

bool Canvas::getConstant(const char *in, Format &out)
{
	return formats.find(in, out);
}

bool Canvas::getConstant(Format in, const char *&out)
{
	return formats.find(in, out);
}

StringMap<Canvas::Format, Canvas::FORMAT_MAX_ENUM>::Entry Canvas::formatEntries[] =
{
	{"normal", FORMAT_NORMAL},
	{"hdr", FORMAT_HDR},
	{"rgba4", FORMAT_RGBA4},
	{"rgb5a1", FORMAT_RGB5A1},
	{"rgb565", FORMAT_RGB565},
	{"r8", FORMAT_R8},
	{"rg8", FORMAT_RG8},
	{"rgba8", FORMAT_RGBA8},
	{"rgb10a2", FORMAT_RGB10A2},
	{"rg11b10f", FORMAT_RG11B10F},
	{"r16f", FORMAT_R16F},
	{"rg16f", FORMAT_RG16F},
	{"rgba16f", FORMAT_RGBA16F},
	{"r32f", FORMAT_R32F},
	{"rg32f", FORMAT_RG32F},
	{"rgba32f", FORMAT_RGBA32F},
	{"srgb", FORMAT_SRGB},
};

StringMap<Canvas::Format, Canvas::FORMAT_MAX_ENUM> Canvas::formats(Canvas::formatEntries, sizeof(Canvas::formatEntries));

} // opengl
} // graphics
} // love
