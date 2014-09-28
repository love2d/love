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

#ifndef LOVE_FONT_FREETYPE_TRUE_TYPE_RASTERIZER_H
#define LOVE_FONT_FREETYPE_TRUE_TYPE_RASTERIZER_H

// LOVE
#include "filesystem/File.h"
#include "font/Rasterizer.h"

// FreeType2
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

namespace love
{
namespace font
{
namespace freetype
{

/**
 * Holds data for a font object.
 **/
class TrueTypeRasterizer : public Rasterizer
{
public:

	TrueTypeRasterizer(FT_Library library, love::filesystem::FileData *data, int size);
	virtual ~TrueTypeRasterizer();

	// Implement Rasterizer
	virtual int getLineHeight() const;
	virtual GlyphData *getGlyphData(uint32 glyph) const;
	virtual int getGlyphCount() const;
	virtual bool hasGlyph(uint32 glyph) const;

	static bool accepts(FT_Library library, love::filesystem::FileData *data);

private:

	// TrueType face
	FT_Face face;

	// File data
	StrongRef<love::filesystem::FileData> data;

}; // TrueTypeRasterizer

} // freetype
} // font
} // love

#endif // LOVE_FONT_FREETYPE_TRUE_TYPE_RASTERIZER_H
