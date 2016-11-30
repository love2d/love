/**
 * Copyright (c) 2006-2016 LOVE Development Team
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
#include "graphics/Color.h"
#include "image/Image.h"
#include "image/ImageData.h"
#include "common/Matrix.h"
#include "common/StringMap.h"
#include "common/int.h"
#include "graphics/Texture.h"
#include "graphics/Volatile.h"
#include "OpenGL.h"

namespace love
{
namespace graphics
{
namespace opengl
{

class Canvas : public Texture, public Volatile
{
public:

	static love::Type type;

	Canvas(int width, int height, PixelFormat format = PIXELFORMAT_NORMAL, int msaa = 0);
	virtual ~Canvas();

	// Implements Volatile.
	bool loadVolatile() override;
	void unloadVolatile() override;

	// Implements Drawable.
	void draw(const Matrix4 &m) override;

	// Implements Texture.
	void drawq(Quad *quad, const Matrix4 &m) override;
	void setFilter(const Texture::Filter &f) override;
	bool setWrap(const Texture::Wrap &w) override;
	const void *getHandle() const override;

	inline GLenum getStatus() const
	{
		return status;
	}

	inline int getMSAA() const
	{
		return actual_samples;
	}

	inline int getRequestedMSAA() const
	{
		return requested_samples;
	}

	inline ptrdiff_t getMSAAHandle() const
	{
		return msaa_buffer;
	}

	inline GLuint getFBO() const
	{
		return fbo;
	}

	static PixelFormat getSizedFormat(PixelFormat format);
	static bool isSupported();
	static bool isMultiFormatMultiCanvasSupported();
	static bool isFormatSupported(PixelFormat format);

	static int canvasCount;

private:

	void drawv(const Matrix4 &t, const Vertex *v);

	GLuint fbo;

	GLuint texture;
	GLuint msaa_buffer;

	PixelFormat requested_format;

	GLenum status;

	int requested_samples;
	int actual_samples;

	size_t texture_memory;

	static bool supportedFormats[PIXELFORMAT_MAX_ENUM];
	static bool checkedFormats[PIXELFORMAT_MAX_ENUM];

}; // Canvas

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_CANVAS_H
