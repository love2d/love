/**
 * Copyright (c) 2006-2023 LOVE Development Team
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

#ifndef LOVE_FONT_BMFONT_RASTERIZER_H
#define LOVE_FONT_BMFONT_RASTERIZER_H

// LOVE
#include "common/config.h"
#include "Rasterizer.h"
#include "image/ImageData.h"

// C++
#include <unordered_map>
#include <vector>

namespace love
{
namespace font
{

/**
 * Rasterizer for BMFont bitmap fonts.
 **/
class BMFontRasterizer : public Rasterizer
{
public:

	BMFontRasterizer(love::filesystem::FileData *fontdef, const std::vector<image::ImageData *> &imagelist, float dpiscale);
	virtual ~BMFontRasterizer();

	// Implements Rasterizer.
	int getLineHeight() const override;
	GlyphData *getGlyphData(uint32 glyph) const override;
	int getGlyphCount() const override;
	bool hasGlyph(uint32 glyph) const override;
	float getKerning(uint32 leftglyph, uint32 rightglyph) const override;
	DataType getDataType() const override;

	static bool accepts(love::filesystem::FileData *fontdef);

private:

	struct BMFontCharacter
	{
		int x;
		int y;
		int page;
		GlyphMetrics metrics;
	};

	void parseConfig(const std::string &config);

	std::string fontFolder;

	// Image pages, indexed by their page id.
	std::unordered_map<int, StrongRef<image::ImageData>> images;

	// Glyph characters, indexed by their glyph id.
	std::unordered_map<uint32, BMFontCharacter> characters;

	// Kerning information, indexed by two (packed) characters.
	std::unordered_map<uint64, int> kerning;

	int fontSize;
	bool unicode;

	int lineHeight;

}; // BMFontRasterizer

} // font
} // love

#endif // LOVE_FONT_BMFONT_RASTERIZER_H
