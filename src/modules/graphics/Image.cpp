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

namespace love
{
namespace graphics
{

love::Type Image::type("Image", &Texture::type);

int Image::imageCount = 0;

Image::Image(const Settings &settings)
	: settings(settings)
{
	++imageCount;
}

Image::~Image()
{
	--imageCount;
}

const Image::Settings &Image::getFlags() const
{
	return settings;
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

} // graphics
} // love
