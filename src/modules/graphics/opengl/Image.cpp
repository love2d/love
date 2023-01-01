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

#include "Image.h"

#include "graphics/Graphics.h"
#include "common/int.h"

// STD
#include <algorithm> // for min/max

namespace love
{
namespace graphics
{
namespace opengl
{

Image::Image(TextureType textype, PixelFormat format, int width, int height, int slices, const Settings &settings)
	: love::graphics::Image(textype, format, width, height, slices, settings)
	, texture(0)
{
	loadVolatile();
}

Image::Image(const Slices &slices, const Settings &settings)
	: love::graphics::Image(slices, settings)
	, texture(0)
{
	loadVolatile();
}

Image::~Image()
{
	unloadVolatile();
}

void Image::generateMipmaps()
{
	if (getMipmapCount() > 1 && !isCompressed() &&
		(GLAD_ES_VERSION_2_0 || GLAD_VERSION_3_0 || GLAD_ARB_framebuffer_object || GLAD_EXT_framebuffer_object))
	{
		gl.bindTextureToUnit(this, 0, false);

		GLenum gltextype = OpenGL::getGLTextureType(texType);

		if (gl.bugs.generateMipmapsRequiresTexture2DEnable)
			glEnable(gltextype);

		glGenerateMipmap(gltextype);
	}
}

void Image::loadDefaultTexture()
{
	usingDefaultTexture = true;

	gl.bindTextureToUnit(this, 0, false);
	setFilter(filter);

	bool isSRGB = false;
	gl.rawTexStorage(texType, 1, PIXELFORMAT_RGBA8, isSRGB, 2, 2, 1);

	// A nice friendly checkerboard to signify invalid textures...
	GLubyte px[] = {0xFF,0xFF,0xFF,0xFF, 0xFF,0xA0,0xA0,0xFF,
	                0xFF,0xA0,0xA0,0xFF, 0xFF,0xFF,0xFF,0xFF};

	int slices = texType == TEXTURE_CUBE ? 6 : 1;
	Rect rect = {0, 0, 2, 2};
	for (int slice = 0; slice < slices; slice++)
		uploadByteData(PIXELFORMAT_RGBA8, px, sizeof(px), 0, slice, rect);
}

void Image::loadData()
{
	int mipcount = getMipmapCount();
	int slicecount = 1;

	if (texType == TEXTURE_VOLUME)
		slicecount = getDepth();
	else if (texType == TEXTURE_2D_ARRAY)
		slicecount = getLayerCount();
	else if (texType == TEXTURE_CUBE)
		slicecount = 6;

	if (!isCompressed())
		gl.rawTexStorage(texType, mipcount, format, sRGB, pixelWidth, pixelHeight, texType == TEXTURE_VOLUME ? depth : layers);

	if (mipmapsType == MIPMAPS_GENERATED)
		mipcount = 1;

	int w = pixelWidth;
	int h = pixelHeight;
	int d = depth;

	OpenGL::TextureFormat fmt = gl.convertPixelFormat(format, false, sRGB);

	for (int mip = 0; mip < mipcount; mip++)
	{
		if (isCompressed() && (texType == TEXTURE_2D_ARRAY || texType == TEXTURE_VOLUME))
		{
			int mipslices = data.getSliceCount(mip);
			size_t mipsize = 0;

			for (int slice = 0; slice < mipslices; slice++)
				mipsize += data.get(slice, mip)->getSize();

			GLenum gltarget = OpenGL::getGLTextureType(texType);
			glCompressedTexImage3D(gltarget, mip, fmt.internalformat, w, h, mipslices, 0, mipsize, nullptr);
		}

		for (int slice = 0; slice < slicecount; slice++)
		{
			love::image::ImageDataBase *id = data.get(slice, mip);

			if (id != nullptr)
				uploadImageData(id, mip, slice, 0, 0);
		}

		w = std::max(w / 2, 1);
		h = std::max(h / 2, 1);

		if (texType == TEXTURE_VOLUME)
			d = std::max(d / 2, 1);
	}

	if (mipmapsType == MIPMAPS_GENERATED)
		generateMipmaps();
}

void Image::uploadByteData(PixelFormat pixelformat, const void *data, size_t size, int level, int slice, const Rect &r)
{
	OpenGL::TempDebugGroup debuggroup("Image data upload");

	gl.bindTextureToUnit(this, 0, false);

	OpenGL::TextureFormat fmt = OpenGL::convertPixelFormat(pixelformat, false, sRGB);
	GLenum gltarget = OpenGL::getGLTextureType(texType);

	if (texType == TEXTURE_CUBE)
		gltarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + slice;

	if (isPixelFormatCompressed(pixelformat))
	{
		if (r.x != 0 || r.y != 0)
			throw love::Exception("x and y parameters must be 0 for compressed images.");

		if (texType == TEXTURE_2D || texType == TEXTURE_CUBE)
			glCompressedTexImage2D(gltarget, level, fmt.internalformat, r.w, r.h, 0, size, data);
		else if (texType == TEXTURE_2D_ARRAY || texType == TEXTURE_VOLUME)
			glCompressedTexSubImage3D(gltarget, level, 0, 0, slice, r.w, r.h, 1, fmt.internalformat, size, data);
	}
	else
	{
		if (texType == TEXTURE_2D || texType == TEXTURE_CUBE)
			glTexSubImage2D(gltarget, level, r.x, r.y, r.w, r.h, fmt.externalformat, fmt.type, data);
		else if (texType == TEXTURE_2D_ARRAY || texType == TEXTURE_VOLUME)
			glTexSubImage3D(gltarget, level, r.x, r.y, slice, r.w, r.h, 1, fmt.externalformat, fmt.type, data);
	}
}

bool Image::loadVolatile()
{
	if (texture != 0)
		return true;

	OpenGL::TempDebugGroup debuggroup("Image load");

	if (!isCompressed())
	{
		// GL_EXT_sRGB doesn't support glGenerateMipmap for sRGB textures.
		if (sRGB && (GLAD_ES_VERSION_2_0 && GLAD_EXT_sRGB && !GLAD_ES_VERSION_3_0)
			&& mipmapsType != MIPMAPS_DATA)
		{
			mipmapsType = MIPMAPS_NONE;
			filter.mipmap = FILTER_NONE;
		}
	}

	// NPOT textures don't support mipmapping without full NPOT support.
	if ((GLAD_ES_VERSION_2_0 && !(GLAD_ES_VERSION_3_0 || GLAD_OES_texture_npot))
		&& (pixelWidth != nextP2(pixelWidth) || pixelHeight != nextP2(pixelHeight)))
	{
		mipmapsType = MIPMAPS_NONE;
		filter.mipmap = FILTER_NONE;
	}

	glGenTextures(1, &texture);
	gl.bindTextureToUnit(this, 0, false);

	// Use a default texture if the size is too big for the system.
	if (!validateDimensions(false))
	{
		loadDefaultTexture();
		return true;
	}

	setFilter(filter);
	setWrap(wrap);
	setMipmapSharpness(mipmapSharpness);

	GLenum gltextype = OpenGL::getGLTextureType(texType);

	if (mipmapsType == MIPMAPS_NONE && (GLAD_ES_VERSION_3_0 || GLAD_VERSION_1_0))
		glTexParameteri(gltextype, GL_TEXTURE_MAX_LEVEL, 0);

	while (glGetError() != GL_NO_ERROR); // Clear errors.

	try
	{
		loadData();

		GLenum glerr = glGetError();
		if (glerr != GL_NO_ERROR)
			throw love::Exception("Cannot create image (OpenGL error: %s)", OpenGL::errorString(glerr));
	}
	catch (love::Exception &)
	{
		gl.deleteTexture(texture);
		texture = 0;
		throw;
	}

	int64 memsize = 0;

	for (int slice = 0; slice < data.getSliceCount(0); slice++)
		memsize += data.get(slice, 0)->getSize();

	if (getMipmapCount() > 1)
		memsize *= 1.33334;

	setGraphicsMemorySize(memsize);

	usingDefaultTexture = false;
	return true;
}

void Image::unloadVolatile()
{
	if (texture == 0)
		return;

	gl.deleteTexture(texture);
	texture = 0;

	setGraphicsMemorySize(0);
}

ptrdiff_t Image::getHandle() const
{
	return texture;
}

void Image::setFilter(const Texture::Filter &f)
{
	Texture::setFilter(f);

	if (!OpenGL::hasTextureFilteringSupport(getPixelFormat()))
	{
		filter.mag = filter.min = FILTER_NEAREST;

		if (filter.mipmap == FILTER_LINEAR)
			filter.mipmap = FILTER_NEAREST;
	}

	// We don't want filtering or (attempted) mipmaps on the default texture.
	if (usingDefaultTexture)
	{
		filter.mipmap = FILTER_NONE;
		filter.min = filter.mag = FILTER_NEAREST;
	}

	gl.bindTextureToUnit(this, 0, false);
	gl.setTextureFilter(texType, filter);
}

bool Image::setWrap(const Texture::Wrap &w)
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

bool Image::setMipmapSharpness(float sharpness)
{
	if (!gl.isSamplerLODBiasSupported())
		return false;

	Graphics::flushStreamDrawsGlobal();

	float maxbias = gl.getMaxLODBias();

	if (maxbias > 0.01f)
		maxbias -= 0.01f;

	mipmapSharpness = std::min(std::max(sharpness, -maxbias), maxbias);

	gl.bindTextureToUnit(this, 0, false);

	// negative bias is sharper
	glTexParameterf(gl.getGLTextureType(texType), GL_TEXTURE_LOD_BIAS, -mipmapSharpness);

	return true;
}

bool Image::isFormatSupported(PixelFormat pixelformat, bool sRGB)
{
	return OpenGL::isPixelFormatSupported(pixelformat, false, true, sRGB);
}

} // opengl
} // graphics
} // love
