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

#include "TrueTypeRasterizer.h"

namespace love
{
namespace font
{

bool TrueTypeRasterizer::getConstant(const char *in, Hinting &out)
{
	return hintings.find(in, out);
}

bool TrueTypeRasterizer::getConstant(Hinting in, const char *&out)
{
	return hintings.find(in, out);
}

StringMap<TrueTypeRasterizer::Hinting, TrueTypeRasterizer::HINTING_MAX_ENUM>::Entry TrueTypeRasterizer::hintingEntries[] =
{
	{"normal", HINTING_NORMAL},
	{"light", HINTING_LIGHT},
	{"mono", HINTING_MONO},
	{"none", HINTING_NONE},
};

StringMap<TrueTypeRasterizer::Hinting, TrueTypeRasterizer::HINTING_MAX_ENUM> TrueTypeRasterizer::hintings(TrueTypeRasterizer::hintingEntries, sizeof(TrueTypeRasterizer::hintingEntries));

} // font
} // love
