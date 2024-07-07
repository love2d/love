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
#include "common/Object.h"
#include "common/Vector.h"
#include "common/int.h"
#include "common/Color.h"
#include "common/Range.h"

#include <vector>
#include <string>
#include <unordered_map>

namespace love
{
namespace font
{

class Rasterizer;

struct ColoredString
{
	std::string str;
	Colorf color;
};

struct IndexedColor
{
	Colorf color;
	int index;
};

struct ColoredCodepoints
{
	std::vector<uint32> cps;
	std::vector<IndexedColor> colors;
};

void getCodepointsFromString(const std::string &str, std::vector<uint32> &codepoints);
void getCodepointsFromString(const std::vector<ColoredString> &strs, ColoredCodepoints &codepoints);

class TextShaper : public Object
{
public:

	struct GlyphIndex
	{
		int index;
		int rasterizerIndex;
	};

	struct GlyphPosition
	{
		Vector2 position;
		GlyphIndex glyphIndex;
	};

	struct TextInfo
	{
		float width;
		float height;
	};

	// This will be used if the Rasterizer doesn't have a tab character itself.
	static const int SPACES_PER_TAB = 4;

	static love::Type type;

	virtual ~TextShaper();

	const std::vector<StrongRef<Rasterizer>> &getRasterizers() const { return rasterizers; }
	bool isUsingSpacesForTab() const { return useSpacesForTab; }

	float getHeight() const;
	float getPixelHeight() const;

	float getCombinedHeight() const;

	/**
	 * Sets the line height (which should be a number to multiply the font size by,
	 * example: line height = 1.2 and size = 12 means that rendered line height = 12*1.2)
	 * @param height The new line height.
	 **/
	void setLineHeight(float height);

	/**
	 * Returns the line height.
	 **/
	float getLineHeight() const;

	// Extra font metrics
	float getAscent() const;
	float getDescent() const;
	float getBaseline() const;

	bool hasGlyph(uint32 glyph) const;
	bool hasGlyphs(const std::string &text) const;

	float getKerning(uint32 leftglyph, uint32 rightglyph);
	float getKerning(const std::string &leftchar, const std::string &rightchar);

	float getGlyphAdvance(uint32 glyph, GlyphIndex *glyphindex = nullptr);

	float getWidth(const std::string &str);

	void getWrap(const std::vector<ColoredString> &text, float wraplimit, std::vector<std::string> &lines, std::vector<float> *linewidths = nullptr);
	void getWrap(const ColoredCodepoints &codepoints, float wraplimit, std::vector<Range> &lineranges, std::vector<float> *linewidths = nullptr);

	virtual void setFallbacks(const std::vector<Rasterizer *> &fallbacks);

	virtual void computeGlyphPositions(const ColoredCodepoints &codepoints, Range range, Vector2 offset, float extraspacing, std::vector<GlyphPosition> *positions, std::vector<IndexedColor> *colors, TextInfo *info) = 0;
	virtual int computeWordWrapIndex(const ColoredCodepoints &codepoints, Range range, float wraplimit, float *width) = 0;

protected:

	TextShaper(Rasterizer *rasterizer);

	static inline bool isWhitespace(uint32 codepoint) { return codepoint == ' ' || codepoint == '\t'; }

	std::vector<StrongRef<Rasterizer>> rasterizers;
	std::vector<float> dpiScales;

private:

	int height;
	int pixelHeight;
	float lineHeight;

	bool useSpacesForTab;

	// maps glyphs to advance and glyph+rasterizer index.
	std::unordered_map<uint32, std::pair<float, GlyphIndex>> glyphAdvances;

	// map of left/right glyph pairs to horizontal kerning.
	std::unordered_map<uint64, float> kerning;

}; // TextShaper

} // font
} // love
