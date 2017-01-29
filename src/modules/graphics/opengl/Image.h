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
#include "graphics/Image.h"
#include "graphics/Volatile.h"

// OpenGL
#include "OpenGL.h"

namespace love
{
namespace graphics
{
namespace opengl
{

class Image : public love::graphics::Image, public Volatile
{
public:

	Image(const std::vector<love::image::ImageData *> &data, const Settings &settings);
	Image(const std::vector<love::image::CompressedImageData *> &cdata, const Settings &settings);

	virtual ~Image();

	// Implements Volatile.
	bool loadVolatile() override;
	void unloadVolatile() override;

	ptrdiff_t getHandle() const override;

	const std::vector<StrongRef<love::image::ImageData>> &getImageData() const override;
	const std::vector<StrongRef<love::image::CompressedImageData>> &getCompressedData() const override;

	void setFilter(const Texture::Filter &f) override;
	bool setWrap(const Texture::Wrap &w) override;

	void setMipmapSharpness(float sharpness) override;
	float getMipmapSharpness() const override;
	bool isCompressed() const override;
	bool refresh(int xoffset, int yoffset, int w, int h) override;

	static bool isFormatSupported(PixelFormat pixelformat);
	static bool hasSRGBSupport();

	static bool getConstant(const char *in, SettingType &out);
	static bool getConstant(SettingType in, const char *&out);

private:

	void preload();

	void generateMipmaps();
	void loadDefaultTexture();
	void loadFromCompressedData();
	void loadFromImageData();

	// The ImageData from which the texture is created. May be empty if
	// Compressed image data was used to create the texture.
	// Each element in the array is a mipmap level.
	std::vector<StrongRef<love::image::ImageData>> data;

	// Or the Compressed Image Data from which the texture is created. May be
	// empty if raw ImageData was used to create the texture.
	std::vector<StrongRef<love::image::CompressedImageData>> cdata;

	// OpenGL texture identifier.
	GLuint texture;

	// Mipmap texture LOD bias (sharpness) value.
	float mipmapSharpness;

	// Whether this Image is using a compressed texture.
	bool compressed;

	bool sRGB;

	// True if the image wasn't able to be properly created and it had to fall
	// back to a default texture.
	bool usingDefaultTexture;

	size_t textureMemorySize;

	static float maxMipmapSharpness;

}; // Image

} // opengl
} // graphics
} // love
