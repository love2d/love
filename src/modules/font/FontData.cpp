/**
* Copyright (c) 2006-2010 LOVE Development Team
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


#include "FontData.h"

namespace love
{
namespace font
{

	FontData::FontData(Rasterizer * raster)
		: raster(raster)
	{
		data = new GlyphData *[MAX_CHARS];
		for (unsigned int i = 0; i < MAX_CHARS; i++) {
			data[i] = raster->getGlyphData(i);
		}
	}

	FontData::~FontData()
	{
		for (unsigned int i = 0; i < MAX_CHARS; i++) {
			data[i]->release();
		}
		delete[] data;
	}

	void * FontData::getData() const
	{
		return (void *)data;
	}

	int FontData::getSize() const
	{
		return MAX_CHARS;
	}

	GlyphData * FontData::getGlyphData(unsigned short glyph) const
	{
		return data[glyph];
	}

	int FontData::getHeight() const
	{
		return raster->getHeight();
	}

} // font
} // love
