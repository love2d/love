/**
 * Copyright (c) 2006-2015 LOVE Development Team
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

#include "common/int.h"

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
	loadVolatile();

	++imageCount;
}

Image::Image(love::image::CompressedImageData *cdata, const Flags &flags)
	: data(nullptr)
	, cdata(cdata)
	, texture(0)
	, mipmapSharpness(defaultMipmapSharpness)
	, compressed(true)
	, flags(flags)
	, usingDefaultTexture(false)
	, textureMemorySize(0)
{
	this->flags.sRGB = (flags.sRGB || cdata->isSRGB());

	width = cdata->getWidth(0);
	height = cdata->getHeight(0);

	if (flags.mipmaps)
	{
		// The mipmap texture data comes from the CompressedImageData in this case,
		// so we should make sure it has all necessary mipmap levels.
		if (cdata->getMipmapCount() < (int) log2(std::max(width, height)) + 1)
			throw love::Exception("Image cannot have mipmaps: compressed image data does not have all required mipmap levels.");
	}

	preload();
	loadVolatile();

	++imageCount;
}

Image::~Image()
{
	unloadVolatile();
	--imageCount;
}

void Image::preload()
{
	for (int i = 0; i < 4; i++)
		vertices[i].r = vertices[i].g = vertices[i].b = vertices[i].a = 255;

	// Vertices are ordered for use with triangle strips:
	// 0----2
	// |  / |
	// | /  |
	// 1----3
	vertices[0].x = 0.0f;
	vertices[0].y = 0.0f;
	vertices[1].x = 0.0f;
	vertices[1].y = (float) height;
	vertices[2].x = (float) width;
	vertices[2].y = 0.0f;
	vertices[3].x = (float) width;
	vertices[3].y = (float) height;

	vertices[0].s = 0.0f;
	vertices[0].t = 0.0f;
	vertices[1].s = 0.0f;
	vertices[1].t = 1.0f;
	vertices[2].s = 1.0f;
	vertices[2].t = 0.0f;
	vertices[3].s = 1.0f;
	vertices[3].t = 1.0f;

	if (flags.mipmaps)
		filter.mipmap = defaultMipmapFilter;
}

void Image::generateMipmaps()
{
	// The GL_GENERATE_MIPMAP texparameter is set in loadVolatile if we don't
	// have support for glGenerateMipmap.
	if (flags.mipmaps && !isCompressed() &&
		(GLAD_ES_VERSION_2_0 || GLAD_VERSION_3_0 || GLAD_ARB_framebuffer_object))
	{
		// Driver bug: http://www.opengl.org/wiki/Common_Mistakes#Automatic_mipmap_generation
#if defined(LOVE_WINDOWS) || defined(LOVE_LINUX)
		if (gl.getVendor() == OpenGL::VENDOR_AMD)
			glEnable(GL_TEXTURE_2D);
#endif

		glGenerateMipmap(GL_TEXTURE_2D);
	}
}

void Image::loadDefaultTexture()
{
	usingDefaultTexture = true;

	gl.bindTexture(texture);
	setFilter(filter);

	// A nice friendly checkerboard to signify invalid textures...
	GLubyte px[] = {0xFF,0xFF,0xFF,0xFF, 0xFF,0xC0,0xC0,0xFF,
	                0xFF,0xC0,0xC0,0xFF, 0xFF,0xFF,0xFF,0xFF};

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
}

void Image::loadFromCompressedData()
{
	GLenum iformat = getCompressedFormat(cdata->getFormat());
	int count = flags.mipmaps ? cdata->getMipmapCount() : 1;

	for (int i = 0; i < count; i++)
	{
		glCompressedTexImage2D(GL_TEXTURE_2D, i, iformat,
		                       cdata->getWidth(i), cdata->getHeight(i), 0,
		                       (GLsizei) cdata->getSize(i), cdata->getData(i));
	}
}

void Image::loadFromImageData()
{
	GLenum iformat = flags.sRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;
	GLenum format  = GL_RGBA;

	// in GLES2, the internalformat and format params of TexImage have to match.
	if (GLAD_ES_VERSION_2_0 && !GLAD_ES_VERSION_3_0)
	{
		format  = flags.sRGB ? GL_SRGB_ALPHA : GL_RGBA;
		iformat = format;
	}

	{
		love::thread::Lock lock(data->getMutex());
		glTexImage2D(GL_TEXTURE_2D, 0, iformat, width, height, 0, format,
		             GL_UNSIGNED_BYTE, data->getData());
	}

	generateMipmaps();
}

bool Image::loadVolatile()
{
	OpenGL::TempDebugGroup debuggroup("Image load");

	if (isCompressed() && !hasCompressedTextureSupport(cdata->getFormat(), flags.sRGB))
	{
		const char *str;
		if (image::CompressedImageData::getConstant(cdata->getFormat(), str))
		{
			throw love::Exception("Cannot create image: "
			                      "%s%s compressed images are not supported on this system.", flags.sRGB ? "sRGB " : "", str);
		}
		else
			throw love::Exception("cannot create image: format is not supported on this system.");
	}
	else if (!isCompressed())
	{
		if (flags.sRGB && !hasSRGBSupport())
			throw love::Exception("sRGB images are not supported on this system.");

		// GL_EXT_sRGB doesn't support glGenerateMipmap for sRGB textures.
		if (flags.sRGB && (GLAD_ES_VERSION_2_0 && GLAD_EXT_sRGB && !GLAD_ES_VERSION_3_0))
		{
			flags.mipmaps = false;
			filter.mipmap = FILTER_NONE;
		}
	}

	// NPOT textures don't support mipmapping without full NPOT support.
	if ((GLAD_ES_VERSION_2_0 && !(GLAD_ES_VERSION_3_0 || GLAD_OES_texture_npot))
		&& (width != next_p2(width) || height != next_p2(height)))
	{
		flags.mipmaps = false;
		filter.mipmap = FILTER_NONE;
	}

	if (maxMipmapSharpness == 0.0f && GLAD_VERSION_1_4)
		glGetFloatv(GL_MAX_TEXTURE_LOD_BIAS, &maxMipmapSharpness);

	glGenTextures(1, &texture);
	gl.bindTexture(texture);

	setFilter(filter);
	setWrap(wrap);
	setMipmapSharpness(mipmapSharpness);

	// Use a default texture if the size is too big for the system.
	if (width > gl.getMaxTextureSize() || height > gl.getMaxTextureSize())
	{
		loadDefaultTexture();
		return true;
	}

	if ((isCompressed() || !flags.mipmaps) && (GLAD_ES_VERSION_3_0 || GLAD_VERSION_1_0))
	{
		int count = (flags.mipmaps && isCompressed()) ? cdata->getMipmapCount() : 1;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, count - 1);
	}

	if (flags.mipmaps && !isCompressed() &&
		!(GLAD_ES_VERSION_2_0 || GLAD_VERSION_3_0 || GLAD_ARB_framebuffer_object))
	{
		// Auto-generate mipmaps every time the texture is modified, if
		// glGenerateMipmap isn't supported.
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	}

	while (glGetError() != GL_NO_ERROR); // Clear errors.

	try
	{
		if (isCompressed())
			loadFromCompressedData();
		else
			loadFromImageData();

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
		for (int i = 0; i < (flags.mipmaps ? cdata->getMipmapCount() : 1); i++)
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
	if (texture == 0)
		return;

	gl.deleteTexture(texture);
	texture = 0;
	
	gl.updateTextureMemorySize(textureMemorySize, 0);
	textureMemorySize = 0;
}

bool Image::refresh(int xoffset, int yoffset, int w, int h)
{
	// No effect if the texture hasn't been created yet.
	if (texture == 0 || usingDefaultTexture)
		return false;

	if (xoffset < 0 || yoffset < 0 || w <= 0 || h <= 0 ||
		(xoffset + w) > width || (yoffset + h) > height)
	{
		throw love::Exception("Invalid rectangle dimensions.");
	}

	gl.bindTexture(texture);

	if (isCompressed())
		loadFromCompressedData();
	else
	{
		const image::pixel *pdata = (const image::pixel *) data->getData();
		pdata += yoffset * data->getWidth() + xoffset;

		{
			thread::Lock lock(data->getMutex());
			glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, w, h, GL_RGBA,
			                GL_UNSIGNED_BYTE, pdata);
		}

		generateMipmaps();
	}
	
	return true;
}

void Image::drawv(const Matrix &t, const Vertex *v)
{
	OpenGL::TempDebugGroup debuggroup("Image draw");

	OpenGL::TempTransform transform(gl);
	transform.get() *= t;

	gl.bindTexture(texture);

	glEnableVertexAttribArray(ATTRIB_POS);
	glEnableVertexAttribArray(ATTRIB_TEXCOORD);

	glVertexAttribPointer(ATTRIB_POS, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), &v[0].x);
	glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), &v[0].s);

	gl.prepareDraw();
	gl.drawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(ATTRIB_TEXCOORD);
	glDisableVertexAttribArray(ATTRIB_POS);
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

const void *Image::getHandle() const
{
	return &texture;
}

love::image::ImageData *Image::getImageData() const
{
	return data.get();
}

love::image::CompressedImageData *Image::getCompressedData() const
{
	return cdata.get();
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

	gl.bindTexture(texture);
	gl.setTextureFilter(filter);
}

bool Image::setWrap(const Texture::Wrap &w)
{
	bool success = true;
	wrap = w;

	if ((GLAD_ES_VERSION_2_0 && !(GLAD_ES_VERSION_3_0 || GLAD_OES_texture_npot))
		&& (width != next_p2(width) || height != next_p2(height)))
	{
		if (wrap.s != WRAP_CLAMP || wrap.t != WRAP_CLAMP)
			success = false;

		// If we only have limited NPOT support then the wrap mode must be CLAMP.
		wrap.s = wrap.t = WRAP_CLAMP;
	}

	gl.bindTexture(texture);
	gl.setTextureWrap(w);

	return success;
}

void Image::setMipmapSharpness(float sharpness)
{
	// OpenGL ES doesn't support LOD bias via glTexParameter.
	if (!GLAD_VERSION_1_4)
		return;

	// LOD bias has the range (-maxbias, maxbias)
	mipmapSharpness = std::min(std::max(sharpness, -maxMipmapSharpness + 0.01f), maxMipmapSharpness - 0.01f);

	gl.bindTexture(texture);

	// negative bias is sharper
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -mipmapSharpness);
}

float Image::getMipmapSharpness() const
{
	return mipmapSharpness;
}

const Image::Flags &Image::getFlags() const
{
	return flags;
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

GLenum Image::getCompressedFormat(image::CompressedImageData::Format cformat) const
{
	switch (cformat)
	{
	case image::CompressedImageData::FORMAT_DXT1:
		if (flags.sRGB)
			return GL_COMPRESSED_SRGB_S3TC_DXT1_EXT;
		else
			return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
	case image::CompressedImageData::FORMAT_DXT3:
		if (flags.sRGB)
			return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
		else
			return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
	case image::CompressedImageData::FORMAT_DXT5:
		if (flags.sRGB)
			return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
		else
			return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
	case image::CompressedImageData::FORMAT_BC4:
		return GL_COMPRESSED_RED_RGTC1;
	case image::CompressedImageData::FORMAT_BC4s:
		return GL_COMPRESSED_SIGNED_RED_RGTC1;
	case image::CompressedImageData::FORMAT_BC5:
		return GL_COMPRESSED_RG_RGTC2;
	case image::CompressedImageData::FORMAT_BC5s:
		return GL_COMPRESSED_SIGNED_RG_RGTC2;
	case image::CompressedImageData::FORMAT_BC6H:
		return GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT;
	case image::CompressedImageData::FORMAT_BC6Hs:
		return GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT;
	case image::CompressedImageData::FORMAT_BC7:
		if (flags.sRGB)
			return GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM;
		else
			return GL_COMPRESSED_RGBA_BPTC_UNORM;
	case image::CompressedImageData::FORMAT_ETC1:
		// The ETC2 format can load ETC1 textures.
		if (GLAD_ES_VERSION_3_0 || GLAD_VERSION_4_3 || GLAD_ARB_ES3_compatibility)
			return GL_COMPRESSED_RGB8_ETC2;
		else
			return GL_ETC1_RGB8_OES;
	case image::CompressedImageData::FORMAT_ETC2_RGB:
		if (flags.sRGB)
			return GL_COMPRESSED_SRGB8_ETC2;
		else
			return GL_COMPRESSED_RGB8_ETC2;
	case image::CompressedImageData::FORMAT_ETC2_RGBA:
		if (flags.sRGB)
			return GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC;
		else
			return GL_COMPRESSED_RGBA8_ETC2_EAC;
	case image::CompressedImageData::FORMAT_ETC2_RGBA1:
		if (flags.sRGB)
			return GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2;
		else
			return GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
	case image::CompressedImageData::FORMAT_EAC_R:
		return GL_COMPRESSED_R11_EAC;
	case image::CompressedImageData::FORMAT_EAC_Rs:
		return GL_COMPRESSED_SIGNED_R11_EAC;
	case image::CompressedImageData::FORMAT_EAC_RG:
		return GL_COMPRESSED_RG11_EAC;
	case image::CompressedImageData::FORMAT_EAC_RGs:
		return GL_COMPRESSED_SIGNED_RG11_EAC;
	case image::CompressedImageData::FORMAT_PVR1_RGB2:
		if (flags.sRGB)
			return GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT;
		else
			return GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
	case image::CompressedImageData::FORMAT_PVR1_RGB4:
		if (flags.sRGB)
			return GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT;
		else
			return GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
	case image::CompressedImageData::FORMAT_PVR1_RGBA2:
		if (flags.sRGB)
			return GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT;
		else
			return GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
	case image::CompressedImageData::FORMAT_PVR1_RGBA4:
		if (flags.sRGB)
			return GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT;
		else
			return GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
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

bool Image::hasCompressedTextureSupport(image::CompressedImageData::Format format, bool sRGB)
{
	switch (format)
	{
	case image::CompressedImageData::FORMAT_DXT1:
		return GLAD_EXT_texture_compression_s3tc || GLAD_EXT_texture_compression_dxt1;
	case image::CompressedImageData::FORMAT_DXT3:
		return GLAD_EXT_texture_compression_s3tc || GLAD_ANGLE_texture_compression_dxt3;
	case image::CompressedImageData::FORMAT_DXT5:
		return GLAD_EXT_texture_compression_s3tc || GLAD_ANGLE_texture_compression_dxt5;
	case image::CompressedImageData::FORMAT_BC4:
	case image::CompressedImageData::FORMAT_BC4s:
	case image::CompressedImageData::FORMAT_BC5:
	case image::CompressedImageData::FORMAT_BC5s:
		return (GLAD_VERSION_3_0 || GLAD_ARB_texture_compression_rgtc || GLAD_EXT_texture_compression_rgtc);
	case image::CompressedImageData::FORMAT_BC6H:
	case image::CompressedImageData::FORMAT_BC6Hs:
	case image::CompressedImageData::FORMAT_BC7:
		return GLAD_VERSION_4_2 || GLAD_ARB_texture_compression_bptc;
	case image::CompressedImageData::FORMAT_ETC1:
		// ETC2 support guarantees ETC1 support as well.
		return GLAD_ES_VERSION_3_0 || GLAD_VERSION_4_3 || GLAD_ARB_ES3_compatibility || GLAD_OES_compressed_ETC1_RGB8_texture;
	case image::CompressedImageData::FORMAT_ETC2_RGB:
	case image::CompressedImageData::FORMAT_ETC2_RGBA:
	case image::CompressedImageData::FORMAT_ETC2_RGBA1:
	case image::CompressedImageData::FORMAT_EAC_R:
	case image::CompressedImageData::FORMAT_EAC_Rs:
	case image::CompressedImageData::FORMAT_EAC_RG:
	case image::CompressedImageData::FORMAT_EAC_RGs:
		return GLAD_ES_VERSION_3_0 || GLAD_VERSION_4_3 || GLAD_ARB_ES3_compatibility;
	case image::CompressedImageData::FORMAT_PVR1_RGB2:
	case image::CompressedImageData::FORMAT_PVR1_RGB4:
	case image::CompressedImageData::FORMAT_PVR1_RGBA2:
	case image::CompressedImageData::FORMAT_PVR1_RGBA4:
		if (sRGB)
			return GLAD_EXT_pvrtc_sRGB;
		else
			return GLAD_IMG_texture_compression_pvrtc;
	default:
		return false;
	}
}

bool Image::hasSRGBSupport()
{
	return GLAD_ES_VERSION_3_0 || GLAD_EXT_sRGB || GLAD_VERSION_2_1 || GLAD_EXT_texture_sRGB;
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
