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

#include "FormatHandler.h"
#include "DevilHandler.h"

namespace love
{
namespace image
{
namespace magpie
{

ImageData::ImageData(love::filesystem::FileData *data)
{
	decode(data);
}

ImageData::ImageData(int width, int height)
{
	this->width = width;
	this->height = height;
	create(width, height);

	// Set to black/transparency.
	memset(data, 0, width*height*sizeof(pixel));
}

ImageData::ImageData(int width, int height, void *data, bool own)
{
	this->width = width;
	this->height = height;

	if (own)
		this->data = (unsigned char *) data;
	else
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

void ImageData::decode(love::filesystem::FileData *data)
{
	FormatHandler::DecodedImage decodedimage;

	if (DevilHandler::canDecode(data))
		decodedimage = DevilHandler::decode(data);
	else
		throw love::Exception("Could not decode image: unrecognized format.");

	// The decoder *must* output a 32 bits-per-pixel image.
	if (decodedimage.size != decodedimage.width*decodedimage.height*sizeof(pixel))
	{
		delete[] decodedimage.data;
		throw love::Exception("Coult not convert image!");
	}

	if (this->data)
		delete[] this->data;

	this->width = decodedimage.width;
	this->height = decodedimage.height;
	this->data = decodedimage.data;
}

void ImageData::encode(love::filesystem::File *f, ImageData::Format format)
{
	FormatHandler::EncodedImage encodedimage;

	{
		// We only need to lock this mutex when actually encoding the ImageData.
		thread::Lock lock(mutex);

		FormatHandler::DecodedImage rawimage;
		rawimage.width = width;
		rawimage.height = height;
		rawimage.size = width*height*sizeof(pixel);
		rawimage.data = data;

		if (DevilHandler::canEncode(format))
			encodedimage = DevilHandler::encode(rawimage, format);
		else
			throw love::Exception("Image format has no suitable encoder.");
	}

	try
	{
		
		f->open(love::filesystem::File::WRITE);
		f->write(encodedimage.data, encodedimage.size);
		f->close();
	}
	catch (love::Exception &)
	{
		delete[] encodedimage.data;
		throw;
	}

	delete[] encodedimage.data;
}

} // magpie
} // image
} // love
