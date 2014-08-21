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

#include "Image.h"

// STD
#include <cstring> // For memcpy
#include <algorithm> // for min/max

namespace love
{
namespace graphics
{
namespace opengl
{

int Image::imageCount = 0;

float Image::maxMipmapSharpness = 0.0f;

Texture::FilterMode Image::defaultMipmapFilter = Texture::FILTER_NEAREST;
float Image::defaultMipmapSharpness = 0.0f;

Image::Image(love::image::ImageData *data, const Flags &flags)
	: data(data)
	, cdata(nullptr)
	, texture(0)
	, mipmapSharpness(defaultMipmapSharpness)
	, compressed(false)
	, flags(flags)
	, usingDefaultTexture(false)
	, textureMemorySize(0)
{
	width = data->getWidth();
	height = data->getHeight();
	preload();

	++imageCount;
}

Image::Image(love::image::CompressedData *cdata, const Flags &flags)
	: data(nullptr)
	, cdata(cdata)
	, texture(0)
	, mipmapSharpness(defaultMipmapSharpness)
	, compressed(true)
	, flags(flags)
	, usingDefaultTexture(false)
	, textureMemorySize(0)
{
	width = cdata->getWidth(0);
	height = cdata->getHeight(0);
	preload();

	++imageCount;
}

Image::~Image()
{
	unload();

	--imageCount;
}

love::image::ImageData *Image::getImageData() const
{
	return data.get();
}

love::image::CompressedData *Image::getCompressedData() const
{
	return cdata.get();
}

void Image::draw(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky)
{
	Matrix t;
	t.setTransformation(x, y, angle, sx, sy, ox, oy, kx, ky);

	drawv(t, vertices);
}

void Image::drawq(Quad *quad, float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky)
{
	Matrix t;
	t.setTransformation(x, y, angle, sx, sy, ox, oy, kx, ky);

	drawv(t, quad->getVertices());
}

void Image::predraw()
{
	bind();
}

void Image::postdraw()
{
}

GLuint Image::getGLTexture() const
{
	return texture;
}

void Image::setFilter(const Texture::Filter &f)
{
	if (!validateFilter(f, flags.mipmaps))
	{
		if (f.mipmap != FILTER_NONE && !flags.mipmaps)
			throw love::Exception("Non-mipmapped image cannot have mipmap filtering.");
		else
			throw love::Exception("Invalid texture filter.");
	}

	filter = f;

	// We don't want filtering or (attempted) mipmaps on the default texture.
	if (usingDefaultTexture)
	{
		filter.mipmap = FILTER_NONE;
		filter.min = filter.mag = FILTER_NEAREST;
	}

	bind();
	gl.setTextureFilter(filter);
}

void Image::setWrap(const Texture::Wrap &w)
{
	wrap = w;

	bind();
	gl.setTextureWrap(w);
}

void Image::setMipmapSharpness(float sharpness)
{
	// LOD bias has the range (-maxbias, maxbias)
	mipmapSharpness = std::min(std::max(sharpness, -maxMipmapSharpness + 0.01f), maxMipmapSharpness - 0.01f);

	bind();

	// negative bias is sharper
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -mipmapSharpness);
}

float Image::getMipmapSharpness() const
{
	return mipmapSharpness;
}

void Image::bind() const
{
	if (texture == 0)
		return;

	gl.bindTexture(texture);
}

void Image::preload()
{
	// For colors.
	memset(vertices, 255, sizeof(Vertex)*4);

	vertices[0].x = 0;
	vertices[0].y = 0;
	vertices[1].x = 0;
	vertices[1].y = (float) height;
	vertices[2].x = (float) width;
	vertices[2].y = (float) height;
	vertices[3].x = (float) width;
	vertices[3].y = 0;

	vertices[0].s = 0;
	vertices[0].t = 0;
	vertices[1].s = 0;
	vertices[1].t = 1;
	vertices[2].s = 1;
	vertices[2].t = 1;
	vertices[3].s = 1;
	vertices[3].t = 0;

	if (flags.mipmaps)
		filter.mipmap = defaultMipmapFilter;
}

bool Image::load()
{
	return loadVolatile();
}

void Image::unload()
{
	return unloadVolatile();
}

void Image::uploadCompressedData()
{
	GLenum format = getCompressedFormat(cdata->getFormat());
	int count = flags.mipmaps ? cdata->getMipmapCount() : 1;

	// We have to inform OpenGL if the image doesn't have all mipmap levels.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, count - 1);

	for (int i = 0; i < count; i++)
	{
		glCompressedTexImage2D(GL_TEXTURE_2D,
		                       i,
		                       format,
		                       cdata->getWidth(i),
		                       cdata->getHeight(i),
		                       0,
		                       GLsizei(cdata->getSize(i)),
		                       cdata->getData(i));
	}
}

void Image::uploadImageData()
{
	if (flags.mipmaps)
	{
		// NPOT mipmap generation isn't always supported on old GPUs/drivers...
		if (width != next_p2(width) || height != next_p2(height))
		{
			throw love::Exception("Cannot create mipmaps: "
			                      "image does not have power-of-two dimensions.");
		}

		if (!GLAD_VERSION_3_0 && !GLAD_ARB_framebuffer_object)
			glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	}

	GLenum iformat = flags.sRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;

	{
		love::thread::Lock lock(data->getMutex());
		glTexImage2D(GL_TEXTURE_2D,
		             0,
		             iformat,
		             (GLsizei)width,
		             (GLsizei)height,
		             0,
		             GL_RGBA,
		             GL_UNSIGNED_BYTE,
		             data->getData());
	}

	if (flags.mipmaps)
	{
		if (GLAD_VERSION_3_0 || GLAD_ARB_framebuffer_object)
		{
			// Driver bug: http://www.opengl.org/wiki/Common_Mistakes#Automatic_mipmap_generation
			if (gl.getVendor() == OpenGL::VENDOR_ATI_AMD)
				glEnable(GL_TEXTURE_2D);

			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
			glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	}
}

void Image::uploadTexture()
{
	bind();

	if (isCompressed() && cdata.get())
		uploadCompressedData();
	else if (data.get())
		uploadImageData();
}

bool Image::loadVolatile()
{
	if (flags.sRGB && !hasSRGBSupport())
		throw love::Exception("sRGB images are not supported on this system.");

	if (isCompressed() && cdata.get() && !hasCompressedTextureSupport(cdata->getFormat()))
	{
		const char *str;
		if (image::CompressedData::getConstant(cdata->getFormat(), str))
		{
			throw love::Exception("Cannot create image: "
			      "%s compressed images are not supported on this system.", str);
		}
		else
			throw love::Exception("cannot create image: format is not supported on this system.");
	}

	if (maxMipmapSharpness == 0.0f)
		glGetFloatv(GL_MAX_TEXTURE_LOD_BIAS, &maxMipmapSharpness);

	glGenTextures(1, &texture);
	gl.bindTexture(texture);

	gl.setTextureFilter(filter);
	gl.setTextureWrap(wrap);
	setMipmapSharpness(mipmapSharpness);

	if (!flags.mipmaps)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	// Use a default texture if the size is too big for the system.
	if (width > gl.getMaxTextureSize() || height > gl.getMaxTextureSize())
	{
		uploadDefaultTexture();
		return true;
	}

	while (glGetError() != GL_NO_ERROR); // Clear errors.

	try
	{
		uploadTexture();

		GLenum glerr = glGetError();
		if (glerr != GL_NO_ERROR)
			throw love::Exception("Cannot create image (error code 0x%x)", glerr);
	}
	catch (love::Exception &)
	{
		gl.deleteTexture(texture);
		texture = 0;
		throw;
	}

	size_t prevmemsize = textureMemorySize;

	if (isCompressed())
	{
		textureMemorySize = 0;
		for (int i = 0; i < flags.mipmaps ? cdata->getMipmapCount() : 1; i++)
			textureMemorySize += cdata->getSize(i);
	}
	else
	{
		textureMemorySize = width * height * 4;
		if (flags.mipmaps)
			textureMemorySize *= 1.333;
	}

	gl.updateTextureMemorySize(prevmemsize, textureMemorySize);

	usingDefaultTexture = false;
	return true;
}

void Image::unloadVolatile()
{
	// Delete the hardware texture.
	if (texture != 0)
	{
		gl.deleteTexture(texture);
		texture = 0;

		gl.updateTextureMemorySize(textureMemorySize, 0);
		textureMemorySize = 0;
	}
}

bool Image::refresh()
{
	// No effect if the texture hasn't been created yet.
	if (texture == 0)
		return false;

	if (usingDefaultTexture)
	{
		uploadDefaultTexture();
		return true;
	}

	while (glGetError() != GL_NO_ERROR); // Clear errors.

	uploadTexture();

	GLenum glerr = glGetError();
	if (glerr != GL_NO_ERROR)
		throw love::Exception("Cannot refresh image (error code 0x%x)", glerr);

	return true;
}

const Image::Flags &Image::getFlags() const
{
	return flags;
}

void Image::uploadDefaultTexture()
{
	usingDefaultTexture = true;

	bind();
	setFilter(filter);

	// A nice friendly checkerboard to signify invalid textures...
	GLubyte px[] = {0xFF,0xFF,0xFF,0xFF, 0xC0,0xC0,0xC0,0xFF,
	                0xC0,0xC0,0xC0,0xFF, 0xFF,0xFF,0xFF,0xFF};

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
}

void Image::drawv(const Matrix &t, const Vertex *v)
{
	OpenGL::TempTransform transform(gl);
	transform.get() *= t;

	predraw();

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(2, GL_FLOAT, sizeof(Vertex), (GLvoid *)&v[0].x);
	glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), (GLvoid *)&v[0].s);

	gl.prepareDraw();
	gl.drawArrays(GL_QUADS, 0, 4);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	postdraw();
}

void Image::setDefaultMipmapSharpness(float sharpness)
{
	defaultMipmapSharpness = sharpness;
}

float Image::getDefaultMipmapSharpness()
{
	return defaultMipmapSharpness;
}

void Image::setDefaultMipmapFilter(Texture::FilterMode f)
{
	defaultMipmapFilter = f;
}

Texture::FilterMode Image::getDefaultMipmapFilter()
{
	return defaultMipmapFilter;
}

bool Image::isCompressed() const
{
	return compressed;
}

GLenum Image::getCompressedFormat(image::CompressedData::Format cformat) const
{
	switch (cformat)
	{
	case image::CompressedData::FORMAT_DXT1:
		if (flags.sRGB)
			return GL_COMPRESSED_SRGB_S3TC_DXT1_EXT;
		else
			return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
	case image::CompressedData::FORMAT_DXT3:
		if (flags.sRGB)
			return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
		else
			return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
	case image::CompressedData::FORMAT_DXT5:
		if (flags.sRGB)
			return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
		else
			return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
	case image::CompressedData::FORMAT_BC4:
		return GL_COMPRESSED_RED_RGTC1;
	case image::CompressedData::FORMAT_BC4s:
		return GL_COMPRESSED_SIGNED_RED_RGTC1;
	case image::CompressedData::FORMAT_BC5:
		return GL_COMPRESSED_RG_RGTC2;
	case image::CompressedData::FORMAT_BC5s:
		return GL_COMPRESSED_SIGNED_RG_RGTC2;
	default:
		if (flags.sRGB)
			return GL_SRGB8_ALPHA8;
		else
			return GL_RGBA8;
	}
}

bool Image::hasAnisotropicFilteringSupport()
{
	return GLAD_EXT_texture_filter_anisotropic;
}

bool Image::hasCompressedTextureSupport(image::CompressedData::Format format)
{
	switch (format)
	{
	case image::CompressedData::FORMAT_DXT1:
	case image::CompressedData::FORMAT_DXT3:
	case image::CompressedData::FORMAT_DXT5:
		return GLAD_EXT_texture_compression_s3tc;
	case image::CompressedData::FORMAT_BC4:
	case image::CompressedData::FORMAT_BC4s:
	case image::CompressedData::FORMAT_BC5:
	case image::CompressedData::FORMAT_BC5s:
		return (GLAD_VERSION_3_0 || GLAD_ARB_texture_compression_rgtc || GLAD_EXT_texture_compression_rgtc);
	default:
		break;
	}

	return false;
}

bool Image::hasSRGBSupport()
{
	return GLAD_VERSION_2_1 || GLAD_EXT_texture_sRGB;
}

bool Image::getConstant(const char *in, FlagType &out)
{
	return flagNames.find(in, out);
}

bool Image::getConstant(FlagType in, const char *&out)
{
	return flagNames.find(in, out);
}

StringMap<Image::FlagType, Image::FLAG_TYPE_MAX_ENUM>::Entry Image::flagNameEntries[] =
{
	{"mipmaps", Image::FLAG_TYPE_MIPMAPS},
	{"srgb", Image::FLAG_TYPE_SRGB},
};

StringMap<Image::FlagType, Image::FLAG_TYPE_MAX_ENUM> Image::flagNames(Image::flagNameEntries, sizeof(Image::flagNameEntries));

} // opengl
} // graphics
} // love
