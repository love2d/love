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
#include "Graphics.h"

// C++
#include <algorithm>

namespace love
{
namespace graphics
{

love::Type Image::type("Image", &Texture::type);

Image::Image(const Slices &data, const Settings &settings, bool validatedata)
	: Texture(data.getTextureType())
	, settings(settings)
	, mipmapsType(settings.mipmaps ? MIPMAPS_GENERATED : MIPMAPS_NONE)
	, usingDefaultTexture(false)
{
	renderTarget = false;
	sRGB = isGammaCorrect() && !settings.linear;
	if (validatedata && data.validate() && data.getMipmapCount() > 1)
		mipmapsType = MIPMAPS_DATA;
}

Image::Image(TextureType textype, PixelFormat format, int width, int height, int slices, const Settings &settings)
	: Image(Slices(textype), settings, false)
{
	if (isPixelFormatCompressed(format))
		throw love::Exception("This constructor is only supported for non-compressed pixel formats.");

	if (textype == TEXTURE_2D_ARRAY)
		layers = slices;
	else if (textype == TEXTURE_VOLUME)
		depth = slices;

	init(format, width, height, settings);
}

Image::Image(const Slices &slices, const Settings &settings)
	: Image(slices, settings, true)
{
	if (texType == TEXTURE_2D_ARRAY)
		this->layers = slices.getSliceCount();
	else if (texType == TEXTURE_VOLUME)
		this->depth = slices.getSliceCount();

	love::image::ImageDataBase *slice = slices.get(0, 0);
	init(slice->getFormat(), slice->getWidth(), slice->getHeight(), settings);
}

Image::~Image()
{
}

void Image::init(PixelFormat fmt, int w, int h, const Settings &settings)
{
	Graphics *gfx = Module::getInstance<Graphics>(Module::M_GRAPHICS);
	if (gfx != nullptr && !gfx->isPixelFormatSupported(fmt, renderTarget, readable, sRGB))
	{
		const char *str;
		if (love::getConstant(fmt, str))
		{
			throw love::Exception("Cannot create image: "
								  "%s%s images are not supported on this system.", sRGB ? "sRGB " : "", str);
		}
		else
			throw love::Exception("cannot create image: format is not supported on this system.");
	}

	pixelWidth = w;
	pixelHeight = h;

	width  = (int) (pixelWidth / settings.dpiScale + 0.5);
	height = (int) (pixelHeight / settings.dpiScale + 0.5);

	format = fmt;

	if (isCompressed() && mipmapsType == MIPMAPS_GENERATED)
		mipmapsType = MIPMAPS_NONE;

	mipmapCount = mipmapsType == MIPMAPS_NONE ? 1 : getTotalMipmapCount(w, h, depth);

	initQuad();
}

void Image::uploadImageData(love::image::ImageDataBase *d, int level, int slice, int x, int y)
{
	love::image::ImageData *id = dynamic_cast<love::image::ImageData *>(d);

	love::thread::EmptyLock lock;
	if (id != nullptr)
		lock.setLock(id->getMutex());

	Rect rect = {x, y, d->getWidth(), d->getHeight()};
	uploadByteData(d->getFormat(), d->getData(), d->getSize(), level, slice, rect, d);
}

void Image::replacePixels(love::image::ImageDataBase *d, int slice, int mipmap, int x, int y, bool reloadmipmaps)
{
	// No effect if the texture hasn't been created yet.
	if (getHandle() == 0 || usingDefaultTexture)
		return;

	if (d->getFormat() != getPixelFormat())
		throw love::Exception("Pixel formats must match.");

	if (mipmap < 0 || (mipmapsType != MIPMAPS_DATA && mipmap > 0) || mipmap >= getMipmapCount())
		throw love::Exception("Invalid image mipmap index %d.", mipmap + 1);

	if (slice < 0 || (texType == TEXTURE_CUBE && slice >= 6)
		|| (texType == TEXTURE_VOLUME && slice >= getDepth(mipmap))
		|| (texType == TEXTURE_2D_ARRAY && slice >= getLayerCount()))
	{
		throw love::Exception("Invalid image slice index %d.", slice + 1);
	}

	Rect rect = {x, y, d->getWidth(), d->getHeight()};

	int mipw = getPixelWidth(mipmap);
	int miph = getPixelHeight(mipmap);

	if (rect.x < 0 || rect.y < 0 || rect.w <= 0 || rect.h <= 0
		|| (rect.x + rect.w) > mipw || (rect.y + rect.h) > miph)
	{
		throw love::Exception("Invalid rectangle dimensions (x=%d, y=%d, w=%d, h=%d) for %dx%d Image.", rect.x, rect.y, rect.w, rect.h, mipw, miph);
	}

	// We don't currently support partial updates of compressed textures.
	if (isPixelFormatCompressed(d->getFormat()) && (rect.x != 0 || rect.y != 0 || rect.w != mipw || rect.h != miph))
		throw love::Exception("Compressed textures only support replacing the entire Image.");

	Graphics::flushStreamDrawsGlobal();

	uploadImageData(d, mipmap, slice, x, y);

	if (reloadmipmaps && mipmap == 0 && getMipmapCount() > 1)
		generateMipmaps();
}

void Image::replacePixels(const void *data, size_t size, int slice, int mipmap, const Rect &rect, bool reloadmipmaps)
{
	Graphics::flushStreamDrawsGlobal();

	uploadByteData(format, data, size, mipmap, slice, rect, nullptr);

	if (reloadmipmaps && mipmap == 0 && getMipmapCount() > 1)
		generateMipmaps();
}

Image::MipmapsType Image::getMipmapsType() const
{
	return mipmapsType;
}

bool Image::getConstant(const char *in, SettingType &out)
{
	return settingTypes.find(in, out);
}

bool Image::getConstant(SettingType in, const char *&out)
{
	return settingTypes.find(in, out);
}

const char *Image::getConstant(SettingType in)
{
	const char *name = nullptr;
	getConstant(in, name);
	return name;
}

std::vector<std::string> Image::getConstants(SettingType)
{
	return settingTypes.getNames();
}

StringMap<Image::SettingType, Image::SETTING_MAX_ENUM>::Entry Image::settingTypeEntries[] =
{
	{ "mipmaps",  SETTING_MIPMAPS   },
	{ "linear",   SETTING_LINEAR    },
	{ "dpiscale", SETTING_DPI_SCALE },
};

StringMap<Image::SettingType, Image::SETTING_MAX_ENUM> Image::settingTypes(Image::settingTypeEntries, sizeof(Image::settingTypeEntries));

} // graphics
} // love
