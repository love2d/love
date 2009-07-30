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

#ifndef LOVE_FONT_GLYPH_DATA_H
#define LOVE_FONT_GLYPH_DATA_H

// LOVE
#include <common/Data.h>

namespace love
{
namespace font
{
	/**
	* Holds the specific glyph data.
	**/
	struct GlyphMetrics
	{
		int height;
		int width;
		int advance;
		int bearingX;
		int bearingY;
	};

	/**
	* Holds data for a specic glyph object.
	**/
	class GlyphData : public Data
	{
	private:
		// The glyph itself
		unsigned short glyph;

		// Glyph metrics
		GlyphMetrics metrics;

		// Glyph texture data
		unsigned char * data;
		
	public:
		GlyphData(unsigned short glyph, GlyphMetrics glyphMetrics);
		virtual ~GlyphData();

		// Implements Data.
		void * getData() const;
		int getSize() const;

		/**
		* Gets the height of the glyph.
		**/
		int getHeight() const;

		/**
		* Gets the width of the glyph.
		**/
		int getWidth() const;

		/**
		* Gets the advance (the space the glyph takes up) of the glyph.
		**/
		int getAdvance() const;

		/**
		* Gets bearing (the spacing from origin) along the x-axis of the glyph.
		**/
		int getBearingX() const;

		/**
		* Gets bearing (the spacing from origin) along the y-axis of the glyph.
		**/
		int getBearingY() const;

		/**
		* Gets the min x value of the glyph.
		**/
		int getMinX() const;

		/**
		* Gets the min y value of the glyph.
		**/
		int getMinY() const;

		/**
		* Gets the max x value of the glyph.
		**/
		int getMaxX() const;

		/**
		* Gets the max y value of the glyph.
		**/
		int getMaxY() const;

	}; // GlyphData

} // font
} // love

#endif // LOVE_FONT_GLYPH_DATA_H
