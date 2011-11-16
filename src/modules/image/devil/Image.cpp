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

#include "ImageData.h"

// DevIL
#include <IL/il.h>

namespace love
{
namespace image
{
namespace devil
{
	Image::Image()
	{
		ilInit();
		ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
		ilEnable(IL_ORIGIN_SET);
	}

	Image::~Image()
	{
		ilShutDown();
	}

	const char * Image::getName() const
	{
		return "love.image.devil";
	}

	love::image::ImageData * Image::newImageData(love::filesystem::File * file)
	{
		return new ImageData(file);
	}

	love::image::ImageData * Image::newImageData(Data * data)
	{
		try
		{
			return new ImageData(data);
		}
		catch (love::Exception & e)
		{
			throw love::Exception(e.what());
		}

	}

	love::image::ImageData * Image::newImageData(int width, int height)
	{
		return new ImageData(width, height);
	}

	love::image::ImageData * Image::newImageData(int width, int height, void *data)
	{
		return new ImageData(width, height, data);
	}

} // devil
} // image
} // love
