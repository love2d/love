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

#pragma once

// LOVE
#include "font/TextShaper.h"

extern "C"
{
typedef struct hb_font_t hb_font_t;
typedef struct hb_buffer_t hb_buffer_t;
}

namespace love
{
namespace font
{
namespace freetype
{

class TrueTypeRasterizer;

class HarfbuzzShaper : public love::font::TextShaper
{
public:

	HarfbuzzShaper(TrueTypeRasterizer *rasterizer);
	virtual ~HarfbuzzShaper();

	void setFallbacks(const std::vector<Rasterizer *> &fallbacks) override;
	void computeGlyphPositions(const ColoredCodepoints &codepoints, Range range, Vector2 offset, float extraspacing, std::vector<GlyphPosition> *positions, std::vector<IndexedColor> *colors, TextInfo *info) override;
	int computeWordWrapIndex(const ColoredCodepoints &codepoints, Range range, float wraplimit, float *width) override;

private:

	struct BufferRange
	{
		size_t index;
		int codepointStart;
		Range range;
	};

	void updateSpacesForTabInfo();
	bool isValidGlyph(uint32 glyphindex, const std::vector<uint32> &codepoints, uint32 codepointindex);
	void computeBufferRanges(const ColoredCodepoints &codepoints, Range range, std::vector<BufferRange> &bufferranges);

	std::vector<hb_font_t *> hbFonts;
	std::vector<hb_buffer_t *> hbBuffers;

	GlyphIndex spaceGlyphIndex;
	int tabSpacesAdvanceX;
	int tabSpacesAdvanceY;

}; // HarfbuzzShaper

} // freetype
} // font
} // love
