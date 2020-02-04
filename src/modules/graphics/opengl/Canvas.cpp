/**
 * Copyright (c) 2006-2020 LOVE Development Team
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
#include "graphics/Graphics.h"
#include "Graphics.h"

#include <algorithm> // For min/max

namespace love
{
namespace graphics
{
namespace opengl
{

static GLenum createFBO(GLuint &framebuffer, TextureType texType, PixelFormat format, GLuint texture, int layers)
{
	// get currently bound fbo to reset to it later
	GLuint current_fbo = gl.getFramebuffer(OpenGL::FRAMEBUFFER_ALL);

	glGenFramebuffers(1, &framebuffer);
	gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, framebuffer);

	if (texture != 0)
	{
		if (isPixelFormatDepthStencil(format) && (GLAD_ES_VERSION_3_0 || !GLAD_ES_VERSION_2_0))
		{
			// glDrawBuffers is an ext in GL2. glDrawBuffer doesn't exist in ES3.
			GLenum none = GL_NONE;
			if (GLAD_ES_VERSION_3_0)
				glDrawBuffers(1, &none);
			else
				glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}

		bool unusedSRGB = false;
		OpenGL::TextureFormat fmt = OpenGL::convertPixelFormat(format, false, unusedSRGB);

		int faces = texType == TEXTURE_CUBE ? 6 : 1;

		// Make sure all faces and layers of the texture are initialized to
		// transparent black. This is unfortunately probably pretty slow for
		// 2D-array and 3D textures with a lot of layers...
		for (int layer = layers - 1; layer >= 0; layer--)
		{
			for (int face = faces - 1; face >= 0; face--)
			{
				for (GLenum attachment : fmt.framebufferAttachments)
				{
					if (attachment == GL_NONE)
						continue;

					gl.framebufferTexture(attachment, texType, texture, 0, layer, face);
				}

				if (isPixelFormatDepthStencil(format))
				{
					bool hadDepthWrites = gl.hasDepthWrites();
					if (!hadDepthWrites) // glDepthMask also affects glClear.
						gl.setDepthWrites(true);

					gl.clearDepth(1.0);
					glClearStencil(0);
					glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

					if (!hadDepthWrites)
						gl.setDepthWrites(hadDepthWrites);
				}
				else
				{
					glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
					glClear(GL_COLOR_BUFFER_BIT);
				}
			}
		}
	}

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, current_fbo);
	return status;
}

static bool createRenderbuffer(int width, int height, int &samples, PixelFormat pixelformat, GLuint &buffer)
{
	int reqsamples = samples;
	bool unusedSRGB = false;
	OpenGL::TextureFormat fmt = OpenGL::convertPixelFormat(pixelformat, true, unusedSRGB);

	GLuint current_fbo = gl.getFramebuffer(OpenGL::FRAMEBUFFER_ALL);

	// Temporary FBO used to clear the renderbuffer.
	GLuint fbo = 0;
	glGenFramebuffers(1, &fbo);
	gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, fbo);

	if (isPixelFormatDepthStencil(pixelformat) && (GLAD_ES_VERSION_3_0 || !GLAD_ES_VERSION_2_0))
	{
		// glDrawBuffers is an ext in GL2. glDrawBuffer doesn't exist in ES3.
		GLenum none = GL_NONE;
		if (GLAD_ES_VERSION_3_0)
			glDrawBuffers(1, &none);
		else
			glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	glGenRenderbuffers(1, &buffer);
	glBindRenderbuffer(GL_RENDERBUFFER, buffer);

	if (samples > 1)
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, fmt.internalformat, width, height);
	else
		glRenderbufferStorage(GL_RENDERBUFFER, fmt.internalformat, width, height);

	for (GLenum attachment : fmt.framebufferAttachments)
	{
		if (attachment != GL_NONE)
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, buffer);
	}

	if (samples > 1)
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES, &samples);
	else
		samples = 0;

	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (status == GL_FRAMEBUFFER_COMPLETE && (reqsamples <= 1 || samples > 1))
	{
		if (isPixelFormatDepthStencil(pixelformat))
		{
			bool hadDepthWrites = gl.hasDepthWrites();
			if (!hadDepthWrites) // glDepthMask also affects glClear.
				gl.setDepthWrites(true);

			gl.clearDepth(1.0);
			glClearStencil(0);
			glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			if (!hadDepthWrites)
				gl.setDepthWrites(hadDepthWrites);
		}
		else
		{
			// Initialize the buffer to transparent black.
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);
		}
	}
	else
	{
		glDeleteRenderbuffers(1, &buffer);
		buffer = 0;
		samples = 0;
	}

	gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, current_fbo);
	gl.deleteFramebuffer(fbo);

	return status == GL_FRAMEBUFFER_COMPLETE;
}

Canvas::Canvas(const Settings &settings)
	: love::graphics::Canvas(settings)
	, fbo(0)
	, texture(0)
    , renderbuffer(0)
	, actualSamples(0)
{
	auto gfx = Module::getInstance<Graphics>(Module::M_GRAPHICS);
	if (gfx != nullptr)
		format = gfx->getSizedFormat(format, renderTarget, readable, sRGB);

	initQuad();
	loadVolatile();

	if (status != GL_FRAMEBUFFER_COMPLETE)
		throw love::Exception("Cannot create Canvas: %s", OpenGL::framebufferStatusString(status));
}

Canvas::~Canvas()
{
	unloadVolatile();
}

bool Canvas::loadVolatile()
{
	if (texture != 0)
		return true;

	OpenGL::TempDebugGroup debuggroup("Canvas load");

	fbo = texture = 0;
	renderbuffer = 0;
	status = GL_FRAMEBUFFER_COMPLETE;

	// getMaxRenderbufferSamples will be 0 on systems that don't support
	// multisampled renderbuffers / don't export FBO multisample extensions.
	actualSamples = std::min(getRequestedMSAA(), gl.getMaxRenderbufferSamples());
	actualSamples = std::max(actualSamples, 0);
	actualSamples = actualSamples == 1 ? 0 : actualSamples;

	if (isReadable())
	{
		glGenTextures(1, &texture);
		gl.bindTextureToUnit(this, 0, false);

		GLenum gltype = OpenGL::getGLTextureType(texType);

		if (GLAD_ANGLE_texture_usage)
			glTexParameteri(gltype, GL_TEXTURE_USAGE_ANGLE, GL_FRAMEBUFFER_ATTACHMENT_ANGLE);

		setSamplerState(samplerState);

		while (glGetError() != GL_NO_ERROR)
			/* Clear the error buffer. */;

		bool isSRGB = format == PIXELFORMAT_sRGBA8_UNORM;
		if (!gl.rawTexStorage(texType, mipmapCount, format, isSRGB, pixelWidth, pixelHeight, texType == TEXTURE_VOLUME ? depth : layers))
		{
			status = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
			return false;
		}

		if (glGetError() != GL_NO_ERROR)
		{
			gl.deleteTexture(texture);
			texture = 0;
			status = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
			return false;
		}

		// Create a local FBO used for glReadPixels as well as MSAA blitting.
		status = createFBO(fbo, texType, format, texture, texType == TEXTURE_VOLUME ? depth : layers);

		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			if (fbo != 0)
			{
				gl.deleteFramebuffer(fbo);
				fbo = 0;
			}
			return false;
		}
	}

	if (!isReadable() || actualSamples > 0)
		createRenderbuffer(pixelWidth, pixelHeight, actualSamples, format, renderbuffer);

	int64 memsize = getPixelFormatSize(format) * pixelWidth * pixelHeight;
	if (getMipmapCount() > 1)
		memsize *= 1.33334;

	if (actualSamples > 1 && isReadable())
		memsize += getPixelFormatSize(format) * pixelWidth * pixelHeight * actualSamples;
	else if (actualSamples > 1)
		memsize *= actualSamples;

	setGraphicsMemorySize(memsize);

	if (getMipmapCount() > 1)
		generateMipmaps();

	return true;
}

void Canvas::unloadVolatile()
{
	if (fbo != 0 || renderbuffer != 0 || texture != 0)
	{
		// This is a bit ugly, but we need some way to destroy the cached FBO
		// when this Canvas' texture is destroyed.
		auto gfx = Module::getInstance<Graphics>(Module::M_GRAPHICS);
		if (gfx != nullptr)
			gfx->cleanupCanvas(this);
	}

	if (fbo != 0)
		gl.deleteFramebuffer(fbo);

	if (renderbuffer != 0)
		glDeleteRenderbuffers(1, &renderbuffer);

	if (texture != 0)
		gl.deleteTexture(texture);

	fbo = 0;
	renderbuffer = 0;
	texture = 0;

	setGraphicsMemorySize(0);
}

void Canvas::setSamplerState(const SamplerState &s)
{
	Texture::setSamplerState(s);

	if (samplerState.depthSampleMode.hasValue && !gl.isDepthCompareSampleSupported())
		throw love::Exception("Depth comparison sampling in shaders is not supported on this system.");

	if (!OpenGL::hasTextureFilteringSupport(getPixelFormat()))
	{
		samplerState.magFilter = samplerState.minFilter = SamplerState::FILTER_NEAREST;

		if (samplerState.mipmapFilter == SamplerState::MIPMAP_FILTER_LINEAR)
			samplerState.mipmapFilter = SamplerState::MIPMAP_FILTER_NEAREST;
	}

	// If we only have limited NPOT support then the wrap mode must be CLAMP.
	if ((GLAD_ES_VERSION_2_0 && !(GLAD_ES_VERSION_3_0 || GLAD_OES_texture_npot))
		&& (pixelWidth != nextP2(pixelWidth) || pixelHeight != nextP2(pixelHeight) || depth != nextP2(depth)))
	{
		samplerState.wrapU = samplerState.wrapV = samplerState.wrapW = SamplerState::WRAP_CLAMP;
	}

	gl.bindTextureToUnit(this, 0, false);
	gl.setSamplerState(texType, samplerState);
}

ptrdiff_t Canvas::getHandle() const
{
	return texture;
}

love::image::ImageData *Canvas::newImageData(love::image::Image *module, int slice, int mipmap, const Rect &r)
{
	love::image::ImageData *data = love::graphics::Canvas::newImageData(module, slice, mipmap, r);

	bool isSRGB = false;
	OpenGL::TextureFormat fmt = gl.convertPixelFormat(data->getFormat(), false, isSRGB);

	GLuint current_fbo = gl.getFramebuffer(OpenGL::FRAMEBUFFER_ALL);
	gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, getFBO());

	if (slice > 0 || mipmap > 0)
	{
		int layer = texType == TEXTURE_CUBE ? 0 : slice;
		int face = texType == TEXTURE_CUBE ? slice : 0;
		gl.framebufferTexture(GL_COLOR_ATTACHMENT0, texType, texture, mipmap, layer, face);
	}

	glReadPixels(r.x, r.y, r.w, r.h, fmt.externalformat, fmt.type, data->getData());

	if (slice > 0 || mipmap > 0)
		gl.framebufferTexture(GL_COLOR_ATTACHMENT0, texType, texture, 0, 0, 0);

	gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, current_fbo);

	return data;
}

void Canvas::generateMipmaps()
{
	if (getMipmapCount() == 1 || getMipmapsMode() == MIPMAPS_NONE)
		throw love::Exception("generateMipmaps can only be called on a Texture which was created with mipmaps enabled.");

	if (isPixelFormatCompressed(format))
		throw love::Exception("generateMipmaps cannot be called on a compressed Texture.");

	gl.bindTextureToUnit(this, 0, false);

	GLenum gltextype = OpenGL::getGLTextureType(texType);

	if (gl.bugs.generateMipmapsRequiresTexture2DEnable)
		glEnable(gltextype);

	glGenerateMipmap(gltextype);
}

} // opengl
} // graphics
} // love
