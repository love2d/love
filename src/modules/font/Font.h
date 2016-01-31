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

#ifndef LOVE_FONT_FONT_H
#define LOVE_FONT_FONT_H

// LOVE
#include "Rasterizer.h"
#include "TrueTypeRasterizer.h"
#include "image/ImageData.h"
#include "filesystem/FileData.h"
#include "common/Module.h"
#include "common/int.h"

// C++
#include <string>
#include <vector>

namespace love
{
namespace font
{

class Font : public Module
{

public:

	virtual ~Font() {}

	virtual Rasterizer *newRasterizer(love::filesystem::FileData *data) = 0;

	virtual Rasterizer *newTrueTypeRasterizer(int size, TrueTypeRasterizer::Hinting hinting);
	virtual Rasterizer *newTrueTypeRasterizer(love::Data *data, int size, TrueTypeRasterizer::Hinting hinting) = 0;

	virtual Rasterizer *newBMFontRasterizer(love::filesystem::FileData *fontdef, const std::vector<image::ImageData *> &images);

	virtual Rasterizer *newImageRasterizer(love::image::ImageData *data, const std::string &glyphs, int extraspacing);
	virtual Rasterizer *newImageRasterizer(love::image::ImageData *data, uint32 *glyphs, int length, int extraspacing);

	virtual GlyphData *newGlyphData(Rasterizer *r, const std::string &glyph);
	virtual GlyphData *newGlyphData(Rasterizer *r, uint32 glyph);

	// Implement Module.
	virtual ModuleType getModuleType() const { return M_FONT; }
	virtual const char *getName() const = 0;

}; // Font

} // font
} // love

#endif // LOVE_FONT_FONT_H
