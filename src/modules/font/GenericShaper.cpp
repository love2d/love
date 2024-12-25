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
#include "GenericShaper.h"
#include "Rasterizer.h"
#include "common/Optional.h"

#include <algorithm>

namespace love
{
namespace font
{

GenericShaper::GenericShaper(Rasterizer *rasterizer)
	: TextShaper(rasterizer)
{
}

GenericShaper::~GenericShaper()
{
}

void GenericShaper::computeGlyphPositions(const ColoredCodepoints &codepoints, Range range, Vector2 offset, float extraspacing, std::vector<GlyphPosition> *positions, std::vector<IndexedColor> *colors, TextInfo *info)
{
	if (!range.isValid())
		range = Range(0, codepoints.cps.size());

	if (rasterizers[0]->getDataType() == Rasterizer::DATA_TRUETYPE)
		offset.y += getBaseline();

	// Spacing counter and newline handling.
	Vector2 curpos = offset;
	float spacingremainder = 0;

	float maxwidth = 0;
	uint32 prevglyph = 0;

	if (positions)
		positions->reserve(range.getSize());

	int colorindex = 0;
	int ncolors = (int) codepoints.colors.size();
	Optional<Colorf> colorToAdd;

	// Make sure the right color is applied to the start of the glyph list,
	// when the start isn't 0.
	if (colors && range.getOffset() > 0 && !codepoints.colors.empty())
	{
		for (; colorindex < ncolors; colorindex++)
		{
			if (codepoints.colors[colorindex].index >= (int) range.getOffset())
				break;
			colorToAdd.set(codepoints.colors[colorindex].color);
		}
	}

	for (int i = (int) range.getMin(); i <= (int) range.getMax(); i++)
	{
		uint32 g = codepoints.cps[i];

		// Do this before anything else so we don't miss colors corresponding
		// to newlines. The actual add to the list happens after newline
		// handling, to make sure the resulting index is valid in the positions
		// array.
		if (colors && colorindex < ncolors && codepoints.colors[colorindex].index == i)
		{
			colorToAdd.set(codepoints.colors[colorindex].color);
			colorindex++;
		}

		if (g == '\n')
		{
			maxwidth = std::max(maxwidth, curpos.x);

			// Wrap newline, but do not output a position for it.
			curpos.y += getCombinedHeight();
			curpos.x = offset.x;
			prevglyph = 0;
			continue;
		}

		// Ignore carriage returns
		if (g == '\r')
		{
			prevglyph = g;
			continue;
		}

		if (colorToAdd.hasValue && colors && positions)
		{
			IndexedColor c = {colorToAdd.value, (int) positions->size()};
			colors->push_back(c);
			colorToAdd.clear();
		}

		// Add kerning to the current horizontal offset.
		curpos.x += getKerning(prevglyph, g);

		GlyphIndex glyphindex;
		float advance = getGlyphAdvance(g, &glyphindex);

		if (positions)
			positions->push_back({ Vector2(curpos.x, curpos.y), glyphindex });

		// Advance the x position for the next glyph.
		curpos.x += advance;

		// Account for extra spacing given to space characters.
		if (g == ' ' && extraspacing != 0.0f)
		{
			spacingremainder += fmod(extraspacing, 1);
			curpos.x += floorf(extraspacing) + floorf(spacingremainder);
			spacingremainder = fmod(spacingremainder, 1);
		}

		prevglyph = g;
	}

	maxwidth = std::max(maxwidth, curpos.x);

	if (info != nullptr)
	{
		info->width = maxwidth - offset.x;
		info->height = curpos.y - offset.y;
		if (curpos.x > offset.x)
			info->height += getCombinedHeight();
	}
}

int GenericShaper::computeWordWrapIndex(const ColoredCodepoints &codepoints, Range range, float wraplimit, float *width)
{
	if (!range.isValid())
		range = Range(0, codepoints.cps.size());

	uint32 prevglyph = 0;

	float w = 0.0f;
	float outwidth = 0.0f;
	float widthbeforelastspace = 0.0f;
	int firstindexafterspace = -1;

	for (int i = (int)range.getMin(); i <= (int)range.getMax(); i++)
	{
		uint32 g = codepoints.cps[i];

		if (g == '\r')
		{
			prevglyph = g;
			continue;
		}

		float newwidth = w + getKerning(prevglyph, g) + getGlyphAdvance(g);

		// Don't count trailing spaces in the output width.
		if (isWhitespace(g))
		{
			if (!isWhitespace(prevglyph))
				widthbeforelastspace = w;
		}
		else
		{
			if (isWhitespace(prevglyph))
				firstindexafterspace = i;

			// Only wrap when there's a non-space character.
			if (newwidth > wraplimit)
			{
				// If this is the first character, wrap from the next one instead of this one.
				int wrapindex = i > (int)range.first ? i : (int)range.first + 1;

				// Rewind to after the last seen space when wrapping.
				if (firstindexafterspace != -1)
				{
					wrapindex = firstindexafterspace;
					outwidth = widthbeforelastspace;
				}

				if (width)
					*width = outwidth;

				return wrapindex;
			}

			outwidth = newwidth;
		}

		w = newwidth;
		prevglyph = g;
	}

	if (width)
		*width = outwidth;

	// There wasn't any wrap in the middle of the range.
	return range.last + 1;
}

} // font
} // love
