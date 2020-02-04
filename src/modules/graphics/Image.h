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

#pragma once

// LOVE
#include "common/config.h"
#include "common/StringMap.h"
#include "common/math.h"
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
		SETTING_DPI_SCALE,
		SETTING_MAX_ENUM
	};

	struct Settings
	{
		bool mipmaps = false;
		bool linear = false;
		float dpiScale = 1.0f;
	};

	virtual ~Image();

	static bool getConstant(const char *in, SettingType &out);
	static bool getConstant(SettingType in, const char *&out);
	static const char *getConstant(SettingType in);
	static std::vector<std::string> getConstants(SettingType);

protected:

	Image(const Slices &data, const Settings &settings);
	Image(TextureType textype, PixelFormat format, int width, int height, int slices, const Settings &settings);

	// The settings used to initialize this Image.
	Settings settings;

private:

	Image(TextureType textype, const Settings &settings);

	void init(PixelFormat fmt, int w, int h, int dataMipmaps, const Settings &settings);

	static StringMap<SettingType, SETTING_MAX_ENUM>::Entry settingTypeEntries[];
	static StringMap<SettingType, SETTING_MAX_ENUM> settingTypes;

}; // Image

} // graphics
} // love
