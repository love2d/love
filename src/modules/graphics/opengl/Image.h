/**
 * Copyright (c) 2006-2012 LOVE Development Team
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

#ifndef LOVE_GRAPHICS_OPENGL_IMAGE_H
#define LOVE_GRAPHICS_OPENGL_IMAGE_H

// LOVE
#include "common/Matrix.h"
#include "common/math.h"
#include "common/config.h"
#include "image/ImageData.h"
#include "graphics/Image.h"

#include "OpenGL.h"

// OpenGL
#include "GLee.h"

namespace love
{
namespace graphics
{
namespace opengl
{

/**
 * A drawable image based on OpenGL-textures. This class takes ImageData
 * objects and create textures on the GPU for fast drawing.
 *
 * @author Anders Ruud
 **/
class Image : public love::graphics::Image
{
public:

	/**
	 * Creates a new Image. Not that anything is ready to use
	 * before load is called.
	 *
	 * @param file The file from which to load the image.
	 **/
	Image(love::image::ImageData *data);

	/**
	 * Destructor. Deletes the hardware texture and other resources.
	 **/
	virtual ~Image();

	float getWidth() const;
	float getHeight() const;

	const vertex *getVertices() const;

	love::image::ImageData *getData() const;

	/**
	 * Generate vertices according to a subimage.
	 *
	 * Note: out-of-range values will be clamped.
	 * Note: the vertex colors will not be changed.
	 *
	 * @param x The top-left corner of the subimage along the x-axis.
	 * @param y The top-left corner of the subimage along the y-axis.
	 * @param w The width of the subimage.
	 * @param h The height of the subimage.
	 * @param vertices A vertex array of size four.
	 **/
	void getRectangleVertices(int x, int y, int w, int h, vertex *vertices) const;

	/**
	 * @copydoc Drawable::draw()
	 **/
	void draw(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky) const;

	/**
	 * @copydoc DrawQable::drawq()
	 **/
	void drawq(love::graphics::Quad *quad, float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky) const;

	/**
	 * Sets the filter mode.
	 *
	 * @param mode The filter mode.
	 **/
	void setFilter(const Image::Filter &f);

	const Image::Filter &getFilter() const;

	void setWrap(Image::Wrap &w);

	const Image::Wrap &getWrap() const;
	
	void setMipmapSharpness(float sharpness);
	
	float getMipmapSharpness() const;

	void bind() const;

	bool load();
	void unload();

	// Implements Volatile.
	bool loadVolatile();
	void unloadVolatile();

	static bool hasNpot();
	static bool hasMipmapSupport();

private:

	void drawv(const Matrix &t, const vertex *v) const;

	friend class PixelEffect;
	GLuint getTextureName() const
	{
		return texture;
	}
	// The ImageData from which the texture is created.
	love::image::ImageData *data;

	// Width and height of the hardware texture.
	float width, height;

	// OpenGL texture identifier.
	GLuint texture;

	// The source vertices of the image.
	vertex vertices[4];
	
	// Mipmap texture LOD bias value
	float mipmapsharpness;
	
	float maxmipmapsharpness;
	
	// The image's filter mode
	Image::Filter filter;
	
	// The image's wrap mode
	Image::Wrap wrap;

	bool loadVolatilePOT();
	bool loadVolatileNPOT();

}; // Image

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_IMAGE_H
