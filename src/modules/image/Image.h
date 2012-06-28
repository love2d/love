/**
 * Copyright (c) 2006-2012 LOVE Development Team
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

#ifndef LOVE_IMAGE_IMAGE_H
#define LOVE_IMAGE_IMAGE_H

// LOVE
#include "common/config.h"
#include "common/Module.h"
#include "filesystem/File.h"
#include "ImageData.h"

namespace love
{
namespace image
{

/**
 * This module is responsible for decoding files such as PNG, GIF, JPEG
 * into raw pixel data. This module does not know how to draw images on
 * screen; only love.graphics knows that.
 **/
class Image : public Module
{
public:

	/**
	 * Destructor.
	 **/
	virtual ~Image() {};

	/**
	 * Creates new ImageData from a file.
	 * @param file The file containing the encoded image data.
	 * @return The new ImageData.
	 **/
	virtual ImageData *newImageData(love::filesystem::File *file) = 0;

	/**
	 * Creates new ImageData from a raw Data.
	 * @param data The object containing encoded pixel data.
	 * @return The new ImageData.
	 **/
	virtual ImageData *newImageData(Data *data) = 0;

	/**
	 * Creates empty ImageData with the given size.
	 * @param The width of the ImageData.
	 * @param The height of the ImageData.
	 * @return The new ImageData.
	 **/
	virtual ImageData *newImageData(int width, int height) = 0;

	/**
	 * Creates empty ImageData with the given size.
	 * @param The width of the ImageData.
	 * @param The height of the ImageData.
	 * @param The data to load into the ImageData.
	 * @return The new ImageData.
	 **/
	virtual ImageData *newImageData(int width, int height, void *data) = 0;

}; // Image

} // image
} // love

#endif // LOVE_IMAGE_IMAGE_H