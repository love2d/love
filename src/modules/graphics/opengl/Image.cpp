/**
 * Copyright (c) 2006-2013 LOVE Development Team
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

Image::FilterMode Image::defaultMipmapFilter = Image::FILTER_NONE;
float Image::defaultMipmapSharpness = 0.0f;

Image::Image(love::image::ImageData *data)
	: data(data)
	, cdata(0)
	, width((float)(data->getWidth()))
	, height((float)(data->getHeight()))
	, texture(0)
	, mipmapSharpness(defaultMipmapSharpness)
	, mipmapsCreated(false)
	, compressed(false)
{
	data->retain();
	preload();
}

Image::Image(love::image::CompressedData *cdata)
	: data(0)
	, cdata(cdata)
	, width((float)(cdata->getWidth(0)))
	, height((float)(cdata->getHeight(0)))
	, texture(0)
	, mipmapSharpness(defaultMipmapSharpness)
	, mipmapsCreated(false)
	, compressed(true)
{
	cdata->retain();
	preload();
}

Image::~Image()
{
	if (data != 0)
		data->release();
	if (cdata != 0)
		cdata->release();
	unload();
}

float Image::getWidth() const
{
	return width;
}

float Image::getHeight() const
{
	return height;
}

const vertex *Image::getVertices() const
{
	return vertices;
}

love::image::ImageData *Image::getImageData() const
{
	return data;
}

love::image::CompressedData *Image::getCompressedData() const
{
	return cdata;
}

void Image::draw(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky) const
{
	static Matrix t;

	t.setTransformation(x, y, angle, sx, sy, ox, oy, kx, ky);
	drawv(t, vertices);
}

void Image::drawg(love::graphics::Geometry *geom, float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky) const
{
	static Matrix t;
	t.setTransformation(x, y, angle, sx, sy, ox, oy, kx, ky);

	const vertex *v = geom->getVertexArray();

	// use colors stored in geometry (horrible, horrible hack)
	if (geom->hasVertexColors())
	{
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(vertex), (GLvoid *) &v[0].r);
	}

	drawv(t, v, geom->getVertexCount(), GL_TRIANGLE_FAN);

	if (geom->hasVertexColors())
	{
		glDisableClientState(GL_COLOR_ARRAY);
		gl.setColor(gl.getColor());
	}
}

void Image::uploadCompressedMipmaps()
{
	if (!isCompressed() || !cdata || !hasCompressedTextureSupport(cdata->getType()))
		return;

	bind();

	int mipmapcount = cdata->getMipmapCount();

	// We have to inform OpenGL if the image doesn't have all mipmap levels.
	if (GLEE_VERSION_1_2 || GLEE_SGIS_texture_lod)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmapcount - 1);
	}
	else if (cdata->getWidth(mipmapcount-1) > 1 || cdata->getHeight(mipmapcount-1) > 1)
	{
		// Telling OpenGL to ignore certain levels isn't always supported.
		throw love::Exception("Cannot load mipmaps: "
		      "compressed image does not have all required levels.");
	}

	for (int i = 1; i < mipmapcount; i++)
	{
		glCompressedTexImage2DARB(GL_TEXTURE_2D,
		                          i,
		                          getCompressedFormat(cdata->getType()),
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
	if (!data)
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
		// AMD/ATI drivers have several bugs when generating mipmaps,
		// re-uploading the entire base image seems to be required.
		glTexImage2D(GL_TEXTURE_2D,
		             0,
		             GL_RGBA8,
		             (GLsizei)width,
		             (GLsizei)height,
		             0,
		             GL_RGBA,
		             GL_UNSIGNED_BYTE,
		             data->getData());

		// More bugs: http://www.opengl.org/wiki/Common_Mistakes#Automatic_mipmap_generation
		glEnable(GL_TEXTURE_2D);
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
	}
}

void Image::checkMipmapsCreated()
{
	if (mipmapsCreated || filter.mipmap == FILTER_NONE)
		return;

	if (isCompressed() && cdata && hasCompressedTextureSupport(cdata->getType()))
		uploadCompressedMipmaps();
	else if (data)
		createMipmaps();
	else
		return;

	mipmapsCreated = true;
}

void Image::setFilter(const Image::Filter &f)
{
	filter = f;

	bind();
	filter.anisotropy = gl.setTextureFilter(f);
	checkMipmapsCreated();
}

const Image::Filter &Image::getFilter() const
{
	return filter;
}

void Image::setWrap(const Image::Wrap &w)
{
	wrap = w;

	bind();
	gl.setTextureWrap(w);
}

const Image::Wrap &Image::getWrap() const
{
	return wrap;
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
	memset(vertices, 255, sizeof(vertex)*4);

	vertices[0].x = 0;
	vertices[0].y = 0;
	vertices[1].x = 0;
	vertices[1].y = height;
	vertices[2].x = width;
	vertices[2].y = height;
	vertices[3].x = width;
	vertices[3].y = 0;

	vertices[0].s = 0;
	vertices[0].t = 0;
	vertices[1].s = 0;
	vertices[1].t = 1;
	vertices[2].s = 1;
	vertices[2].t = 1;
	vertices[3].s = 1;
	vertices[3].t = 0;

	filter = getDefaultFilter();
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
	// glTexImage2D is guaranteed to throw an error in this case.
	if (width > gl.getMaxTextureSize())
	{
		throw love::Exception("Cannot create image: "
		      "width of %d pixels is too large for this system.", (int) width);
	}
	else if (height > gl.getMaxTextureSize())
	{
		throw love::Exception("Cannot create image:"
		      "height of %d pixels is too large for this system.", (int) height);
	}

	if (isCompressed() && cdata && !hasCompressedTextureSupport(cdata->getType()))
	{
		const char *str;
		if (image::CompressedData::getConstant(cdata->getType(), str))
		{
			throw love::Exception("Cannot create image: "
			      "%s compressed images are not supported on this system.", str);
		}
		else
			throw love::Exception("cannot create image: format is not supported on this system.");
	}

	if (hasMipmapSharpnessSupport() && maxMipmapSharpness == 0.0f)
		glGetFloatv(GL_MAX_TEXTURE_LOD_BIAS, &maxMipmapSharpness);

	if (hasNpot())
		return loadVolatileNPOT();
	else
		return loadVolatilePOT();
}

bool Image::loadVolatilePOT()
{
	glGenTextures(1,(GLuint *)&texture);
	gl.bindTexture(texture);

	filter.anisotropy = gl.setTextureFilter(filter);
	gl.setTextureWrap(wrap);
	setMipmapSharpness(mipmapSharpness);

	float p2width = next_p2(width);
	float p2height = next_p2(height);
	float s = width/p2width;
	float t = height/p2height;

	vertices[1].t = t;
	vertices[2].t = t;
	vertices[2].s = s;
	vertices[3].s = s;

	// We want this lock to potentially cover mipmap creation as well.
	love::thread::EmptyLock lock;

	while (glGetError() != GL_NO_ERROR); // clear errors

	if (isCompressed() && cdata)
	{
		if (s < 1.0f || t < 1.0f)
		{
			throw love::Exception("Cannot create image: "
				  "compressed NPOT images are not supported on this system.");
		}

		glCompressedTexImage2DARB(GL_TEXTURE_2D,
		                          0,
		                          getCompressedFormat(cdata->getType()),
		                          cdata->getWidth(0),
		                          cdata->getHeight(0),
		                          0,
		                          GLsizei(cdata->getSize(0)),
		                          cdata->getData(0));
	}
	else if (data)
	{
		glTexImage2D(GL_TEXTURE_2D,
		             0,
		             GL_RGBA8,
		             (GLsizei)p2width,
		             (GLsizei)p2height,
		             0,
		             GL_RGBA,
		             GL_UNSIGNED_BYTE,
		             0);

		lock.setLock(data->getMutex());
		glTexSubImage2D(GL_TEXTURE_2D,
		                0,
		                0, 0,
		                (GLsizei)width,
		                (GLsizei)height,
		                GL_RGBA,
		                GL_UNSIGNED_BYTE,
		                data->getData());
	}

	if (glGetError() != GL_NO_ERROR)
		throw love::Exception("Cannot create image: size may be too large for this system.");

	mipmapsCreated = false;
	checkMipmapsCreated();

	return true;
}

bool Image::loadVolatileNPOT()
{
	glGenTextures(1,(GLuint *)&texture);
	gl.bindTexture(texture);

	filter.anisotropy = gl.setTextureFilter(filter);
	gl.setTextureWrap(wrap);
	setMipmapSharpness(mipmapSharpness);

	// We want this lock to potentially cover mipmap creation as well.
	love::thread::EmptyLock lock;

	while (glGetError() != GL_NO_ERROR); // clear errors

	if (isCompressed() && cdata)
	{
		GLenum format = getCompressedFormat(cdata->getType());
		glCompressedTexImage2DARB(GL_TEXTURE_2D,
		                          0,
		                          format,
		                          cdata->getWidth(0),
		                          cdata->getHeight(0),
		                          0,
		                          GLsizei(cdata->getSize(0)),
		                          cdata->getData(0));
	}
	else if (data)
	{
		lock.setLock(data->getMutex());
		glTexImage2D(GL_TEXTURE_2D,
		             0,
		             GL_RGBA8,
		             (GLsizei)width,
		             (GLsizei)height,
		             0,
		             GL_RGBA,
		             GL_UNSIGNED_BYTE,
		             data->getData());
	}

	if (glGetError() != GL_NO_ERROR)
		throw love::Exception("Cannot create image: size may be too large for this system.");

	mipmapsCreated = false;
	checkMipmapsCreated();

	return true;
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

	// We want this lock to potentially cover mipmap creation as well.
	love::thread::EmptyLock lock;

	bind();

	if (isCompressed() && cdata)
	{
		GLenum format = getCompressedFormat(cdata->getType());
		glCompressedTexSubImage2DARB(GL_TEXTURE_2D,
		                             0,
		                             0, 0,
		                             cdata->getWidth(0),
		                             cdata->getHeight(0),
		                             format,
		                             GLsizei(cdata->getSize(0)),
		                             cdata->getData(0));
	}
	else if (data)
	{
		lock.setLock(data->getMutex());
		glTexSubImage2D(GL_TEXTURE_2D,
		                0,
		                0, 0,
		                (GLsizei)width,
		                (GLsizei)height,
		                GL_RGBA,
		                GL_UNSIGNED_BYTE,
		                data->getData());
	}

	mipmapsCreated = false;
	checkMipmapsCreated();

	return true;
}

void Image::drawv(const Matrix &t, const vertex *v, GLsizei count, GLenum mode) const
{
	bind();

	glPushMatrix();

	glMultMatrixf((const GLfloat *)t.getElements());

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	// XXX: drawg() enables/disables GL_COLOR_ARRAY in order to use the color
	//      defined in the geometry to draw itself.
	//      if the drawing method below is changed to use something other than
	//      glDrawArrays(), drawg() needs to be updated accordingly!
	glVertexPointer(2, GL_FLOAT, sizeof(vertex), (GLvoid *)&v[0].x);
	glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), (GLvoid *)&v[0].s);
	glDrawArrays(mode, 0, count);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glPopMatrix();
}

void Image::setDefaultMipmapSharpness(float sharpness)
{
	defaultMipmapSharpness = sharpness;
}

float Image::getDefaultMipmapSharpness()
{
	return defaultMipmapSharpness;
}

void Image::setDefaultMipmapFilter(Image::FilterMode f)
{
	defaultMipmapFilter = f;
}

Image::FilterMode Image::getDefaultMipmapFilter()
{
	return defaultMipmapFilter;
}

bool Image::isCompressed() const
{
	return compressed;
}

GLenum Image::getCompressedFormat(image::CompressedData::TextureType type) const
{
	switch (type)
	{
	case image::CompressedData::TYPE_DXT1:
		return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
	case image::CompressedData::TYPE_DXT3:
		return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
	case image::CompressedData::TYPE_DXT5:
		return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
	case image::CompressedData::TYPE_BC5:
		return GL_COMPRESSED_RG_RGTC2;
	case image::CompressedData::TYPE_BC5s:
		return GL_COMPRESSED_SIGNED_RG_RGTC2;
	default:
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
	return GLEE_VERSION_1_4 || GLEE_EXT_texture_lod_bias;
}

bool Image::hasCompressedTextureSupport()
{
	return GLEE_VERSION_1_3 || GLEE_ARB_texture_compression;
}

bool Image::hasCompressedTextureSupport(image::CompressedData::TextureType type)
{
	if (!hasCompressedTextureSupport())
		return false;

	switch (type)
	{
	case image::CompressedData::TYPE_DXT1:
	case image::CompressedData::TYPE_DXT3:
	case image::CompressedData::TYPE_DXT5:
		return GLEE_EXT_texture_compression_s3tc;
	case image::CompressedData::TYPE_BC5:
	case image::CompressedData::TYPE_BC5s:
		return (GLEE_VERSION_3_0 || GLEE_ARB_texture_compression_rgtc || GLEE_EXT_texture_compression_rgtc);
	default:
		break;
	}

	return false;
}

} // opengl
} // graphics
} // love
