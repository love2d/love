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

#ifndef LOVE_GRAPHICS_IMAGE_H
#define LOVE_GRAPHICS_IMAGE_H

// LOVE
#include "graphics/Volatile.h"
#include "graphics/DrawQable.h"
#include "common/StringMap.h"

namespace love
{
namespace graphics
{

class Image : public DrawQable, public Volatile
{
public:

	enum WrapMode
	{
		WRAP_CLAMP = 1,
		WRAP_REPEAT,
		WRAP_MAX_ENUM
	};

	enum FilterMode
	{
		FILTER_LINEAR = 1,
		FILTER_NEAREST,
		FILTER_NONE,
		FILTER_MAX_ENUM
	};

	struct Filter
	{
		Filter();
		FilterMode min;
		FilterMode mag;
		FilterMode mipmap;
	};

	struct Wrap
	{
		Wrap();
		WrapMode s;
		WrapMode t;
	};

	virtual ~Image();
	
	// The default filter.
	static void setDefaultFilter(const Filter &f);
	static const Filter &getDefaultFilter();

	// The default amount of anisotropic filtering.
	static void setDefaultAnisotropy(float anisotropy);
	static float getDefaultAnisotropy();

	static bool getConstant(const char *in, FilterMode &out);
	static bool getConstant(FilterMode in, const char  *&out);
	static bool getConstant(const char *in, WrapMode &out);
	static bool getConstant(WrapMode in, const char  *&out);

private:

	// The default image filter.
	static Filter defaultFilter;

	// The default amount of anisotropic filtering.
	static float defaultAnisotropy;

	static StringMap<FilterMode, FILTER_MAX_ENUM>::Entry filterModeEntries[];
	static StringMap<FilterMode, FILTER_MAX_ENUM> filterModes;
	static StringMap<WrapMode, WRAP_MAX_ENUM>::Entry wrapModeEntries[];
	static StringMap<WrapMode, WRAP_MAX_ENUM> wrapModes;

}; // Image

} // graphics
} // love

#endif // LOVE_GRAPHICS_IMAGE_H
