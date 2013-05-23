/**
 * Copyright (c) 2006-2013 LOVE Development Team
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

#include "graphics/DrawGable.h"
#include "graphics/Volatile.h"
#include "graphics/Image.h"
#include "graphics/Color.h"
#include "image/Image.h"
#include "image/ImageData.h"
#include "common/math.h"
#include "common/Matrix.h"
#include "OpenGL.h"

namespace love
{
namespace graphics
{
namespace opengl
{

class Canvas : public DrawGable, public Volatile
{
public:
	enum TextureType {
		TYPE_NORMAL,
		TYPE_HDR,
		TYPE_MAX_ENUM
	};

	Canvas(int width, int height, TextureType texture_type = TYPE_NORMAL);
	virtual ~Canvas();

	/**
	 * @param canvases A list of other canvases to temporarily attach to this one,
	 * to allow drawing to multiple canvases at once.
	 **/
	void startGrab(const std::vector<Canvas *> &canvases);
	void startGrab();
	void stopGrab();

	void clear(const Color &c);

	virtual void draw(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky) const;

	/**
	 * @copydoc DrawGable::drawg()
	 **/
	void drawg(love::graphics::Geometry *geom, float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky) const;

	/**
	 * Create and attach a stencil buffer to this Canvas' framebuffer, if necessary.
	 **/
	bool checkCreateStencil();

	love::image::ImageData *getImageData(love::image::Image *image);

	void getPixel(unsigned char* pixel_rgba, int x, int y);

	const std::vector<Canvas *> &getAttachedCanvases() const;

void setFilter(const Image::Filter &f);
	Image::Filter getFilter() const;

	void setWrap(const Image::Wrap &w);
	Image::Wrap getWrap() const;

	int getWidth();
	int getHeight();

	unsigned int getStatus() const
	{
		return status;
	}

	TextureType getTextureType() const
	{
		return texture_type;
	}

	bool loadVolatile();
	void unloadVolatile();

	static bool isSupported();
	static bool isHDRSupported();
	static bool isMultiCanvasSupported();
	static bool getConstant(const char *in, TextureType &out);
	static bool getConstant(TextureType in, const char *&out);

	static Canvas *current;
	static void bindDefaultCanvas();

	GLuint getTextureName() const
	{
		return img;
	}

private:
	friend class Shader;

	GLsizei width;
	GLsizei height;
	GLuint fbo;
	GLuint depth_stencil;
	GLuint img;

	TextureType texture_type;

	vertex vertices[4];

	GLenum status;

	struct
	{
		Image::Filter filter;
		Image::Wrap   wrap;
	} settings;

	std::vector<Canvas *> attachedCanvases;

	void setupGrab();
	void drawv(const Matrix &t, const vertex *v, GLsizei count = 4, GLenum mode = GL_QUADS) const;

	static StringMap<TextureType, TYPE_MAX_ENUM>::Entry textureTypeEntries[];
	static StringMap<TextureType, TYPE_MAX_ENUM> textureTypes;
};

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_CANVAS_H
