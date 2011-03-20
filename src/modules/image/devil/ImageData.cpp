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

// STD
#include <iostream>

// LOVE
#include <common/Exception.h>
#include <filesystem/File.h>

namespace love
{
namespace image
{
namespace devil
{
	void ImageData::load(Data * data)
	{
		// Generate DevIL image.
		ilGenImages(1, &image);

		// Bind the image.
		ilBindImage(image);

		// Try to load the image.
		ILboolean success = ilLoadL(IL_TYPE_UNKNOWN, (void*)data->getData(), data->getSize());

		// Check for errors
		if(!success)
		{
			throw love::Exception("Could not decode image!");
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

	ImageData::ImageData(Data * data)
	{
		load(data);
	}

	ImageData::ImageData(filesystem::File * file)
	{
		Data * data = file->read();
		load(data);
		data->release();
	}

	ImageData::ImageData(int width, int height)
		: width(width), height(height), origin(IL_ORIGIN_UPPER_LEFT), bpp(4)
	{
		// Generate DevIL image.
		ilGenImages(1, &image);

		// Bind the image.
		ilBindImage(image);

		ilTexImage(width, height, 1, bpp, IL_RGBA, IL_UNSIGNED_BYTE, 0);

		// Set to black.
		memset((void*)ilGetData(), 0, width*height*4);
	}

	ImageData::ImageData(int width, int height, void *data)
	: width(width), height(height), origin(IL_ORIGIN_UPPER_LEFT), bpp(4)
	{
		// Generate DevIL image.
		ilGenImages(1, &image);
		// Bind the image.
		ilBindImage(image);
		// Try to load the data.
		bool success = (ilTexImage(width, height, 1, bpp, IL_RGBA, IL_UNSIGNED_BYTE, data) == IL_TRUE);
		int err = ilGetError();
		if (err != IL_NO_ERROR){
			switch (err) {
				case IL_ILLEGAL_OPERATION:
					throw love::Exception("Error: Illegal operation");
					break;
				case IL_INVALID_PARAM:
					throw love::Exception("Error: invalid parameters");
					break;
				case IL_OUT_OF_MEMORY:
					throw love::Exception("Error: out of memory");
					break;
				default:
					throw love::Exception("Error: unknown error");
					break;
			}
		}

		if(!success) {
			throw love::Exception("Could not decode image data.");
		}
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
		//int tx = x > width-1 ? width-1 : x;
		//int ty = y > height-1 ? height-1 : y; // not using these seems to not break anything
		if (x > width-1 || y > height-1 || x < 0 || y < 0) throw love::Exception("Attempt to set out-of-range pixel!");
		pixel * pixels = (pixel *)getData();
		pixels[y*width+x] = c;
	}

	pixel ImageData::getPixel(int x, int y) const
	{
		//int tx = x > width-1 ? width-1 : x;
		//int ty = y > height-1 ? height-1 : y; // not using these seems to not break anything
		if (x > width-1 || y > height-1 || x < 0 || y < 0) throw love::Exception("Attempt to get out-of-range pixel!");
		pixel * pixels = (pixel *)getData();
		return pixels[y*width+x];
	}

	void ImageData::encode(love::filesystem::File * f, ImageData::Format format) {
		ilBindImage(image);
		ILuint ilFormat;
		switch (format) {
			case ImageData::FORMAT_BMP:
				ilFormat = IL_BMP;
				break;
			case ImageData::FORMAT_TGA:
				ilFormat = IL_TGA;
				break;
			case ImageData::FORMAT_GIF:
				ilFormat = IL_GIF;
				break;
			case ImageData::FORMAT_JPG:
				ilFormat = IL_JPG;
				break;
			case ImageData::FORMAT_PNG:
			default: // PNG is the default format
				ilFormat = IL_PNG;
				break;
		}
		ILuint size = ilSaveL(ilFormat, NULL, 0);
		ILubyte * data = new ILubyte[size];
		ilSaveL(ilFormat, data, size);
		f->open(love::filesystem::File::WRITE);
		f->write(data, size);
		f->close();
		delete[] data;
	}

} // devil
} // image
} // love
