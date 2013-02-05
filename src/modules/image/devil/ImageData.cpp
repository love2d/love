/**
 * Copyright (c) 2006-2013 LOVE Development Team
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
#include "common/Exception.h"
#include "common/math.h"
#include "filesystem/File.h"

using love::thread::Lock;

static Mutex devilMutex;

namespace love
{
namespace image
{
namespace devil
{

static inline void ilxClearErrors()
{
	while (ilGetError() != IL_NO_ERROR);
}

ImageData::ImageData(Data *data)
{
	load(data);
}

ImageData::ImageData(filesystem::File *file)
{
	Data *data = file->read();
	load(data);
	data->release();
}

ImageData::ImageData(int width, int height)
{
	this->width = width;
	this->height = height;
	create(width, height);

	// Set to black.
	memset(data, 0, width*height*4);
}

ImageData::ImageData(int width, int height, void *data)
{
	this->width = width;
	this->height = height;
	create(width, height, data);
}

ImageData::~ImageData()
{
	delete[] data;
}

void ImageData::create(int width, int height, void *data)
{
	try
	{
		this->data = new unsigned char[width*height*sizeof(pixel)];
	}
	catch(std::bad_alloc &)
	{
		throw love::Exception("Out of memory");
	}

	if (data)
		memcpy(this->data, data, width*height*sizeof(pixel));
}

void ImageData::load(Data *data)
{
	Lock lock(devilMutex);
	ILuint image;
	ilGenImages(1, &image);
	ilBindImage(image);

	try
	{
		bool success = IL_TRUE == ilLoadL(IL_TYPE_UNKNOWN, (void *)data->getData(), data->getSize());

		if (!success)
			throw love::Exception("Could not decode image!");

		width = ilGetInteger(IL_IMAGE_WIDTH);
		height = ilGetInteger(IL_IMAGE_HEIGHT);

		// Make sure the image is in RGBA format.
		ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

		// This should always be four.
		int bpp = ilGetInteger(IL_IMAGE_BPP);
		if (bpp != sizeof(pixel))
			throw love::Exception("Could not convert image!");

		create(width, height, ilGetData());
	}
	catch(std::exception &)
	{
		ilDeleteImages(1, &image);
		throw;
	}

	ilDeleteImages(1, &image);
}

void ImageData::encode(love::filesystem::File *f, ImageData::Format format)
{
	Lock lock1(devilMutex);
	Lock lock2(mutex);

	ILuint tempimage;
	ilGenImages(1, &tempimage);
	ilBindImage(tempimage);
	ilxClearErrors();

	ILubyte *encoded_data = NULL;

	try
	{
		bool success = IL_TRUE == ilTexImage(width, height, 1, sizeof(pixel), IL_RGBA, IL_UNSIGNED_BYTE, this->data);

		ILenum err = ilGetError();
		ilxClearErrors();

		if (!success)
		{
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
			throw love::Exception("Could not encode image!");

		try
		{
			encoded_data = new ILubyte[size];
		}
		catch(std::bad_alloc &)
		{
			throw love::Exception("Out of memory");
		}

		ilSaveL(ilFormat, encoded_data, size);

		f->open(love::filesystem::File::WRITE);
		f->write(encoded_data, size);
		f->close();
	}
	catch(std::exception &)
	{
		ilDeleteImages(1, &tempimage);
		delete[] encoded_data;
		throw;
	}

	ilDeleteImages(1, &tempimage);
	delete[] encoded_data;
}

} // devil
} // image
} // love
