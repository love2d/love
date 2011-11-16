/**
* Copyright (c) 2006-2011 LOVE Development Team
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

#ifndef LOVE_FONT_IMAGE_RASTERIZER_H
#define LOVE_FONT_IMAGE_RASTERIZER_H

// LOVE
#include <filesystem/File.h>
#include <font/Rasterizer.h>
#include <image/ImageData.h>

namespace love
{
namespace font
{
	/**
	* Holds data for a font object.
	**/
	class ImageRasterizer : public Rasterizer
	{
	private:
		// Load all the glyph positions into memory
		void load();

		// The image data
		love::image::ImageData * imageData;
		// The glyphs in the font
		unsigned short * glyphs;
		// The length of the glyph array
		unsigned int length;
		// The positions of each glyph
		unsigned int * positions;
		// The widths of each glyph
		unsigned int * widths;
		// The spacing of each glyph
		unsigned int * spacing;

	public:
		ImageRasterizer(love::image::ImageData * imageData, unsigned short * glyphs, int length);
		virtual ~ImageRasterizer();

		// Implement Rasterizer
		virtual int getLineHeight() const;
		virtual GlyphData * getGlyphData(unsigned short glyph) const;
		virtual int getNumGlyphs() const;

		static const unsigned int MAX_CHARS = 256;

	}; // ImageRasterizer

} // font
} // love

#endif // LOVE_FONT_IMAGE_RASTERIZER_H