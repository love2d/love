/**
 * Copyright (c) 2006-2015 LOVE Development Team
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
#include "JPEGHandler.h"
#include "common/Exception.h"
#include "common/math.h"

namespace love
{
namespace image
{
namespace magpie
{

JPEGHandler::JPEGHandler()
{
	mutex = love::thread::newMutex();

	decompressor = tjInitDecompress();
	compressor = tjInitCompress();
}

JPEGHandler::~JPEGHandler()
{
	delete mutex;

	if (decompressor)
		tjDestroy(decompressor);

	if (compressor)
		tjDestroy(compressor);
}

bool JPEGHandler::canDecode(love::filesystem::FileData *data)
{
	if (!decompressor)
		return false;

	love::thread::Lock lock(mutex);

	int w, h, subsamp;
	int status = tjDecompressHeader2(decompressor,
	                                 (unsigned char *) data->getData(),
	                                 data->getSize(),
	                                 &w, &h, &subsamp);

	return (status == 0);
}

bool JPEGHandler::canEncode(ImageData::Format format)
{
	if (!compressor)
		return false;

	return format == ImageData::FORMAT_JPG;
}

FormatHandler::DecodedImage JPEGHandler::decode(love::filesystem::FileData *data)
{
	if (!decompressor)
		throw love::Exception("Could not decode jpeg image: %s", tjGetErrorStr());

	DecodedImage img;
	unsigned char *jpegdata = (unsigned char *) data->getData();

	love::thread::Lock lock(mutex);

	int unused;
	int status = tjDecompressHeader2(decompressor,
	                                 jpegdata, data->getSize(),
	                                 &img.width, &img.height,
	                                 &unused);

	if (status < 0)
		throw love::Exception("Could not decode jpeg image: %s", tjGetErrorStr());

	img.size = img.width * img.height * sizeof(pixel);

	try
	{
		img.data = new unsigned char[img.size];
	}
	catch (std::bad_alloc &)
	{
		throw love::Exception("Out of memory.");
	}

	status = tjDecompress2(decompressor,
	                       jpegdata, data->getSize(),
	                       img.data, 0, 0, 0, TJPF_RGBA, 0);

	if (status < 0)
	{
		delete[] img.data;
		throw love::Exception("Could not decode jpeg image: %s", tjGetErrorStr());
	}

	return img;
}

FormatHandler::EncodedImage JPEGHandler::encode(const DecodedImage &img, ImageData::Format format)
{
	if (!canEncode(format))
		throw love::Exception("JPEG encoder cannot encode specified format.");

	EncodedImage encodedimage;

	love::thread::Lock lock(mutex);

	unsigned long tjsize = tjBufSize(img.width, img.height, TJSAMP_444);

	try
	{
		// We want to allocate the memory ourselves instead of letting
		// TurboJPEG do it, so we can safely use delete[] in ImageData.cpp.
		encodedimage.data = new unsigned char[tjsize];
	}
	catch (std::bad_alloc &)
	{
		throw love::Exception("Out of memory.");
	}

	unsigned long jpegSize = tjsize;

	int status = tjCompress2(compressor,
	                         img.data,
	                         img.width, 0, img.height,
	                         TJPF_RGBA,
	                         &encodedimage.data, &jpegSize,
	                         TJSAMP_444, COMPRESS_QUALITY, TJFLAG_NOREALLOC);

	if (status < 0)
	{
		delete[] encodedimage.data;
		throw love::Exception("Could not encode jpeg image: %s", tjGetErrorStr());
	}

	encodedimage.size = jpegSize;

	return encodedimage;
}

} // magpie
} // image
} // love
