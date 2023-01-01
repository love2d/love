/**
 * Copyright (c) 2006-2023 LOVE Development Team
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
#include "Image.h"
#include "common/config.h"

#include "magpie/PNGHandler.h"
#include "magpie/STBHandler.h"
#include "magpie/EXRHandler.h"

#include "magpie/ddsHandler.h"
#include "magpie/PVRHandler.h"
#include "magpie/KTXHandler.h"
#include "magpie/PKMHandler.h"
#include "magpie/ASTCHandler.h"

namespace love
{
namespace image
{

love::Type Image::type("image", &Module::type);

Image::Image()
{
	using namespace magpie;

	float16Init(); // Makes sure half-float conversions can be used.

	formatHandlers = {
		new PNGHandler,
		new STBHandler,
		new EXRHandler,
		new DDSHandler,
		new PVRHandler,
		new KTXHandler,
		new PKMHandler,
		new ASTCHandler,
	};
}

Image::~Image()
{
	// ImageData objects reference the FormatHandlers in our list, so we should
	// release them instead of deleting them completely here.
	for (FormatHandler *handler : formatHandlers)
		handler->release();
}

const char *Image::getName() const
{
	return "love.image.magpie";
}

love::image::ImageData *Image::newImageData(Data *data)
{
	return new ImageData(data);
}

love::image::ImageData *Image::newImageData(int width, int height, PixelFormat format)
{
	return new ImageData(width, height, format);
}

love::image::ImageData *Image::newImageData(int width, int height, PixelFormat format, void *data, bool own)
{
	return new ImageData(width, height, format, data, own);
}

love::image::CompressedImageData *Image::newCompressedData(Data *data)
{
	return new CompressedImageData(formatHandlers, data);
}

bool Image::isCompressed(Data *data)
{
	for (FormatHandler *handler : formatHandlers)
	{
		if (handler->canParseCompressed(data))
			return true;
	}

	return false;
}

const std::list<FormatHandler *> &Image::getFormatHandlers() const
{
	return formatHandlers;
}

ImageData *Image::newPastedImageData(ImageData *src, int sx, int sy, int w, int h)
{
	ImageData *res = newImageData(w, h, src->getFormat());
	try
	{
		res->paste(src, 0, 0, sx, sy, w, h);
	}
	catch (love::Exception &)
	{
		res->release();
		throw;
	}
	return res;
}

std::vector<StrongRef<ImageData>> Image::newCubeFaces(love::image::ImageData *src)
{
	// The faces array is always ordered +x, -x, +y, -y, +z, -z.
	std::vector<StrongRef<ImageData>> faces;

	int totalW = src->getWidth();
	int totalH = src->getHeight();

	if (totalW % 3 == 0 && totalH % 4 == 0 && totalW / 3 == totalH / 4)
	{
		//    +y
		// +z +x -z
		//    -y
		//    -x

		int w = totalW / 3;
		int h = totalH / 4;

		faces.emplace_back(newPastedImageData(src, 1*w, 1*h, w, h), Acquire::NORETAIN);
		faces.emplace_back(newPastedImageData(src, 1*w, 3*h, w, h), Acquire::NORETAIN);
		faces.emplace_back(newPastedImageData(src, 1*w, 0*h, w, h), Acquire::NORETAIN);
		faces.emplace_back(newPastedImageData(src, 1*w, 2*h, w, h), Acquire::NORETAIN);
		faces.emplace_back(newPastedImageData(src, 0*w, 1*h, w, h), Acquire::NORETAIN);
		faces.emplace_back(newPastedImageData(src, 2*w, 1*h, w, h), Acquire::NORETAIN);
	}
	else if (totalW % 4 == 0 && totalH % 3 == 0 && totalW / 4 == totalH / 3)
	{
		//    +y
		// -x +z +x -z
		//    -y

		int w = totalW / 4;
		int h = totalH / 3;

		faces.emplace_back(newPastedImageData(src, 2*w, 1*h, w, h), Acquire::NORETAIN);
		faces.emplace_back(newPastedImageData(src, 0*w, 1*h, w, h), Acquire::NORETAIN);
		faces.emplace_back(newPastedImageData(src, 1*w, 0*h, w, h), Acquire::NORETAIN);
		faces.emplace_back(newPastedImageData(src, 1*w, 2*h, w, h), Acquire::NORETAIN);
		faces.emplace_back(newPastedImageData(src, 1*w, 1*h, w, h), Acquire::NORETAIN);
		faces.emplace_back(newPastedImageData(src, 3*w, 1*h, w, h), Acquire::NORETAIN);
	}
	else if (totalH % 6 == 0 && totalW == totalH / 6)
	{
		// +x
		// -x
		// +y
		// -y
		// +z
		// -z

		int w = totalW;
		int h = totalH / 6;

		for (int i = 0; i < 6; i++)
			faces.emplace_back(newPastedImageData(src, 0, i * h, w, h), Acquire::NORETAIN);
	}
	else if (totalW % 6 == 0 && totalW / 6 == totalH)
	{
		// +x -x +y -y +z -z

		int w = totalW / 6;
		int h = totalH;

		for (int i = 0; i < 6; i++)
			faces.emplace_back(newPastedImageData(src, i * w, 0, w, h), Acquire::NORETAIN);
	}
	else
		throw love::Exception("Unknown cubemap image dimensions!");

	return faces;
}

std::vector<StrongRef<ImageData>> Image::newVolumeLayers(ImageData *src)
{
	std::vector<StrongRef<ImageData>> layers;

	int totalW = src->getWidth();
	int totalH = src->getHeight();

	if (totalW % totalH == 0)
	{
		for (int i = 0; i < totalW / totalH; i++)
			layers.emplace_back(newPastedImageData(src, i * totalH, 0, totalH, totalH), Acquire::NORETAIN);
	}
	else if (totalH % totalW == 0)
	{
		for (int i = 0; i < totalH / totalW; i++)
			layers.emplace_back(newPastedImageData(src, 0, i * totalW, totalW, totalW), Acquire::NORETAIN);
	}
	else
		throw love::Exception("Cannot extract volume layers from source ImageData.");

	return layers;
}

} // image
} // love
