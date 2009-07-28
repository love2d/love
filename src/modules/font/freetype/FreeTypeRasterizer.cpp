/**
* Copyright (c) 2006-2009 LOVE Development Team
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
#include "FreeTypeRasterizer.h"

#include <common/Exception.h>

namespace love
{
namespace font
{
namespace freetype
{
	inline int next_p2(int num)
	{
		int powered = 2;
		while(powered < num) powered <<= 1;
		return powered;
	}

	FreeTypeRasterizer::FreeTypeRasterizer(love::filesystem::File * file, int size)
	{
		// Initialize
		data = file->read();

		if(FT_Init_FreeType(&library))
			throw love::Exception("TrueTypeFont Loading error: FT_Init_FreeType failed\n");

		if(FT_New_Memory_Face(	library,
								(const FT_Byte *)data->getData(),	/* first byte in memory */
								data->getSize(),					/* size in bytes        */
								0,									/* face_index           */
								&face))
			throw love::Exception("TrueTypeFont Loading error: FT_New_Face failed (there is probably a problem with your font file)\n");
		
		FT_Set_Pixel_Sizes(face, size, size);

		// Set global metrics
		metrics.advance = face->max_advance_width;
		metrics.ascent = face->ascender;
		metrics.descent = face->descender;
		metrics.height = face->height;
	}

	FreeTypeRasterizer::~FreeTypeRasterizer()
	{
		FT_Done_Face(face);
		FT_Done_FreeType(library);
		data->release();
	}

	int FreeTypeRasterizer::getLineHeight() const
	{
		return (int)(getHeight() * 1.25);
	}

	GlyphData * FreeTypeRasterizer::getGlyphData(const wchar_t glyph) const
	{
		unsigned char * textureData;
		love::font::GlyphMetrics glyphMetrics;
		int bpp = 2;
		FT_Glyph ftglyph;

		// Initialize
		if(FT_Load_Glyph(face, FT_Get_Char_Index(face, glyph), FT_LOAD_DEFAULT))
			throw love::Exception("TrueTypeFont Loading vm->error: FT_Load_Glyph failed\n");
		
		if( FT_Get_Glyph(face->glyph, &ftglyph) )
			throw love::Exception("TrueTypeFont Loading vm->error: FT_Get_Glyph failed\n");

		FT_Glyph_To_Bitmap(&ftglyph, FT_RENDER_MODE_NORMAL, 0, 1);
		FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)ftglyph;
		FT_Bitmap& bitmap = bitmap_glyph->bitmap; //just to make things easier

		// Get metrics
		glyphMetrics.bearingX = face->glyph->metrics.horiBearingX;
		glyphMetrics.bearingY = face->glyph->metrics.horiBearingY;
		glyphMetrics.height = face->glyph->metrics.height;
		glyphMetrics.width = face->glyph->metrics.width;
		glyphMetrics.advance = face->glyph->advance.x >> 6;

		// Get texture
		int w = next_p2(bitmap.width);
		int h = next_p2(bitmap.rows);
		textureData = new unsigned char[bpp * w * h];

		for(int j = 0; j < h; j++) for(int i = 0; i < w; i++)
		{
			textureData[bpp * (i + j * w)] = 255;
			textureData[bpp * (i + j * w) + 1] = (i >= bitmap.width || j >= bitmap.rows) ? 0 : bitmap.buffer[i + bitmap.width * j];
		}

		// Return data
		GlyphData * glyphData = new GlyphData(glyph, textureData, glyphMetrics, bpp);
		return glyphData;
	}

} // freetype
} // font
} // love