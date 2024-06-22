/**
 * Copyright (c) 2006-2024 LOVE Development Team
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
#include "GenericShaper.h"
#include "common/Exception.h"

#include <string.h>

namespace love
{
namespace font
{

static_assert(sizeof(Color32) == 4, "sizeof(Color32) must equal 4 bytes!");

ImageRasterizer::ImageRasterizer(love::image::ImageData *data, const uint32 *glyphs, int numglyphs, int extraspacing, float dpiscale)
	: imageData(data)
	, numglyphs(numglyphs + 1) // Always have a null glyph at the start of the array.
	, extraSpacing(extraspacing)
{
	this->dpiScale = dpiscale;

	if (data->getFormat() != PIXELFORMAT_RGBA8_UNORM)
		throw love::Exception("Only 32-bit RGBA images are supported in Image Fonts!");

	load(glyphs, numglyphs);
}

ImageRasterizer::~ImageRasterizer()
{
}

int ImageRasterizer::getLineHeight() const
{
	return getHeight();
}

int ImageRasterizer::getGlyphSpacing(uint32 glyph) const
{
	auto it = glyphIndices.find(glyph);
	if (it == glyphIndices.end())
		return 0;
	return imageGlyphs[it->second].width + extraSpacing;
}

int ImageRasterizer::getGlyphIndex(uint32 glyph) const
{
	auto it = glyphIndices.find(glyph);
	if (it == glyphIndices.end())
		return 0;
	return it->second;
}

GlyphData *ImageRasterizer::getGlyphDataForIndex(int index) const
{
	GlyphMetrics gm = {};
	uint32 glyph = 0;

	// Set relevant glyph metrics if the glyph is in this ImageFont
	if (index >= 0 && index < (int) imageGlyphs.size())
	{
		gm.width = imageGlyphs[index].width;
		gm.advance = imageGlyphs[index].width + extraSpacing;
		glyph = imageGlyphs[index].glyph;
	}

	gm.height = metrics.height;

	GlyphData *g = new GlyphData(glyph, gm, PIXELFORMAT_RGBA8_UNORM);

	if (gm.width == 0)
		return g;

	// Always lock the mutex since the user can't know when to do it.
	love::thread::Lock lock(imageData->getMutex());

	Color32 *gdpixels = (Color32 *) g->getData();
	const Color32 *imagepixels = (const Color32 *) imageData->getData();

	// copy glyph pixels from imagedata to glyphdata
	for (int i = 0; i < g->getWidth() * g->getHeight(); i++)
	{
		Color32 p = imagepixels[imageGlyphs[index].x + (i % gm.width) + (imageData->getWidth() * (i / gm.width))];

		// Use transparency instead of the spacer color
		if (p == spacer)
			gdpixels[i] = Color32(0, 0, 0, 0);
		else
			gdpixels[i] = p;
	}

	return g;
}

void ImageRasterizer::load(const uint32 *glyphs, int glyphcount)
{
	auto pixels = (const Color32 *) imageData->getData();

	int imgw = imageData->getWidth();
	int imgh = imageData->getHeight();

	// Set the only metric that matters
	metrics.height = imgh;

	// Reading texture data begins
	spacer = pixels[0];

	int start = 0;
	int end = 0;

	{
		ImageGlyphData nullglyph;
		nullglyph.x = 0;
		nullglyph.width = 0;
		nullglyph.glyph = 0;
		imageGlyphs.push_back(nullglyph);
		glyphIndices[0] = (int) imageGlyphs.size() - 1;
	}

	for (int i = 0; i < glyphcount; ++i)
	{
		start = end;

		// Finds out where the first character starts
		while (start < imgw && pixels[start] == spacer)
			++start;

		end = start;

		// Find where glyph ends.
		while (end < imgw && pixels[end] != spacer)
			++end;

		if (start >= end)
			break;

		ImageGlyphData imageGlyph;
		imageGlyph.x = start;
		imageGlyph.width = end - start;
		imageGlyph.glyph = glyphs[i];

		imageGlyphs.push_back(imageGlyph);
		glyphIndices[glyphs[i]] = (int) imageGlyphs.size() - 1;
	}
}

int ImageRasterizer::getGlyphCount() const
{
	return numglyphs;
}

bool ImageRasterizer::hasGlyph(uint32 glyph) const
{
	return glyphIndices.find(glyph) != glyphIndices.end();
}

Rasterizer::DataType ImageRasterizer::getDataType() const
{
	return DATA_IMAGE;
}

TextShaper *ImageRasterizer::newTextShaper()
{
	return new GenericShaper(this);
}

} // font
} // love
