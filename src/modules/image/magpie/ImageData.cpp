/**
 * Copyright (c) 2006-2016 LOVE Development Team
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

ImageData::ImageData(std::list<FormatHandler *> formatHandlers, love::filesystem::FileData *data)
	: formatHandlers(formatHandlers)
	, decodeHandler(nullptr)
{
	for (FormatHandler *handler : formatHandlers)
		handler->retain();

	decode(data);
}

ImageData::ImageData(std::list<FormatHandler *> formatHandlers, int width, int height, Format format)
	: formatHandlers(formatHandlers)
	, decodeHandler(nullptr)
{
	for (FormatHandler *handler : formatHandlers)
		handler->retain();

	this->width = width;
	this->height = height;
	this->format = format;

	create(width, height, format);

	// Set to black/transparency.
	memset(data, 0, getSize());
}

ImageData::ImageData(std::list<FormatHandler *> formatHandlers, int width, int height, Format format, void *data, bool own)
	: formatHandlers(formatHandlers)
	, decodeHandler(nullptr)
{
	for (FormatHandler *handler : formatHandlers)
		handler->retain();

	this->width = width;
	this->height = height;
	this->format = format;

	if (own)
		this->data = (unsigned char *) data;
	else
		create(width, height, format, data);
}

ImageData::~ImageData()
{
	if (decodeHandler)
		decodeHandler->free(data);
	else
		delete[] data;

	for (FormatHandler *handler : formatHandlers)
		handler->release();
}

void ImageData::create(int width, int height, Format format, void *data)
{
	size_t datasize = width * height * getPixelSize(format);

	try
	{
		this->data = new unsigned char[datasize];
	}
	catch(std::bad_alloc &)
	{
		throw love::Exception("Out of memory");
	}

	if (data)
		memcpy(this->data, data, datasize);

	decodeHandler = nullptr;
	this->format = format;
}

void ImageData::decode(love::filesystem::FileData *data)
{
	FormatHandler *decoder = nullptr;
	FormatHandler::DecodedImage decodedimage;

	for (FormatHandler *handler : formatHandlers)
	{
		if (handler->canDecode(data))
		{
			decoder = handler;
			break;
		}
	}

	if (decoder)
		decodedimage = decoder->decode(data);

	if (decodedimage.data == nullptr)
	{
		const std::string &name = data->getFilename();
		throw love::Exception("Could not decode file '%s' to ImageData: unsupported file format", name.c_str());
	}

	if (decodedimage.size != decodedimage.width * decodedimage.height * getPixelSize(decodedimage.format))
	{
		decoder->free(decodedimage.data);
		throw love::Exception("Could not convert image!");
	}

	// Clean up any old data.
	if (decodeHandler)
		decodeHandler->free(this->data);
	else
		delete[] this->data;

	this->width  = decodedimage.width;
	this->height = decodedimage.height;
	this->data   = decodedimage.data;
	this->format = decodedimage.format;

	decodeHandler = decoder;
}

love::filesystem::FileData *ImageData::encode(EncodedFormat encodedFormat, const char *filename)
{
	FormatHandler *encoder = nullptr;
	FormatHandler::EncodedImage encodedimage;
	FormatHandler::DecodedImage rawimage;

	rawimage.width = width;
	rawimage.height = height;
	rawimage.size = getSize();
	rawimage.data = data;

	for (FormatHandler *handler : formatHandlers)
	{
		if (handler->canEncode(format, encodedFormat))
		{
			encoder = handler;
			break;
		}
	}

	if (encoder != nullptr)
	{
		thread::Lock lock(mutex);
		encodedimage = encoder->encode(rawimage, encodedFormat);
	}

	if (encoder == nullptr || encodedimage.data == nullptr)
	{
		const char *fname = "unknown";
		getConstant(format, fname);
		throw love::Exception("No suitable image encoder for %s format.", fname);
	}

	love::filesystem::FileData *filedata = nullptr;

	try
	{
		filedata = new love::filesystem::FileData(encodedimage.size, filename);
	}
	catch (love::Exception &)
	{
		encoder->free(encodedimage.data);
		throw;
	}

	memcpy(filedata->getData(), encodedimage.data, encodedimage.size);
	encoder->free(encodedimage.data);

	return filedata;
}

} // magpie
} // image
} // love
