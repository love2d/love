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

Image::Image(TextureType textype, const Settings &settings)
	: Texture(textype)
	, settings(settings)
{
	renderTarget = false;
	sRGB = isGammaCorrect() && !settings.linear;
}

Image::Image(TextureType textype, PixelFormat format, int width, int height, int slices, const Settings &settings)
	: Image(textype, settings)
{
	if (isPixelFormatCompressed(format))
		throw love::Exception("This constructor is only supported for non-compressed pixel formats.");

	if (textype == TEXTURE_2D_ARRAY)
		layers = slices;
	else if (textype == TEXTURE_VOLUME)
		depth = slices;

	init(format, width, height, 1, settings);
}

Image::Image(const Slices &slices, const Settings &settings)
	: Image(slices.getTextureType(), settings)
{
	int dataMipmaps = 1;
	if (slices.validate() && slices.getMipmapCount() > 1)
		dataMipmaps = slices.getMipmapCount();

	if (texType == TEXTURE_2D_ARRAY)
		this->layers = slices.getSliceCount();
	else if (texType == TEXTURE_VOLUME)
		this->depth = slices.getSliceCount();

	love::image::ImageDataBase *slice = slices.get(0, 0);
	init(slice->getFormat(), slice->getWidth(), slice->getHeight(), dataMipmaps, settings);
}

Image::~Image()
{
}

void Image::init(PixelFormat fmt, int w, int h, int dataMipmaps, const Settings &settings)
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
	mipmapsMode = settings.mipmaps ? MIPMAPS_MANUAL : MIPMAPS_NONE;

	width  = (int) (pixelWidth / settings.dpiScale + 0.5);
	height = (int) (pixelHeight / settings.dpiScale + 0.5);

	format = fmt;

	if (mipmapsMode == MIPMAPS_NONE || (isCompressed() && dataMipmaps <= 1))
		mipmapCount = 1;
	else
		mipmapCount = getTotalMipmapCount(w, h, depth);

	initQuad();
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
