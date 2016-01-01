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

#ifndef LOVE_GRAPHICS_OPENGL_IMAGE_H
#define LOVE_GRAPHICS_OPENGL_IMAGE_H

// LOVE
#include "common/config.h"
#include "common/Matrix.h"
#include "common/Vector.h"
#include "common/StringMap.h"
#include "common/math.h"
#include "image/ImageData.h"
#include "image/CompressedImageData.h"
#include "graphics/Texture.h"
#include "graphics/Volatile.h"

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
class Image : public Texture, public Volatile
{
public:

	enum FlagType
	{
		FLAG_TYPE_MIPMAPS,
		FLAG_TYPE_LINEAR,
		FLAG_TYPE_MAX_ENUM
	};

	struct Flags
	{
		bool mipmaps = false;
		bool linear = false;
	};

	/**
	 * Creates a new Image. Not that anything is ready to use
	 * before load is called.
	 *
	 * @param data The data from which to load the image. Each element in the
	 * array is a mipmap level. If more than the base level is present, all
	 * mip levels must be present.
	 **/
	Image(const std::vector<love::image::ImageData *> &data, const Flags &flags);

	/**
	 * Creates a new Image with compressed image data.
	 *
	 * @param cdata The compressed data from which to load the image.
	 **/
	Image(const std::vector<love::image::CompressedImageData *> &cdata, const Flags &flags);

	virtual ~Image();

	// Implements Volatile.
	bool loadVolatile();
	void unloadVolatile();

	/**
	 * @copydoc Drawable::draw()
	 **/
	void draw(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky);

	/**
	 * @copydoc Texture::drawq()
	 **/
	void drawq(Quad *quad, float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky);

	virtual const void *getHandle() const;

	const std::vector<StrongRef<love::image::ImageData>> &getImageData() const;
	const std::vector<StrongRef<love::image::CompressedImageData>> &getCompressedData() const;

	virtual void setFilter(const Texture::Filter &f);
	virtual bool setWrap(const Texture::Wrap &w);

	void setMipmapSharpness(float sharpness);
	float getMipmapSharpness() const;

	/**
	 * Whether this Image is using a compressed texture (via CompressedImageData).
	 **/
	bool isCompressed() const;

	/**
	 * Re-uploads the ImageData or CompressedImageData associated with this Image to
	 * the GPU.
	 **/
	bool refresh(int xoffset, int yoffset, int w, int h);

	const Flags &getFlags() const;

	static void setDefaultMipmapSharpness(float sharpness);
	static float getDefaultMipmapSharpness();
	static void setDefaultMipmapFilter(FilterMode f);
	static FilterMode getDefaultMipmapFilter();

	static bool hasAnisotropicFilteringSupport();
	static bool hasCompressedTextureSupport(image::CompressedImageData::Format format, bool sRGB);
	static bool hasSRGBSupport();

	static bool getConstant(const char *in, FlagType &out);
	static bool getConstant(FlagType in, const char *&out);

	static int imageCount;

private:

	void drawv(const Matrix4 &t, const Vertex *v);

	void preload();

	void generateMipmaps();
	void loadDefaultTexture();
	void loadFromCompressedData();
	void loadFromImageData();

	GLenum getCompressedFormat(image::CompressedImageData::Format cformat, bool &isSRGB) const;

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

	// The flags used to initialize this Image.
	Flags flags;

	bool sRGB;

	// True if the image wasn't able to be properly created and it had to fall
	// back to a default texture.
	bool usingDefaultTexture;

	size_t textureMemorySize;

	static float maxMipmapSharpness;

	static FilterMode defaultMipmapFilter;
	static float defaultMipmapSharpness;

	static StringMap<FlagType, FLAG_TYPE_MAX_ENUM>::Entry flagNameEntries[];
	static StringMap<FlagType, FLAG_TYPE_MAX_ENUM> flagNames;

}; // Image

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_IMAGE_H
