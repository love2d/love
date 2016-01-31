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

#ifndef LOVE_FONT_TRUE_TYPE_RASTERIZER_H
#define LOVE_FONT_TRUE_TYPE_RASTERIZER_H

// LOVE
#include "Rasterizer.h"
#include "common/StringMap.h"

namespace love
{
namespace font
{

class TrueTypeRasterizer : public Rasterizer
{
public:

	// Types of hinting for TrueType font glyphs.
	enum Hinting
	{
		HINTING_NORMAL,
		HINTING_LIGHT,
		HINTING_MONO,
		HINTING_NONE,
		HINTING_MAX_ENUM
	};

	virtual ~TrueTypeRasterizer() {}

	static bool getConstant(const char *in, Hinting &out);
	static bool getConstant(Hinting in, const char *&out);

private:

	static StringMap<Hinting, HINTING_MAX_ENUM>::Entry hintingEntries[];
	static StringMap<Hinting, HINTING_MAX_ENUM> hintings;

}; // TrueTypeRasterizer

} // font
} // love

#endif // LOVE_FONT_TRUE_TYPE_RASTERIZER_H
