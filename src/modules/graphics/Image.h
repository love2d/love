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

#pragma once

// LOVE
#include "common/config.h"
#include "common/StringMap.h"
#include "common/math.h"
#include "image/ImageData.h"
#include "image/CompressedImageData.h"
#include "Texture.h"

namespace love
{
namespace graphics
{

class Image : public Texture
{
public:

	static love::Type type;

	enum SettingType
	{
		SETTING_MIPMAPS,
		SETTING_LINEAR,
		SETTING_PIXELDENSITY,
		SETTING_MAX_ENUM
	};

	struct Settings
	{
		bool mipmaps = false;
		bool linear = false;
		float pixeldensity = 1.0f;
	};

	Image(const Settings &settings);
	virtual ~Image();

	virtual const std::vector<StrongRef<love::image::ImageData>> &getImageData() const = 0;
	virtual const std::vector<StrongRef<love::image::CompressedImageData>> &getCompressedData() const = 0;

	virtual void setMipmapSharpness(float sharpness) = 0;
	virtual float getMipmapSharpness() const = 0;

	virtual bool isCompressed() const = 0;

	virtual bool refresh(int xoffset, int yoffset, int w, int h) = 0;

	const Settings &getFlags() const;

	static bool getConstant(const char *in, SettingType &out);
	static bool getConstant(SettingType in, const char *&out);

	static int imageCount;

protected:

	// The settings used to initialize this Image.
	Settings settings;
	
	static StringMap<SettingType, SETTING_MAX_ENUM>::Entry settingTypeEntries[];
	static StringMap<SettingType, SETTING_MAX_ENUM> settingTypes;
	
}; // Image

} // graphics
} // love
