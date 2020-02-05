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

Image::Image(const Settings &settings, const Slices *data)
	: love::graphics::Texture(settings, data)
	, slices(settings.type)
	, texture(0)
{
	if (data != nullptr)
		slices = *data;
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
	setSamplerState(samplerState);

	bool isSRGB = false;
	gl.rawTexStorage(texType, 1, PIXELFORMAT_RGBA8_UNORM, isSRGB, 2, 2, 1);

	// A nice friendly checkerboard to signify invalid textures...
	GLubyte px[] = {0xFF,0xFF,0xFF,0xFF, 0xFF,0xA0,0xA0,0xFF,
	                0xFF,0xA0,0xA0,0xFF, 0xFF,0xFF,0xFF,0xFF};

	int slices = texType == TEXTURE_CUBE ? 6 : 1;
	Rect rect = {0, 0, 2, 2};
	for (int slice = 0; slice < slices; slice++)
		uploadByteData(PIXELFORMAT_RGBA8_UNORM, px, sizeof(px), 0, slice, rect, nullptr);
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

	int w = pixelWidth;
	int h = pixelHeight;
	int d = depth;

	OpenGL::TextureFormat fmt = gl.convertPixelFormat(format, false, sRGB);

	for (int mip = 0; mip < mipcount; mip++)
	{
		if (isCompressed() && (texType == TEXTURE_2D_ARRAY || texType == TEXTURE_VOLUME))
		{
			size_t mipsize = 0;

			if (texType == TEXTURE_2D_ARRAY || texType == TEXTURE_VOLUME)
			{
				for (int slice = 0; slice < slices.getSliceCount(mip); slice++)
				{
					auto id = slices.get(slice, mip);
					if (id != nullptr)
						mipsize += id->getSize();
				}
			}

			if (mipsize > 0)
			{
				GLenum gltarget = OpenGL::getGLTextureType(texType);
				glCompressedTexImage3D(gltarget, mip, fmt.internalformat, w, h, d, 0, mipsize, nullptr);
			}
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

	if (getMipmapCount() > 1 && slices.getMipmapCount() <= 1)
		generateMipmaps();
}

void Image::uploadByteData(PixelFormat pixelformat, const void *data, size_t size, int level, int slice, const Rect &r, love::image::ImageDataBase *imgd)
{
	love::image::ImageDataBase *oldd = slices.get(slice, level);

	// We can only replace the internal Data (used when reloading due to setMode)
	// if the dimensions match.
	if (imgd != nullptr && oldd != nullptr && oldd->getWidth() == imgd->getWidth()
		&& oldd->getHeight() == imgd->getHeight())
	{
		slices.set(slice, level, imgd);
	}

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

	// NPOT textures don't support mipmapping without full NPOT support.
	if ((GLAD_ES_VERSION_2_0 && !(GLAD_ES_VERSION_3_0 || GLAD_OES_texture_npot))
		&& (pixelWidth != nextP2(pixelWidth) || pixelHeight != nextP2(pixelHeight)))
	{
		mipmapCount = 1;
		samplerState.mipmapFilter = SamplerState::MIPMAP_FILTER_NONE;
	}

	glGenTextures(1, &texture);
	gl.bindTextureToUnit(this, 0, false);

	// Use a default texture if the size is too big for the system.
	if (!validateDimensions(false))
	{
		loadDefaultTexture();
		return true;
	}

	setSamplerState(samplerState);

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

	for (int slice = 0; slice < slices.getSliceCount(0); slice++)
		memsize += slices.get(slice, 0)->getSize();

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

void Image::setSamplerState(const SamplerState &s)
{
	Texture::setSamplerState(s);

	if (!OpenGL::hasTextureFilteringSupport(getPixelFormat()))
	{
		samplerState.magFilter = samplerState.minFilter = SamplerState::FILTER_NEAREST;

		if (samplerState.mipmapFilter == SamplerState::MIPMAP_FILTER_LINEAR)
			samplerState.mipmapFilter = SamplerState::MIPMAP_FILTER_NEAREST;
	}

	// We don't want filtering or (attempted) mipmaps on the default texture.
	if (usingDefaultTexture)
	{
		samplerState.mipmapFilter = SamplerState::MIPMAP_FILTER_NONE;
		samplerState.minFilter = samplerState.magFilter = SamplerState::FILTER_NEAREST;
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

} // opengl
} // graphics
} // love
