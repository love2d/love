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

#ifndef LOVE_GRAPHICS_OPENGL_CANVAS_H
#define LOVE_GRAPHICS_OPENGL_CANVAS_H

#include "common/config.h"
#include "common/Color.h"
#include "common/int.h"
#include "graphics/Canvas.h"
#include "graphics/Volatile.h"
#include "OpenGL.h"

namespace love
{
namespace graphics
{
namespace opengl
{

class Canvas final : public love::graphics::Canvas, public Volatile
{
public:

	Canvas(const Settings &settings);
	virtual ~Canvas();

	// Implements Volatile.
	bool loadVolatile() override;
	void unloadVolatile() override;

	// Implements Texture.
	void setFilter(const Texture::Filter &f) override;
	bool setWrap(const Texture::Wrap &w) override;
	bool setMipmapSharpness(float sharpness) override;
	void setDepthSampleMode(Optional<CompareMode> mode) override;
	ptrdiff_t getHandle() const override;

	love::image::ImageData *newImageData(love::image::Image *module, int slice, int mipmap, const Rect &rect) override;
	void generateMipmaps() override;

	int getMSAA() const override
	{
		return actualSamples;
	}

	ptrdiff_t getRenderTargetHandle() const override
	{
		return renderbuffer != 0 ? renderbuffer : texture;
	}

	inline GLuint getFBO() const
	{
		return fbo;
	}

	static PixelFormat getSizedFormat(PixelFormat format);
	static bool isSupported();
	static bool isMultiFormatMultiCanvasSupported();
	static bool isFormatSupported(PixelFormat format, bool readable);
	static bool isFormatSupported(PixelFormat format);
	static void resetFormatSupport();

private:

	struct SupportedFormat
	{
		bool readable = false;
		bool nonreadable = false;

		bool get(bool getreadable)
		{
			return getreadable ? readable : nonreadable;
		}

		void set(bool setreadable, bool val)
		{
			if (setreadable)
				readable = val;
			else
				nonreadable = val;
		}
	};

	GLuint fbo;

	GLuint texture;
	GLuint renderbuffer;

	GLenum status;

	int actualSamples;

	static SupportedFormat supportedFormats[PIXELFORMAT_MAX_ENUM];
	static SupportedFormat checkedFormats[PIXELFORMAT_MAX_ENUM];

}; // Canvas

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_CANVAS_H
