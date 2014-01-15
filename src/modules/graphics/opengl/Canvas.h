/**
 * Copyright (c) 2006-2014 LOVE Development Team
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

#include "graphics/Color.h"
#include "image/Image.h"
#include "image/ImageData.h"
#include "common/Matrix.h"
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

	enum TextureType
	{
		TYPE_NORMAL,
		TYPE_HDR,
		TYPE_MAX_ENUM
	};

	Canvas(int width, int height, TextureType texture_type = TYPE_NORMAL);
	virtual ~Canvas();

	// Implements Volatile.
	virtual bool loadVolatile();
	virtual void unloadVolatile();

	// Implements Drawable.
	virtual void draw(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky) const;

	// Implements Texture.
	virtual void drawq(Quad *quad, float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky) const;
	virtual void setFilter(const Texture::Filter &f);
	virtual void setWrap(const Texture::Wrap &w);
	virtual GLuint getGLTexture() const;
	virtual void predraw() const;

	/**
	 * @param canvases A list of other canvases to temporarily attach to this one,
	 * to allow drawing to multiple canvases at once.
	 **/
	void startGrab(const std::vector<Canvas *> &canvases);
	void startGrab();
	void stopGrab();

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

	inline TextureType getTextureType() const
	{
		return texture_type;
	}

	static bool isSupported();
	static bool isHDRSupported();
	static bool isMultiCanvasSupported();

	static bool getConstant(const char *in, TextureType &out);
	static bool getConstant(TextureType in, const char *&out);

	static Canvas *current;
	static void bindDefaultCanvas();

	// The viewport dimensions of the system (default) framebuffer.
	static OpenGL::Viewport systemViewport;

private:

	GLuint fbo;
	GLuint texture;
	GLuint depth_stencil;

	TextureType texture_type;

	GLenum status;

	std::vector<Canvas *> attachedCanvases;

	void setupGrab();
	void drawv(const Matrix &t, const Vertex *v) const;

	static StringMap<TextureType, TYPE_MAX_ENUM>::Entry textureTypeEntries[];
	static StringMap<TextureType, TYPE_MAX_ENUM> textureTypes;
};

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_CANVAS_H
