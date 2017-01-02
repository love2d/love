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

love::Type Image::type("Image", &Texture::type);

int Image::imageCount = 0;

float Image::maxMipmapSharpness = 0.0f;

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

	auto format = miplevels[0]->getFormat();

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

		if (miplevels[i]->getFormat() != format)
			throw love::Exception("All image mipmap levels must have the same format.");
	}

	return true;
}

Image::Image(const std::vector<love::image::ImageData *> &imagedata, const Settings &settings)
	: texture(0)
	, mipmapSharpness(defaultMipmapSharpness)
	, compressed(false)
	, settings(settings)
	, sRGB(false)
	, usingDefaultTexture(false)
	, textureMemorySize(0)
{
	if (imagedata.empty())
		throw love::Exception("");

	pixelWidth  = imagedata[0]->getWidth();
	pixelHeight = imagedata[0]->getHeight();

	width  = (int) (pixelWidth / settings.pixeldensity + 0.5);
	height = (int) (pixelHeight / settings.pixeldensity + 0.5);

	if (verifyMipmapLevels(imagedata))
		this->settings.mipmaps = true;

	for (const auto &id : imagedata)
		data.push_back(id);

	format = data[0]->getFormat();

	preload();
	loadVolatile();

	++imageCount;
}

Image::Image(const std::vector<love::image::CompressedImageData *> &compresseddata, const Settings &settings)
	: texture(0)
	, mipmapSharpness(defaultMipmapSharpness)
	, compressed(true)
	, settings(settings)
	, sRGB(false)
	, usingDefaultTexture(false)
	, textureMemorySize(0)
{
	pixelWidth  = compresseddata[0]->getWidth(0);
	pixelHeight = compresseddata[0]->getHeight(0);

	width  = (int) (pixelWidth / settings.pixeldensity + 0.5);
	height = (int) (pixelHeight / settings.pixeldensity + 0.5);

	if (verifyMipmapLevels(compresseddata))
		this->settings.mipmaps = true;
	else if (settings.mipmaps && getMipmapCount(pixelWidth, pixelHeight) != compresseddata[0]->getMipmapCount())
	{
		if (compresseddata[0]->getMipmapCount() == 1)
			this->settings.mipmaps = false;
		else
		{
			throw love::Exception("Image cannot have mipmaps: compressed image data does not have all required mipmap levels (expected %d, got %d)",
			                      getMipmapCount(width, height),
			                      compresseddata[0]->getMipmapCount());
		}
	}

	for (image::CompressedImageData *cd : compresseddata)
		cdata.push_back(cd);

	format = cdata[0]->getFormat();

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
		vertices[i].color = Color(255, 255, 255, 255);

	// Vertices are ordered for use with triangle strips:
	// 0---2
	// | / |
	// 1---3
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

	if (settings.mipmaps)
		filter.mipmap = defaultMipmapFilter;

	if (!isGammaCorrect())
		settings.linear = false;

	if (isGammaCorrect() && !settings.linear)
		sRGB = true;
	else
		sRGB = false;
}

void Image::generateMipmaps()
{
	// The GL_GENERATE_MIPMAP texparameter is set in loadVolatile if we don't
	// have support for glGenerateMipmap.
	if (settings.mipmaps && !isCompressed() &&
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

	gl.bindTextureToUnit(texture, 0, false);
	setFilter(filter);

	// A nice friendly checkerboard to signify invalid textures...
	GLubyte px[] = {0xFF,0xFF,0xFF,0xFF, 0xFF,0xA0,0xA0,0xFF,
	                0xFF,0xA0,0xA0,0xFF, 0xFF,0xFF,0xFF,0xFF};

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
}

void Image::loadFromCompressedData()
{
	OpenGL::TextureFormat fmt = OpenGL::convertPixelFormat(format, false, sRGB);

	if (isGammaCorrect() && !sRGB)
		settings.linear = true;

	int count = 1;

	if (settings.mipmaps && cdata.size() > 1)
		count = (int) cdata.size();
	else if (settings.mipmaps)
		count = cdata[0]->getMipmapCount();

	for (int i = 0; i < count; i++)
	{
		// Compressed image mipmaps can come from separate CompressedImageData
		// objects, or all from a single object.
		auto cd = cdata.size() > 1 ? cdata[i].get() : cdata[0].get();
		int datamip = cdata.size() > 1 ? 0 : i;

		glCompressedTexImage2D(GL_TEXTURE_2D, i, fmt.internalformat,
		                       cd->getWidth(datamip), cd->getHeight(datamip), 0,
		                       (GLsizei) cd->getSize(datamip), cd->getData(datamip));
	}
}

void Image::loadFromImageData()
{
	OpenGL::TextureFormat fmt = OpenGL::convertPixelFormat(format, false, sRGB);

	if (isGammaCorrect() && !sRGB)
		settings.linear = true;

	int mipcount = settings.mipmaps ? (int) data.size() : 1;

	for (int i = 0; i < mipcount; i++)
	{
		love::image::ImageData *id = data[i].get();
		love::thread::Lock lock(id->getMutex());

		glTexImage2D(GL_TEXTURE_2D, i, fmt.internalformat, id->getWidth(), id->getHeight(),
		             0, fmt.externalformat, fmt.type, id->getData());
	}

	if (data.size() <= 1)
		generateMipmaps();
}

bool Image::loadVolatile()
{
	if (texture != 0)
		return true;

	OpenGL::TempDebugGroup debuggroup("Image load");

	if (!OpenGL::isPixelFormatSupported(format, false, sRGB))
	{
		const char *str;
		if (love::getConstant(format, str))
		{
			throw love::Exception("Cannot create image: "
			                      "%s%s images are not supported on this system.", sRGB ? "sRGB " : "", str);
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
			settings.mipmaps = false;
			filter.mipmap = FILTER_NONE;
		}
	}

	// NPOT textures don't support mipmapping without full NPOT support.
	if ((GLAD_ES_VERSION_2_0 && !(GLAD_ES_VERSION_3_0 || GLAD_OES_texture_npot))
		&& (pixelWidth != nextP2(pixelWidth) || pixelHeight != nextP2(pixelHeight)))
	{
		settings.mipmaps = false;
		filter.mipmap = FILTER_NONE;
	}

	if (maxMipmapSharpness == 0.0f && GLAD_VERSION_1_4)
		glGetFloatv(GL_MAX_TEXTURE_LOD_BIAS, &maxMipmapSharpness);

	glGenTextures(1, &texture);
	gl.bindTextureToUnit(texture, 0, false);

	setFilter(filter);
	setWrap(wrap);
	setMipmapSharpness(mipmapSharpness);

	// Use a default texture if the size is too big for the system.
	if (pixelWidth > gl.getMaxTextureSize() || pixelHeight > gl.getMaxTextureSize())
	{
		loadDefaultTexture();
		return true;
	}

	if (!settings.mipmaps && (GLAD_ES_VERSION_3_0 || GLAD_VERSION_1_0))
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	if (settings.mipmaps && !isCompressed() && data.size() <= 1 &&
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

	if (settings.mipmaps)
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
		(xoffset + w) > pixelWidth || (yoffset + h) > pixelHeight)
	{
		throw love::Exception("Invalid rectangle dimensions.");
	}

	OpenGL::TempDebugGroup debuggroup("Image refresh");

	gl.bindTextureToUnit(texture, 0, false);

	if (isCompressed())
	{
		loadFromCompressedData();
		return true;
	}

	bool isSRGB = sRGB;
	OpenGL::TextureFormat fmt = OpenGL::convertPixelFormat(format, false, isSRGB);

	int mipcount = settings.mipmaps ? (int) data.size() : 1;

	// Reupload the sub-rectangle of each mip level (if we have custom mipmaps.)
	for (int i = 0; i < mipcount; i++)
	{
		const image::pixel *pdata = (const image::pixel *) data[i]->getData();
		pdata += yoffset * data[i]->getWidth() + xoffset;

		thread::Lock lock(data[i]->getMutex());
		glTexSubImage2D(GL_TEXTURE_2D, i, xoffset, yoffset, w, h,
		                fmt.externalformat, fmt.type, pdata);

		xoffset /= 2;
		yoffset /= 2;
		w = std::max(w / 2, 1);
		h = std::max(h / 2, 1);
	}

	if (data.size() <= 1)
		generateMipmaps();

	return true;
}

ptrdiff_t Image::getHandle() const
{
	return texture;
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
	if (!validateFilter(f, settings.mipmaps))
	{
		if (f.mipmap != FILTER_NONE && !settings.mipmaps)
			throw love::Exception("Non-mipmapped image cannot have mipmap filtering.");
		else
			throw love::Exception("Invalid texture filter.");
	}

	filter = f;

	if (!data.empty() && !OpenGL::hasTextureFilteringSupport(data[0]->getFormat()))
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

	gl.bindTextureToUnit(texture, 0, false);
	gl.setTextureFilter(filter);
}

bool Image::setWrap(const Texture::Wrap &w)
{
	bool success = true;
	wrap = w;

	if ((GLAD_ES_VERSION_2_0 && !(GLAD_ES_VERSION_3_0 || GLAD_OES_texture_npot))
		&& (pixelWidth != nextP2(pixelWidth) || pixelHeight != nextP2(pixelHeight)))
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

	gl.bindTextureToUnit(texture, 0, false);
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

	gl.bindTextureToUnit(texture, 0, false);

	// negative bias is sharper
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -mipmapSharpness);
}

float Image::getMipmapSharpness() const
{
	return mipmapSharpness;
}

const Image::Settings &Image::getFlags() const
{
	return settings;
}

bool Image::isCompressed() const
{
	return compressed;
}

bool Image::isFormatSupported(PixelFormat pixelformat)
{
	return OpenGL::isPixelFormatSupported(pixelformat, false, false);
}

bool Image::hasSRGBSupport()
{
	return GLAD_ES_VERSION_3_0 || GLAD_EXT_sRGB || GLAD_VERSION_2_1 || GLAD_EXT_texture_sRGB;
}

bool Image::getConstant(const char *in, SettingType &out)
{
	return settingTypes.find(in, out);
}

bool Image::getConstant(SettingType in, const char *&out)
{
	return settingTypes.find(in, out);
}

StringMap<Image::SettingType, Image::SETTING_MAX_ENUM>::Entry Image::settingTypeEntries[] =
{
	{ "mipmaps",      SETTING_MIPMAPS      },
	{ "linear",       SETTING_LINEAR       },
	{ "pixeldensity", SETTING_PIXELDENSITY },
};

StringMap<Image::SettingType, Image::SETTING_MAX_ENUM> Image::settingTypes(Image::settingTypeEntries, sizeof(Image::settingTypeEntries));

} // opengl
} // graphics
} // love
