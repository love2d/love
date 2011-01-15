/**
* Copyright (c) 2006-2011 LOVE Development Team
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

#include "GlyphData.h"

#include <iostream>

namespace love
{
namespace font
{

	GlyphData::GlyphData(unsigned short glyph, GlyphMetrics glyphMetrics, GlyphData::Format f)
		: glyph(glyph), metrics(glyphMetrics), data(0), format(f), padded(false)
	{
		if (metrics.width && metrics.height) {
			switch (f) {
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

	void * GlyphData::getData() const
	{
		return (void *) data;
	}

	int GlyphData::getSize() const
	{
		switch(format) {
			case GlyphData::FORMAT_LUMINANCE_ALPHA:
				return getWidth() * getHeight() * 2;
				break;
			case GlyphData::FORMAT_RGBA:
			default:
				return getWidth() * getHeight() * 4;
				break;
		}

	}

	int GlyphData::getHeight() const
	{
		return (padded ? getPaddedHeight() : metrics.height);
	}

	int GlyphData::getWidth() const
	{
		return (padded ? getPaddedWidth() : metrics.width);
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

	void GlyphData::pad()
	{
		if (data == 0)
			return;
		int w = getWidth();
		int h = getHeight();
		int pw = next_p2(w);
		int ph = next_p2(h);
		unsigned char * d = new unsigned char[pw * ph * (format == GlyphData::FORMAT_LUMINANCE_ALPHA ? 2 : 4)];
		for (int j = 0; j < ph; j++) {
			for (int i = 0; i < pw; i++) {
				int n = i+j*w;
				int p = i+j*pw;
				if (i < w && j < h) {
					if (format == GlyphData::FORMAT_LUMINANCE_ALPHA) {
						p *= 2;
						n *= 2;
						d[p] = data[n];
						d[p+1] = data[n+1];
					} else {
						p *= 4;
						n *= 4;
						d[p] = data[n];
						d[p+1] = data[n+1];
						d[p+2] = data[n+2];
						d[p+3] = data[n+3];
					}
				} else {
					if (format == GlyphData::FORMAT_LUMINANCE_ALPHA) {
						p *= 2;
						d[p] = d[p+1] = 0;
					} else {
						p *= 4;
						d[p] = d[p+1] = d[p+2] = d[p+3] = 0;
					}
				}
			}
		}
		delete[] data;
		data = d;
		padded = true;
	}

	bool GlyphData::isPadded() const
	{
		return padded;
	}

	int GlyphData::getPaddedWidth() const
	{
		return next_p2(metrics.width);
	}

	int GlyphData::getPaddedHeight() const
	{
		return next_p2(metrics.height);
	}

	inline int GlyphData::next_p2(int num) const
	{
		int powered = 2;
		while(powered < num) powered <<= 1;
		return powered;
	}

} // font
} // love
