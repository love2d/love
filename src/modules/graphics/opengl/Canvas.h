/**
 * Copyright (c) 2006-2015 LOVE Development Team
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
#include "Texture.h"
#include "OpenGL.h"

namespace love
{
namespace graphics
{
namespace opengl
{

class Canvas : public Texture
{
public:

	// Different Canvas render target formats.
	enum Format
	{
		FORMAT_NORMAL,   // Usually RGBA8 or a similar fallback. Always supported.
		FORMAT_HDR,      // Usually RGBA16F. Not always supported.
		FORMAT_RGBA8,    // RGBA with 8 bits per component.
		FORMAT_RGBA4,    // RGBA with 4 bits per component.
		FORMAT_RGB5A1,   // RGB with 5 bits per component, and A with 1 bit.
		FORMAT_RGB565,   // RGB with 5, 6, and 5 bits each, respectively.
		FORMAT_RGB10A2,  // RGB with 10 bits each, and A with 2 bits.
		FORMAT_RG11B10F, // Floating point [0, +inf]. RG with 11 FP bits each, and B with 10 FP bits.
		FORMAT_RGBA16F,  // Floating point [-inf, +inf]. RGBA with 16 FP bits per component.
		FORMAT_RGBA32F,  // Floating point [-inf, +inf]. RGBA with 32 FP bits per component.
		FORMAT_SRGB,     // sRGB with 8 bits per component, plus 8 bit linear A.
		FORMAT_MAX_ENUM
	};

	Canvas(int width, int height, Format format = FORMAT_NORMAL, int msaa = 0);
	virtual ~Canvas();

	// Implements Volatile.
	virtual bool loadVolatile();
	virtual void unloadVolatile();

	// Implements Drawable.
	virtual void draw(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky);

	// Implements Texture.
	virtual void drawq(Quad *quad, float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky);
	virtual void setFilter(const Texture::Filter &f);
	virtual void setWrap(const Texture::Wrap &w);
	virtual GLuint getGLTexture() const;
	virtual void predraw();

	/**
	 * @param canvases A list of other canvases to temporarily attach to this one,
	 * to allow drawing to multiple canvases at once.
	 **/
	void startGrab(const std::vector<Canvas *> &canvases);
	void startGrab();
	void stopGrab(bool switchingToOtherCanvas = false);

	void clear(Color c);

	/**
	 * Create and attach a stencil buffer to this Canvas' framebuffer, if necessary.
	 **/
	bool checkCreateStencil();

	love::image::ImageData *getImageData(love::image::Image *image);

	void getPixel(unsigned char* pixel_rgba, int x, int y);

	inline const std::vector<Canvas *> &getAttachedCanvases() const
	{
		return attachedCanvases;
	}

	inline GLenum getStatus() const
	{
		return status;
	}

	inline Format getTextureFormat() const
	{
		return format;
	}

	inline int getMSAA() const
	{
		return actual_samples;
	}

	bool resolveMSAA();

	static bool isSupported();
	static bool isMultiCanvasSupported();
	static bool isFormatSupported(Format format);

	static Canvas *current;

	// The viewport dimensions of the system (default) framebuffer.
	static OpenGL::Viewport systemViewport;

	// Whether the main screen should have linear -> sRGB conversions enabled.
	static bool screenHasSRGB;

	static int switchCount;
	static int canvasCount;

	static bool getConstant(const char *in, Format &out);
	static bool getConstant(Format in, const char *&out);

private:

	bool createMSAAFBO(GLenum internalformat);

	static Format getSizedFormat(Format format);
	static void convertFormat(Format format, GLenum &internalformat, GLenum &externalformat, GLenum &type);
	static size_t getFormatBitsPerPixel(Format format);

	GLuint fbo;
	GLuint resolve_fbo;

	GLuint texture;
	GLuint msaa_buffer;
	GLuint depth_stencil;

	Format format;

	GLenum status;

	std::vector<Canvas *> attachedCanvases;

	int requested_samples;
	int actual_samples;
	bool msaa_dirty;

	size_t texture_memory;

	void setupGrab();
	void drawv(const Matrix &t, const Vertex *v);

	static bool supportedFormats[FORMAT_MAX_ENUM];
	static bool checkedFormats[FORMAT_MAX_ENUM];

	static StringMap<Format, FORMAT_MAX_ENUM>::Entry formatEntries[];
	static StringMap<Format, FORMAT_MAX_ENUM> formats;

}; // Canvas

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_CANVAS_H
