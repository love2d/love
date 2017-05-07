/**
 * Copyright (c) 2006-2017 LOVE Development Team
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

#include <algorithm> // For min/max

namespace love
{
namespace graphics
{
namespace opengl
{

static GLenum createFBO(GLuint &framebuffer, TextureType texType, PixelFormat format, GLuint texture, int layers, int mips)
{
	// get currently bound fbo to reset to it later
	GLuint current_fbo = gl.getFramebuffer(OpenGL::FRAMEBUFFER_ALL);

	glGenFramebuffers(1, &framebuffer);
	gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, framebuffer);

	if (texture != 0)
	{
		bool unusedSRGB = false;
		OpenGL::TextureFormat fmt = OpenGL::convertPixelFormat(format, false, unusedSRGB);

		int faces = texType == TEXTURE_CUBE ? 6 : 1;

		// Make sure all faces and layers of the texture are initialized to
		// transparent black. This is unfortunately probably pretty slow for
		// 2D-array and 3D textures with a lot of layers...
		for (int mip = mips - 1; mip >= 0; mip--)
		{
			int nlayers = layers;
			if (texType == TEXTURE_VOLUME)
				nlayers = std::max(layers >> mip, 1);

			for (int layer = nlayers - 1; layer >= 0; layer--)
			{
				for (int face = faces - 1; face >= 0; face--)
				{
					for (GLenum attachment : fmt.framebufferAttachments)
					{
						if (attachment == GL_NONE)
							continue;

						gl.framebufferTexture(attachment, texType, texture, mip, layer, face);
					}

					if (isPixelFormatDepthStencil(format))
					{
						glClearDepth(1.0);
						glClearStencil(0);
						glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
					}
					else
					{
						glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
						glClear(GL_COLOR_BUFFER_BIT);
					}
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
			glClearDepth(1.0);
			glClearStencil(0);
			glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
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
	format = getSizedFormat(format);

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

	if (!Canvas::isSupported())
		throw love::Exception("Canvases are not supported by your OpenGL drivers!");

	if (!Canvas::isFormatSupported(format, readable))
	{
		const char *fstr = "rgba8";
		const char *readablestr = "";
		if (readable != !isPixelFormatDepthStencil(format))
			readablestr = readable ? " readable" : " non-readable";
		love::getConstant(Canvas::getSizedFormat(format), fstr);
		throw love::Exception("The %s%s canvas format is not supported by your OpenGL drivers.", fstr, readablestr);
	}

	if (getRequestedMSAA() > 1 && texType != TEXTURE_2D)
		throw love::Exception("MSAA is only supported for 2D texture types.");

	if (!readable && texType != TEXTURE_2D)
		throw love::Exception("Non-readable pixel formats are only supported for 2D texture types.");

	if (!gl.isTextureTypeSupported(texType))
	{
		const char *textypestr = "unknown";
		Texture::getConstant(texType, textypestr);
		throw love::Exception("%s textures are not supported on this system!", textypestr);
	}

	switch (texType)
	{
	case TEXTURE_2D:
		if (pixelWidth > gl.getMax2DTextureSize())
			throw TextureTooLargeException("width", pixelWidth);
		else if (pixelHeight > gl.getMax2DTextureSize())
			throw TextureTooLargeException("height", pixelHeight);
		break;
	case TEXTURE_VOLUME:
		if (pixelWidth > gl.getMax3DTextureSize())
			throw TextureTooLargeException("width", pixelWidth);
		else if (pixelHeight > gl.getMax3DTextureSize())
			throw TextureTooLargeException("height", pixelHeight);
		else if (depth > gl.getMax3DTextureSize())
			throw TextureTooLargeException("depth", depth);
		break;
	case TEXTURE_2D_ARRAY:
		if (pixelWidth > gl.getMax2DTextureSize())
			throw TextureTooLargeException("width", pixelWidth);
		else if (pixelHeight > gl.getMax2DTextureSize())
			throw TextureTooLargeException("height", pixelHeight);
		else if (layers > gl.getMaxTextureLayers())
			throw TextureTooLargeException("array layer count", layers);
		break;
	case TEXTURE_CUBE:
		if (pixelWidth != pixelHeight)
			throw love::Exception("Cubemap textures must have equal width and height.");
		else if (pixelWidth > gl.getMaxCubeTextureSize())
			throw TextureTooLargeException("width", pixelWidth);
		break;
	default:
		break;
	}

	OpenGL::TempDebugGroup debuggroup("Canvas load");

	fbo = texture = 0;
	renderbuffer = 0;
	status = GL_FRAMEBUFFER_COMPLETE;

	// getMaxRenderbufferSamples will be 0 on systems that don't support
	// multisampled renderbuffers / don't export FBO multisample extensions.
	actualSamples = getRequestedMSAA();
	actualSamples = std::min(actualSamples, gl.getMaxRenderbufferSamples());
	actualSamples = std::max(actualSamples, 0);
	actualSamples = actualSamples == 1 ? 0 : actualSamples;

	if (isReadable())
	{
		glGenTextures(1, &texture);
		gl.bindTextureToUnit(this, 0, false);

		GLenum gltype = OpenGL::getGLTextureType(texType);

		if (GLAD_ANGLE_texture_usage)
			glTexParameteri(gltype, GL_TEXTURE_USAGE_ANGLE, GL_FRAMEBUFFER_ATTACHMENT_ANGLE);

		setFilter(filter);
		setWrap(wrap);
		setMipmapSharpness(mipmapSharpness);
		setDepthSampleMode(depthCompareMode);

		while (glGetError() != GL_NO_ERROR)
			/* Clear the error buffer. */;

		bool isSRGB = format == PIXELFORMAT_sRGBA8;
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

		// Create a canvas-local FBO used for glReadPixels as well as MSAA blitting.
		status = createFBO(fbo, texType, format, texture, texType == TEXTURE_VOLUME ? depth : layers, mipmapCount);

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

	return true;
}

void Canvas::unloadVolatile()
{
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

void Canvas::setFilter(const Texture::Filter &f)
{
	Texture::setFilter(f);

	if (!OpenGL::hasTextureFilteringSupport(getPixelFormat()))
	{
		filter.mag = filter.min = FILTER_NEAREST;

		if (filter.mipmap == FILTER_LINEAR)
			filter.mipmap = FILTER_NEAREST;
	}

	gl.bindTextureToUnit(this, 0, false);
	gl.setTextureFilter(texType, filter);
}

bool Canvas::setWrap(const Texture::Wrap &w)
{
	Graphics::flushStreamDrawsGlobal();

	bool success = true;
	bool forceclamp = texType == TEXTURE_CUBE;
	wrap = w;

	// If we only have limited NPOT support then the wrap mode must be CLAMP.
	if ((GLAD_ES_VERSION_2_0 && !(GLAD_ES_VERSION_3_0 || GLAD_OES_texture_npot))
		&& (pixelWidth != nextP2(pixelWidth) || pixelHeight != nextP2(pixelHeight) || depth != nextP2(depth)))
	{
		forceclamp = true;
	}

	if (forceclamp)
	{
		if (wrap.s != WRAP_CLAMP || wrap.t != WRAP_CLAMP || wrap.r != WRAP_CLAMP)
			success = false;

		wrap.s = wrap.t = wrap.r = WRAP_CLAMP;
	}

	if (!gl.isClampZeroTextureWrapSupported())
	{
		if (wrap.s == WRAP_CLAMP_ZERO) wrap.s = WRAP_CLAMP;
		if (wrap.t == WRAP_CLAMP_ZERO) wrap.t = WRAP_CLAMP;
		if (wrap.r == WRAP_CLAMP_ZERO) wrap.r = WRAP_CLAMP;
	}

	gl.bindTextureToUnit(this, 0, false);
	gl.setTextureWrap(texType, wrap);

	return success;
}

bool Canvas::setMipmapSharpness(float sharpness)
{
	if (!gl.isSamplerLODBiasSupported())
		return false;

	Graphics::flushStreamDrawsGlobal();

	float maxbias = gl.getMaxLODBias();
	if (maxbias > 0.01f)
		maxbias -= 0.0f;

	mipmapSharpness = std::min(std::max(sharpness, -maxbias), maxbias);

	gl.bindTextureToUnit(this, 0, false);

	// negative bias is sharper
	glTexParameterf(gl.getGLTextureType(texType), GL_TEXTURE_LOD_BIAS, -mipmapSharpness);

	return true;
}

void Canvas::setDepthSampleMode(Optional<CompareMode> mode)
{
	Texture::setDepthSampleMode(mode);

	bool supported = gl.isDepthCompareSampleSupported();

	if (mode.hasValue)
	{
		if (!supported)
			throw love::Exception("Depth comparison sampling in shaders is not supported on this system.");

		Graphics::flushStreamDrawsGlobal();

		gl.bindTextureToUnit(texType, texture, 0, false);
		GLenum gltextype = OpenGL::getGLTextureType(texType);

		// See the comment in depthstencil.h
		GLenum glmode = OpenGL::getGLCompareMode(getReversedCompareMode(mode.value));

		glTexParameteri(gltextype, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(gltextype, GL_TEXTURE_COMPARE_FUNC, glmode);
		
	}
	else if (isPixelFormatDepth(format) && supported)
	{
		Graphics::flushStreamDrawsGlobal();

		gl.bindTextureToUnit(texType, texture, 0, false);
		GLenum gltextype = OpenGL::getGLTextureType(texType);

		glTexParameteri(gltextype, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	}

	depthCompareMode = mode;
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
	if (getMipmapCount() == 1 || getMipmapMode() == MIPMAPS_NONE)
		throw love::Exception("generateMipmaps can only be called on a Canvas which was created with mipmaps enabled.");

	gl.bindTextureToUnit(this, 0, false);

	GLenum gltextype = OpenGL::getGLTextureType(texType);

	if (gl.bugs.generateMipmapsRequiresTexture2DEnable)
		glEnable(gltextype);

	glGenerateMipmap(gltextype);
}

PixelFormat Canvas::getSizedFormat(PixelFormat format)
{
	switch (format)
	{
	case PIXELFORMAT_NORMAL:
		if (isGammaCorrect())
			return PIXELFORMAT_sRGBA8;
		else if (!OpenGL::isPixelFormatSupported(PIXELFORMAT_RGBA8, true, true, false))
			// 32-bit render targets don't have guaranteed support on GLES2.
			return PIXELFORMAT_RGBA4;
		else
			return PIXELFORMAT_RGBA8;
	case PIXELFORMAT_HDR:
		return PIXELFORMAT_RGBA16F;
	default:
		return format;
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

Canvas::SupportedFormat Canvas::supportedFormats[] = {};
Canvas::SupportedFormat Canvas::checkedFormats[] = {};

bool Canvas::isFormatSupported(PixelFormat format)
{
	return isFormatSupported(format, !isPixelFormatDepthStencil(format));
}

bool Canvas::isFormatSupported(PixelFormat format, bool readable)
{
	if (!isSupported())
		return false;

	const char *fstr = "?";
	love::getConstant(format, fstr);

	bool supported = true;
	format = getSizedFormat(format);

	if (!OpenGL::isPixelFormatSupported(format, true, readable, false))
		return false;

	if (checkedFormats[format].get(readable))
		return supportedFormats[format].get(readable);

	// Even though we might have the necessary OpenGL version or extension,
	// drivers are still allowed to throw FRAMEBUFFER_UNSUPPORTED when attaching
	// a texture to a FBO whose format the driver doesn't like. So we should
	// test with an actual FBO.
	GLuint texture = 0;
	GLuint renderbuffer = 0;

	bool unusedSRGB = false;
	OpenGL::TextureFormat fmt = OpenGL::convertPixelFormat(format, readable, unusedSRGB);

	GLuint current_fbo = gl.getFramebuffer(OpenGL::FRAMEBUFFER_ALL);

	GLuint fbo = 0;
	glGenFramebuffers(1, &fbo);
	gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, fbo);

	// Make sure at least something is bound to a color attachment. I believe
	// this is required on ES2 but I'm not positive.
	if (isPixelFormatDepthStencil(format))
		gl.framebufferTexture(GL_COLOR_ATTACHMENT0, TEXTURE_2D, gl.getDefaultTexture(TEXTURE_2D), 0, 0, 0);

	if (readable)
	{
		glGenTextures(1, &texture);
		gl.bindTextureToUnit(TEXTURE_2D, texture, 0, false);

		Texture::Filter f;
		f.min = f.mag = Texture::FILTER_NEAREST;
		gl.setTextureFilter(TEXTURE_2D, f);

		Texture::Wrap w;
		gl.setTextureWrap(TEXTURE_2D, w);

		unusedSRGB = false;
		gl.rawTexStorage(TEXTURE_2D, 1, format, unusedSRGB, 1, 1);
	}
	else
	{
		glGenRenderbuffers(1, &renderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, fmt.internalformat, 1, 1);
	}

	for (GLenum attachment : fmt.framebufferAttachments)
	{
		if (attachment == GL_NONE)
			continue;

		if (readable)
			gl.framebufferTexture(attachment, TEXTURE_2D, texture, 0, 0, 0);
		else
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, renderbuffer);
	}

	supported = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

	gl.bindFramebuffer(OpenGL::FRAMEBUFFER_ALL, current_fbo);
	gl.deleteFramebuffer(fbo);

	if (texture != 0)
		gl.deleteTexture(texture);

	if (renderbuffer != 0)
		glDeleteRenderbuffers(1, &renderbuffer);

	// Cache the result so we don't do this for every isFormatSupported call.
	checkedFormats[format].set(readable, true);
	supportedFormats[format].set(readable, supported);

	return supported;
}

} // opengl
} // graphics
} // love
