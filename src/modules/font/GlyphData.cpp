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
#include "GlyphData.h"

// UTF-8
#include "libraries/utf8/utf8.h"

// stdlib
#include <iostream>
#include <cstddef>

namespace love
{
namespace font
{

GlyphData::GlyphData(uint32 glyph, GlyphMetrics glyphMetrics, GlyphData::Format f)
	: glyph(glyph)
	, metrics(glyphMetrics)
	, data(nullptr)
	, format(f)
{
	if (metrics.width > 0 && metrics.height > 0)
		data = new uint8[metrics.width * metrics.height * getPixelSize()];
}

GlyphData::~GlyphData()
{
	delete[] data;
}

void *GlyphData::getData() const
{
	return data;
}

size_t GlyphData::getPixelSize() const
{
	switch (format)
	{
	case FORMAT_LUMINANCE_ALPHA:
		return 2;
	case FORMAT_RGBA:
	default:
		return 4;
	}
}

void *GlyphData::getData(int x, int y) const
{
	size_t offset = (y * getWidth() + x) * getPixelSize();
	return data + offset;
}

size_t GlyphData::getSize() const
{
	return size_t(getWidth() * getHeight()) * getPixelSize();
}

int GlyphData::getHeight() const
{
	return metrics.height;
}

int GlyphData::getWidth() const
{
	return metrics.width;
}

uint32 GlyphData::getGlyph() const
{
	return glyph;
}

std::string GlyphData::getGlyphString() const
{
	char u[5] = {0, 0, 0, 0, 0};
	ptrdiff_t length = 0;

	try
	{
		char *end = utf8::append(glyph, u);
		length = end - u;
	}
	catch (utf8::exception &e)
	{
		throw love::Exception("UTF-8 decoding error: %s", e.what());
	}

	// Just in case...
	if (length < 0)
		return "";

	return std::string(u, length);
}

int GlyphData::getAdvance() const
{
	return metrics.advance;
}

int GlyphData::getBearingX() const
{
	return metrics.bearingX;
}

int GlyphData::getBearingY() const
{
	return metrics.bearingY;
}

int GlyphData::getMinX() const
{
	return getBearingX();
}

int GlyphData::getMinY() const
{
	return getHeight() - getBearingY();
}

int GlyphData::getMaxX() const
{
	return getBearingX() + getWidth();
}

int GlyphData::getMaxY() const
{
	return getBearingY();
}

GlyphData::Format GlyphData::getFormat() const
{
	return format;
}

bool GlyphData::getConstant(const char *in, GlyphData::Format &out)
{
	return formats.find(in, out);
}

bool GlyphData::getConstant(GlyphData::Format in, const char *&out)
{
	return formats.find(in, out);
}

StringMap<GlyphData::Format, GlyphData::FORMAT_MAX_ENUM>::Entry GlyphData::formatEntries[] =
{
	{"luminancealpha", FORMAT_LUMINANCE_ALPHA},
	{"rgba", FORMAT_RGBA},
};

StringMap<GlyphData::Format, GlyphData::FORMAT_MAX_ENUM> GlyphData::formats(GlyphData::formatEntries, sizeof(GlyphData::formatEntries));

} // font
} // love
