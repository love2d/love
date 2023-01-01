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

#ifndef LOVE_GRAPHICS_TEXTURE_H
#define LOVE_GRAPHICS_TEXTURE_H

// LOVE
#include "common/StringMap.h"
#include "common/math.h"
#include "common/pixelformat.h"
#include "common/Exception.h"
#include "common/Optional.h"
#include "common/int.h"
#include "Drawable.h"
#include "Quad.h"
#include "vertex.h"
#include "depthstencil.h"
#include "Resource.h"

// C
#include <stddef.h>

namespace love
{
namespace graphics
{

class Graphics;

enum TextureType
{
	TEXTURE_2D,
	TEXTURE_VOLUME,
	TEXTURE_2D_ARRAY,
	TEXTURE_CUBE,
	TEXTURE_MAX_ENUM
};

/**
 * Base class for 2D textures. All textures can be drawn with Quads, have a
 * width and height, and have filter and wrap modes.
 **/
class Texture : public Drawable, public Resource
{
public:

	static love::Type type;

	enum WrapMode
	{
		WRAP_CLAMP,
		WRAP_CLAMP_ZERO,
		WRAP_REPEAT,
		WRAP_MIRRORED_REPEAT,
		WRAP_MAX_ENUM
	};

	enum FilterMode
	{
		FILTER_NONE,
		FILTER_LINEAR,
		FILTER_NEAREST,
		FILTER_MAX_ENUM
	};

	struct Filter
	{
		FilterMode min    = FILTER_LINEAR;
		FilterMode mag    = FILTER_LINEAR;
		FilterMode mipmap = FILTER_NONE;
		float anisotropy  = 1.0f;
	};

	struct Wrap
	{
		WrapMode s = WRAP_CLAMP;
		WrapMode t = WRAP_CLAMP;
		WrapMode r = WRAP_CLAMP;
	};

	static Filter defaultFilter;
	static FilterMode defaultMipmapFilter;
	static float defaultMipmapSharpness;

	static int64 totalGraphicsMemory;

	Texture(TextureType texType);
	virtual ~Texture();

	// Drawable.
	void draw(Graphics *gfx, const Matrix4 &m) override;

	/**
	 * Draws the texture using the specified transformation with a Quad applied.
	 **/
	virtual void draw(Graphics *gfx, Quad *quad, const Matrix4 &m);

	void drawLayer(Graphics *gfx, int layer, const Matrix4 &m);
	virtual void drawLayer(Graphics *gfx, int layer, Quad *quad, const Matrix4 &m);

	TextureType getTextureType() const;
	PixelFormat getPixelFormat() const;

	bool isReadable() const;

	bool isValidSlice(int slice) const;

	int getWidth(int mip = 0) const;
	int getHeight(int mip = 0) const;
	int getDepth(int mip = 0) const;
	int getLayerCount() const;
	int getMipmapCount() const;

	int getPixelWidth(int mip = 0) const;
	int getPixelHeight(int mip = 0) const;

	float getDPIScale() const;

	virtual void setFilter(const Filter &f);
	virtual const Filter &getFilter() const;

	virtual bool setWrap(const Wrap &w) = 0;
	virtual const Wrap &getWrap() const;

	// Sets the mipmap texture LOD bias (sharpness) value.
	virtual bool setMipmapSharpness(float sharpness) = 0;
	float getMipmapSharpness() const;

	virtual void setDepthSampleMode(Optional<CompareMode> mode = Optional<CompareMode>());
	Optional<CompareMode> getDepthSampleMode() const;

	Quad *getQuad() const;

	static bool validateFilter(const Filter &f, bool mipmapsAllowed);

	static int getTotalMipmapCount(int w, int h);
	static int getTotalMipmapCount(int w, int h, int d);

	static bool getConstant(const char *in, TextureType &out);
	static bool getConstant(TextureType in, const char *&out);
	static std::vector<std::string> getConstants(TextureType);

	static bool getConstant(const char *in, FilterMode &out);
	static bool getConstant(FilterMode in, const char *&out);
	static std::vector<std::string> getConstants(FilterMode);

	static bool getConstant(const char *in, WrapMode &out);
	static bool getConstant(WrapMode in, const char *&out);
	static std::vector<std::string> getConstants(WrapMode);

protected:

	void initQuad();
	void setGraphicsMemorySize(int64 size);

	bool validateDimensions(bool throwException) const;

	TextureType texType;

	PixelFormat format;
	bool readable;

	int width;
	int height;

	int depth;
	int layers;
	int mipmapCount;

	int pixelWidth;
	int pixelHeight;

	Filter filter;
	Wrap wrap;

	float mipmapSharpness;

	Optional<CompareMode> depthCompareMode;

	StrongRef<Quad> quad;

	int64 graphicsMemorySize;

private:

	static StringMap<TextureType, TEXTURE_MAX_ENUM>::Entry texTypeEntries[];
	static StringMap<TextureType, TEXTURE_MAX_ENUM> texTypes;

	static StringMap<FilterMode, FILTER_MAX_ENUM>::Entry filterModeEntries[];
	static StringMap<FilterMode, FILTER_MAX_ENUM> filterModes;

	static StringMap<WrapMode, WRAP_MAX_ENUM>::Entry wrapModeEntries[];
	static StringMap<WrapMode, WRAP_MAX_ENUM> wrapModes;

}; // Texture

} // graphics
} // love

#endif // LOVE_GRAPHICS_TEXTURE_H
