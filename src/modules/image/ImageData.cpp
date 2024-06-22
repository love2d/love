/**
 * Copyright (c) 2006-2024 LOVE Development Team
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
#include "Image.h"
#include "filesystem/Filesystem.h"

#include <algorithm> // min/max

using love::thread::Lock;

namespace love
{
namespace image
{

love::Type ImageData::type("ImageData", &Data::type);

ImageData::ImageData(Data *data)
	: ImageDataBase(PIXELFORMAT_UNKNOWN, 0, 0)
{
	decode(data);
}

ImageData::ImageData(int width, int height, PixelFormat format)
	: ImageDataBase(format, width, height)
{
	if (!validPixelFormat(format))
		throw love::Exception("ImageData does not support the %s pixel format.", getPixelFormatName(format));

	create(width, height, format);

	// Set to black/transparency.
	memset(data, 0, getSize());
}

ImageData::ImageData(int width, int height, PixelFormat format, void *data, bool own)
	: ImageDataBase(format, width, height)
{
	if (!validPixelFormat(format))
		throw love::Exception("ImageData does not support the %s pixel format.", getPixelFormatName(format));

	if (own)
		this->data = (unsigned char *) data;
	else
		create(width, height, format, data);
}

ImageData::ImageData(const ImageData &c)
	: ImageDataBase(c.format, c.width, c.height)
{
	create(width, height, format, c.getData());
}

ImageData::~ImageData()
{
	if (decodeHandler.get())
		decodeHandler->freeRawPixels(data);
	else
		delete[] data;
}

love::image::ImageData *ImageData::clone() const
{
	return new ImageData(*this);
}

void ImageData::create(int width, int height, PixelFormat format, void *data)
{
	size_t datasize = getPixelFormatSliceSize(format, width, height);

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

	pixelSetFunction = getPixelSetFunction(format);
	pixelGetFunction = getPixelGetFunction(format);
}

void ImageData::decode(Data *data)
{
	FormatHandler *decoder = nullptr;
	FormatHandler::DecodedImage decodedimage;

	auto module = Module::getInstance<Image>(Module::M_IMAGE);

	if (module == nullptr)
		throw love::Exception("love.image must be loaded in order to decode an ImageData.");

	for (FormatHandler *handler : module->getFormatHandlers())
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
		auto filedata = dynamic_cast<filesystem::FileData *>(data);

		if (filedata != nullptr)
		{
			const std::string &name = filedata->getFilename();
			throw love::Exception("Could not decode file '%s' to ImageData: unsupported file format", name.c_str());
		}
		else
			throw love::Exception("Could not decode data to ImageData: unsupported encoded format");
	}

	if (decodedimage.size != getPixelFormatSliceSize(decodedimage.format, decodedimage.width, decodedimage.height))
	{
		decoder->freeRawPixels(decodedimage.data);
		throw love::Exception("Could not convert image!");
	}

	// Clean up any old data.
	if (decodeHandler)
		decodeHandler->freeRawPixels(this->data);
	else
		delete[] this->data;

	// This throws away some information the decoder could give us, but we
	// can't really rely on it I think...
	decodedimage.format = getLinearPixelFormat(decodedimage.format);

	this->width  = decodedimage.width;
	this->height = decodedimage.height;
	this->data   = decodedimage.data;
	this->format = decodedimage.format;

	decodeHandler = decoder;

	pixelSetFunction = getPixelSetFunction(format);
	pixelGetFunction = getPixelGetFunction(format);
}

love::filesystem::FileData *ImageData::encode(FormatHandler::EncodedFormat encodedFormat, const char *filename, bool writefile) const
{
	FormatHandler *encoder = nullptr;
	FormatHandler::EncodedImage encodedimage;
	FormatHandler::DecodedImage rawimage;

	rawimage.width = width;
	rawimage.height = height;
	rawimage.size = getSize();
	rawimage.data = data;
	rawimage.format = format;

	auto module = Module::getInstance<Image>(Module::M_IMAGE);

	if (module == nullptr)
		throw love::Exception("love.image must be loaded in order to encode an ImageData.");

	for (FormatHandler *handler : module->getFormatHandlers())
	{
		if (handler->canEncode(format, encodedFormat))
		{
			encoder = handler;
			break;
		}
	}

	if (encoder != nullptr)
		encodedimage = encoder->encode(rawimage, encodedFormat);

	if (encoder == nullptr || encodedimage.data == nullptr)
		throw love::Exception("No suitable image encoder for the %s pixel format.", getPixelFormatName(format));

	love::filesystem::FileData *filedata = nullptr;

	try
	{
		filedata = new love::filesystem::FileData(encodedimage.size, filename);
	}
	catch (love::Exception &)
	{
		encoder->freeEncodedImage(encodedimage.data);
		throw;
	}

	memcpy(filedata->getData(), encodedimage.data, encodedimage.size);
	encoder->freeEncodedImage(encodedimage.data);

	if (writefile)
	{
		auto fs = Module::getInstance<filesystem::Filesystem>(Module::M_FILESYSTEM);

		if (fs == nullptr)
		{
			filedata->release();
			throw love::Exception("love.filesystem must be loaded in order to write an encoded ImageData to a file.");
		}

		try
		{
			fs->write(filename, filedata->getData(), filedata->getSize());
		}
		catch (love::Exception &)
		{
			filedata->release();
			throw;
		}
	}

	return filedata;
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

static float clamp01(float x)
{
	return std::min(std::max(x, 0.0f), 1.0f);
}

static void setPixelR8(const Colorf &c, ImageData::Pixel *p)
{
	p->rgba8[0] = (uint8) (clamp01(c.r) * 255.0f + 0.5f);
}

static void setPixelRG8(const Colorf &c, ImageData::Pixel *p)
{
	p->rgba8[0] = (uint8) (clamp01(c.r) * 255.0f + 0.5f);
	p->rgba8[1] = (uint8) (clamp01(c.g) * 255.0f + 0.5f);
}

static void setPixelRGBA8(const Colorf &c, ImageData::Pixel *p)
{
	p->rgba8[0] = (uint8) (clamp01(c.r) * 255.0f + 0.5f);
	p->rgba8[1] = (uint8) (clamp01(c.g) * 255.0f + 0.5f);
	p->rgba8[2] = (uint8) (clamp01(c.b) * 255.0f + 0.5f);
	p->rgba8[3] = (uint8) (clamp01(c.a) * 255.0f + 0.5f);
}

static void setPixelR16(const Colorf &c, ImageData::Pixel *p)
{
	p->rgba16[0] = (uint16) (clamp01(c.r) * 65535.0f + 0.5f);
}

static void setPixelRG16(const Colorf &c, ImageData::Pixel *p)
{
	p->rgba16[0] = (uint16) (clamp01(c.r) * 65535.0f + 0.5f);
	p->rgba16[1] = (uint16) (clamp01(c.g) * 65535.0f + 0.5f);
}

static void setPixelRGBA16(const Colorf &c, ImageData::Pixel *p)
{
	p->rgba16[0] = (uint16) (clamp01(c.r) * 65535.0f + 0.5f);
	p->rgba16[1] = (uint16) (clamp01(c.b) * 65535.0f + 0.5f);
	p->rgba16[2] = (uint16) (clamp01(c.g) * 65535.0f + 0.5f);
	p->rgba16[3] = (uint16) (clamp01(c.a) * 65535.0f + 0.5f);
}

static void setPixelR16F(const Colorf &c, ImageData::Pixel *p)
{
	p->rgba16f[0] = float32to16(c.r);
}

static void setPixelRG16F(const Colorf &c, ImageData::Pixel *p)
{
	p->rgba16f[0] = float32to16(c.r);
	p->rgba16f[1] = float32to16(c.g);
}

static void setPixelRGBA16F(const Colorf &c, ImageData::Pixel *p)
{
	p->rgba16f[0] = float32to16(c.r);
	p->rgba16f[1] = float32to16(c.g);
	p->rgba16f[2] = float32to16(c.b);
	p->rgba16f[3] = float32to16(c.a);
}

static void setPixelR32F(const Colorf &c, ImageData::Pixel *p)
{
	p->rgba32f[0] = c.r;
}

static void setPixelRG32F(const Colorf &c, ImageData::Pixel *p)
{
	p->rgba32f[0] = c.r;
	p->rgba32f[1] = c.g;
}

static void setPixelRGBA32F(const Colorf &c, ImageData::Pixel *p)
{
	p->rgba32f[0] = c.r;
	p->rgba32f[1] = c.g;
	p->rgba32f[2] = c.b;
	p->rgba32f[3] = c.a;
}

static void setPixelRGBA4(const Colorf &c, ImageData::Pixel *p)
{
	// LSB->MSB: [a, b, g, r]
	uint16 r = (uint16) (clamp01(c.r) * 0xF + 0.5);
	uint16 g = (uint16) (clamp01(c.g) * 0xF + 0.5);
	uint16 b = (uint16) (clamp01(c.b) * 0xF + 0.5);
	uint16 a = (uint16) (clamp01(c.a) * 0xF + 0.5);
	p->packed16 = (r << 12) | (g << 8) | (b << 4) | (a << 0);
}

static void setPixelRGB5A1(const Colorf &c, ImageData::Pixel *p)
{
	// LSB->MSB: [a, b, g, r]
	uint16 r = (uint16) (clamp01(c.r) * 0x1F + 0.5);
	uint16 g = (uint16) (clamp01(c.g) * 0x1F + 0.5);
	uint16 b = (uint16) (clamp01(c.b) * 0x1F + 0.5);
	uint16 a = (uint16) (clamp01(c.a) * 0x1 + 0.5);
	p->packed16 = (r << 11) | (g << 6) | (b << 1) | (a << 0);
}

static void setPixelRGB565(const Colorf &c, ImageData::Pixel *p)
{
	// LSB->MSB: [b, g, r]
	uint16 r = (uint16) (clamp01(c.r) * 0x1F + 0.5);
	uint16 g = (uint16) (clamp01(c.g) * 0x3F + 0.5);
	uint16 b = (uint16) (clamp01(c.b) * 0x1F + 0.5);
	p->packed16 = (r << 11) | (g << 5) | (b << 0);
}

static void setPixelRGB10A2(const Colorf &c, ImageData::Pixel *p)
{
	// LSB->MSB: [r, g, b, a]
	uint32 r = (uint32) (clamp01(c.r) * 0x3FF + 0.5);
	uint32 g = (uint32) (clamp01(c.g) * 0x3FF + 0.5);
	uint32 b = (uint32) (clamp01(c.b) * 0x3FF + 0.5);
	uint32 a = (uint32) (clamp01(c.a) * 0x3 + 0.5);
	p->packed32 = (r << 0) | (g << 10) | (b << 20) | (a << 30);
}

static void setPixelRG11B10F(const Colorf &c, ImageData::Pixel *p)
{
	// LSB->MSB: [r, g, b]
	float11 r = float32to11(c.r);
	float11 g = float32to11(c.g);
	float10 b = float32to10(c.b);
	p->packed32 = (r << 0) | (g << 11) | (b << 22);
}

static void getPixelR8(const ImageData::Pixel *p, Colorf &c)
{
	c.r = p->rgba8[0] / 255.0f;
	c.g = 0.0f;
	c.b = 0.0f;
	c.a = 1.0f;
}

static void getPixelRG8(const ImageData::Pixel *p, Colorf &c)
{
	c.r = p->rgba8[0] / 255.0f;
	c.g = p->rgba8[1] / 255.0f;
	c.b = 0.0f;
	c.a = 1.0f;
}

static void getPixelRGBA8(const ImageData::Pixel *p, Colorf &c)
{
	c.r = p->rgba8[0] / 255.0f;
	c.g = p->rgba8[1] / 255.0f;
	c.b = p->rgba8[2] / 255.0f;
	c.a = p->rgba8[3] / 255.0f;
}

static void getPixelR16(const ImageData::Pixel *p, Colorf &c)
{
	c.r = p->rgba16[0] / 65535.0f;
	c.g = 0.0f;
	c.b = 0.0f;
	c.a = 1.0f;
}

static void getPixelRG16(const ImageData::Pixel *p, Colorf &c)
{
	c.r = p->rgba16[0] / 65535.0f;
	c.g = p->rgba16[1] / 65535.0f;
	c.b = 0.0f;
	c.a = 1.0f;
}

static void getPixelRGBA16(const ImageData::Pixel *p, Colorf &c)
{
	c.r = p->rgba16[0] / 65535.0f;
	c.g = p->rgba16[1] / 65535.0f;
	c.b = p->rgba16[2] / 65535.0f;
	c.a = p->rgba16[3] / 65535.0f;
}

static void getPixelR16F(const ImageData::Pixel *p, Colorf &c)
{
	c.r = float16to32(p->rgba16f[0]);
	c.g = 0.0f;
	c.b = 0.0f;
	c.a = 1.0f;
}

static void getPixelRG16F(const ImageData::Pixel *p, Colorf &c)
{
	c.r = float16to32(p->rgba16f[0]);
	c.g = float16to32(p->rgba16f[1]);
	c.b = 0.0f;
	c.a = 1.0f;
}

static void getPixelRGBA16F(const ImageData::Pixel *p, Colorf &c)
{
	c.r = float16to32(p->rgba16f[0]);
	c.g = float16to32(p->rgba16f[1]);
	c.b = float16to32(p->rgba16f[2]);
	c.a = float16to32(p->rgba16f[3]);
}

static void getPixelR32F(const ImageData::Pixel *p, Colorf &c)
{
	c.r = p->rgba32f[0];
	c.g = 0.0f;
	c.b = 0.0f;
	c.a = 1.0f;
}

static void getPixelRG32F(const ImageData::Pixel *p, Colorf &c)
{
	c.r = p->rgba32f[0];
	c.g = p->rgba32f[1];
	c.b = 0.0f;
	c.a = 1.0f;
}

static void getPixelRGBA32F(const ImageData::Pixel *p, Colorf &c)
{
	c.r = p->rgba32f[0];
	c.g = p->rgba32f[1];
	c.b = p->rgba32f[2];
	c.a = p->rgba32f[3];
}

static void getPixelRGBA4(const ImageData::Pixel *p, Colorf &c)
{
	// LSB->MSB: [a, b, g, r]
	c.r = ((p->packed16 >> 12) & 0xF) / (float)0xF;
	c.g = ((p->packed16 >>  8) & 0xF) / (float)0xF;
	c.b = ((p->packed16 >>  4) & 0xF) / (float)0xF;
	c.a = ((p->packed16 >>  0) & 0xF) / (float)0xF;
}

static void getPixelRGB5A1(const ImageData::Pixel *p, Colorf &c)
{
	// LSB->MSB: [a, b, g, r]
	c.r = ((p->packed16 >> 11) & 0x1F) / (float)0x1F;
	c.g = ((p->packed16 >>  6) & 0x1F) / (float)0x1F;
	c.b = ((p->packed16 >>  1) & 0x1F) / (float)0x1F;
	c.a = ((p->packed16 >>  0) & 0x1)  / (float)0x1;
}

static void getPixelRGB565(const ImageData::Pixel *p, Colorf &c)
{
	// LSB->MSB: [b, g, r]
	c.r = ((p->packed16 >> 11) & 0x1F) / (float)0x1F;
	c.g = ((p->packed16 >>  5) & 0x3F) / (float)0x3F;
	c.b = ((p->packed16 >>  0) & 0x1F) / (float)0x1F;
	c.a = 1.0f;
}

static void getPixelRGB10A2(const ImageData::Pixel *p, Colorf &c)
{
	// LSB->MSB: [r, g, b, a]
	c.r = ((p->packed32 >>  0) & 0x3FF) / (float)0x3FF;
	c.g = ((p->packed32 >> 10) & 0x3FF) / (float)0x3FF;
	c.b = ((p->packed32 >> 20) & 0x3FF) / (float)0x3FF;
	c.a = ((p->packed32 >> 30) & 0x3)   / (float)0x3;
}

static void getPixelRG11B10F(const ImageData::Pixel *p, Colorf &c)
{
	// LSB->MSB: [r, g, b]
	c.r = float11to32((float11) ((p->packed32 >>  0) & 0x7FF));
	c.g = float11to32((float11) ((p->packed32 >> 11) & 0x7FF));
	c.b = float10to32((float10) ((p->packed32 >> 22) & 0x3FF));
	c.a = 1.0f;
}

void ImageData::setPixel(int x, int y, const Colorf &c)
{
	if (!inside(x, y))
		throw love::Exception("Attempt to set out-of-range pixel!");

	size_t pixelsize = getPixelSize();
	Pixel *p = (Pixel *) (data + ((y * width + x) * pixelsize));

	if (pixelSetFunction == nullptr)
		throw love::Exception("ImageData:setPixel does not currently support the %s pixel format.", getPixelFormatName(format));

	pixelSetFunction(c, p);
}

void ImageData::getPixel(int x, int y, Colorf &c) const
{
	if (!inside(x, y))
		throw love::Exception("Attempt to get out-of-range pixel!");

	size_t pixelsize = getPixelSize();
	const Pixel *p = (const Pixel *) (data + ((y * width + x) * pixelsize));

	if (pixelGetFunction == nullptr)
		throw love::Exception("ImageData:getPixel does not currently support the %s pixel format.", getPixelFormatName(format));

	pixelGetFunction(p, c);
}

Colorf ImageData::getPixel(int x, int y) const
{
	Colorf c;
	getPixel(x, y, c);
	return c;
}

union Row
{
	uint8 *u8;
	uint16 *u16;
	float16 *f16;
	float *f32;
};

static void pasteRGBA8toRGBA16(Row src, Row dst, int w)
{
	for (int i = 0; i < w * 4; i++)
		dst.u16[i] = (uint16) src.u8[i] << 8u;
}

static void pasteRGBA8toRGBA16F(Row src, Row dst, int w)
{
	for (int i = 0; i < w * 4; i++)
		dst.f16[i] = float32to16(src.u8[i] / 255.0f);
}

static void pasteRGBA8toRGBA32F(Row src, Row dst, int w)
{
	for (int i = 0; i < w * 4; i++)
		dst.f32[i] = src.u8[i] / 255.0f;
}

static void pasteRGBA16toRGBA8(Row src, Row dst, int w)
{
	for (int i = 0; i < w * 4; i++)
		dst.u8[i] = src.u16[i] >> 8u;
}

static void pasteRGBA16toRGBA16F(Row src, Row dst, int w)
{
	for (int i = 0; i < w * 4; i++)
		dst.f16[i] = float32to16(src.u16[i] / 65535.0f);
}

static void pasteRGBA16toRGBA32F(Row src, Row dst, int w)
{
	for (int i = 0; i < w * 4; i++)
		dst.f32[i] = src.u16[i] / 65535.0f;
}

static void pasteRGBA16FtoRGBA8(Row src, Row dst, int w)
{
	for (int i = 0; i < w * 4; i++)
		dst.u8[i] = (uint8) (clamp01(float16to32(src.f16[i])) * 255.0f + 0.5f);
}

static void pasteRGBA16FtoRGBA16(Row src, Row dst, int w)
{
	for (int i = 0; i < w * 4; i++)
		dst.u16[i] = (uint16) (clamp01(float16to32(src.f16[i])) * 65535.0f + 0.5f);
}

static void pasteRGBA16FtoRGBA32F(Row src, Row dst, int w)
{
	for (int i = 0; i < w * 4; i++)
		dst.f32[i] = float16to32(src.f16[i]);
}

static void pasteRGBA32FtoRGBA8(Row src, Row dst, int w)
{
	for (int i = 0; i < w * 4; i++)
		dst.u8[i] = (uint8) (clamp01(src.f32[i]) * 255.0f + 0.5f);
}

static void pasteRGBA32FtoRGBA16(Row src, Row dst, int w)
{
	for (int i = 0; i < w * 4; i++)
		dst.u16[i] = (uint16) (clamp01(src.f32[i]) * 65535.0f + 0.5f);
}

static void pasteRGBA32FtoRGBA16F(Row src, Row dst, int w)
{
	for (int i = 0; i < w * 4; i++)
		dst.f16[i] = float32to16(src.f32[i]);
}

void ImageData::paste(ImageData *src, int dx, int dy, int sx, int sy, int sw, int sh)
{
	PixelFormat dstformat = getFormat();
	PixelFormat srcformat = src->getFormat();

	int srcW = src->getWidth();
	int srcH = src->getHeight();
	int dstW = getWidth();
	int dstH = getHeight();

	size_t srcpixelsize = src->getPixelSize();
	size_t dstpixelsize = getPixelSize();

	// Check bounds; if the data ends up completely out of bounds, get out early.
	if (sx >= srcW || sx + sw < 0 || sy >= srcH || sy + sh < 0
			|| dx >= dstW || dx + sw < 0 || dy >= dstH || dy + sh < 0)
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

	if (dx + sw > dstW)
		sw = dstW - dx;

	if (dy + sh > dstH)
		sh = dstH - dy;

	if (sx + sw > srcW)
		sw = srcW - sx;

	if (sy + sh > srcH)
		sh = srcH - sy;

	uint8 *s = (uint8 *) src->getData();
	uint8 *d = (uint8 *) getData();

	auto getfunction = src->pixelGetFunction;
	auto setfunction = pixelSetFunction;

	// If the dimensions match up, copy the entire memory stream in one go
	if (srcformat == dstformat && (sw == dstW && dstW == srcW && sh == dstH && dstH == srcH))
	{
		memcpy(d, s, srcpixelsize * sw * sh);
	}
	else if (sw > 0)
	{
		// Otherwise, copy each row individually.
		for (int i = 0; i < sh; i++)
		{
			Row rowsrc = {s + (sx + (i + sy) * srcW) * srcpixelsize};
			Row rowdst = {d + (dx + (i + dy) * dstW) * dstpixelsize};

			if (srcformat == dstformat)
				memcpy(rowdst.u8, rowsrc.u8, srcpixelsize * sw);

			else if (srcformat == PIXELFORMAT_RGBA8_UNORM && dstformat == PIXELFORMAT_RGBA16_UNORM)
				pasteRGBA8toRGBA16(rowsrc, rowdst, sw);
			else if (srcformat == PIXELFORMAT_RGBA8_UNORM && dstformat == PIXELFORMAT_RGBA16_FLOAT)
				pasteRGBA8toRGBA16F(rowsrc, rowdst, sw);
			else if (srcformat == PIXELFORMAT_RGBA8_UNORM && dstformat == PIXELFORMAT_RGBA32_FLOAT)
				pasteRGBA8toRGBA32F(rowsrc, rowdst, sw);

			else if (srcformat == PIXELFORMAT_RGBA16_UNORM && dstformat == PIXELFORMAT_RGBA8_UNORM)
				pasteRGBA16toRGBA8(rowsrc, rowdst, sw);
			else if (srcformat == PIXELFORMAT_RGBA16_UNORM && dstformat == PIXELFORMAT_RGBA16_FLOAT)
				pasteRGBA16toRGBA16F(rowsrc, rowdst, sw);
			else if (srcformat == PIXELFORMAT_RGBA16_UNORM && dstformat == PIXELFORMAT_RGBA32_FLOAT)
				pasteRGBA16toRGBA32F(rowsrc, rowdst, sw);

			else if (srcformat == PIXELFORMAT_RGBA16_FLOAT && dstformat == PIXELFORMAT_RGBA8_UNORM)
				pasteRGBA16FtoRGBA8(rowsrc, rowdst, sw);
			else if (srcformat == PIXELFORMAT_RGBA16_FLOAT && dstformat == PIXELFORMAT_RGBA16_UNORM)
				pasteRGBA16FtoRGBA16(rowsrc, rowdst, sw);
			else if (srcformat == PIXELFORMAT_RGBA16_FLOAT && dstformat == PIXELFORMAT_RGBA32_FLOAT)
				pasteRGBA16FtoRGBA32F(rowsrc, rowdst, sw);

			else if (srcformat == PIXELFORMAT_RGBA32_FLOAT && dstformat == PIXELFORMAT_RGBA8_UNORM)
				pasteRGBA32FtoRGBA8(rowsrc, rowdst, sw);
			else if (srcformat == PIXELFORMAT_RGBA32_FLOAT && dstformat == PIXELFORMAT_RGBA16_UNORM)
				pasteRGBA32FtoRGBA16(rowsrc, rowdst, sw);
			else if (srcformat == PIXELFORMAT_RGBA32_FLOAT && dstformat == PIXELFORMAT_RGBA16_FLOAT)
				pasteRGBA32FtoRGBA16F(rowsrc, rowdst, sw);

			else if (getfunction != nullptr && setfunction != nullptr)
			{
				// Slow path: convert src -> Colorf -> dst.
				Colorf c;
				for (int x = 0; x < sw; x++)
				{
					auto srcp = (const Pixel *) (rowsrc.u8 + x * srcpixelsize);
					auto dstp = (Pixel *) (rowdst.u8 + x * dstpixelsize);
					getfunction(srcp, c);
					setfunction(c, dstp);
				}
			}
			else if (getfunction == nullptr)
				throw love::Exception("ImageData:paste does not currently support converting from the %s pixel format.", getPixelFormatName(srcformat));
			else
				throw love::Exception("ImageData:paste does not currently support converting to the %s pixel format.", getPixelFormatName(dstformat));
		}
	}
}

size_t ImageData::getPixelSize() const
{
	return getPixelFormatBlockSize(format);
}

bool ImageData::validPixelFormat(PixelFormat format)
{
	return isPixelFormatColor(format) && !isPixelFormatCompressed(format);
}

ImageData::PixelSetFunction ImageData::getPixelSetFunction(PixelFormat format)
{
	switch (format)
	{
		case PIXELFORMAT_R8_UNORM: return setPixelR8;
		case PIXELFORMAT_RG8_UNORM: return setPixelRG8;
		case PIXELFORMAT_RGBA8_UNORM: return setPixelRGBA8;
		case PIXELFORMAT_R16_UNORM: return setPixelR16;
		case PIXELFORMAT_RG16_UNORM: return setPixelRG16;
		case PIXELFORMAT_RGBA16_UNORM: return setPixelRGBA16;
		case PIXELFORMAT_R16_FLOAT: return setPixelR16F;
		case PIXELFORMAT_RG16_FLOAT: return setPixelRG16F;
		case PIXELFORMAT_RGBA16_FLOAT: return setPixelRGBA16F;
		case PIXELFORMAT_R32_FLOAT: return setPixelR32F;
		case PIXELFORMAT_RG32_FLOAT: return setPixelRG32F;
		case PIXELFORMAT_RGBA32_FLOAT: return setPixelRGBA32F;
		case PIXELFORMAT_RGBA4_UNORM: return setPixelRGBA4;
		case PIXELFORMAT_RGB5A1_UNORM: return setPixelRGB5A1;
		case PIXELFORMAT_RGB565_UNORM: return setPixelRGB565;
		case PIXELFORMAT_RGB10A2_UNORM: return setPixelRGB10A2;
		case PIXELFORMAT_RG11B10_FLOAT: return setPixelRG11B10F;
		default: return nullptr;
	}
}

ImageData::PixelGetFunction ImageData::getPixelGetFunction(PixelFormat format)
{
	switch (format)
	{
		case PIXELFORMAT_R8_UNORM: return getPixelR8;
		case PIXELFORMAT_RG8_UNORM: return getPixelRG8;
		case PIXELFORMAT_RGBA8_UNORM: return getPixelRGBA8;
		case PIXELFORMAT_R16_UNORM: return getPixelR16;
		case PIXELFORMAT_RG16_UNORM: return getPixelRG16;
		case PIXELFORMAT_RGBA16_UNORM: return getPixelRGBA16;
		case PIXELFORMAT_R16_FLOAT: return getPixelR16F;
		case PIXELFORMAT_RG16_FLOAT: return getPixelRG16F;
		case PIXELFORMAT_RGBA16_FLOAT: return getPixelRGBA16F;
		case PIXELFORMAT_R32_FLOAT: return getPixelR32F;
		case PIXELFORMAT_RG32_FLOAT: return getPixelRG32F;
		case PIXELFORMAT_RGBA32_FLOAT: return getPixelRGBA32F;
		case PIXELFORMAT_RGBA4_UNORM: return getPixelRGBA4;
		case PIXELFORMAT_RGB5A1_UNORM: return getPixelRGB5A1;
		case PIXELFORMAT_RGB565_UNORM: return getPixelRGB565;
		case PIXELFORMAT_RGB10A2_UNORM: return getPixelRGB10A2;
		case PIXELFORMAT_RG11B10_FLOAT: return getPixelRG11B10F;
		default: return nullptr;
	}
}

bool ImageData::getConstant(const char *in, FormatHandler::EncodedFormat &out)
{
	return encodedFormats.find(in, out);
}

bool ImageData::getConstant(FormatHandler::EncodedFormat in, const char *&out)
{
	return encodedFormats.find(in, out);
}

std::vector<std::string> ImageData::getConstants(FormatHandler::EncodedFormat)
{
	return encodedFormats.getNames();
}

StringMap<FormatHandler::EncodedFormat, FormatHandler::ENCODED_MAX_ENUM>::Entry ImageData::encodedFormatEntries[] =
{
	{"tga", FormatHandler::ENCODED_TGA},
	{"png", FormatHandler::ENCODED_PNG},
	{"exr", FormatHandler::ENCODED_EXR},
};

StringMap<FormatHandler::EncodedFormat, FormatHandler::ENCODED_MAX_ENUM> ImageData::encodedFormats(ImageData::encodedFormatEntries, sizeof(ImageData::encodedFormatEntries));

} // image
} // love
