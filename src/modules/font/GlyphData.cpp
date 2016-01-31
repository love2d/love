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
	{
		switch (f)
		{
		case GlyphData::FORMAT_LUMINANCE_ALPHA:
			data = new unsigned char[metrics.width * metrics.height * 2];
			break;
		case GlyphData::FORMAT_RGBA:
		default:
			data = new unsigned char[metrics.width * metrics.height * 4];
			break;
		}
	}
}

GlyphData::~GlyphData()
{
	delete[] data;
}

void *GlyphData::getData() const
{
	return (void *) data;
}

size_t GlyphData::getSize() const
{
	switch (format)
	{
	case GlyphData::FORMAT_LUMINANCE_ALPHA:
		return size_t(getWidth() * getHeight() * 2);
		break;
	case GlyphData::FORMAT_RGBA:
	default:
		return size_t(getWidth() * getHeight() * 4);
		break;
	}

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
	return this->getBearingX();
}

int GlyphData::getMinY() const
{
	return this->getHeight() - this->getBearingY();
}

int GlyphData::getMaxX() const
{
	return this->getBearingX() + this->getWidth();
}

int GlyphData::getMaxY() const
{
	return this->getBearingY();
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
	{"luminancealpha", GlyphData::FORMAT_LUMINANCE_ALPHA},
	{"rgba", GlyphData::FORMAT_RGBA},
};

StringMap<GlyphData::Format, GlyphData::FORMAT_MAX_ENUM> GlyphData::formats(GlyphData::formatEntries, sizeof(GlyphData::formatEntries));

} // font
} // love
