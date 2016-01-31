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
#include "Rasterizer.h"

// UTF-8
#include "libraries/utf8/utf8.h"

namespace love
{
namespace font
{

Rasterizer::~Rasterizer()
{
}

int Rasterizer::getHeight() const
{
	return metrics.height;
}

int Rasterizer::getAdvance() const
{
	return metrics.advance;
}

int Rasterizer::getAscent() const
{
	return metrics.ascent;
}

int Rasterizer::getDescent() const
{
	return metrics.descent;
}

GlyphData *Rasterizer::getGlyphData(const std::string &text) const
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

	return getGlyphData(codepoint);
}

bool Rasterizer::hasGlyphs(const std::string &text) const
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

float Rasterizer::getKerning(uint32 /*leftglyph*/, uint32 /*rightglyph*/) const
{
	return 0.0f;
}

} // font
} // love
