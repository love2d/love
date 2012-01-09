/**
* Copyright (c) 2006-2012 LOVE Development Team
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
#include <cstring>
#include <iostream>

// LOVE
#include <common/Exception.h>
#include <common/math.h>
#include <filesystem/File.h>

using love::thread::Lock;

static Mutex devilMutex;

namespace love
{
namespace image
{
namespace devil
{
	void ImageData::create(int width, int height, void * data)
	{
		Lock lock(devilMutex); //automatically lock and unlock
		ILuint image;

		//create the image
		ilGenImages(1, &image);

		//bind it
		ilBindImage(image);

		while (ilGetError() != IL_NO_ERROR);

		//create and populate the image
		bool success = (ilTexImage(width, height, 1, bpp, IL_RGBA, IL_UNSIGNED_BYTE, data) == IL_TRUE);

		ILenum err = ilGetError();
		while (ilGetError() != IL_NO_ERROR);

		if (!success)
		{
			ilDeleteImages(1, &image);

			if (err != IL_NO_ERROR)
			{
				switch (err)
				{
					case IL_ILLEGAL_OPERATION:
						throw love::Exception("Illegal operation");
					case IL_INVALID_PARAM:
						throw love::Exception("Invalid parameters");
					case IL_OUT_OF_MEMORY:
						throw love::Exception("Out of memory");
					default:
						throw love::Exception("Unknown error (%d)", (int) err);
				}
			}

			throw love::Exception("Could not decode image data.");
		}

		try
		{
			this->data = new unsigned char[width*height*bpp];
		}
		catch (std::bad_alloc)
		{
			ilDeleteImages(1, &image);
			throw love::Exception("Out of memory");
		}

		memcpy(this->data, ilGetData(), width*height*bpp);

		ilDeleteImages(1, &image);
	}

	void ImageData::load(Data * data)
	{
		Lock lock(devilMutex);
		ILuint image;

		// Generate DevIL image.
		ilGenImages(1, &image);

		// Bind the image.
		ilBindImage(image);

		// Try to load the image.
		ILboolean success = ilLoadL(IL_TYPE_UNKNOWN, (void*)data->getData(), data->getSize());

		// Check for errors
		if (!success)
		{
			throw love::Exception("Could not decode image!");
		}

		width = ilGetInteger(IL_IMAGE_WIDTH);
		height = ilGetInteger(IL_IMAGE_HEIGHT);
		origin = ilGetInteger(IL_IMAGE_ORIGIN);

		// Make sure the image is in RGBA format.
		ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

		// This should always be four.
		bpp = ilGetInteger(IL_IMAGE_BPP);

		if (bpp != 4)
		{
			ilDeleteImages(1, &image);
			std::cerr << "Bits per pixel != 4" << std::endl;
			return;
		}

		try
		{
			this->data = new unsigned char[width*height*bpp];
		}
		catch (std::bad_alloc)
		{
			ilDeleteImages(1, &image);
			throw love::Exception("Out of memory");
		}

		memcpy(this->data, ilGetData(), width*height*bpp);

		ilDeleteImages(1, &image);
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
		create(width, height);

		// Set to black.
		memset(data, 0, width*height*4);
	}

	ImageData::ImageData(int width, int height, void *data)
	: width(width), height(height), origin(IL_ORIGIN_UPPER_LEFT), bpp(4)
	{
		create(width, height, data);
	}

	ImageData::~ImageData()
	{
		delete[] data;
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
		return data;
	}

	int ImageData::getSize() const
	{
		return width*height*bpp;
	}

	void ImageData::setPixel(int x, int y, pixel c)
	{
		Lock lock(mutex);
		//int tx = x > width-1 ? width-1 : x;
		//int ty = y > height-1 ? height-1 : y; // not using these seems to not break anything
		if (x > width-1 || y > height-1 || x < 0 || y < 0)
			throw love::Exception("Attempt to set out-of-range pixel!");

		pixel * pixels = (pixel *)getData();
		pixels[y*width+x] = c;
	}

	pixel ImageData::getPixel(int x, int y)
	{
		Lock lock(mutex);
		//int tx = x > width-1 ? width-1 : x;
		//int ty = y > height-1 ? height-1 : y; // not using these seems to not break anything
		if (x > width-1 || y > height-1 || x < 0 || y < 0)
			throw love::Exception("Attempt to get out-of-range pixel!");

		pixel * pixels = (pixel *)getData();
		return pixels[y*width+x];
	}

	void ImageData::encode(love::filesystem::File * f, ImageData::Format format) {
		Lock lock(devilMutex);
		Lock lock2(mutex);

		ILuint tempimage;
		ilGenImages(1, &tempimage);
		ilBindImage(tempimage);

		while (ilGetError() != IL_NO_ERROR);

		bool success = ilTexImage(width, height, 1, bpp, IL_RGBA, IL_UNSIGNED_BYTE, this->data) == IL_TRUE;

		ILenum err = ilGetError();
		while (ilGetError() != IL_NO_ERROR);

		if (!success)
		{
			ilDeleteImages(1, &tempimage);

			if (err != IL_NO_ERROR)
			{
				switch (err)
				{
					case IL_ILLEGAL_OPERATION:
						throw love::Exception("Illegal operation");
					case IL_INVALID_PARAM:
						throw love::Exception("Invalid parameters");
					case IL_OUT_OF_MEMORY:
						throw love::Exception("Out of memory");
					default:
						throw love::Exception("Unknown error (%d)", (int) err);
				}
			}

			throw love::Exception("Could not create image for the encoding!");
		}

		ilRegisterOrigin(IL_ORIGIN_UPPER_LEFT);

		ILuint ilFormat;
		switch (format)
		{
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
		if (!size)
		{
			ilDeleteImages(1, &tempimage);
			throw love::Exception("Could not encode image!");
		}

		ILubyte * encoded_data;
		try
		{
			encoded_data = new ILubyte[size];
		}
		catch (std::bad_alloc)
		{
			ilDeleteImages(1, &tempimage);
			throw love::Exception("Out of memory");
		}

		ilSaveL(ilFormat, encoded_data, size);
		ilDeleteImages(1, &tempimage);

		f->open(love::filesystem::File::WRITE);
		f->write(encoded_data, size);
		f->close();

		delete[] encoded_data;
	}

} // devil
} // image
} // love
