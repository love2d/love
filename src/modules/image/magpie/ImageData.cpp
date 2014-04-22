/**
 * Copyright (c) 2006-2014 LOVE Development Team
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

// LOVE
#include "ImageData.h"

namespace love
{
namespace image
{
namespace magpie
{

ImageData::ImageData(std::list<FormatHandler *> formats, love::filesystem::FileData *data)
	: formatHandlers(formats)
	, decodeHandler(nullptr)
{
	for (auto it = formatHandlers.begin(); it != formatHandlers.end(); ++it)
		(*it)->retain();

	decode(data);
}

ImageData::ImageData(std::list<FormatHandler *> formats, int width, int height)
	: formatHandlers(formats)
	, decodeHandler(nullptr)
{
	for (auto it = formatHandlers.begin(); it != formatHandlers.end(); ++it)
		(*it)->retain();

	this->width = width;
	this->height = height;

	create(width, height);

	// Set to black/transparency.
	memset(data, 0, width*height*sizeof(pixel));
}

ImageData::ImageData(std::list<FormatHandler *> formats, int width, int height, void *data, bool own)
	: formatHandlers(formats)
	, decodeHandler(nullptr)
{
	for (auto it = formatHandlers.begin(); it != formatHandlers.end(); ++it)
		(*it)->retain();

	this->width = width;
	this->height = height;

	if (own)
		this->data = (unsigned char *) data;
	else
		create(width, height, data);
}

ImageData::~ImageData()
{
	if (decodeHandler)
		decodeHandler->free(data);
	else
		delete[] data;

	for (auto it = formatHandlers.begin(); it != formatHandlers.end(); ++it)
		(*it)->release();
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

	decodeHandler = nullptr;
}

void ImageData::decode(love::filesystem::FileData *data)
{
	FormatHandler *handler = nullptr;
	FormatHandler::DecodedImage decodedimage;

	for (auto it = formatHandlers.begin(); it != formatHandlers.end(); ++it)
	{
		if ((*it)->canDecode(data))
		{
			handler = *it;
			break;
		}
	}

	if (handler)
		decodedimage = handler->decode(data);

	if (decodedimage.data == nullptr)
	{
		const char *ext = data->getExtension().c_str();
		throw love::Exception("Could not decode to ImageData: unrecognized format (%s)", ext);
	}

	// The decoder *must* output a 32 bits-per-pixel image.
	if (decodedimage.size != decodedimage.width*decodedimage.height*sizeof(pixel))
	{
		delete[] decodedimage.data;
		throw love::Exception("Could not convert image!");
	}

	// Clean up any old data.
	if (decodeHandler)
		decodeHandler->free(this->data);
	else
		delete[] this->data;

	this->width = decodedimage.width;
	this->height = decodedimage.height;
	this->data = decodedimage.data;

	decodeHandler = handler;
}

void ImageData::encode(love::filesystem::File *f, ImageData::Format format)
{
	FormatHandler *handler = nullptr;
	FormatHandler::EncodedImage encodedimage;

	{
		// We only need to lock this mutex when actually encoding the ImageData.
		thread::Lock lock(mutex);

		FormatHandler::DecodedImage rawimage;
		rawimage.width = width;
		rawimage.height = height;
		rawimage.size = width*height*sizeof(pixel);
		rawimage.data = data;

		for (auto it = formatHandlers.begin(); it != formatHandlers.end(); ++it)
		{
			if ((*it)->canEncode(format))
			{
				handler = *it;
				break;
			}
		}

		if (handler)
			handler->encode(rawimage, format);

		if (encodedimage.data == nullptr)
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
		if (handler)
			handler->free(encodedimage.data);
		else
			delete[] encodedimage.data;

		throw;
	}

	if (handler)
		handler->free(encodedimage.data);
	else
		delete[] encodedimage.data;
}

} // magpie
} // image
} // love
