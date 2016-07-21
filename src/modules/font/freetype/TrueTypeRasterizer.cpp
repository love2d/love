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

// LOVE
#include "TrueTypeRasterizer.h"

#include "common/Exception.h"

namespace love
{
namespace font
{
namespace freetype
{

TrueTypeRasterizer::TrueTypeRasterizer(FT_Library library, love::Data *data, int size, Hinting hinting)
	: data(data)
	, hinting(hinting)
{
	if (size <= 0)
		throw love::Exception("Invalid TrueType font size: %d", size);

	FT_Error err = FT_Err_Ok;
	err = FT_New_Memory_Face(library,
	                         (const FT_Byte *)data->getData(), /* first byte in memory */
	                         data->getSize(),                  /* size in bytes        */
	                         0,                                /* face_index           */
	                         &face);

	if (err != FT_Err_Ok)
		throw love::Exception("TrueType Font loading error: FT_New_Face failed: 0x%x (problem with font file?)", err);

	err = FT_Set_Pixel_Sizes(face, size, size);

	if (err != FT_Err_Ok)
		throw love::Exception("TrueType Font loading error: FT_Set_Pixel_Sizes failed: 0x%x (invalid size?)", err);

	// Set global metrics
	FT_Size_Metrics s = face->size->metrics;
	metrics.advance = (int) (s.max_advance >> 6);
	metrics.ascent  = (int) (s.ascender >> 6);
	metrics.descent = (int) (s.descender >> 6);
	metrics.height  = (int) (s.height >> 6);
}

TrueTypeRasterizer::~TrueTypeRasterizer()
{
	FT_Done_Face(face);
}

int TrueTypeRasterizer::getLineHeight() const
{
	return (int)(getHeight() * 1.25);
}

GlyphData *TrueTypeRasterizer::getGlyphData(uint32 glyph) const
{
	love::font::GlyphMetrics glyphMetrics = {};
	FT_Glyph ftglyph;

	FT_Error err = FT_Err_Ok;
	FT_ULong loadoption = hintingToLoadOption(hinting);

	// Initialize
	err = FT_Load_Glyph(face, FT_Get_Char_Index(face, glyph), FT_LOAD_DEFAULT | loadoption);

	if (err != FT_Err_Ok)
		throw love::Exception("TrueType Font glyph error: FT_Load_Glyph failed (0x%x)", err);

	err = FT_Get_Glyph(face->glyph, &ftglyph);

	if (err != FT_Err_Ok)
		throw love::Exception("TrueType Font glyph error: FT_Get_Glyph failed (0x%x)", err);

	FT_Render_Mode rendermode = FT_RENDER_MODE_NORMAL;
	if (hinting == HINTING_MONO)
		rendermode = FT_RENDER_MODE_MONO;

	err = FT_Glyph_To_Bitmap(&ftglyph, rendermode, 0, 1);

	if (err != FT_Err_Ok)
		throw love::Exception("TrueType Font glyph error: FT_Glyph_To_Bitmap failed (0x%x)", err);

	FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph) ftglyph;
	FT_Bitmap &bitmap = bitmap_glyph->bitmap; //just to make things easier

	// Get metrics
	glyphMetrics.bearingX = bitmap_glyph->left;
	glyphMetrics.bearingY = bitmap_glyph->top;
	glyphMetrics.height = bitmap.rows;
	glyphMetrics.width = bitmap.width;
	glyphMetrics.advance = (int) (ftglyph->advance.x >> 16);

	GlyphData *glyphData = new GlyphData(glyph, glyphMetrics, GlyphData::FORMAT_LUMINANCE_ALPHA);

	const uint8 *pixels = bitmap.buffer;
	uint8 *dest = (uint8 *) glyphData->getData();

	// We treat the luminance of the FreeType bitmap as alpha in the GlyphData.
	if (bitmap.pixel_mode == FT_PIXEL_MODE_MONO)
	{
		for (int y = 0; y < (int) bitmap.rows; y++)
		{
			for (int x = 0; x < (int) bitmap.width; x++)
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
		for (int y = 0; y < (int) bitmap.rows; y++)
		{
			for (int x = 0; x < (int) bitmap.width; x++)
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
	return (int) face->num_glyphs;
}

bool TrueTypeRasterizer::hasGlyph(uint32 glyph) const
{
	return FT_Get_Char_Index(face, glyph) != 0;
}

float TrueTypeRasterizer::getKerning(uint32 leftglyph, uint32 rightglyph) const
{
	FT_Vector kerning = {};
	FT_Get_Kerning(face,
	               FT_Get_Char_Index(face, leftglyph),
	               FT_Get_Char_Index(face, rightglyph),
	               FT_KERNING_DEFAULT,
	               &kerning);
	return float(kerning.x >> 6);
}

bool TrueTypeRasterizer::accepts(FT_Library library, love::Data *data)
{
	const FT_Byte *fbase = (const FT_Byte *) data->getData();
	FT_Long fsize = (FT_Long) data->getSize();

	// Pasing in -1 for the face index lets us test if the data is valid.
	return FT_New_Memory_Face(library, fbase, fsize, -1, nullptr) == 0;
}

FT_ULong TrueTypeRasterizer::hintingToLoadOption(Hinting hint)
{
	switch (hint)
	{
	case HINTING_NORMAL:
	default:
		return FT_LOAD_TARGET_NORMAL;
	case HINTING_LIGHT:
		return FT_LOAD_TARGET_LIGHT;
	case HINTING_MONO:
		return FT_LOAD_TARGET_MONO;
	case HINTING_NONE:
		return FT_LOAD_NO_HINTING;
	}
}

} // freetype
} // font
} // love
