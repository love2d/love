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

#ifndef LOVE_IMAGE_IMAGE_H
#define LOVE_IMAGE_IMAGE_H

// LOVE
#include "common/config.h"
#include "common/Module.h"
#include "filesystem/File.h"
#include "ImageData.h"
#include "CompressedImageData.h"

namespace love
{
namespace image
{

/**
 * This module is responsible for decoding files such as PNG, GIF, JPEG
 * into raw pixel data, as well as parsing compressed formats which are designed
 * to be uploaded to the GPU and rendered without being un-compressed.
 * This module does not know how to draw images on screen; only love.graphics
 * knows that.
 **/
class Image : public Module
{
public:

	virtual ~Image() {}

	// Implements Module.
	virtual ModuleType getModuleType() const { return M_IMAGE; }

	/**
	 * Creates new ImageData from FileData.
	 * @param data The FileData containing the encoded image data.
	 * @return The new ImageData.
	 **/
	virtual ImageData *newImageData(love::filesystem::FileData *data) = 0;

	/**
	 * Creates empty ImageData with the given size.
	 * @param width The width of the ImageData.
	 * @param height The height of the ImageData.
	 * @return The new ImageData.
	 **/
	virtual ImageData *newImageData(int width, int height) = 0;

	/**
	 * Creates empty ImageData with the given size.
	 * @param width The width of the ImageData.
	 * @param height The height of the ImageData.
	 * @param data The data to load into the ImageData.
	 * @param own Whether the new ImageData should take ownership of the data or
	 *        copy it.
	 * @return The new ImageData.
	 **/
	virtual ImageData *newImageData(int width, int height, void *data, bool own = false) = 0;

	/**
	 * Creates new CompressedImageData from FileData.
	 * @param data The FileData containing the compressed image data.
	 * @return The new CompressedImageData.
	 **/
	virtual CompressedImageData *newCompressedData(love::filesystem::FileData *data) = 0;

	/**
	 * Determines whether a FileData is Compressed image data or not.
	 * @param data The FileData to test.
	 **/
	virtual bool isCompressed(love::filesystem::FileData *data) = 0;

}; // Image

} // image
} // love

#endif // LOVE_IMAGE_IMAGE_H
