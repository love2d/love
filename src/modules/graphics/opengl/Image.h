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

#ifndef LOVE_GRAPHICS_OPENGL_IMAGE_H
#define LOVE_GRAPHICS_OPENGL_IMAGE_H

// LOVE
#include "common/config.h"
#include "common/Matrix.h"
#include "common/Vector.h"
#include "common/math.h"
#include "image/ImageData.h"
#include "image/CompressedData.h"
#include "graphics/Image.h"

// OpenGL
#include "OpenGL.h"

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
	 * @param data The data from which to load the image.
	 **/
	Image(love::image::ImageData *data);

	/**
	 * Creates a new Image with compressed image data.
	 *
	 * @param cdata The compressed data from which to load the image.
	 **/
	Image(love::image::CompressedData *cdata);

	/**
	 * Destructor. Deletes the hardware texture and other resources.
	 **/
	virtual ~Image();

	float getWidth() const;
	float getHeight() const;

	const vertex *getVertices() const;

	love::image::ImageData *getImageData() const;
	love::image::CompressedData *getCompressedData() const;

	/**
	 * @copydoc Drawable::draw()
	 **/
	void draw(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky) const;

	/**
	 * @copydoc DrawGable::drawg()
	 **/
	void drawg(love::graphics::Geometry *geom, float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky) const;

	/**
	 * Sets the filter mode.
	 * @param f The filter mode.
	 **/
	void setFilter(const Image::Filter &f);

	const Image::Filter &getFilter() const;

	void setWrap(const Image::Wrap &w);

	const Image::Wrap &getWrap() const;

	void setMipmapSharpness(float sharpness);
	float getMipmapSharpness() const;

	/**
	 * Whether this Image is using a compressed texture (via CompressedData).
	 **/
	bool isCompressed() const;

	void bind() const;

	bool load();
	void unload();

	// Implements Volatile.
	bool loadVolatile();
	void unloadVolatile();

	/**
	 * Re-uploads the ImageData or CompressedData associated with this Image to
	 * the GPU, allowing situations where lovers modify an ImageData after image
	 * creation from the ImageData, and apply the changes with Image:refresh().
	 **/
	bool refresh();

	/**
	 * Gets the texture coordinate scale used for drawing auto-padded NPOT
	 * images correctly.
	 **/
	love::Vector getTexCoordScale() const;

	static void setDefaultMipmapSharpness(float sharpness);
	static float getDefaultMipmapSharpness();
	static void setDefaultMipmapFilter(FilterMode f);
	static FilterMode getDefaultMipmapFilter();

	static bool hasNpot();
	static bool hasAnisotropicFilteringSupport();
	static bool hasMipmapSupport();
	static bool hasMipmapSharpnessSupport();

	static bool hasCompressedTextureSupport();
	static bool hasCompressedTextureSupport(image::CompressedData::TextureType type);

private:

	vertex *scaleNPOT(const vertex *v, size_t count) const;
	void drawv(const Matrix &t, const vertex *v, GLsizei count = 4, GLenum mode = GL_QUADS, const uint16 *e = 0, GLsizei ecount = 0) const;

	friend class Shader;
	GLuint getTextureName() const
	{
		return texture;
	}

	// The ImageData from which the texture is created. May be null if
	// Compressed image data was used to create the texture.
	love::image::ImageData *data;

	// Or the Compressed Image Data from which the texture is created. May be
	// null if raw ImageData was used to create the texture.
	love::image::CompressedData *cdata;

	// Width and height of the hardware texture.
	float width, height;

	// OpenGL texture identifier.
	GLuint texture;

	// The source vertices of the image.
	vertex vertices[4];

	// Mipmap texture LOD bias (sharpness) value.
	float mipmapSharpness;

	// True if mipmaps have been created for this Image.
	bool mipmapsCreated;

	// Whether this Image is using a compressed texture.
	bool compressed;

	// The image's filter mode
	Image::Filter filter;

	// The image's wrap mode
	Image::Wrap wrap;

	void preload();

	bool loadVolatilePOT();
	bool loadVolatileNPOT();

	void uploadCompressedMipmaps();
	void createMipmaps();
	void checkMipmapsCreated();

	static float maxMipmapSharpness;

	static FilterMode defaultMipmapFilter;
	static float defaultMipmapSharpness;

	GLenum getCompressedFormat(image::CompressedData::TextureType type) const;

}; // Image

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_IMAGE_H
