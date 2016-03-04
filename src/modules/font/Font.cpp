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
#include "Font.h"
#include "BMFontRasterizer.h"
#include "ImageRasterizer.h"

#include "libraries/utf8/utf8.h"

namespace love
{
namespace font
{

// Default TrueType font.
#include "Vera.ttf.h"

class DefaultFontData : public love::Data
{
public:

	void *getData() const override { return Vera_ttf; }
	size_t getSize() const override { return sizeof(Vera_ttf); }
};

Rasterizer *Font::newTrueTypeRasterizer(int size, TrueTypeRasterizer::Hinting hinting)
{
	StrongRef<DefaultFontData> data(new DefaultFontData, Acquire::NORETAIN);
	return newTrueTypeRasterizer(data.get(), size, hinting);
}

Rasterizer *Font::newBMFontRasterizer(love::filesystem::FileData *fontdef, const std::vector<image::ImageData *> &images)
{
	return new BMFontRasterizer(fontdef, images);
}

Rasterizer *Font::newImageRasterizer(love::image::ImageData *data, const std::string &text, int extraspacing)
{
	std::vector<uint32> glyphs;
	glyphs.reserve(text.size());

	try
	{
		utf8::iterator<std::string::const_iterator> i(text.begin(), text.begin(), text.end());
		utf8::iterator<std::string::const_iterator> end(text.end(), text.begin(), text.end());

		while (i != end)
			glyphs.push_back(*i++);
	}
	catch (utf8::exception &e)
	{
		throw love::Exception("UTF-8 decoding error: %s", e.what());
	}

	return newImageRasterizer(data, &glyphs[0], (int) glyphs.size(), extraspacing);
}

Rasterizer *Font::newImageRasterizer(love::image::ImageData *data, uint32 *glyphs, int numglyphs, int extraspacing)
{
	return new ImageRasterizer(data, glyphs, numglyphs, extraspacing);
}

GlyphData *Font::newGlyphData(Rasterizer *r, const std::string &text)
{
	uint32 codepoint = 0;

	try
	{
		codepoint = utf8::peek_next(text.begin(), text.end());
	}
	catch (utf8::exception &e)
	{
		throw love::Exception("UTF-8 decoding error: %s", e.what());
	}

	return r->getGlyphData(codepoint);
}

GlyphData *Font::newGlyphData(Rasterizer *r, uint32 glyph)
{
	return r->getGlyphData(glyph);
}

} // font
} // love
