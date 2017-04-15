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

class Image final : public love::graphics::Image, public Volatile
{
public:

	Image(const Slices &data, const Settings &settings);
	Image(TextureType textype, PixelFormat format, int width, int height, int slices, const Settings &settings);

	virtual ~Image();

	// Implements Volatile.
	bool loadVolatile() override;
	void unloadVolatile() override;

	ptrdiff_t getHandle() const override;

	void setFilter(const Texture::Filter &f) override;
	bool setWrap(const Texture::Wrap &w) override;

	bool setMipmapSharpness(float sharpness) override;

	void replacePixels(love::image::ImageDataBase *d, int slice, int mipmap, bool reloadmipmaps) override;
	void replacePixels(const void *data, size_t size, const Rect &rect, int slice, int mipmap, bool reloadmipmaps) override;

	bool isFormatLinear() const override;
	bool isCompressed() const override;
	MipmapsType getMipmapsType() const override;

	static bool isFormatSupported(PixelFormat pixelformat);
	static bool hasSRGBSupport();

private:

	void init(PixelFormat fmt, int w, int h, const Settings &settings);

	void generateMipmaps();
	void loadDefaultTexture();
	void loadData();
	void uploadByteData(PixelFormat pixelformat, const void *data, size_t size, const Rect &rect, int level, int slice);
	void uploadImageData(love::image::ImageDataBase *d, int level, int slice);

	// OpenGL texture identifier.
	GLuint texture;

	// Whether this Image is using a compressed texture.
	bool compressed;

	// True if the image wasn't able to be properly created and it had to fall
	// back to a default texture.
	bool usingDefaultTexture;

	size_t textureMemorySize;

}; // Image

} // opengl
} // graphics
} // love
