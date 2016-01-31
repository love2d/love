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

#include "Font.h"

#include "TrueTypeRasterizer.h"
#include "font/BMFontRasterizer.h"

#include <string.h>

namespace love
{
namespace font
{
namespace freetype
{

Font::Font()
{
	if (FT_Init_FreeType(&library))
		throw love::Exception("TrueTypeFont Loading error: FT_Init_FreeType failed");
}

Font::~Font()
{
	FT_Done_FreeType(library);
}

Rasterizer *Font::newRasterizer(love::filesystem::FileData *data)
{
	if (TrueTypeRasterizer::accepts(library, data))
		return newTrueTypeRasterizer(data, 12, TrueTypeRasterizer::HINTING_NORMAL);
	else if (BMFontRasterizer::accepts(data))
		return newBMFontRasterizer(data, {});

	throw love::Exception("Invalid font file: %s", data->getFilename().c_str());
}

Rasterizer *Font::newTrueTypeRasterizer(love::Data *data, int size, TrueTypeRasterizer::Hinting hinting)
{
	return new TrueTypeRasterizer(library, data, size, hinting);
}

const char *Font::getName() const
{
	return "love.font.freetype";
}

} // freetype
} // font
} // love
