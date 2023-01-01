/**
 * Copyright (c) 2006-2023 LOVE Development Team
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

	enum MipmapsType
	{
		MIPMAPS_NONE,
		MIPMAPS_DATA,
		MIPMAPS_GENERATED,
	};

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

	struct Slices
	{
	public:

		Slices(TextureType textype);

		void clear();
		void set(int slice, int mipmap, love::image::ImageDataBase *data);
		love::image::ImageDataBase *get(int slice, int mipmap) const;

		void add(love::image::CompressedImageData *cdata, int startslice, int startmip, bool addallslices, bool addallmips);

		int getSliceCount(int mip = 0) const;
		int getMipmapCount(int slice = 0) const;

		MipmapsType validate() const;

		TextureType getTextureType() const { return textureType; }

	private:

		TextureType textureType;

		// For 2D/Cube/2DArray texture types, each element in the data array has
		// an array of mipmap levels. For 3D texture types, each mipmap level
		// has an array of layers.
		std::vector<std::vector<StrongRef<love::image::ImageDataBase>>> data;

	}; // Slices

	virtual ~Image();

	void replacePixels(love::image::ImageDataBase *d, int slice, int mipmap, int x, int y, bool reloadmipmaps);
	void replacePixels(const void *data, size_t size, int slice, int mipmap, const Rect &rect, bool reloadmipmaps);

	bool isFormatLinear() const;
	bool isCompressed() const;
	MipmapsType getMipmapsType() const;

	static int imageCount;

	static bool getConstant(const char *in, SettingType &out);
	static bool getConstant(SettingType in, const char *&out);
	static const char *getConstant(SettingType in);
	static std::vector<std::string> getConstants(SettingType);

protected:

	Image(const Slices &data, const Settings &settings);
	Image(TextureType textype, PixelFormat format, int width, int height, int slices, const Settings &settings);

	void uploadImageData(love::image::ImageDataBase *d, int level, int slice, int x, int y);
	virtual void uploadByteData(PixelFormat pixelformat, const void *data, size_t size, int level, int slice, const Rect &r) = 0;

	virtual void generateMipmaps() = 0;

	// The settings used to initialize this Image.
	Settings settings;

	Slices data;

	MipmapsType mipmapsType;
	bool sRGB;

	// True if the image wasn't able to be properly created and it had to fall
	// back to a default texture.
	bool usingDefaultTexture;

private:

	Image(const Slices &data, const Settings &settings, bool validatedata);

	void init(PixelFormat fmt, int w, int h, const Settings &settings);

	static StringMap<SettingType, SETTING_MAX_ENUM>::Entry settingTypeEntries[];
	static StringMap<SettingType, SETTING_MAX_ENUM> settingTypes;

}; // Image

} // graphics
} // love
