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

float Image::maxMipmapSharpness = 0.0f;

Texture::FilterMode Image::defaultMipmapFilter = Texture::FILTER_NONE;
float Image::defaultMipmapSharpness = 0.0f;

Image::Image(love::image::ImageData *data, Format format)
	: data(data)
	, cdata(nullptr)
	, paddedWidth(width)
	, paddedHeight(height)
	, texture(0)
	, mipmapSharpness(defaultMipmapSharpness)
	, mipmapsCreated(false)
	, compressed(false)
	, format(format)
	, usingDefaultTexture(false)
{
	width = data->getWidth();
	height = data->getHeight();
	preload();
}

Image::Image(love::image::CompressedData *cdata, Format format)
	: data(nullptr)
	, cdata(cdata)
	, paddedWidth(width)
	, paddedHeight(height)
	, texture(0)
	, mipmapSharpness(defaultMipmapSharpness)
	, mipmapsCreated(false)
	, compressed(true)
	, format(format)
	, usingDefaultTexture(false)
{
	width = cdata->getWidth(0);
	height = cdata->getHeight(0);
	preload();
}

Image::~Image()
{
	unload();
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

	if (width != paddedWidth || height != paddedHeight)
	{
		// NPOT image padded to POT size, so the texcoords should be scaled.
		glMatrixMode(GL_TEXTURE);
		glPushMatrix();
		glScalef(float(width) / float(paddedWidth), float(height) / float(paddedHeight), 0.0f);
		glMatrixMode(GL_MODELVIEW);
	}
}

void Image::postdraw()
{
	if (width != paddedWidth || height != paddedHeight)
	{
		glMatrixMode(GL_TEXTURE);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	}
}

GLuint Image::getGLTexture() const
{
	return texture;
}

void Image::uploadCompressedMipmaps()
{
	if (!isCompressed() || !cdata.get() || !hasCompressedTextureSupport(cdata->getFormat()))
		return;

	bind();

	int count = cdata->getMipmapCount();

	// We have to inform OpenGL if the image doesn't have all mipmap levels.
	if (GLEE_VERSION_1_2 || GLEE_SGIS_texture_lod)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, count - 1);
	}
	else if (cdata->getWidth(count-1) > 1 || cdata->getHeight(count-1) > 1)
	{
		// Telling OpenGL to ignore certain levels isn't always supported.
		throw love::Exception("Cannot load mipmaps: "
		      "compressed image does not have all required levels.");
	}

	for (int i = 1; i < count; i++)
	{
		glCompressedTexImage2DARB(GL_TEXTURE_2D,
		                          i,
		                          getCompressedFormat(cdata->getFormat()),
		                          cdata->getWidth(i),
		                          cdata->getHeight(i),
		                          0,
		                          GLsizei(cdata->getSize(i)),
		                          cdata->getData(i));
	}
}

void Image::createMipmaps()
{
	// Only valid for Images created with ImageData.
	if (!data.get() || isCompressed())
		return;

	if (!hasMipmapSupport())
		throw love::Exception("Mipmap filtering is not supported on this system.");

	// Some old drivers claim support for NPOT textures, but fail when creating
	// mipmaps. We can't detect which systems will do this, so we fail gracefully
	// for all NPOT images.
	int w = int(width), h = int(height);
	if (w != next_p2(w) || h != next_p2(h))
	{
		throw love::Exception("Cannot create mipmaps: "
		      "image does not have power of two dimensions.");
	}

	bind();

	// Prevent other threads from changing the ImageData while we upload it.
	love::thread::Lock lock(data->getMutex());

	if (hasNpot() && (GLEE_VERSION_3_0 || GLEE_ARB_framebuffer_object))
	{
		if (gl.getVendor() == OpenGL::VENDOR_ATI_AMD)
		{
			// AMD/ATI drivers have several bugs when generating mipmaps,
			// re-uploading the entire base image seems to be required.
			uploadTexture();

			// More bugs: http://www.opengl.org/wiki/Common_Mistakes#Automatic_mipmap_generation
			glEnable(GL_TEXTURE_2D);
		}

		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexSubImage2D(GL_TEXTURE_2D,
		                0,
		                0,
		                0,
		                (GLsizei)width,
		                (GLsizei)height,
		                GL_RGBA,
		                GL_UNSIGNED_BYTE,
		                data->getData());
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	}
}

void Image::checkMipmapsCreated()
{
	if (mipmapsCreated || filter.mipmap == FILTER_NONE || usingDefaultTexture)
		return;

	if (isCompressed() && cdata.get() && hasCompressedTextureSupport(cdata->getFormat()))
		uploadCompressedMipmaps();
	else if (data.get())
		createMipmaps();
	else
		return;

	mipmapsCreated = true;
}

void Image::setFilter(const Texture::Filter &f)
{
	filter = f;

	// We don't want filtering or (attempted) mipmaps on the default texture.
	if (usingDefaultTexture)
	{
		filter.mipmap = FILTER_NONE;
		filter.min = filter.mag = FILTER_NEAREST;
	}

	bind();
	gl.setTextureFilter(filter);
	checkMipmapsCreated();
}

void Image::setWrap(const Texture::Wrap &w)
{
	wrap = w;

	bind();
	gl.setTextureWrap(w);
}

void Image::setMipmapSharpness(float sharpness)
{
	if (hasMipmapSharpnessSupport())
	{
		// LOD bias has the range (-maxbias, maxbias)
		mipmapSharpness = std::min(std::max(sharpness, -maxMipmapSharpness + 0.01f), maxMipmapSharpness - 0.01f);

		bind();

		// negative bias is sharper
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -mipmapSharpness);
	}
	else
		mipmapSharpness = 0.0f;
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

bool Image::loadVolatile()
{
	if (format == FORMAT_SRGB && !hasSRGBSupport())
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

	if (hasMipmapSharpnessSupport() && maxMipmapSharpness == 0.0f)
		glGetFloatv(GL_MAX_TEXTURE_LOD_BIAS, &maxMipmapSharpness);

	glGenTextures(1, &texture);
	gl.bindTexture(texture);

	filter.anisotropy = gl.setTextureFilter(filter);
	gl.setTextureWrap(wrap);
	setMipmapSharpness(mipmapSharpness);

	paddedWidth = width;
	paddedHeight = height;

	if (!hasNpot())
	{
		// NPOT textures will be padded to POT dimensions if necessary.
		paddedWidth = next_p2(width);
		paddedHeight = next_p2(height);
	}

	// Use a default texture if the size is too big for the system.
	if (paddedWidth > gl.getMaxTextureSize() || paddedHeight > gl.getMaxTextureSize())
	{
		uploadDefaultTexture();
		return true;
	}

	// Mutex lock will potentially cover texture loading and mipmap creation.
	love::thread::EmptyLock lock;
	if (data.get())
		lock.setLock(data->getMutex());

	while (glGetError() != GL_NO_ERROR); // Clear errors.

	if (hasNpot() || (width == paddedWidth && height == paddedHeight))
		uploadTexture();
	else
		uploadTexturePadded();

	GLenum glerr = glGetError();
	if (glerr != GL_NO_ERROR)
		throw love::Exception("Cannot create image (error code 0x%x)", glerr);

	usingDefaultTexture = false;
	mipmapsCreated = false;
	checkMipmapsCreated();

	return true;
}

void Image::uploadTexturePadded()
{
	if (isCompressed() && cdata.get())
	{
		// Padded textures don't really work if they're compressed...
		throw love::Exception("Cannot create image: "
		                      "compressed NPOT images are not supported on this system.");
	}
	else if (data.get())
	{
		GLenum iformat = (format == FORMAT_SRGB) ? GL_SRGB8_ALPHA8 : GL_RGBA8;
		glTexImage2D(GL_TEXTURE_2D,
		             0,
		             iformat,
		             (GLsizei)paddedWidth,
		             (GLsizei)paddedHeight,
		             0,
		             GL_RGBA,
		             GL_UNSIGNED_BYTE,
		             0);

		glTexSubImage2D(GL_TEXTURE_2D,
		                0,
		                0, 0,
		                (GLsizei)width,
		                (GLsizei)height,
		                GL_RGBA,
		                GL_UNSIGNED_BYTE,
		                data->getData());
	}
}

void Image::uploadTexture()
{
	if (isCompressed() && cdata.get())
	{
		GLenum format = getCompressedFormat(cdata->getFormat());
		glCompressedTexImage2DARB(GL_TEXTURE_2D,
		                          0,
		                          format,
		                          cdata->getWidth(0),
		                          cdata->getHeight(0),
		                          0,
		                          GLsizei(cdata->getSize(0)),
		                          cdata->getData(0));
	}
	else if (data.get())
	{
		GLenum iformat = (format == FORMAT_SRGB) ? GL_SRGB8_ALPHA8 : GL_RGBA8;
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
}

void Image::unloadVolatile()
{
	// Delete the hardware texture.
	if (texture != 0)
	{
		gl.deleteTexture(texture);
		texture = 0;
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

	// We want this lock to potentially cover mipmap creation as well.
	love::thread::EmptyLock lock;

	bind();

	if (data.get() && !isCompressed())
		lock.setLock(data->getMutex());

	while (glGetError() != GL_NO_ERROR); // Clear errors.

	if (hasNpot() || (width == paddedWidth && height == paddedHeight))
		uploadTexture();
	else
		uploadTexturePadded();

	if (glGetError() != GL_NO_ERROR)
		uploadDefaultTexture();
	else
		usingDefaultTexture = false;

	mipmapsCreated = false;
	checkMipmapsCreated();

	return true;
}

Image::Format Image::getFormat() const
{
	return format;
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
	glDrawArrays(GL_QUADS, 0, 4);

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
	bool srgb = format == FORMAT_SRGB;

	switch (cformat)
	{
	case image::CompressedData::FORMAT_DXT1:
		if (srgb)
			return GL_COMPRESSED_SRGB_S3TC_DXT1_EXT;
		else
			return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
	case image::CompressedData::FORMAT_DXT3:
		if (srgb)
			return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
		else
			return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
	case image::CompressedData::FORMAT_DXT5:
		if (srgb)
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
		if (srgb)
			return GL_SRGB8_ALPHA8;
		else
			return GL_RGBA8;
	}
}

bool Image::hasNpot()
{
	return GLEE_VERSION_2_0 || GLEE_ARB_texture_non_power_of_two;
}

bool Image::hasAnisotropicFilteringSupport()
{
	return GLEE_EXT_texture_filter_anisotropic;
}

bool Image::hasMipmapSupport()
{
	return GLEE_VERSION_1_4 || GLEE_SGIS_generate_mipmap;
}

bool Image::hasMipmapSharpnessSupport()
{
	return GLEE_VERSION_1_4;
}

bool Image::hasCompressedTextureSupport()
{
	return GLEE_VERSION_1_3 || GLEE_ARB_texture_compression;
}

bool Image::hasCompressedTextureSupport(image::CompressedData::Format format)
{
	if (!hasCompressedTextureSupport())
		return false;

	switch (format)
	{
	case image::CompressedData::FORMAT_DXT1:
	case image::CompressedData::FORMAT_DXT3:
	case image::CompressedData::FORMAT_DXT5:
		return GLEE_EXT_texture_compression_s3tc;
	case image::CompressedData::FORMAT_BC4:
	case image::CompressedData::FORMAT_BC4s:
	case image::CompressedData::FORMAT_BC5:
	case image::CompressedData::FORMAT_BC5s:
		return (GLEE_VERSION_3_0 || GLEE_ARB_texture_compression_rgtc || GLEE_EXT_texture_compression_rgtc);
	default:
		break;
	}

	return false;
}

bool Image::hasSRGBSupport()
{
	return GLEE_VERSION_2_1 || GLEE_EXT_texture_sRGB;
}

bool Image::getConstant(const char *in, Format &out)
{
	return formats.find(in, out);
}

bool Image::getConstant(Format in, const char *&out)
{
	return formats.find(in, out);
}

StringMap<Image::Format, Image::FORMAT_MAX_ENUM>::Entry Image::formatEntries[] =
{
	{"normal", Image::FORMAT_NORMAL},
	{"srgb", Image::FORMAT_SRGB},
};

StringMap<Image::Format, Image::FORMAT_MAX_ENUM> Image::formats(Image::formatEntries, sizeof(Image::formatEntries));

} // opengl
} // graphics
} // love
