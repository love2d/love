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
#include "TrueTypeRasterizer.h"

#include <common/Exception.h>

namespace love
{
namespace font
{
namespace freetype
{
	struct la { unsigned char l,a; };

	TrueTypeRasterizer::TrueTypeRasterizer(FT_Library library, Data * data, int size)
		: data(data)
	{
		data->retain();

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

	TrueTypeRasterizer::~TrueTypeRasterizer()
	{
		FT_Done_Face(face);
		data->release();
	}

	int TrueTypeRasterizer::getLineHeight() const
	{
		return (int)(getHeight() * 1.25);
	}

	GlyphData * TrueTypeRasterizer::getGlyphData(unsigned short glyph) const
	{
		love::font::GlyphMetrics glyphMetrics;
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
		glyphMetrics.height = bitmap.rows;
		glyphMetrics.width = bitmap.width;
		glyphMetrics.advance = face->glyph->advance.x >> 6;

		GlyphData * glyphData = new GlyphData(glyph, glyphMetrics);

		{
			int size = bitmap.rows*bitmap.width;
			unsigned char * dst = (unsigned char *)glyphData->getData();

			// Note that bitmap.buffer contains only luminocity. We copy that single value to 
			// our luminocity-alpha format. 
			for(int i = 0; i<size; i++)
			{
				dst[2*i] = dst[2*i+1] = bitmap.buffer[i];
			}
		}

		// Return data
		return glyphData;
	}

} // freetype
} // font
} // love