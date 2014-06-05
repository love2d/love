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

#ifndef LOVE_GRAPHICS_OPENGL_IMAGE_H
#define LOVE_GRAPHICS_OPENGL_IMAGE_H

// LOVE
#include "common/config.h"
#include "common/Matrix.h"
#include "common/Vector.h"
#include "common/StringMap.h"
#include "common/math.h"
#include "image/ImageData.h"
#include "image/CompressedData.h"
#include "Texture.h"

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
class Image : public Texture
{
public:

	enum FlagType
	{
		FLAG_TYPE_MIPMAPS,
		FLAG_TYPE_SRGB,
		FLAG_TYPE_MAX_ENUM
	};

	struct Flags
	{
		bool mipmaps;
		bool sRGB;

		Flags() : mipmaps(false), sRGB(false) {}
	};

	/**
	 * Creates a new Image. Not that anything is ready to use
	 * before load is called.
	 *
	 * @param data The data from which to load the image.
	 **/
	Image(love::image::ImageData *data, const Flags &flags);

	/**
	 * Creates a new Image with compressed image data.
	 *
	 * @param cdata The compressed data from which to load the image.
	 **/
	Image(love::image::CompressedData *cdata, const Flags &flags);

	/**
	 * Destructor. Deletes the hardware texture and other resources.
	 **/
	virtual ~Image();

	love::image::ImageData *getImageData() const;
	love::image::CompressedData *getCompressedData() const;

	/**
	 * @copydoc Drawable::draw()
	 **/
	void draw(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky);

	/**
	 * @copydoc Texture::drawq()
	 **/
	void drawq(Quad *quad, float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky);

	/**
	 * Call before using this Image's texture to draw. Binds the texture,
	 * globally scales texture coordinates if the Image has NPOT dimensions and
	 * NPOT isn't supported, etc.
	 **/
	virtual void predraw();
	virtual void postdraw();

	virtual GLuint getGLTexture() const;

	virtual void setFilter(const Texture::Filter &f);
	virtual void setWrap(const Texture::Wrap &w);

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
	 * the GPU.
	 **/
	bool refresh();

	const Flags &getFlags() const;

	static void setDefaultMipmapSharpness(float sharpness);
	static float getDefaultMipmapSharpness();
	static void setDefaultMipmapFilter(FilterMode f);
	static FilterMode getDefaultMipmapFilter();

	static bool hasAnisotropicFilteringSupport();
	static bool hasCompressedTextureSupport(image::CompressedData::Format format);
	static bool hasSRGBSupport();

	static bool getConstant(const char *in, FlagType &out);
	static bool getConstant(FlagType in, const char *&out);

private:

	void uploadDefaultTexture();

	void drawv(const Matrix &t, const Vertex *v);

	// The ImageData from which the texture is created. May be null if
	// Compressed image data was used to create the texture.
	love::image::ImageData *data;

	// Or the Compressed Image Data from which the texture is created. May be
	// null if raw ImageData was used to create the texture.
	love::image::CompressedData *cdata;

	// OpenGL texture identifier.
	GLuint texture;

	// Mipmap texture LOD bias (sharpness) value.
	float mipmapSharpness;

	// Whether this Image is using a compressed texture.
	bool compressed;

	// The flags used to initialize this Image.
	Flags flags;

	// True if the image wasn't able to be properly created and it had to fall
	// back to a default texture.
	bool usingDefaultTexture;

	void preload();

	void uploadCompressedData();
	void uploadImageData();
	void uploadTexture();

	static float maxMipmapSharpness;

	static FilterMode defaultMipmapFilter;
	static float defaultMipmapSharpness;

	GLenum getCompressedFormat(image::CompressedData::Format cformat) const;

	static StringMap<FlagType, FLAG_TYPE_MAX_ENUM>::Entry flagNameEntries[];
	static StringMap<FlagType, FLAG_TYPE_MAX_ENUM> flagNames;

}; // Image

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_IMAGE_H
