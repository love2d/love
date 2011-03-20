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

#include "ImageData.h"

namespace love
{
namespace image
{
	void ImageData::paste(ImageData * src, int dx, int dy, int sx, int sy, int sw, int sh)
	{
		pixel * s = (pixel *)src->getData();
		pixel * d = (pixel *)getData();

		for(int i = 0; i < sh; i++)
		{
			for(int j = 0; j < sw; j++)
			{
				if(inside(dx+j, dy+i) && src->inside(sx+j, sy+i))
				{
					d[(dy+i)*getWidth() + (dx+j)] = s[(sy+i)*src->getWidth() + (sx+j)];
				}
			}
		}
	}

	bool ImageData::inside(int x, int y) const
	{
		return (x >= 0 && x < getWidth() && y >= 0 && y < getHeight());
	}
	
	bool ImageData::getConstant(const char * in, ImageData::Format & out)
	{
		return formats.find(in, out);
	}
	
	bool ImageData::getConstant(ImageData::Format in, const char *& out)
	{
		return formats.find(in, out);
	}
	
	StringMap<ImageData::Format, ImageData::FORMAT_MAX_ENUM>::Entry ImageData::formatEntries[] =
	{
		{"tga", ImageData::FORMAT_TGA},
		{"bmp", ImageData::FORMAT_BMP},
		{"gif", ImageData::FORMAT_GIF},
		{"jpg", ImageData::FORMAT_JPG},
		{"png", ImageData::FORMAT_PNG},
	};
	
	StringMap<ImageData::Format, ImageData::FORMAT_MAX_ENUM> ImageData::formats(ImageData::formatEntries, sizeof(ImageData::formatEntries));

} // image
} // love
