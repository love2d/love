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

#include "ImageData.h"

// STD
#include <iostream>

namespace love
{
namespace image
{
namespace devil
{
	ImageData::ImageData(filesystem::File * file)
	{
		// Read the data.
		Data * data = file->read();

		// Generate DevIL image.
		ilGenImages(1, &image);

		// Bind the image.
		ilBindImage(image);

		// Try to load the image.
		ILboolean success = ilLoadL(IL_TYPE_UNKNOWN, (void*)data->getData(), data->getSize());

		// Free local image data.
		data->release();

		// Check for errors
		if(!success)
		{
			std::cerr << "Could not decode image." << std::endl;
			return;
		}

		width = ilGetInteger(IL_IMAGE_WIDTH);
		height = ilGetInteger(IL_IMAGE_HEIGHT);
		origin = ilGetInteger(IL_IMAGE_ORIGIN);

		// Make sure the image is in RGBA format.
		ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

		// This should always be four.
		bpp = ilGetInteger(IL_IMAGE_BPP);

		if(bpp != 4)
		{
			std::cerr << "Bits per pixel != 4" << std::endl;
			return;
		}
	}

	ImageData::ImageData(int width, int height)
		: width(width), height(height), origin(IL_ORIGIN_UPPER_LEFT), bpp(4)
	{
		// Generate DevIL image.
		ilGenImages(1, &image);

		// Bind the image.
		ilBindImage(image);	

		ilTexImage(width, height, 0, bpp, IL_RGBA, IL_UNSIGNED_BYTE, 0);
	}

	ImageData::~ImageData()
	{
		ilDeleteImages(1, &image);
	}

	int ImageData::getWidth() const 
	{
		return width;
	}

	int ImageData::getHeight() const 
	{
		return height;
	}

	void * ImageData::getData() const
	{
		ilBindImage(image);
		return ilGetData();
	}

	int ImageData::getSize() const
	{
		return width*height*bpp;
	}

	void ImageData::setPixel(int x, int y, pixel c)
	{
		int tx = x > width-1 ? width-1 : x;
		int ty = y > height-1 ? height-1 : y;
		pixel * pixels = (pixel *)getData();
		pixels[y*width+x] = c;
	}

	pixel ImageData::getPixel(int x, int y) const
	{
		int tx = x > width-1 ? width-1 : x;
		int ty = y > height-1 ? height-1 : y;
		pixel * pixels = (pixel *)getData();
		return pixels[y*width+x];
	}

} // devil
} // image
} // love
