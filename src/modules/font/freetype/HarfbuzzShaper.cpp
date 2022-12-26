/**
 * Copyright (c) 2006-2022 LOVE Development Team
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

namespace love
{
namespace font
{
namespace freetype
{

HarfbuzzShaper::HarfbuzzShaper(TrueTypeRasterizer *rasterizer)
	: TextShaper(rasterizer)
{
	hbFonts.push_back(hb_ft_font_create_referenced((FT_Face)rasterizer->getHandle()));

	if (hbFonts[0] == nullptr || hbFonts[0] == hb_font_get_empty())
		throw love::Exception("");
}

HarfbuzzShaper::~HarfbuzzShaper()
{
	for (hb_font_t *font : hbFonts)
		hb_font_destroy(font);
}

void HarfbuzzShaper::setFallbacks(const std::vector<Rasterizer*>& fallbacks)
{
	for (size_t i = 1; i < hbFonts.size(); i++)
		hb_font_destroy(hbFonts[i]);

	TextShaper::setFallbacks(fallbacks);

	hbFonts.resize(rasterizers.size());

	for (size_t i = 1; i < hbFonts.size(); i++)
		hbFonts[i] = hb_ft_font_create_referenced((FT_Face)rasterizers[i]->getHandle());
}

void HarfbuzzShaper::computeGlyphPositions(const ColoredCodepoints &codepoints, Range range, Vector2 offset, float extraspacing, std::vector<GlyphPosition> *positions, std::vector<IndexedColor> *colors, TextInfo *info)
{
	if (!range.isValid())
		range = Range(0, codepoints.cps.size());

	offset.y += getBaseline();
	Vector2 curpos = offset;

	hb_buffer_t *hbbuffer = hb_buffer_create();

	hb_buffer_add_codepoints(hbbuffer, codepoints.cps.data(), codepoints.cps.size(), (unsigned int) range.getOffset(), (int) range.getSize());

	// TODO: Expose APIs for direction and script?
	hb_buffer_guess_segment_properties(hbbuffer);

	hb_shape(hbFonts[0], hbbuffer, nullptr, 0);

	int glyphcount = (int) hb_buffer_get_length(hbbuffer);
	hb_glyph_info_t* glyphinfos = hb_buffer_get_glyph_infos(hbbuffer, nullptr);
	hb_glyph_position_t* glyphpositions = hb_buffer_get_glyph_positions(hbbuffer, nullptr);

	if (positions)
		positions->reserve(positions->size() + glyphcount);

	if (rasterizers.size() > 1)
	{
		std::vector<Range> fallbackranges;

		for (int i = 0; i < glyphcount; i++)
		{
			bool adding = false;
			if (glyphinfos[i].codepoint == 0)
			{
				if (fallbackranges.empty() || fallbackranges.back().getMax() != glyphinfos[i-1].cluster)
					fallbackranges.push_back(Range(glyphinfos[i].cluster, 1));
				else
					fallbackranges.back().encapsulate(glyphinfos[i].cluster);
			}
		}

		if (!fallbackranges.empty())
		{
			for (size_t rasti = 1; rasti < rasterizers.size(); rasti++)
			{
				hb_buffer_t* hbbuffer = hb_buffer_create();

				hb_buffer_destroy(hbbuffer);
			}
		}
	}

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

	int maxwidth = (int)curpos.x;

	// TODO: fallbacks
	// TODO: use spaces for tab
	for (int i = 0; i < glyphcount; i++)
	{
		const hb_glyph_info_t& info = glyphinfos[i];
		const hb_glyph_position_t& glyphpos = glyphpositions[i];

		// TODO: this doesn't handle situations where the user inserted a color
		// change in the middle of some characters that get combined into a single
		// cluster.
		if (colors && colorindex < ncolors && codepoints.colors[colorindex].index == info.cluster)
		{
			colorToAdd.set(codepoints.colors[colorindex].color);
			colorindex++;
		}

		uint32 clustercodepoint = codepoints.cps[info.cluster];

		// Harfbuzz doesn't handle newlines itself, but it does leave them in
		// the glyph list so we can do it manually.
		if (clustercodepoint == '\n')
		{
			if (curpos.x > maxwidth)
				maxwidth = (int)curpos.x;

			// Wrap newline, but do not output a position for it.
			curpos.y += floorf(getHeight() * getLineHeight() + 0.5f);
			curpos.x = offset.x;
			continue;
		}

		// Ignore carriage returns
		if (clustercodepoint == '\r')
			continue;

		if (colorToAdd.hasValue && colors && positions)
		{
			IndexedColor c = {colorToAdd.value, positions->size()};
			colors->push_back(c);
			colorToAdd.clear();
		}

		if (positions)
		{
			// Despite the name this is a glyph index at this point.
			GlyphIndex gindex = { info.codepoint, 0 };
			GlyphPosition p = { curpos, gindex };

			// Harfbuzz position coordinate systems are based on the given font.
			// Freetype uses 26.6 fixed point coordinates, so harfbuzz does too.
			p.position.x += floorf((glyphpos.x_offset >> 6) / dpiScales[0] + 0.5f);
			p.position.y += floorf((glyphpos.y_offset >> 6) / dpiScales[0] + 0.5f);

			positions->push_back(p);
		}

		curpos.x += floorf((glyphpos.x_advance >> 6) / dpiScales[0] + 0.5f);
		curpos.y += floorf((glyphpos.y_advance >> 6) / dpiScales[0] + 0.5f);

		// Account for extra spacing given to space characters.
		if (clustercodepoint == ' ' && extraspacing != 0.0f)
			curpos.x = floorf(curpos.x + extraspacing);
	}

	hb_buffer_destroy(hbbuffer);

	if (curpos.x > maxwidth)
		maxwidth = (int)curpos.x;

	if (info != nullptr)
	{
		info->width = maxwidth - offset.x;
		info->height = curpos.y - offset.y;
		if (curpos.x > offset.x)
			info->height += floorf(getHeight() * getLineHeight() + 0.5f);
	}
}

int HarfbuzzShaper::computeWordWrapIndex(const ColoredCodepoints& codepoints, Range range, float wraplimit, float *width)
{
	if (!range.isValid())
		range = Range(0, codepoints.cps.size());

	hb_buffer_t* hbbuffer = hb_buffer_create();

	hb_buffer_add_codepoints(hbbuffer, codepoints.cps.data(), codepoints.cps.size(), (unsigned int)range.getOffset(), (int)range.getSize());

	// TODO: Expose APIs for direction and script?
	hb_buffer_guess_segment_properties(hbbuffer);

	hb_shape(hbFonts[0], hbbuffer, nullptr, 0);

	int glyphcount = (int)hb_buffer_get_length(hbbuffer);
	hb_glyph_info_t* glyphinfos = hb_buffer_get_glyph_infos(hbbuffer, nullptr);
	hb_glyph_position_t* glyphpositions = hb_buffer_get_glyph_positions(hbbuffer, nullptr);

	float w = 0.0f;
	float outwidth = 0.0f;
	float widthbeforelastspace = 0.0f;
	int wrapindex = -1;
	int lastspaceindex = -1;

	uint32 prevcodepoint = 0;

	for (int i = 0; i < glyphcount; i++)
	{
		const hb_glyph_info_t& info = glyphinfos[i];
		const hb_glyph_position_t& glyphpos = glyphpositions[i];

		uint32 clustercodepoint = codepoints.cps[info.cluster];

		if (clustercodepoint == '\r')
		{
			prevcodepoint = clustercodepoint;
			continue;
		}

		float newwidth = w + floorf((glyphpos.x_advance >> 6) / dpiScales[0] + 0.5f);

		// Only wrap when there's a non-space character.
		if (newwidth > wraplimit && clustercodepoint != ' ')
		{
			// Rewind to the last seen space when wrapping.
			if (lastspaceindex != -1)
			{
				wrapindex = lastspaceindex;
				outwidth = widthbeforelastspace;
			}
			break;
		}

		// Don't count trailing spaces in the output width.
		if (clustercodepoint == ' ')
		{
			lastspaceindex = info.cluster;
			if (prevcodepoint != ' ')
				widthbeforelastspace = w;
		}
		else
			outwidth = newwidth;

		w = newwidth;
		prevcodepoint = clustercodepoint;
		wrapindex = info.cluster;
	}

	hb_buffer_destroy(hbbuffer);

	if (width)
		*width = outwidth;

	return wrapindex;
}

} // freetype
} // font
} // love
