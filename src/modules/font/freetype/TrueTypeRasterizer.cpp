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

// LOVE
#include "TrueTypeRasterizer.h"

#include "common/Exception.h"

namespace love
{
namespace font
{
namespace freetype
{

TrueTypeRasterizer::TrueTypeRasterizer(FT_Library library, Data *data, int size)
	: data(data)
{
	if (FT_New_Memory_Face(library,
	                      (const FT_Byte *)data->getData(), /* first byte in memory */
	                      data->getSize(),                  /* size in bytes        */
	                      0,                                /* face_index           */
	                      &face))
		throw love::Exception("TrueTypeFont Loading error: FT_New_Face failed (there is probably a problem with your font file)");

	FT_Set_Pixel_Sizes(face, size, size);

	// Set global metrics
	FT_Size_Metrics s = face->size->metrics;
	metrics.advance = s.max_advance >> 6;
	metrics.ascent = s.ascender >> 6;
	metrics.descent = s.descender >> 6;
	metrics.height = s.height >> 6;

	data->retain();
}

TrueTypeRasterizer::~TrueTypeRasterizer()
{
	FT_Done_Face(face);
	data->release();
}

int TrueTypeRasterizer::getLineHeight() const
{
	return (int)(getHeight() * 1.25);
}

GlyphData *TrueTypeRasterizer::getGlyphData(uint32 glyph) const
{
	love::font::GlyphMetrics glyphMetrics = {};
	FT_Glyph ftglyph;

	// Initialize
	if (FT_Load_Glyph(face, FT_Get_Char_Index(face, glyph), FT_LOAD_DEFAULT))
		throw love::Exception("TrueTypeFont Loading error: FT_Load_Glyph failed");

	if (FT_Get_Glyph(face->glyph, &ftglyph))
		throw love::Exception("TrueTypeFont Loading error: FT_Get_Glyph failed");

	FT_Glyph_To_Bitmap(&ftglyph, FT_RENDER_MODE_NORMAL, 0, 1);

	FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph) ftglyph;
	FT_Bitmap &bitmap = bitmap_glyph->bitmap; //just to make things easier

	// Get metrics
	glyphMetrics.bearingX = bitmap_glyph->left;
	glyphMetrics.bearingY = bitmap_glyph->top;
	glyphMetrics.height = bitmap.rows;
	glyphMetrics.width = bitmap.width;
	glyphMetrics.advance = ftglyph->advance.x >> 16;

	GlyphData *glyphData = new GlyphData(glyph, glyphMetrics, GlyphData::FORMAT_LUMINANCE_ALPHA);

	const uint8 *pixels = bitmap.buffer;
	uint8 *dest = (uint8 *) glyphData->getData();

	// We treat the luminance of the FreeType bitmap as alpha in the GlyphData.
	if (bitmap.pixel_mode == FT_PIXEL_MODE_MONO)
	{
		for (int y = 0; y < bitmap.rows; y++)
		{
			for (int x = 0; x < bitmap.width; x++)
			{
				// Extract the 1-bit value and convert it to uint8.
				uint8 v = ((pixels[x / 8]) & (1 << (7 - (x % 8)))) ? 255 : 0;
				dest[2 * (y * bitmap.width + x) + 0] = 255;
				dest[2 * (y * bitmap.width + x) + 1] = v;
			}

			pixels += bitmap.pitch;
		}
	}
	else if (bitmap.pixel_mode == FT_PIXEL_MODE_GRAY)
	{
		for (int y = 0; y < bitmap.rows; y++)
		{
			for (int x = 0; x < bitmap.width; x++)
			{
				dest[2 * (y * bitmap.width + x) + 0] = 255;
				dest[2 * (y * bitmap.width + x) + 1] = pixels[x];
			}

			pixels += bitmap.pitch;
		}
	}
	else
	{
		delete glyphData;
		FT_Done_Glyph(ftglyph);
		throw love::Exception("Unknown TrueType glyph pixel mode.");
	}

	// Having copied the data over, we can destroy the glyph.
	FT_Done_Glyph(ftglyph);

	return glyphData;
}

int TrueTypeRasterizer::getGlyphCount() const
{
	return face->num_glyphs;
}

bool TrueTypeRasterizer::hasGlyph(uint32 glyph) const
{
	return FT_Get_Char_Index(face, glyph) != 0;
}

} // freetype
} // font
} // love