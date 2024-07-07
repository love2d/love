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
#include "TextShaper.h"
#include "Rasterizer.h"
#include "common/Exception.h"

#include "libraries/utf8/utf8.h"

namespace love
{
namespace font
{

void getCodepointsFromString(const std::string &text, std::vector<uint32> &codepoints)
{
	codepoints.reserve(text.size());

	try
	{
		utf8::iterator<std::string::const_iterator> i(text.begin(), text.begin(), text.end());
		utf8::iterator<std::string::const_iterator> end(text.end(), text.begin(), text.end());

		while (i != end)
		{
			uint32 g = *i++;
			codepoints.push_back(g);
		}
	}
	catch (utf8::exception &e)
	{
		throw love::Exception("UTF-8 decoding error: %s", e.what());
	}
}

void getCodepointsFromString(const std::vector<ColoredString> &strs, ColoredCodepoints &codepoints)
{
	if (strs.empty())
		return;

	codepoints.cps.reserve(strs[0].str.size());

	for (const ColoredString &cstr : strs)
	{
		// No need to add the color if the string is empty anyway, and the code
		// further on assumes no two colors share the same starting position.
		if (cstr.str.size() == 0)
			continue;

		IndexedColor c = { cstr.color, (int)codepoints.cps.size() };
		codepoints.colors.push_back(c);

		getCodepointsFromString(cstr.str, codepoints.cps);
	}

	if (codepoints.colors.size() == 1)
	{
		IndexedColor c = codepoints.colors[0];

		if (c.index == 0 && c.color == Colorf(1.0f, 1.0f, 1.0f, 1.0f))
			codepoints.colors.pop_back();
	}
}

love::Type TextShaper::type("TextShaper", &Object::type);

TextShaper::TextShaper(Rasterizer *rasterizer)
	: rasterizers{rasterizer}
	, dpiScales{rasterizer->getDPIScale()}
	, height(floorf(rasterizer->getHeight() / rasterizer->getDPIScale() + 0.5f))
	, pixelHeight(rasterizer->getHeight())
	, lineHeight(1)
	, useSpacesForTab(false)
{
	if (!rasterizer->hasGlyph('\t'))
		useSpacesForTab = true;
}

TextShaper::~TextShaper()
{
}

float TextShaper::getHeight() const
{
	return height;
}

float TextShaper::getPixelHeight() const
{
	return pixelHeight;
}

float TextShaper::getCombinedHeight() const
{
	return floorf(pixelHeight * lineHeight + 0.5f) / rasterizers[0]->getDPIScale();
}

void TextShaper::setLineHeight(float h)
{
	lineHeight = h;
}

float TextShaper::getLineHeight() const
{
	return lineHeight;
}

float TextShaper::getAscent() const
{
	return rasterizers[0]->getAscent() / rasterizers[0]->getDPIScale();
}

float TextShaper::getDescent() const
{
	return rasterizers[0]->getDescent() / rasterizers[0]->getDPIScale();
}

float TextShaper::getBaseline() const
{
	float ascent = getAscent();
	if (ascent != 0.0f)
		return ascent;
	else if (rasterizers[0]->getDataType() == font::Rasterizer::DATA_TRUETYPE)
		return floorf(getPixelHeight() / 1.25f + 0.5f) / rasterizers[0]->getDPIScale(); // 1.25 is magic line height for true type fonts
	else
		return 0.0f;
}

bool TextShaper::hasGlyph(uint32 glyph) const
{
	for (const StrongRef<Rasterizer> &r : rasterizers)
	{
		if (r->hasGlyph(glyph))
			return true;
	}

	return false;
}

bool TextShaper::hasGlyphs(const std::string &text) const
{
	if (text.size() == 0)
		return false;

	try
	{
		utf8::iterator<std::string::const_iterator> i(text.begin(), text.begin(), text.end());
		utf8::iterator<std::string::const_iterator> end(text.end(), text.begin(), text.end());

		while (i != end)
		{
			uint32 codepoint = *i++;

			if (!hasGlyph(codepoint))
				return false;
		}
	}
	catch (utf8::exception &e)
	{
		throw love::Exception("UTF-8 decoding error: %s", e.what());
	}

	return true;
}

float TextShaper::getKerning(uint32 leftglyph, uint32 rightglyph)
{
	uint64 packedglyphs = ((uint64)leftglyph << 32) | (uint64)rightglyph;

	const auto it = kerning.find(packedglyphs);
	if (it != kerning.end())
		return it->second;

	float k = 0.0f;
	bool found = false;

	for (const auto &r : rasterizers)
	{
		if (r->hasGlyph(leftglyph) && r->hasGlyph(rightglyph))
		{
			found = true;
			k = r->getKerning(leftglyph, rightglyph) / r->getDPIScale();
			break;
		}
	}

	if (!found)
		k = rasterizers[0]->getKerning(leftglyph, rightglyph) / rasterizers[0]->getDPIScale();

	kerning[packedglyphs] = k;
	return k;
}

float TextShaper::getKerning(const std::string &leftchar, const std::string &rightchar)
{
	uint32 left = 0;
	uint32 right = 0;

	try
	{
		left = utf8::peek_next(leftchar.begin(), leftchar.end());
		right = utf8::peek_next(rightchar.begin(), rightchar.end());
	}
	catch (utf8::exception &e)
	{
		throw love::Exception("UTF-8 decoding error: %s", e.what());
	}

	return getKerning(left, right);
}

float TextShaper::getGlyphAdvance(uint32 glyph, GlyphIndex *glyphindex)
{
	const auto it = glyphAdvances.find(glyph);
	if (it != glyphAdvances.end())
	{
		if (glyphindex)
			*glyphindex = it->second.second;
		return it->second.first;
	}

	int rasterizeri = 0;
	uint32 realglyph = glyph;

	if (glyph == '\t' && isUsingSpacesForTab())
		realglyph = ' ';

	for (size_t i = 0; i < rasterizers.size(); i++)
	{
		if (rasterizers[i]->hasGlyph(realglyph))
		{
			rasterizeri = (int) i;
			break;
		}
	}

	const auto &r = rasterizers[rasterizeri];
	float advance = r->getGlyphSpacing(realglyph) / r->getDPIScale();

	if (glyph == '\t' && realglyph == ' ')
		advance *= SPACES_PER_TAB;

	GlyphIndex glyphi = {r->getGlyphIndex(realglyph), rasterizeri};

	glyphAdvances[glyph] = std::make_pair(advance, glyphi);
	if (glyphindex)
		*glyphindex = glyphi;
	return advance;
}

float TextShaper::getWidth(const std::string& str)
{
	if (str.size() == 0) return 0;

	ColoredCodepoints codepoints;
	getCodepointsFromString(str, codepoints.cps);

	TextInfo info;
	computeGlyphPositions(codepoints, Range(), Vector2(0.0f, 0.0f), 0.0f, nullptr, nullptr, &info);

	return info.width;
}

static size_t findNewline(const ColoredCodepoints &codepoints, size_t start)
{
	for (size_t i = start; i < codepoints.cps.size(); i++)
	{
		if (codepoints.cps[i] == '\n')
		{
			return i;
		}
	}

	return codepoints.cps.size();
}

void TextShaper::getWrap(const ColoredCodepoints &codepoints, float wraplimit, std::vector<Range> &lineranges, std::vector<float> *linewidths)
{
	size_t nextnewline = findNewline(codepoints, 0);

	for (size_t i = 0; i < codepoints.cps.size();)
	{
		if (nextnewline < i)
			nextnewline = findNewline(codepoints, i);

		if (nextnewline == i) // Empty line.
		{
			lineranges.push_back(Range());
			if (linewidths)
				linewidths->push_back(0);
			i++;
		}
		else
		{
			Range r(i, nextnewline - i);
			float width = 0.0f;
			int wrapindex = computeWordWrapIndex(codepoints, r, wraplimit, &width);

			if (wrapindex > (int) i)
			{
				r = Range(i, (size_t) wrapindex - i);
				i = (size_t)wrapindex;
			}
			else
			{
				r = Range();
				i++;
			}

			// We've already handled this line, skip the newline character.
			if (nextnewline == i)
				i++;

			lineranges.push_back(r);
			if (linewidths)
				linewidths->push_back(width);
		}
	}
}

void TextShaper::getWrap(const std::vector<ColoredString> &text, float wraplimit, std::vector<std::string> &lines, std::vector<float> *linewidths)
{
	ColoredCodepoints cps;
	getCodepointsFromString(text, cps);

	std::vector<Range> codepointranges;
	getWrap(cps, wraplimit, codepointranges, linewidths);

	std::string line;

	for (const auto &range : codepointranges)
	{
		line.clear();

		if (range.isValid())
		{
			line.reserve(range.getSize());

			for (size_t i = range.getMin(); i <= range.getMax(); i++)
			{
				char character[5] = { '\0' };
				char *end = utf8::unchecked::append(cps.cps[i], character);
				line.append(character, end - character);
			}
		}

		lines.push_back(line);
	}
}

void TextShaper::setFallbacks(const std::vector<Rasterizer*> &fallbacks)
{
	for (Rasterizer *r : fallbacks)
	{
		if (r->getDataType() != rasterizers[0]->getDataType())
			throw love::Exception("Font fallbacks must be of the same font type.");
	}

	// Clear caches.
	kerning.clear();
	glyphAdvances.clear();

	rasterizers.resize(1);
	dpiScales.resize(1);

	for (Rasterizer *r : fallbacks)
	{
		rasterizers.push_back(r);
		dpiScales.push_back(r->getDPIScale());
	}
}

} // font
} // love
