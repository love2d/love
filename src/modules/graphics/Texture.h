/**
 * Copyright (c) 2006-2017 LOVE Development Team
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
#include "Drawable.h"
#include "Quad.h"
#include "vertex.h"

// C
#include <stddef.h>

namespace love
{
namespace graphics
{

/**
 * Base class for 2D textures. All textures can be drawn with Quads, have a
 * width and height, and have filter and wrap modes.
 **/
class Texture : public Drawable
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
	};

	Texture();
	virtual ~Texture();

	static Filter defaultFilter;
	static FilterMode defaultMipmapFilter;
	static float defaultMipmapSharpness;

	// Drawable.
	void draw(Graphics *gfx, const Matrix4 &m) override;

	/**
	 * Draws the texture using the specified transformation with a Quad applied.
	 **/
	void drawq(Graphics *gfx, Quad *quad, const Matrix4 &m);

	PixelFormat getPixelFormat() const;

	virtual int getWidth() const;
	virtual int getHeight() const;

	virtual int getPixelWidth() const;
	virtual int getPixelHeight() const;

	float getPixelDensity() const;

	virtual void setFilter(const Filter &f) = 0;
	virtual const Filter &getFilter() const;

	virtual bool setWrap(const Wrap &w) = 0;
	virtual const Wrap &getWrap() const;

	virtual const Vertex *getVertices() const;

	virtual ptrdiff_t getHandle() const = 0;

	static bool validateFilter(const Filter &f, bool mipmapsAllowed);

	static bool getConstant(const char *in, FilterMode &out);
	static bool getConstant(FilterMode in, const char  *&out);

	static bool getConstant(const char *in, WrapMode &out);
	static bool getConstant(WrapMode in, const char  *&out);

protected:

	virtual void drawv(Graphics *gfx, const Matrix4 &localTransform, const Vertex *v);

	PixelFormat format;

	int width;
	int height;

	int pixelWidth;
	int pixelHeight;

	Filter filter;
	Wrap wrap;

	Vertex vertices[4];

private:

	static StringMap<FilterMode, FILTER_MAX_ENUM>::Entry filterModeEntries[];
	static StringMap<FilterMode, FILTER_MAX_ENUM> filterModes;

	static StringMap<WrapMode, WRAP_MAX_ENUM>::Entry wrapModeEntries[];
	static StringMap<WrapMode, WRAP_MAX_ENUM> wrapModes;

}; // Texture

} // graphics
} // love

#endif // LOVE_GRAPHICS_TEXTURE_H
