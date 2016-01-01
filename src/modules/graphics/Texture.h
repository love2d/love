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

#ifndef LOVE_GRAPHICS_TEXTURE_H
#define LOVE_GRAPHICS_TEXTURE_H

// LOVE
#include "common/StringMap.h"
#include "common/math.h"
#include "Drawable.h"
#include "Quad.h"

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

	/**
	 * Draws the texture using the specified transformation with a Quad applied.
	 *
	 * @param quad The Quad object to use to draw the object.
	 * @param x The position of the object along the x-axis.
	 * @param y The position of the object along the y-axis.
	 * @param angle The angle of the object (in radians).
	 * @param sx The scale factor along the x-axis.
	 * @param sy The scale factor along the y-axis.
	 * @param ox The origin offset along the x-axis.
	 * @param oy The origin offset along the y-axis.
	 * @param kx Shear along the x-axis.
	 * @param ky Shear along the y-axis.
	 **/
	virtual void drawq(Quad *quad, float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky) = 0;

	virtual int getWidth() const;
	virtual int getHeight() const;

	virtual void setFilter(const Filter &f) = 0;
	virtual const Filter &getFilter() const;

	virtual bool setWrap(const Wrap &w) = 0;
	virtual const Wrap &getWrap() const;

	virtual const Vertex *getVertices() const;

	virtual const void *getHandle() const = 0;

	// The default filter.
	static void setDefaultFilter(const Filter &f);
	static const Filter &getDefaultFilter();

	static bool validateFilter(const Filter &f, bool mipmapsAllowed);

	static bool getConstant(const char *in, FilterMode &out);
	static bool getConstant(FilterMode in, const char  *&out);

	static bool getConstant(const char *in, WrapMode &out);
	static bool getConstant(WrapMode in, const char  *&out);

protected:

	int width;
	int height;

	Filter filter;
	Wrap wrap;

	Vertex vertices[4];

private:

	// The default texture filter.
	static Filter defaultFilter;

	static StringMap<FilterMode, FILTER_MAX_ENUM>::Entry filterModeEntries[];
	static StringMap<FilterMode, FILTER_MAX_ENUM> filterModes;

	static StringMap<WrapMode, WRAP_MAX_ENUM>::Entry wrapModeEntries[];
	static StringMap<WrapMode, WRAP_MAX_ENUM> wrapModes;

}; // Texture

} // graphics
} // love

#endif // LOVE_GRAPHICS_TEXTURE_H
