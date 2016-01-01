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
#include "ImageRasterizer.h"

#include "common/Exception.h"
#include <string.h>

namespace love
{
namespace font
{

inline bool equal(const love::image::pixel &a, const love::image::pixel &b)
{
	return (a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a);
}

ImageRasterizer::ImageRasterizer(love::image::ImageData *data, uint32 *glyphs, int numglyphs, int extraspacing)
	: imageData(data)
	, glyphs(glyphs)
	, numglyphs(numglyphs)
	, extraSpacing(extraspacing)
{
	load();
}

ImageRasterizer::~ImageRasterizer()
{
}

int ImageRasterizer::getLineHeight() const
{
	return getHeight();
}

GlyphData *ImageRasterizer::getGlyphData(uint32 glyph) const
{
	GlyphMetrics gm = {};

	// Set relevant glyph metrics if the glyph is in this ImageFont
	std::map<uint32, ImageGlyphData>::const_iterator it = imageGlyphs.find(glyph);
	if (it != imageGlyphs.end())
	{
		gm.width = it->second.width;
		gm.advance = it->second.width + extraSpacing;
	}

	gm.height = metrics.height;

	GlyphData *g = new GlyphData(glyph, gm, GlyphData::FORMAT_RGBA);

	if (gm.width == 0)
		return g;

	// We don't want another thread modifying our ImageData mid-copy.
	love::thread::Lock lock(imageData->getMutex());

	love::image::pixel *gdpixels = (love::image::pixel *) g->getData();
	love::image::pixel *imagepixels = (love::image::pixel *) imageData->getData();

	// copy glyph pixels from imagedata to glyphdata
	for (int i = 0; i < g->getWidth() * g->getHeight(); i++)
	{
		love::image::pixel p = imagepixels[it->second.x + (i % gm.width) + (imageData->getWidth() * (i / gm.width))];

		// Use transparency instead of the spacer color
		if (equal(p, spacer))
			gdpixels[i].r = gdpixels[i].g = gdpixels[i].b = gdpixels[i].a = 0;
		else
			gdpixels[i] = p;
	}

	return g;
}

void ImageRasterizer::load()
{
	love::image::pixel *pixels = (love::image::pixel *) imageData->getData();

	int imgw = imageData->getWidth();
	int imgh = imageData->getHeight();

	// We don't want another thread modifying our ImageData mid-parse.
	love::thread::Lock lock(imageData->getMutex());

	// Set the only metric that matters
	metrics.height = imgh;

	// Reading texture data begins
	spacer = pixels[0];

	int start = 0;
	int end = 0;

	for (int i = 0; i < numglyphs; ++i)
	{
		start = end;

		// Finds out where the first character starts
		while (start < imgw && equal(pixels[start], spacer))
			++start;

		end = start;

		// Find where glyph ends.
		while (end < imgw && !equal(pixels[end], spacer))
			++end;

		if (start >= end)
			break;

		ImageGlyphData imageGlyph;
		imageGlyph.x = start;
		imageGlyph.width = end - start;

		imageGlyphs[glyphs[i]] = imageGlyph;
	}
}

int ImageRasterizer::getGlyphCount() const
{
	return numglyphs;
}

bool ImageRasterizer::hasGlyph(uint32 glyph) const
{
	return imageGlyphs.find(glyph) != imageGlyphs.end();
}

} // font
} // love
