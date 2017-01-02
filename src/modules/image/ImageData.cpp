/**
 * Copyright (c) 2006-2017 LOVE Development Team
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

using love::thread::Lock;

namespace love
{
namespace image
{

love::Type ImageData::type("ImageData", &Data::type);

ImageData::ImageData()
	: format(PIXELFORMAT_UNKNOWN)
	, width(0)
	, height(0)
	, data(nullptr)
{
}

ImageData::~ImageData()
{
}

size_t ImageData::getSize() const
{
	return size_t(getWidth() * getHeight()) * getPixelSize();
}

void *ImageData::getData() const
{
	return data;
}

bool ImageData::inside(int x, int y) const
{
	return x >= 0 && x < getWidth() && y >= 0 && y < getHeight();
}

PixelFormat ImageData::getFormat() const
{
	return format;
}

int ImageData::getWidth() const
{
	return width;
}

int ImageData::getHeight() const
{
	return height;
}

void ImageData::setPixel(int x, int y, const Pixel &p)
{
	if (!inside(x, y))
		throw love::Exception("Attempt to set out-of-range pixel!");

	size_t pixelsize = getPixelSize();
	unsigned char *pixeldata = data + ((y * width + x) * pixelsize);

	Lock lock(mutex);
	memcpy(pixeldata, &p, pixelsize);
}

void ImageData::getPixel(int x, int y, Pixel &p) const
{
	if (!inside(x, y))
		throw love::Exception("Attempt to get out-of-range pixel!");

	size_t pixelsize = getPixelSize();

	Lock lock(mutex);
	memcpy(&p, data + ((y * width + x) * pixelsize), pixelsize);
}

void ImageData::paste(ImageData *src, int dx, int dy, int sx, int sy, int sw, int sh)
{
	if (getFormat() != src->getFormat())
		throw love::Exception("ImageData formats must match!");

	Lock lock2(src->mutex);
	Lock lock1(mutex);

	uint8 *s = (uint8 *) src->getData();
	uint8 *d = (uint8 *) getData();

	size_t pixelsize = getPixelSize();

	// Check bounds; if the data ends up completely out of bounds, get out early.
	if (sx >= src->getWidth() || sx + sw < 0 || sy >= src->getHeight() || sy + sh < 0
			|| dx >= getWidth() || dx + sw < 0 || dy >= getHeight() || dy + sh < 0)
		return;

	// Normalize values to the inside of both images.
	if (dx < 0)
	{
		sw += dx;
		sx -= dx;
		dx = 0;
	}
	if (dy < 0)
	{
		sh += dy;
		sy -= dy;
		dy = 0;
	}
	if (sx < 0)
	{
		sw += sx;
		dx -= sx;
		sx = 0;
	}
	if (sy < 0)
	{
		sh += sy;
		dy -= sy;
		sy = 0;
	}

	if (dx + sw > getWidth())
		sw = getWidth() - dx;

	if (dy + sh > getHeight())
		sh = getHeight() - dy;

	if (sx + sw > src->getWidth())
		sw = src->getWidth() - sx;

	if (sy + sh > src->getHeight())
		sh = src->getHeight() - sy;

	// If the dimensions match up, copy the entire memory stream in one go
	if (sw == getWidth() && getWidth() == src->getWidth()
		&& sh == getHeight() && getHeight() == src->getHeight())
	{
		memcpy(d, s, pixelsize * sw * sh);
	}
	else if (sw > 0)
	{
		// Otherwise, copy each row individually.
		for (int i = 0; i < sh; i++)
		{
			const uint8 *rowsrc = s + (sx + (i + sy) * src->getWidth()) * pixelsize;
			uint8 *rowdst = d + (dx + (i + dy) * getWidth()) * pixelsize;
			memcpy(rowdst, rowsrc, pixelsize * sw);
		}
	}
}

love::thread::Mutex *ImageData::getMutex() const
{
	return mutex;
}

size_t ImageData::getPixelSize() const
{
	return getPixelFormatSize(format);
}

bool ImageData::validPixelFormat(PixelFormat format)
{
	switch (format)
	{
	case PIXELFORMAT_RGBA8:
	case PIXELFORMAT_RGBA16:
	case PIXELFORMAT_RGBA16F:
	case PIXELFORMAT_RGBA32F:
		return true;
	default:
		return false;
	}
}

bool ImageData::getConstant(const char *in, EncodedFormat &out)
{
	return encodedFormats.find(in, out);
}

bool ImageData::getConstant(EncodedFormat in, const char *&out)
{
	return encodedFormats.find(in, out);
}

StringMap<ImageData::EncodedFormat, ImageData::ENCODED_MAX_ENUM>::Entry ImageData::encodedFormatEntries[] =
{
	{"tga", ENCODED_TGA},
	{"png", ENCODED_PNG},
};

StringMap<ImageData::EncodedFormat, ImageData::ENCODED_MAX_ENUM> ImageData::encodedFormats(ImageData::encodedFormatEntries, sizeof(ImageData::encodedFormatEntries));

} // image
} // love
