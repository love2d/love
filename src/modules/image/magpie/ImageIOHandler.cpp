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

#include "ImageIOHandler.h"

#ifdef LOVE_SUPPORT_IMAGEIO

// LOVE
#include "common/Exception.h"
#include "common/math.h"
#include "filesystem/File.h"

// ImageIO
#include <ImageIO/ImageIO.h>

#if defined(LOVE_IOS)
#include <MobileCoreServices/MobileCoreServices.h>
#elif defined(LOVE_MACOSX)
#include <CoreServices/CoreServices.h>
#endif

// STL
#include <cstring>
#include <iostream>

using love::thread::Lock;

namespace love
{
namespace image
{
namespace magpie
{

static const CFStringRef invalidFormat = CFSTR("");

static CFStringRef getFormatType(ImageData::Format format)
{
	switch (format)
	{
	case ImageData::FORMAT_JPG:
		return kUTTypeJPEG;
	case ImageData::FORMAT_TGA:
		return CFSTR("com.truevision.tga-image");
	case ImageData::FORMAT_PNG:
		return kUTTypePNG;
	default:
		return invalidFormat;
	}
}

ImageIOHandler::ImageIOHandler()
	: mutex(nullptr)
{
	mutex = love::thread::newMutex();
}

ImageIOHandler::~ImageIOHandler()
{
	delete mutex;
}

bool ImageIOHandler::canDecode(love::filesystem::FileData *data)
{
	CFStringRef cfext = CFStringCreateWithCString(nullptr, data->getExtension().c_str(), kCFStringEncodingUTF8);
	CFStringRef UTI = UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension, cfext, nullptr);

	CFRelease(cfext);

	if (!UTI)
		return false;

	CFArrayRef types = CGImageSourceCopyTypeIdentifiers();
	Boolean isdecodable = CFArrayContainsValue(types, CFRangeMake(0, CFArrayGetCount(types)), UTI);

	CFRelease(UTI);
	CFRelease(types);

	return isdecodable;
}

bool ImageIOHandler::canEncode(ImageData::Format format)
{
	CFStringRef cftype = getFormatType(format);
	if (cftype == invalidFormat)
		return false;

	// Is the format supported for writing on this system?
	CFArrayRef types = CGImageDestinationCopyTypeIdentifiers();

	Boolean iswritable = CFArrayContainsValue(types, CFRangeMake(0, CFArrayGetCount(types)), cftype);
	CFRelease(types);

	return iswritable;
}

ImageIOHandler::DecodedImage ImageIOHandler::decode(love::filesystem::FileData *data)
{
	Lock lock(mutex);

	DecodedImage img;

	CFDataRef cfdata = CFDataCreateWithBytesNoCopy(nullptr, (const UInt8 *) data->getData(), data->getSize(), kCFAllocatorNull);
	CGImageSourceRef source = CGImageSourceCreateWithData(cfdata, nullptr);
	CGImageRef image = CGImageSourceCreateImageAtIndex(source, 0, nullptr);

	CFRelease(cfdata);
	CFRelease(source);

	if (!image)
		throw love::Exception("Could not decode image!");

	img.width  = (int) CGImageGetWidth(image);
	img.height = (int) CGImageGetHeight(image);

	CGImageAlphaInfo alphainfo = CGImageGetAlphaInfo(image);

	size_t bpp = CGImageGetBitsPerPixel(image);

	// If the bpp doesn't match, we need to draw the image onto a new 32 bpp canvas
	if (bpp != 32)
	{
		// Create a new bitmap canvas
		CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
		CGContextRef context = CGBitmapContextCreate(nullptr, img.width, img.height, 8, img.width*sizeof(pixel), colorspace, kCGImageAlphaPremultipliedLast);
		CGColorSpaceRelease(colorspace);

		if (!context)
		{
			CGImageRelease(image);
			throw love::Exception("Could not decode image: error converting to 32 bpp!");
		}

		// Draw a black background
		CGContextSetRGBFillColor(context, 0, 0, 0, 1);
		CGContextFillRect(context, CGRectMake(0, 0, img.width, img.height));

		CGContextDrawImage(context, CGRectMake(0, 0, img.width, img.height), image);

		// Replace the old image with the canvas
		CGImageRef contextimage = CGBitmapContextCreateImage(context);
		CGContextRelease(context);

		CGImageRelease(image);
		image = contextimage;

		if (!image)
			throw love::Exception("Could not decode image: error converting to 32 bpp!");
	}

	cfdata = CGDataProviderCopyData(CGImageGetDataProvider(image));
	CGImageRelease(image);

	img.size = CFDataGetLength(cfdata);

	try
	{
		img.data = new unsigned char[img.size];
	}
	catch (std::bad_alloc &)
	{
		CFRelease(cfdata);
		throw love::Exception("Out of memory.");
	}

	CFDataGetBytes(cfdata, CFRangeMake(0, img.size), (UInt8 *) img.data);
	CFRelease(cfdata);

	// ImageIO might try to be "helpful" and premultiply the image's alpha for us,
	// now we have to un-premultiply it ourselves. Thanks...
	if (alphainfo == kCGImageAlphaPremultipliedLast)
	{
		pixel *pixels = (pixel *) img.data;
		for (int i = 0; i < img.width * img.height; i++)
		{
			unsigned char alpha = pixels[i].a;

			if (alpha > 0 && alpha < 255)
			{
				pixels[i].r = ((short) pixels[i].r * 255) / alpha;
				pixels[i].g = ((short) pixels[i].g * 255) / alpha;
				pixels[i].b = ((short) pixels[i].b * 255) / alpha;
			}
		}
	}

	return img;
}

ImageIOHandler::EncodedImage ImageIOHandler::encode(const DecodedImage &img, ImageData::Format format)
{
	Lock lock(mutex);

	EncodedImage encodedimage;

	CFStringRef cftype = getFormatType(format);

	if (cftype == invalidFormat || !canEncode(format))
		throw love::Exception("Could not encode image: format is not supported on this system.");

	CGDataProviderRef provider = CGDataProviderCreateWithData(nullptr, img.data, img.size, nullptr);

	CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
	CGBitmapInfo bitmapinfo = kCGBitmapByteOrderDefault | kCGImageAlphaLast;
	CGColorRenderingIntent intent = kCGRenderingIntentDefault;
	size_t pixsize = sizeof(pixel);

	// Create a CoreGraphics image using the ImageData
	CGImageRef image = CGImageCreate(img.width, img.height, 8, 8*pixsize, img.width*pixsize, space, bitmapinfo, provider, nullptr, false, intent);

	CGColorSpaceRelease(space);
	CGDataProviderRelease(provider);

	CFMutableDataRef encodeddata = CFDataCreateMutable(nullptr, 0);
	if (!encodeddata)
	{
		CFRelease(image);
		throw love::Exception("Could not create image for encoding!");
	}

	// Set up the encoder, using encodeddata as a chunk of memory to store the final result
	CGImageDestinationRef dest = CGImageDestinationCreateWithData(encodeddata, cftype, 1, nullptr);
	CGImageDestinationAddImage(dest, image, nullptr);

	// Encode the CoreGraphics image to the specified format
	bool success = CGImageDestinationFinalize(dest);

	CFRelease(image);
	CFRelease(dest);

	CFIndex encodedsize = CFDataGetLength(encodeddata);

	if (!success || encodedsize <= 0)
	{
		CFRelease(encodeddata);
		throw love::Exception("Could not encode image!");
	}

	encodedimage.size = encodedsize;

	try
	{
		encodedimage.data = new unsigned char[encodedsize];
	}
	catch (std::bad_alloc &)
	{
		CFRelease(encodeddata);
		throw love::Exception("Out of memory");
	}

	CFDataGetBytes(encodeddata, CFRangeMake(0, encodedsize), encodedimage.data);
	CFRelease(encodeddata);

	return encodedimage;
}

} // magpie
} // image
} // love

#endif // LOVE_SUPPORT_IMAGEIO
