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

#include "Image.h"

namespace love
{
namespace graphics
{
	Image::Filter::Filter() : min(FILTER_LINEAR), mag(FILTER_LINEAR)
	{
	}

	Image::Wrap::Wrap() : s(WRAP_CLAMP), t(WRAP_CLAMP)
	{
	}

	Image::~Image()
	{
	}

	bool Image::getConstant(const char * in, FilterMode & out)
	{
		return filterModes.find(in, out);
	}

	bool Image::getConstant(FilterMode in, const char *& out)
	{
		return filterModes.find(in, out);
	}

	bool Image::getConstant(const char * in, WrapMode & out)
	{
		return wrapModes.find(in, out);
	}

	bool Image::getConstant(WrapMode in, const char *& out)
	{
		return wrapModes.find(in, out);
	}

	StringMap<Image::FilterMode, Image::FILTER_MAX_ENUM>::Entry Image::filterModeEntries[] =
	{
		{ "linear", Image::FILTER_LINEAR },
		{ "nearest", Image::FILTER_NEAREST },
	};

	StringMap<Image::FilterMode, Image::FILTER_MAX_ENUM> Image::filterModes(Image::filterModeEntries, sizeof(Image::filterModeEntries));

	StringMap<Image::WrapMode, Image::WRAP_MAX_ENUM>::Entry Image::wrapModeEntries[] =
	{
		{ "clamp", Image::WRAP_CLAMP },
		{ "repeat", Image::WRAP_REPEAT },
	};

	StringMap<Image::WrapMode, Image::WRAP_MAX_ENUM> Image::wrapModes(Image::wrapModeEntries, sizeof(Image::wrapModeEntries));


} // graphics
} // love
