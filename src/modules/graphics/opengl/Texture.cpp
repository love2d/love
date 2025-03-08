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

#include "Texture.h"

#include "graphics/Graphics.h"
#include "Graphics.h"
#include "Buffer.h"
#include "common/int.h"

// STD
#include <algorithm> // for min/max

namespace love
{
namespace graphics
{
namespace opengl
{

static GLenum createFBO(GLuint &framebuffer, TextureType texType, PixelFormat format, GLuint texture, int mips, int layers, bool clear)
{
	// get currently bound fbo to reset to it later
	GLuint current_fbo = gl.getFramebuffer(OpenGL::FRAMEBUFFER_ALL);

	glGenFramebuffers(1, &framebuffer);
	gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, framebuffer);

	if (texture != 0)
	{
		if (isPixelFormatDepthStencil(format))
		{
			GLenum none = GL_NONE;
			glDrawBuffers(1, &none);
			glReadBuffer(GL_NONE);
		}

		OpenGL::TextureFormat fmt = OpenGL::convertPixelFormat(format);

		int faces = texType == TEXTURE_CUBE ? 6 : 1;

		// Make sure all faces and layers of the texture are initialized to
		// transparent black. This is unfortunately probably pretty slow for
		// 2D-array and 3D textures with a lot of layers...
		for (int mip = mips - 1; mip >= 0; mip--)
		{
			for (int layer = layers - 1; layer >= 0; layer--)
			{
				for (int face = faces - 1; face >= 0; face--)
				{
					for (GLenum attachment : fmt.framebufferAttachments)
					{
						if (attachment == GL_NONE)
							continue;

						gl.framebufferTexture(attachment, texType, texture, mip, layer, face);
					}

					if (clear && isPixelFormatInteger(format))
					{
						PixelFormatType datatype = getPixelFormatInfo(format).dataType;
						if (datatype == PIXELFORMATTYPE_SINT)
						{
							const GLint carray[] = { 0, 0, 0, 0 };
							glClearBufferiv(GL_COLOR, 0, carray);
						}
						else
						{
							const GLuint carray[] = { 0, 0, 0, 0 };
							glClearBufferuiv(GL_COLOR, 0, carray);
						}
					}
					else if (clear)
					{
						bool ds = isPixelFormatDepthStencil(format);

						GLbitfield clearflags = ds ? GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT : GL_COLOR_BUFFER_BIT;
						OpenGL::CleanClearState cleanClearState(clearflags);

						if (ds)
						{
							gl.clearDepth(1.0);
							glClearStencil(0);
						}
						else
						{
							glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
						}

						glClear(clearflags);
					}
				}
			}
		}
	}

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, current_fbo);

	return status;
}

static GLenum newRenderbuffer(int width, int height, int &samples, PixelFormat pixelformat, GLuint &buffer)
{
	OpenGL::TextureFormat fmt = OpenGL::convertPixelFormat(pixelformat);

	GLuint current_fbo = gl.getFramebuffer(OpenGL::FRAMEBUFFER_ALL);

	// Temporary FBO used to clear the renderbuffer.
	GLuint fbo = 0;
	glGenFramebuffers(1, &fbo);
	gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, fbo);

	if (isPixelFormatDepthStencil(pixelformat))
	{
		GLenum none = GL_NONE;
		glDrawBuffers(1, &none);
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
	{
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES, &samples);
		samples = std::max(1, samples);
	}

	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (status == GL_FRAMEBUFFER_COMPLETE)
	{
		if (isPixelFormatInteger(pixelformat))
		{
			PixelFormatType datatype = getPixelFormatInfo(pixelformat).dataType;
			if (datatype == PIXELFORMATTYPE_SINT)
			{
				const GLint carray[] = { 0, 0, 0, 0 };
				glClearBufferiv(GL_COLOR, 0, carray);
			}
			else
			{
				const GLuint carray[] = { 0, 0, 0, 0 };
				glClearBufferuiv(GL_COLOR, 0, carray);
			}
		}
		else
		{
			bool ds = isPixelFormatDepthStencil(pixelformat);

			GLbitfield clearflags = ds ? GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT : GL_COLOR_BUFFER_BIT;
			OpenGL::CleanClearState cleanClearState(clearflags);

			if (ds)
			{
				gl.clearDepth(1.0);
				glClearStencil(0);
			}
			else
			{
				// Initialize the buffer to transparent black.
				glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			}

			glClear(clearflags);
		}
	}
	else
	{
		glDeleteRenderbuffers(1, &buffer);
		buffer = 0;
		samples = 1;
	}

	gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, current_fbo);
	gl.deleteFramebuffer(fbo);

	return status;
}

Texture::Texture(love::graphics::Graphics *gfx, const Settings &settings, const Slices *data)
	: love::graphics::Texture(gfx, settings, data)
	, slices(settings.type)
	, fbo(0)
	, texture(0)
	, renderbuffer(0)
	, framebufferStatus(GL_FRAMEBUFFER_COMPLETE)
	, textureGLError(GL_NO_ERROR)
	, actualSamples(1)
{
	if (data != nullptr)
		slices = *data;

	if (!loadVolatile())
	{
		if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE)
			throw love::Exception("Cannot create Texture (OpenGL framebuffer error: %s)", OpenGL::framebufferStatusString(framebufferStatus));
		if (textureGLError != GL_NO_ERROR)
			throw love::Exception("Cannot create Texture (OpenGL error: %s)", OpenGL::errorString(textureGLError));
	}

	// ImageData is referenced by the first loadVolatile call, but we don't
	// hang on to it after that so we can save memory.
	slices.clear();
}

Texture::Texture(love::graphics::Graphics *gfx, love::graphics::Texture *base, const Texture::ViewSettings &viewsettings)
	: love::graphics::Texture(gfx, base, viewsettings)
	, slices(viewsettings.type.get(base->getTextureType()))
	, fbo(0)
	, texture(0)
	, renderbuffer(0)
	, framebufferStatus(GL_FRAMEBUFFER_COMPLETE)
	, textureGLError(GL_NO_ERROR)
	, actualSamples(1)
{
	if (!loadVolatile())
	{
		if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE)
			throw love::Exception("Cannot create texture view (OpenGL framebuffer error: %s)", OpenGL::framebufferStatusString(framebufferStatus));
		if (textureGLError != GL_NO_ERROR)
			throw love::Exception("Cannot create texture view (OpenGL error: %s)", OpenGL::errorString(textureGLError));
	}
}

Texture::~Texture()
{
	unloadVolatile();
}

void Texture::createTexture()
{
	// The base class handles some validation. For example, if ImageData is
	// given then it must exist for all mip levels, a render target can't use
	// a compressed format, etc.
	glGenTextures(1, &texture);
	GLenum gltype = OpenGL::getGLTextureType(texType);

	if (parentView.texture != this)
	{
		OpenGL::TextureFormat fmt = gl.convertPixelFormat(format);
		Texture *basetex = (Texture *) parentView.texture;
		int layers = texType == TEXTURE_CUBE ? 6 : getLayerCount();

		glTextureView(texture, gltype, basetex->texture, fmt.internalformat,
		              parentView.startMipmap, getMipmapCount(),
		              parentView.startLayer, layers);

		gl.bindTextureToUnit(this, 0, false);
		setSamplerState(samplerState);
		return;
	}

	gl.bindTextureToUnit(this, 0, false);

	if (renderTarget && GLAD_ANGLE_texture_usage)
		glTexParameteri(gltype, GL_TEXTURE_USAGE_ANGLE, GL_FRAMEBUFFER_ATTACHMENT_ANGLE);

	setSamplerState(samplerState);

	int mipcount = getMipmapCount();
	int slicecount = 1;

	if (texType == TEXTURE_VOLUME)
		slicecount = getDepth();
	else if (texType == TEXTURE_2D_ARRAY)
		slicecount = getLayerCount();
	else if (texType == TEXTURE_CUBE)
		slicecount = 6;

	// For a couple flimsy reasons, we don't initialize the texture here if it's
	// compressed. I need to verify that getPixelFormatSliceSize will return the
	// correct value for all compressed texture formats, and I also vaguely
	// remember some driver issues on some old Android systems, maybe...
	// For now, the base class enforces data on init for compressed textures.
	if (!isCompressed())
		gl.rawTexStorage(texType, mipcount, format, pixelWidth, pixelHeight, texType == TEXTURE_VOLUME ? depth : layers);

	// rawTexStorage handles this for uncompressed textures.
	if (isCompressed())
		glTexParameteri(gltype, GL_TEXTURE_MAX_LEVEL, mipcount - 1);

	int w = pixelWidth;
	int h = pixelHeight;
	int d = depth;

	OpenGL::TextureFormat fmt = gl.convertPixelFormat(format);

	for (int mip = 0; mip < mipcount; mip++)
	{
		if (isCompressed() && (texType == TEXTURE_2D_ARRAY || texType == TEXTURE_VOLUME))
		{
			int slicecount = slices.getSliceCount(mip);
			size_t mipsize = 0;

			for (int slice = 0; slice < slicecount; slice++)
			{
				auto id = slices.get(slice, mip);
				if (id != nullptr)
					mipsize += id->getSize();
			}

			if (mipsize > 0)
				glCompressedTexImage3D(gltype, mip, fmt.internalformat, w, h, slicecount, 0, mipsize, nullptr);
		}

		for (int slice = 0; slice < slicecount; slice++)
		{
			love::image::ImageDataBase *id = slices.get(slice, mip);
			if (id != nullptr)
				uploadImageData(id, mip, slice, 0, 0);
		}

		w = std::max(w / 2, 1);
		h = std::max(h / 2, 1);

		if (texType == TEXTURE_VOLUME)
			d = std::max(d / 2, 1);
	}

	bool hasdata = slices.get(0, 0) != nullptr;

	// All mipmap levels need to be initialized - for color formats we can clear
	// the base mip and use glGenerateMipmap after that's done. Depth and
	// stencil formats don't always support glGenerateMipmap so we need to
	// individually clear each mip level in that case. We avoid doing that for
	// color formats because of an Intel driver bug:
	// https://github.com/love2d/love/issues/1585
	int clearmips = 1;
	if (isPixelFormatDepthStencil(format))
		clearmips = mipmapCount;

	// Create a local FBO used for glReadPixels as well as MSAA blitting.
	if (isRenderTarget())
	{
		bool clear = !hasdata;
		int slices = texType == TEXTURE_VOLUME ? depth : layers;
		framebufferStatus = createFBO(fbo, texType, format, texture, clearmips, slices, clear);
	}
	else if (!hasdata)
	{
		// Initialize all slices to transparent black.
		for (int mip = 0; mip < clearmips; mip++)
		{
			int mipw = getPixelWidth(mip);
			int miph = getPixelHeight(mip);
			std::vector<uint8> emptydata(getPixelFormatSliceSize(format, mipw, miph));

			Rect r = {0, 0, mipw, miph};
			int slices = texType == TEXTURE_VOLUME ? getDepth(mip) : layers;
			slices = texType == TEXTURE_CUBE ? 6 : slices;
			for (int i = 0; i < slices; i++)
				uploadByteData(emptydata.data(), emptydata.size(), mip, i, r);
		}
	}

	// Non-readable textures can't have mipmaps (enforced in the base class),
	// so generateMipmaps here is fine - when they aren't already initialized.
	if (clearmips < mipmapCount && slices.getMipmapCount() <= 1 && getMipmapsMode() != MIPMAPS_NONE)
		generateMipmaps();
}

bool Texture::loadVolatile()
{
	if (texture != 0 || renderbuffer != 0)
		return true;

	if (parentView.texture != this)
	{
		Texture *basetex = (Texture *) parentView.texture;
		basetex->loadVolatile();
	}

	OpenGL::TempDebugGroup debuggroup("Texture load");

	actualSamples = std::max(1, std::min(getRequestedMSAA(), gl.getMaxSamples()));

	while (glGetError() != GL_NO_ERROR); // Clear errors.

	framebufferStatus = GL_FRAMEBUFFER_COMPLETE;
	textureGLError = GL_NO_ERROR;

	if (isReadable())
		createTexture();

	if (framebufferStatus == GL_FRAMEBUFFER_COMPLETE && (!isReadable() || actualSamples > 1))
		framebufferStatus = newRenderbuffer(pixelWidth, pixelHeight, actualSamples, format, renderbuffer);

	textureGLError = glGetError();

	if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE || textureGLError != GL_NO_ERROR)
	{
		unloadVolatile();
		return false;
	}

	if (!debugName.empty() && (GLAD_VERSION_4_3 || GLAD_ES_VERSION_3_2))
	{
		if (texture)
			glObjectLabel(GL_TEXTURE, texture, -1, debugName.c_str());

		if (renderbuffer)
		{
			std::string rname = debugName;
			if (actualSamples > 1)
				rname += " (MSAA buffer)";
			glObjectLabel(GL_RENDERBUFFER, renderbuffer, -1, rname.c_str());
		}
	}

	if (parentView.texture == this)
		updateGraphicsMemorySize(true);

	return true;
}

void Texture::unloadVolatile()
{
	if (isRenderTarget() && (fbo != 0 || renderbuffer != 0 || texture != 0))
	{
		// This is a bit ugly, but we need some way to destroy the cached FBO
		// when this texture's GL object is destroyed.
		auto gfx = Module::getInstance<Graphics>(Module::M_GRAPHICS);
		if (gfx != nullptr)
			gfx->cleanupRenderTexture(this);
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

	updateGraphicsMemorySize(false);
}

void Texture::uploadByteData(const void *data, size_t size, int level, int slice, const Rect &r)
{
	OpenGL::TempDebugGroup debuggroup("Texture data upload");

	gl.bindTextureToUnit(this, 0, false);

	OpenGL::TextureFormat fmt = OpenGL::convertPixelFormat(format);
	GLenum gltarget = OpenGL::getGLTextureType(texType);

	if (texType == TEXTURE_CUBE)
		gltarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + slice;

	if (isPixelFormatCompressed(format))
	{
		if (texType == TEXTURE_2D || texType == TEXTURE_CUBE)
		{
			// Possible issues on some very old drivers if TexSubImage is used.
			if (r.x != 0 || r.y != 0 || r.w != getPixelWidth(level) || r.h != getPixelHeight(level))
				glCompressedTexSubImage2D(gltarget, level, r.x, r.y, r.w, r.h, fmt.internalformat, size, data);
			else
				glCompressedTexImage2D(gltarget, level, fmt.internalformat, r.w, r.h, 0, size, data);
		}
		else if (texType == TEXTURE_2D_ARRAY || texType == TEXTURE_VOLUME)
			glCompressedTexSubImage3D(gltarget, level, r.x, r.y, slice, r.w, r.h, 1, fmt.internalformat, size, data);
	}
	else
	{
		if (texType == TEXTURE_2D || texType == TEXTURE_CUBE)
			glTexSubImage2D(gltarget, level, r.x, r.y, r.w, r.h, fmt.externalformat, fmt.type, data);
		else if (texType == TEXTURE_2D_ARRAY || texType == TEXTURE_VOLUME)
			glTexSubImage3D(gltarget, level, r.x, r.y, slice, r.w, r.h, 1, fmt.externalformat, fmt.type, data);
	}
}

void Texture::generateMipmapsInternal()
{
	gl.bindTextureToUnit(this, 0, false);

	GLenum gltextype = OpenGL::getGLTextureType(texType);

	if (gl.bugs.generateMipmapsRequiresTexture2DEnable)
		glEnable(gltextype);

	glGenerateMipmap(gltextype);
}

void Texture::readbackInternal(int slice, int mipmap, const Rect &rect, int destwidth, size_t size, void *dest)
{
	// Not supported in GL with compressed textures...
	if (!isCompressed())
		glPixelStorei(GL_PACK_ROW_LENGTH, destwidth);

	gl.bindTextureToUnit(this, 0, false);

	OpenGL::TextureFormat fmt = gl.convertPixelFormat(format);

	if (gl.isCopyTextureToBufferSupported())
	{
		if (isCompressed())
			glGetCompressedTextureSubImage(texture, mipmap, rect.x, rect.y, slice, rect.w, rect.h, 1, size, dest);
		else
			glGetTextureSubImage(texture, mipmap, rect.x, rect.y, slice, rect.w, rect.h, 1, fmt.externalformat, fmt.type, size, dest);
	}
	else if (fbo)
	{
		GLuint current_fbo = gl.getFramebuffer(OpenGL::FRAMEBUFFER_ALL);
		gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, getFBO());

		if (slice > 0 || mipmap > 0)
		{
			int layer = texType == TEXTURE_CUBE ? 0 : slice;
			int face = texType == TEXTURE_CUBE ? slice : 0;
			gl.framebufferTexture(GL_COLOR_ATTACHMENT0, texType, texture, mipmap, layer, face);
		}

		glReadPixels(rect.x, rect.y, rect.w, rect.h, fmt.externalformat, fmt.type, dest);

		if (slice > 0 || mipmap > 0)
			gl.framebufferTexture(GL_COLOR_ATTACHMENT0, texType, texture, 0, 0, 0);

		gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, current_fbo);
	}

	if (!isCompressed())
		glPixelStorei(GL_PACK_ROW_LENGTH, 0);
}

void Texture::copyFromBuffer(love::graphics::Buffer *source, size_t sourceoffset, int sourcewidth, size_t size, int slice, int mipmap, const Rect &rect)
{
	// Higher level code does validation.

	GLuint glbuffer = (GLuint) source->getHandle();
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, glbuffer);

	if (!isCompressed()) // Not supported in GL with compressed textures...
		glPixelStorei(GL_UNPACK_ROW_LENGTH, sourcewidth);

	// glTexSubImage and friends copy from the active pixel_unpack_buffer by
	// treating the pointer as a byte offset.
	const uint8 *byteoffset = (const uint8 *)(ptrdiff_t)sourceoffset;
	uploadByteData(byteoffset, size, mipmap, slice, rect);

	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

void Texture::copyToBuffer(love::graphics::Buffer *dest, int slice, int mipmap, const Rect &rect, size_t destoffset, int destwidth, size_t size)
{
	// Higher level code does validation.

	GLuint glbuffer = (GLuint) dest->getHandle();
	glBindBuffer(GL_PIXEL_PACK_BUFFER, glbuffer);

	// glTexSubImage and friends copy to the active PIXEL_PACK_BUFFER by
	// treating the pointer as a byte offset.
	uint8 *byteoffset = (uint8 *)(ptrdiff_t)destoffset;

	readbackInternal(slice, mipmap, rect, destwidth, size, byteoffset);

	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

void Texture::setSamplerState(const SamplerState &s)
{
	samplerState = validateSamplerState(s);

	gl.bindTextureToUnit(this, 0, false);
	gl.setSamplerState(texType, samplerState);
}

ptrdiff_t Texture::getHandle() const
{
	return texture;
}

ptrdiff_t Texture::getRenderTargetHandle() const
{
	return renderTarget ? (renderbuffer != 0 ? renderbuffer : texture) : 0;
}

} // opengl
} // graphics
} // love
