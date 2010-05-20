/**
* Copyright (c) 2006-2010 LOVE Development Team
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
#include "ImageRasterizer.h"

#include <common/Exception.h>
#include <string.h>

namespace love
{
namespace font
{

	inline bool equal(const love::image::pixel& a, const love::image::pixel& b)
	{
		return (a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a);
	}

	ImageRasterizer::ImageRasterizer(love::image::ImageData * data, unsigned short * glyphs, int length)
		: imageData(data), glyphs(glyphs), length(length)
	{
		imageData->retain();
		positions = new unsigned int[MAX_CHARS];
		memset(positions, 0, MAX_CHARS*4);
		widths = new unsigned int[MAX_CHARS];
		memset(widths, 0, MAX_CHARS*4);
		spacing = new unsigned int[MAX_CHARS];
		memset(spacing, 0, MAX_CHARS*4);
		load();
	}

	ImageRasterizer::~ImageRasterizer()
	{
		imageData->release();
		delete[] positions;
		delete[] widths;
		delete[] spacing;
	}

	int ImageRasterizer::getLineHeight() const
	{
		return getHeight();
	}

	GlyphData * ImageRasterizer::getGlyphData(unsigned short glyph) const
	{
		GlyphMetrics gm;
		gm.height = metrics.height;
		gm.width = widths[glyph];
		gm.advance = spacing[glyph] + widths[glyph];
		gm.bearingX = 0;
		gm.bearingY = 0;
		GlyphData * g = new GlyphData(glyph, gm, GlyphData::FORMAT_RGBA);
		if (gm.width == 0) return g;
		unsigned char * gd = (unsigned char*)g->getData();
		love::image::pixel * pixels = (love::image::pixel *)(imageData->getData());
		for (unsigned int i = 0; i < widths[glyph]*getHeight(); i++) {
			love::image::pixel p = pixels[ positions[glyph] + (i % widths[glyph]) + (imageData->getWidth() * (i / widths[glyph])) ];
			gd[i*4] = p.r;
			gd[i*4+1] = p.g;
			gd[i*4+2] = p.b;
			gd[i*4+3] = p.a;
		}
		return g;
	}

	void ImageRasterizer::load()
	{
		love::image::pixel * pixels = (love::image::pixel *)(imageData->getData());

		unsigned imgw = (unsigned)imageData->getWidth();
		unsigned imgh = (unsigned)imageData->getHeight();
		unsigned imgs = imgw*imgh;

		// Set the only metric that matters
		metrics.height = imgh;

		// Reading texture data begins
		love::image::pixel spacer = pixels[0];

		unsigned int start = 0;
		unsigned int end = 0;

		for(unsigned int i = 0; i < length; ++i)
		{
			if(i >= MAX_CHARS)
				break;

			start = end;

			// Finds out where the first character starts
			while(start < imgw && equal(pixels[start], spacer))
				++start;

			if(i > 0)
				spacing[glyphs[i - 1]] = (start > end) ? (start - end) : 0;

			end = start;

			// Find where glyph ends.
			while(end < imgw && !equal(pixels[end], spacer))
				++end;

			if(start >= end)
				break;

			unsigned c = glyphs[i];

			positions[c] = start;
			widths[c] = (end - start);
		}

		// Replace spacer color with an empty pixel
		for(unsigned int i = 0; i < imgs; ++i)
		{
			if(equal(pixels[i], spacer))
			{
				pixels[i].r = 0;
				pixels[i].g = 0;
				pixels[i].b = 0;
				pixels[i].a = 0;
			}
		}
	}

	int ImageRasterizer::getNumGlyphs() const
	{
		return length;
	}

} // font
} // love
