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

// LOVE
#include "Image.h"

namespace love
{
namespace image
{

love::Type Image::type("image", &Module::type);

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

} // image
} // love
