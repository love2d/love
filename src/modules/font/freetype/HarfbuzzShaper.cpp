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
#include "HarfbuzzShaper.h"
#include "TrueTypeRasterizer.h"
#include "common/Optional.h"

// harfbuzz
#include <hb.h>
#include <hb-ft.h>

#include <algorithm>

namespace love
{
namespace font
{
namespace freetype
{

HarfbuzzShaper::HarfbuzzShaper(TrueTypeRasterizer *rasterizer)
	: TextShaper(rasterizer)
	, spaceGlyphIndex()
	, tabSpacesAdvanceX(0)
	, tabSpacesAdvanceY(0)
{
	hbFonts.push_back(hb_ft_font_create_referenced((FT_Face)rasterizer->getHandle()));
	hbBuffers.push_back(hb_buffer_create());

	if (hbFonts[0] == nullptr || hbFonts[0] == hb_font_get_empty())
		throw love::Exception("Could not create Harfbuzz font object.");

	if (hbBuffers[0] == nullptr || hbBuffers[0] == hb_buffer_get_empty())
		throw love::Exception("Could not create Harfbuzz buffer object.");

	updateSpacesForTabInfo();
}

HarfbuzzShaper::~HarfbuzzShaper()
{
	for (hb_buffer_t *buffer : hbBuffers)
		hb_buffer_destroy(buffer);
	for (hb_font_t *font : hbFonts)
		hb_font_destroy(font);
}

void HarfbuzzShaper::setFallbacks(const std::vector<Rasterizer*> &fallbacks)
{
	for (size_t i = 1; i < rasterizers.size(); i++)
	{
		hb_buffer_destroy(hbBuffers[i]);
		hb_font_destroy(hbFonts[i]);
	}

	TextShaper::setFallbacks(fallbacks);

	hbFonts.resize(rasterizers.size());
	hbBuffers.resize(rasterizers.size());

	for (size_t i = 1; i < rasterizers.size(); i++)
	{
		hbFonts[i] = hb_ft_font_create_referenced((FT_Face)rasterizers[i]->getHandle());
		hbBuffers[i] = hb_buffer_create();
	}

	updateSpacesForTabInfo();
}

void HarfbuzzShaper::updateSpacesForTabInfo()
{
	if (!isUsingSpacesForTab())
		return;

	hb_codepoint_t glyphid = 0;
	for (size_t i = 0; i < hbFonts.size(); i++)
	{
		hb_font_t *hbfont = hbFonts[i];
		if (hb_font_get_glyph(hbfont, ' ', 0, &glyphid))
		{
			spaceGlyphIndex.index = glyphid;
			spaceGlyphIndex.rasterizerIndex = i;
			tabSpacesAdvanceX = hb_font_get_glyph_h_advance(hbfont, glyphid) * SPACES_PER_TAB;
			tabSpacesAdvanceY = hb_font_get_glyph_v_advance(hbfont, glyphid) * SPACES_PER_TAB;
			break;
		}
	}
}

bool HarfbuzzShaper::isValidGlyph(uint32 glyphindex, const std::vector<uint32> &codepoints, uint32 codepointindex)
{
	if (glyphindex != 0)
		return true;

	uint32 codepoint = codepoints[codepointindex];
	if (codepoint == '\n' || codepoint == '\r' || (codepoint == '\t' && isUsingSpacesForTab()))
		return true;

	return false;
}

void HarfbuzzShaper::computeBufferRanges(const ColoredCodepoints &codepoints, Range range, std::vector<BufferRange> &bufferranges)
{
	bufferranges.clear();

	if (codepoints.cps.size() == 0)
		return;

	// Less computation for the typical case (no fallback fonts).
	if (rasterizers.size() == 1)
	{
		hb_buffer_reset(hbBuffers[0]);
		hb_buffer_add_codepoints(hbBuffers[0], codepoints.cps.data(), codepoints.cps.size(), (unsigned int)range.getOffset(), (int)range.getSize());

		// TODO: Expose APIs for direction and script?
		hb_buffer_guess_segment_properties(hbBuffers[0]);

		hb_shape(hbFonts[0], hbBuffers[0], nullptr, 0);

		bufferranges.push_back({0, (int) range.first, Range(0, hb_buffer_get_length(hbBuffers[0]))});
		return;
	}

	std::vector<Range> fallbackranges = { range };

	// For each font, figure out the ranges of valid glyphs in the given string,
	// and add the rest to a list to be shaped by the next fallback font.
	// Harfbuzz doesn't have its own fallback API.
	for (size_t rasti = 0; rasti < rasterizers.size(); rasti++)
	{
		hb_buffer_t *hbb = hbBuffers[rasti];
		hb_buffer_reset(hbb);

		for (Range r : fallbackranges)
			hb_buffer_add_codepoints(hbb, codepoints.cps.data(), codepoints.cps.size(), (unsigned int)r.getOffset(), (int)r.getSize());

		hb_buffer_guess_segment_properties(hbb);

		hb_shape(hbFonts[rasti], hbb, nullptr, 0);

		size_t glyphcount = (size_t)hb_buffer_get_length(hbb);
		const hb_glyph_info_t *glyphinfos = hb_buffer_get_glyph_infos(hbb, nullptr);
		hb_direction_t direction = hb_buffer_get_direction(hbb);

		fallbackranges.clear();

		for (size_t i = 0; i < glyphcount; i++)
		{
			if (isValidGlyph(glyphinfos[i].codepoint, codepoints.cps, glyphinfos[i].cluster))
			{
				if (bufferranges.empty() || bufferranges.back().index != rasti || bufferranges.back().range.getMax() + 1 != i)
					bufferranges.push_back({rasti, (int)glyphinfos[i].cluster, Range(i, 1)});
				else
					bufferranges.back().range.last++;
			}
			else if (rasti == rasterizers.size() - 1)
			{
				// Use the first font for remaining invalid glyphs when no
				// fallback font supports them.
				if (bufferranges.empty() || bufferranges.back().index != 0 || bufferranges.back().range.getMax() + 1 != i)
					bufferranges.push_back({0, (int)glyphinfos[i].cluster, Range(i, 1)});
				else
					bufferranges.back().range.last++;
			}
			else
			{
				// Harfbuzz puts RTL text into the buffer in reverse order, so
				// it'll start with the last cluster (character index).
				if (fallbackranges.empty() || (direction == HB_DIRECTION_RTL ? fallbackranges.back().getMin() : fallbackranges.back().getMax()) != glyphinfos[i - 1].cluster)
					fallbackranges.push_back(Range(glyphinfos[i].cluster, 1));
				else
					fallbackranges.back().encapsulate(glyphinfos[i].cluster);
			}
		}
	}

	std::sort(bufferranges.begin(), bufferranges.end(), [](const BufferRange &a, const BufferRange &b)
	{
		if (a.codepointStart != b.codepointStart)
			return a.codepointStart < b.codepointStart;
		if (a.index != b.index)
			return a.index < b.index;
		return a.range.first < b.range.first;
	});
}

void HarfbuzzShaper::computeGlyphPositions(const ColoredCodepoints &codepoints, Range range, Vector2 offset, float extraspacing, std::vector<GlyphPosition> *positions, std::vector<IndexedColor> *colors, TextInfo *info)
{
	if (!range.isValid() && !codepoints.cps.empty())
		range = Range(0, codepoints.cps.size());

	offset.y += getBaseline();
	Vector2 curpos = offset;
	float spacingremainder = 0;

	int colorindex = 0;
	int ncolors = (int)codepoints.colors.size();
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

	std::vector<BufferRange> bufferranges;
	computeBufferRanges(codepoints, range, bufferranges);

	float maxwidth = curpos.x;

	for (const auto &bufferrange : bufferranges)
	{
		if (positions)
			positions->reserve(positions->size() + bufferrange.range.getSize());

		hb_buffer_t *hbbuffer = hbBuffers[bufferrange.index];

		const hb_glyph_info_t *glyphinfos = hb_buffer_get_glyph_infos(hbbuffer, nullptr);
		hb_glyph_position_t *glyphpositions = hb_buffer_get_glyph_positions(hbbuffer, nullptr);
		hb_direction_t direction = hb_buffer_get_direction(hbbuffer);

		for (size_t i = bufferrange.range.first; i <= bufferrange.range.last; i++)
		{
			const hb_glyph_info_t &info = glyphinfos[i];
			hb_glyph_position_t &glyphpos = glyphpositions[i];

			// TODO: this doesn't handle situations where the user inserted a color
			// change in the middle of some characters that get combined into a single
			// cluster.
			if (colors && colorindex < ncolors && codepoints.colors[colorindex].index == (int)info.cluster)
			{
				colorToAdd.set(codepoints.colors[colorindex].color);
				colorindex++;
			}

			uint32 clustercodepoint = codepoints.cps[info.cluster];

			// Harfbuzz doesn't handle newlines itself, but it does leave them in
			// the glyph list so we can do it manually.
			if (clustercodepoint == '\n')
			{
				maxwidth = std::max(maxwidth, curpos.x);

				// Wrap newline, but do not output a position for it.
				curpos.y += getCombinedHeight();
				curpos.x = offset.x;
				continue;
			}

			// Ignore carriage returns
			if (clustercodepoint == '\r')
				continue;

			// This is a glyph index at this point, despite the name.
			GlyphIndex gindex = { (int) info.codepoint, (int) bufferrange.index };

			if (clustercodepoint == '\t' && isUsingSpacesForTab())
			{
				gindex = spaceGlyphIndex;

				// This should be safe to overwrite.
				// TODO: RTL support?
				glyphpos.x_offset = 0;
				glyphpos.y_offset = 0;
				glyphpos.x_advance = HB_DIRECTION_IS_HORIZONTAL(direction) ? tabSpacesAdvanceX : 0;
				glyphpos.y_advance = HB_DIRECTION_IS_VERTICAL(direction) ? tabSpacesAdvanceY : 0;
			}

			if (colorToAdd.hasValue && colors && positions)
			{
				IndexedColor c = {colorToAdd.value, (int) positions->size()};
				colors->push_back(c);
				colorToAdd.clear();
			}

			if (positions)
			{
				GlyphPosition p = { curpos, gindex };

				// Harfbuzz position coordinate systems are based on the given font.
				// Freetype uses 26.6 fixed point coordinates, so harfbuzz does too.
				p.position.x += (glyphpos.x_offset >> 6) / dpiScales[bufferrange.index];
				p.position.y += (glyphpos.y_offset >> 6) / dpiScales[bufferrange.index];

				positions->push_back(p);
			}

			curpos.x += (glyphpos.x_advance >> 6) / dpiScales[bufferrange.index];
			curpos.y += (glyphpos.y_advance >> 6) / dpiScales[bufferrange.index];

			// Account for extra spacing given to space characters.
			if (clustercodepoint == ' ' && extraspacing != 0.0f)
			{
				spacingremainder += fmod(extraspacing, 1);
				curpos.x += floorf(extraspacing) + floorf(spacingremainder);
				spacingremainder = fmod(spacingremainder, 1);
			}
		}
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

int HarfbuzzShaper::computeWordWrapIndex(const ColoredCodepoints &codepoints, Range range, float wraplimit, float *width)
{
	if (!range.isValid())
		range = Range(0, codepoints.cps.size());

	float w = 0.0f;
	float outwidth = 0.0f;
	float widthbeforelastspace = 0.0f;
	int firstindexafterspace = -1;

	uint32 prevcodepoint = 0;

	std::vector<BufferRange> bufferranges;
	computeBufferRanges(codepoints, range, bufferranges);

	for (const auto &bufferrange : bufferranges)
	{
		hb_buffer_t *hbbuffer = hbBuffers[bufferrange.index];

		const hb_glyph_info_t *glyphinfos = hb_buffer_get_glyph_infos(hbbuffer, nullptr);
		hb_glyph_position_t *glyphpositions = hb_buffer_get_glyph_positions(hbbuffer, nullptr);
		hb_direction_t direction = hb_buffer_get_direction(hbbuffer);

		for (size_t i = bufferrange.range.first; i <= bufferrange.range.last; i++)
		{
			const hb_glyph_info_t &info = glyphinfos[i];
			hb_glyph_position_t &glyphpos = glyphpositions[i];

			uint32 clustercodepoint = codepoints.cps[info.cluster];

			if (clustercodepoint == '\r')
			{
				prevcodepoint = clustercodepoint;
				continue;
			}

			if (clustercodepoint == '\t' && isUsingSpacesForTab())
			{
				// This should be safe to overwrite.
				// TODO: RTL support?
				glyphpos.x_offset = 0;
				glyphpos.y_offset = 0;
				glyphpos.x_advance = HB_DIRECTION_IS_HORIZONTAL(direction) ? tabSpacesAdvanceX : 0;
				glyphpos.y_advance = HB_DIRECTION_IS_VERTICAL(direction) ? tabSpacesAdvanceY : 0;
			}

			float newwidth = w + (glyphpos.x_advance >> 6) / dpiScales[bufferrange.index];

			// Don't count trailing spaces in the output width.
			if (isWhitespace(clustercodepoint))
			{
				if (!isWhitespace(prevcodepoint))
					widthbeforelastspace = w;
			}
			else
			{
				if (isWhitespace(prevcodepoint))
					firstindexafterspace = info.cluster;

				// Only wrap when there's a non-space character.
				if (newwidth > wraplimit)
				{
					// If this is the first character, wrap from the next one instead of this one.
					int wrapindex = (int)info.cluster > (int)range.first ? (int)info.cluster : (int)range.first + 1;

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
			prevcodepoint = clustercodepoint;
		}
	}

	if (width)
		*width = outwidth;

	// There wasn't any wrap in the middle of the range.
	return (int) range.last + 1;
}

} // freetype
} // font
} // love
