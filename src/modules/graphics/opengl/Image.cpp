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

#include "Image.h"

#include "graphics/Graphics.h"
#include "common/int.h"

// STD
#include <algorithm> // for min/max

#ifdef LOVE_ANDROID
// log2 is not declared in the math.h shipped with the Android NDK
#include <cmath>
inline double log2(double n)
{ 
	// log(n)/log(2) is log2.  
	return std::log(n) / std::log(2);
}
#endif

namespace love
{
namespace graphics
{
namespace opengl
{

int Image::imageCount = 0;

float Image::maxMipmapSharpness = 0.0f;

Texture::FilterMode Image::defaultMipmapFilter = Texture::FILTER_LINEAR;
float Image::defaultMipmapSharpness = 0.0f;

static int getMipmapCount(int basewidth, int baseheight)
{
	return (int) log2(std::max(basewidth, baseheight)) + 1;
}

template <typename T>
static bool verifyMipmapLevels(const std::vector<T> &miplevels)
{
	int numlevels = (int) miplevels.size();

	if (numlevels == 1)
		return false;

	int width  = miplevels[0]->getWidth();
	int height = miplevels[0]->getHeight();

	int expectedlevels = getMipmapCount(width, height);

	// All mip levels must be present when not using auto-generated mipmaps.
	if (numlevels != expectedlevels)
		throw love::Exception("Image does not have all required mipmap levels (expected %d, got %d)", expectedlevels, numlevels);

	// Verify the size of each mip level.
	for (int i = 1; i < numlevels; i++)
	{
		width  = std::max(width / 2, 1);
		height = std::max(height / 2, 1);

		if (miplevels[i]->getWidth() != width)
			throw love::Exception("Width of image mipmap level %d is incorrect (expected %d, got %d)", i+1, width, miplevels[i]->getWidth());
		if (miplevels[i]->getHeight() != height)
			throw love::Exception("Height of image mipmap level %d is incorrect (expected %d, got %d)", i+1, height, miplevels[i]->getHeight());
	}

	return true;
}

Image::Image(const std::vector<love::image::ImageData *> &imagedata, const Flags &flags)
	: texture(0)
	, mipmapSharpness(defaultMipmapSharpness)
	, compressed(false)
	, flags(flags)
	, sRGB(false)
	, usingDefaultTexture(false)
	, textureMemorySize(0)
{
	if (imagedata.empty())
		throw love::Exception("");

	width = imagedata[0]->getWidth();
	height = imagedata[0]->getHeight();

	if (verifyMipmapLevels(imagedata))
		this->flags.mipmaps = true;

	for (const auto &id : imagedata)
		data.push_back(id);

	preload();
	loadVolatile();

	++imageCount;
}

Image::Image(const std::vector<love::image::CompressedImageData *> &compresseddata, const Flags &flags)
	: texture(0)
	, mipmapSharpness(defaultMipmapSharpness)
	, compressed(true)
	, flags(flags)
	, sRGB(false)
	, usingDefaultTexture(false)
	, textureMemorySize(0)
{
	width = compresseddata[0]->getWidth(0);
	height = compresseddata[0]->getHeight(0);

	if (verifyMipmapLevels(compresseddata))
		this->flags.mipmaps = true;
	else if (flags.mipmaps && getMipmapCount(width, height) != compresseddata[0]->getMipmapCount())
	{
		if (compresseddata[0]->getMipmapCount() == 1)
			this->flags.mipmaps = false;
		else
		{
			throw love::Exception("Image cannot have mipmaps: compressed image data does not have all required mipmap levels (expected %d, got %d)",
			                      getMipmapCount(width, height),
			                      compresseddata[0]->getMipmapCount());
		}
	}

	for (const auto &cd : compresseddata)
	{
		cdata.push_back(cd);
		if (cd->getFormat() != cdata[0]->getFormat())
			throw love::Exception("All image mipmap levels must have the same format.");
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

	if (!isGammaCorrect())
		flags.linear = false;

	if (isGammaCorrect() && !flags.linear)
		sRGB = true;
	else
		sRGB = false;
}

void Image::generateMipmaps()
{
	// The GL_GENERATE_MIPMAP texparameter is set in loadVolatile if we don't
	// have support for glGenerateMipmap.
	if (flags.mipmaps && !isCompressed() &&
		(GLAD_ES_VERSION_2_0 || GLAD_VERSION_3_0 || GLAD_ARB_framebuffer_object))
	{
		if (gl.bugs.generateMipmapsRequiresTexture2DEnable)
			glEnable(GL_TEXTURE_2D);

		glGenerateMipmap(GL_TEXTURE_2D);
	}
}

void Image::loadDefaultTexture()
{
	usingDefaultTexture = true;

	gl.bindTexture(texture);
	setFilter(filter);

	// A nice friendly checkerboard to signify invalid textures...
	GLubyte px[] = {0xFF,0xFF,0xFF,0xFF, 0xFF,0xA0,0xA0,0xFF,
	                0xFF,0xA0,0xA0,0xFF, 0xFF,0xFF,0xFF,0xFF};

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
}

void Image::loadFromCompressedData()
{
	GLenum iformat = getCompressedFormat(cdata[0]->getFormat(), sRGB);

	if (isGammaCorrect() && !sRGB)
		flags.linear = true;

	int count = 1;

	if (flags.mipmaps && cdata.size() > 1)
		count = (int) cdata.size();
	else if (flags.mipmaps)
		count = cdata[0]->getMipmapCount();

	for (int i = 0; i < count; i++)
	{
		// Compressed image mipmaps can come from separate CompressedImageData
		// objects, or all from a single object.
		auto cd = cdata.size() > 1 ? cdata[i].get() : cdata[0].get();
		int datamip = cdata.size() > 1 ? 0 : i;

		glCompressedTexImage2D(GL_TEXTURE_2D, i, iformat, cd->getWidth(datamip),
		                       cd->getHeight(datamip), 0,
		                       (GLsizei) cd->getSize(datamip), cd->getData(datamip));
	}
}

void Image::loadFromImageData()
{
	GLenum iformat = sRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;
	GLenum format  = GL_RGBA;

	// in GLES2, the internalformat and format params of TexImage have to match.
	if (GLAD_ES_VERSION_2_0 && !GLAD_ES_VERSION_3_0)
	{
		format  = sRGB ? GL_SRGB_ALPHA : GL_RGBA;
		iformat = format;
	}

	int mipcount = flags.mipmaps ? (int) data.size() : 1;

	for (int i = 0; i < mipcount; i++)
	{
		love::image::ImageData *id = data[i].get();
		love::thread::Lock lock(id->getMutex());

		glTexImage2D(GL_TEXTURE_2D, i, iformat, id->getWidth(), id->getHeight(),
		             0, format, GL_UNSIGNED_BYTE, id->getData());
	}

	if (data.size() <= 1)
		generateMipmaps();
}

bool Image::loadVolatile()
{
	OpenGL::TempDebugGroup debuggroup("Image load");

	if (isCompressed() && !hasCompressedTextureSupport(cdata[0]->getFormat(), sRGB))
	{
		const char *str;
		if (image::CompressedImageData::getConstant(cdata[0]->getFormat(), str))
		{
			throw love::Exception("Cannot create image: "
			                      "%s%s compressed images are not supported on this system.", sRGB ? "sRGB " : "", str);
		}
		else
			throw love::Exception("cannot create image: format is not supported on this system.");
	}
	else if (!isCompressed())
	{
		if (sRGB && !hasSRGBSupport())
			throw love::Exception("sRGB images are not supported on this system.");

		// GL_EXT_sRGB doesn't support glGenerateMipmap for sRGB textures.
		if (sRGB && (GLAD_ES_VERSION_2_0 && GLAD_EXT_sRGB && !GLAD_ES_VERSION_3_0)
			&& data.size() <= 1)
		{
			flags.mipmaps = false;
			filter.mipmap = FILTER_NONE;
		}
	}

	// NPOT textures don't support mipmapping without full NPOT support.
	if ((GLAD_ES_VERSION_2_0 && !(GLAD_ES_VERSION_3_0 || GLAD_OES_texture_npot))
		&& (width != nextP2(width) || height != nextP2(height)))
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

	if (!flags.mipmaps && (GLAD_ES_VERSION_3_0 || GLAD_VERSION_1_0))
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	if (flags.mipmaps && !isCompressed() && data.size() <= 1 &&
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
			throw love::Exception("Cannot create image (OpenGL error: %s)", OpenGL::errorString(glerr));
	}
	catch (love::Exception &)
	{
		gl.deleteTexture(texture);
		texture = 0;
		throw;
	}

	size_t prevmemsize = textureMemorySize;

	if (isCompressed())
		textureMemorySize = cdata[0]->getSize();
	else
		textureMemorySize = data[0]->getSize();

	if (flags.mipmaps)
		textureMemorySize *= 1.33334;

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

	OpenGL::TempDebugGroup debuggroup("Image refresh");

	gl.bindTexture(texture);

	if (isCompressed())
	{
		loadFromCompressedData();
		return true;
	}

	GLenum format = GL_RGBA;

	// In ES2, the format parameter of TexSubImage must match the internal
	// format of the texture.
	if (sRGB && (GLAD_ES_VERSION_2_0 && !GLAD_ES_VERSION_3_0))
		format = GL_SRGB_ALPHA;

	int mipcount = flags.mipmaps ? (int) data.size() : 1;

	// Reupload the sub-rectangle of each mip level (if we have custom mipmaps.)
	for (int i = 0; i < mipcount; i++)
	{
		const image::pixel *pdata = (const image::pixel *) data[i]->getData();
		pdata += yoffset * data[i]->getWidth() + xoffset;

		thread::Lock lock(data[i]->getMutex());
		glTexSubImage2D(GL_TEXTURE_2D, i, xoffset, yoffset, w, h, format,
						GL_UNSIGNED_BYTE, pdata);

		xoffset /= 2;
		yoffset /= 2;
		w = std::max(w / 2, 1);
		h = std::max(h / 2, 1);
	}

	if (data.size() <= 1)
		generateMipmaps();

	return true;
}

void Image::drawv(const Matrix4 &t, const Vertex *v)
{
	OpenGL::TempDebugGroup debuggroup("Image draw");

	OpenGL::TempTransform transform(gl);
	transform.get() *= t;

	gl.bindTexture(texture);

	gl.useVertexAttribArrays(ATTRIBFLAG_POS | ATTRIBFLAG_TEXCOORD);

	glVertexAttribPointer(ATTRIB_POS, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), &v[0].x);
	glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), &v[0].s);

	gl.prepareDraw();
	gl.drawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Image::draw(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky)
{
	Matrix4 t(x, y, angle, sx, sy, ox, oy, kx, ky);

	drawv(t, vertices);
}

void Image::drawq(Quad *quad, float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky)
{
	Matrix4 t(x, y, angle, sx, sy, ox, oy, kx, ky);

	drawv(t, quad->getVertices());
}

const void *Image::getHandle() const
{
	return &texture;
}

const std::vector<StrongRef<love::image::ImageData>> &Image::getImageData() const
{
	return data;
}

const std::vector<StrongRef<love::image::CompressedImageData>> &Image::getCompressedData() const
{
	return cdata;
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

	gl.bindTexture(texture);
	gl.setTextureWrap(wrap);

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

GLenum Image::getCompressedFormat(image::CompressedImageData::Format cformat, bool &isSRGB) const
{
	using image::CompressedImageData;

	switch (cformat)
	{
	case CompressedImageData::FORMAT_DXT1:
		return isSRGB ? GL_COMPRESSED_SRGB_S3TC_DXT1_EXT : GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
	case CompressedImageData::FORMAT_DXT3:
		return isSRGB ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT : GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
	case CompressedImageData::FORMAT_DXT5:
		return isSRGB ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT : GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
	case CompressedImageData::FORMAT_BC4:
		isSRGB = false;
		return GL_COMPRESSED_RED_RGTC1;
	case CompressedImageData::FORMAT_BC4s:
		isSRGB = false;
		return GL_COMPRESSED_SIGNED_RED_RGTC1;
	case CompressedImageData::FORMAT_BC5:
		isSRGB = false;
		return GL_COMPRESSED_RG_RGTC2;
	case CompressedImageData::FORMAT_BC5s:
		isSRGB = false;
		return GL_COMPRESSED_SIGNED_RG_RGTC2;
	case CompressedImageData::FORMAT_BC6H:
		isSRGB = false;
		return GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT;
	case CompressedImageData::FORMAT_BC6Hs:
		isSRGB = false;
		return GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT;
	case CompressedImageData::FORMAT_BC7:
		return isSRGB ? GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM : GL_COMPRESSED_RGBA_BPTC_UNORM;
	case CompressedImageData::FORMAT_PVR1_RGB2:
		return isSRGB ? GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT : GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
	case CompressedImageData::FORMAT_PVR1_RGB4:
		return isSRGB ? GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT : GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
	case CompressedImageData::FORMAT_PVR1_RGBA2:
		return isSRGB ? GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT : GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
	case CompressedImageData::FORMAT_PVR1_RGBA4:
		return isSRGB ? GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT : GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
	case CompressedImageData::FORMAT_ETC1:
		// The ETC2 format can load ETC1 textures.
		if (GLAD_ES_VERSION_3_0 || GLAD_VERSION_4_3 || GLAD_ARB_ES3_compatibility)
			return isSRGB ? GL_COMPRESSED_SRGB8_ETC2 : GL_COMPRESSED_RGB8_ETC2;
		else
		{
			isSRGB = false;
			return GL_ETC1_RGB8_OES;
		}
	case CompressedImageData::FORMAT_ETC2_RGB:
		return isSRGB ? GL_COMPRESSED_SRGB8_ETC2 : GL_COMPRESSED_RGB8_ETC2;
	case CompressedImageData::FORMAT_ETC2_RGBA:
		return isSRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC : GL_COMPRESSED_RGBA8_ETC2_EAC;
	case CompressedImageData::FORMAT_ETC2_RGBA1:
		return isSRGB ? GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 : GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
	case CompressedImageData::FORMAT_EAC_R:
		isSRGB = false;
		return GL_COMPRESSED_R11_EAC;
	case CompressedImageData::FORMAT_EAC_Rs:
		isSRGB = false;
		return GL_COMPRESSED_SIGNED_R11_EAC;
	case CompressedImageData::FORMAT_EAC_RG:
		isSRGB = false;
		return GL_COMPRESSED_RG11_EAC;
	case CompressedImageData::FORMAT_EAC_RGs:
		isSRGB = false;
		return GL_COMPRESSED_SIGNED_RG11_EAC;
	case CompressedImageData::FORMAT_ASTC_4x4:
		return isSRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR : GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
	case CompressedImageData::FORMAT_ASTC_5x4:
		return isSRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR : GL_COMPRESSED_RGBA_ASTC_5x4_KHR;
	case CompressedImageData::FORMAT_ASTC_5x5:
		return isSRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR : GL_COMPRESSED_RGBA_ASTC_5x5_KHR;
	case CompressedImageData::FORMAT_ASTC_6x5:
		return isSRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR : GL_COMPRESSED_RGBA_ASTC_6x5_KHR;
	case CompressedImageData::FORMAT_ASTC_6x6:
		return isSRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR : GL_COMPRESSED_RGBA_ASTC_6x6_KHR;
	case CompressedImageData::FORMAT_ASTC_8x5:
		return isSRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR : GL_COMPRESSED_RGBA_ASTC_8x5_KHR;
	case CompressedImageData::FORMAT_ASTC_8x6:
		return isSRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR : GL_COMPRESSED_RGBA_ASTC_8x6_KHR;
	case CompressedImageData::FORMAT_ASTC_8x8:
		return isSRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR : GL_COMPRESSED_RGBA_ASTC_8x8_KHR;
	case CompressedImageData::FORMAT_ASTC_10x5:
		return isSRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR : GL_COMPRESSED_RGBA_ASTC_10x5_KHR;
	case CompressedImageData::FORMAT_ASTC_10x6:
		return isSRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR : GL_COMPRESSED_RGBA_ASTC_10x6_KHR;
	case CompressedImageData::FORMAT_ASTC_10x8:
		return isSRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR : GL_COMPRESSED_RGBA_ASTC_10x8_KHR;
	case CompressedImageData::FORMAT_ASTC_10x10:
		return isSRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR : GL_COMPRESSED_RGBA_ASTC_10x10_KHR;
	case CompressedImageData::FORMAT_ASTC_12x10:
		return isSRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR : GL_COMPRESSED_RGBA_ASTC_12x10_KHR;
	case CompressedImageData::FORMAT_ASTC_12x12:
		return isSRGB ? GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR : GL_COMPRESSED_RGBA_ASTC_12x12_KHR;
	default:
		return isSRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;
	}
}

bool Image::hasAnisotropicFilteringSupport()
{
	return GLAD_EXT_texture_filter_anisotropic != GL_FALSE;
}

bool Image::hasCompressedTextureSupport(image::CompressedImageData::Format format, bool sRGB)
{
	using image::CompressedImageData;

	switch (format)
	{
	case CompressedImageData::FORMAT_DXT1:
		return GLAD_EXT_texture_compression_s3tc || GLAD_EXT_texture_compression_dxt1;
	case CompressedImageData::FORMAT_DXT3:
		return GLAD_EXT_texture_compression_s3tc || GLAD_ANGLE_texture_compression_dxt3;
	case CompressedImageData::FORMAT_DXT5:
		return GLAD_EXT_texture_compression_s3tc || GLAD_ANGLE_texture_compression_dxt5;
	case CompressedImageData::FORMAT_BC4:
	case CompressedImageData::FORMAT_BC4s:
	case CompressedImageData::FORMAT_BC5:
	case CompressedImageData::FORMAT_BC5s:
		return (GLAD_VERSION_3_0 || GLAD_ARB_texture_compression_rgtc || GLAD_EXT_texture_compression_rgtc);
	case CompressedImageData::FORMAT_BC6H:
	case CompressedImageData::FORMAT_BC6Hs:
	case CompressedImageData::FORMAT_BC7:
		return GLAD_VERSION_4_2 || GLAD_ARB_texture_compression_bptc;
	case CompressedImageData::FORMAT_PVR1_RGB2:
	case CompressedImageData::FORMAT_PVR1_RGB4:
	case CompressedImageData::FORMAT_PVR1_RGBA2:
	case CompressedImageData::FORMAT_PVR1_RGBA4:
		return sRGB ? GLAD_EXT_pvrtc_sRGB : GLAD_IMG_texture_compression_pvrtc;
	case CompressedImageData::FORMAT_ETC1:
		// ETC2 support guarantees ETC1 support as well.
		return GLAD_ES_VERSION_3_0 || GLAD_VERSION_4_3 || GLAD_ARB_ES3_compatibility || GLAD_OES_compressed_ETC1_RGB8_texture;
	case CompressedImageData::FORMAT_ETC2_RGB:
	case CompressedImageData::FORMAT_ETC2_RGBA:
	case CompressedImageData::FORMAT_ETC2_RGBA1:
	case CompressedImageData::FORMAT_EAC_R:
	case CompressedImageData::FORMAT_EAC_Rs:
	case CompressedImageData::FORMAT_EAC_RG:
	case CompressedImageData::FORMAT_EAC_RGs:
		return GLAD_ES_VERSION_3_0 || GLAD_VERSION_4_3 || GLAD_ARB_ES3_compatibility;
	case CompressedImageData::FORMAT_ASTC_4x4:
	case CompressedImageData::FORMAT_ASTC_5x4:
	case CompressedImageData::FORMAT_ASTC_5x5:
	case CompressedImageData::FORMAT_ASTC_6x5:
	case CompressedImageData::FORMAT_ASTC_6x6:
	case CompressedImageData::FORMAT_ASTC_8x5:
	case CompressedImageData::FORMAT_ASTC_8x6:
	case CompressedImageData::FORMAT_ASTC_8x8:
	case CompressedImageData::FORMAT_ASTC_10x5:
	case CompressedImageData::FORMAT_ASTC_10x6:
	case CompressedImageData::FORMAT_ASTC_10x8:
	case CompressedImageData::FORMAT_ASTC_10x10:
	case CompressedImageData::FORMAT_ASTC_12x10:
	case CompressedImageData::FORMAT_ASTC_12x12:
		return GLAD_ES_VERSION_3_2 || GLAD_KHR_texture_compression_astc_ldr;
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
	{"mipmaps", FLAG_TYPE_MIPMAPS},
	{"linear", FLAG_TYPE_LINEAR},
};

StringMap<Image::FlagType, Image::FLAG_TYPE_MAX_ENUM> Image::flagNames(Image::flagNameEntries, sizeof(Image::flagNameEntries));

} // opengl
} // graphics
} // love
