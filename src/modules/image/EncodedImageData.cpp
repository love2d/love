/**
 * Copyright (c) 2006-2009 LOVE Development Team
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

#include "EncodedImageData.h"

namespace love
{
namespace image
{
	EncodedImageData::EncodedImageData(void * d, Format f, int s)
		: data(d), format(f), size(s)
	{
	}
	
	void * EncodedImageData::getData() const 
	{
		return data;
	}
	
	EncodedImageData::Format EncodedImageData::getFormat() const
	{
		return format;
	}
	
	int EncodedImageData::getSize() const 
	{
		return size;
	}

	bool EncodedImageData::getConstant(const char * in, EncodedImageData::Format & out)
	{
		return formats.find(in, out);
	}

	bool EncodedImageData::getConstant(EncodedImageData::Format in, const char *& out)
	{
		return formats.find(in, out);
	}

	StringMap<EncodedImageData::Format, EncodedImageData::FORMAT_MAX_ENUM>::Entry EncodedImageData::formatEntries[] = 
	{
		{"tga", EncodedImageData::FORMAT_TGA},
		{"bmp", EncodedImageData::FORMAT_BMP},
	};

	StringMap<EncodedImageData::Format, EncodedImageData::FORMAT_MAX_ENUM> EncodedImageData::formats(EncodedImageData::formatEntries, sizeof(EncodedImageData::formatEntries));

} // image
} // love
