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

#ifndef LOVE_FONT_FREETYPE_FONT_H
#define LOVE_FONT_FREETYPE_FONT_H

// LOVE
#include "font/Font.h"

// FreeType2
#ifdef LOVE_MACOSX
#include <freetype/ft2build.h>
#else
#include <ft2build.h>
#endif
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

namespace love
{
namespace font
{
namespace freetype
{

class Font : public love::font::Font
{
public:

	Font();

	/**
	 * Destructor.
	 **/
	virtual ~Font();

	// Implements Font
	Rasterizer *newRasterizer(Data *data, int size);
	Rasterizer *newRasterizer(love::image::ImageData *data, std::string glyphs);
	Rasterizer *newRasterizer(love::image::ImageData *data, unsigned short *glyphs, int length);
	GlyphData *newGlyphData(Rasterizer *r, unsigned short glyph);

	// Implement Module
	const char *getName() const;

private:

	// FreeType library
	FT_Library library;
}; // Font

} // freetype
} // font
} // love

#endif // LOVE_FONT_FREETYPE_FONT_H