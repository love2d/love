/**
* Copyright (c) 2006-2009 LOVE Development Team
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

#ifndef LOVE_DEVIL_IMAGE_DATA_H
#define LOVE_DEVIL_IMAGE_DATA_H

// LOVE
#include <filesystem/File.h>
#include <image/ImageData.h>

// DevIL
#include <IL/il.h>

namespace love
{	
namespace image
{
namespace devil
{
	class ImageData : public love::image::ImageData
	{
	private:

		// The width of the image data.
		int width;

		// The height of the image data.
		int height;

		// The origin of the image.
		int origin;

		// The bits per pixel.
		int bpp;

		// DevIL image identifier.
		ILuint image;

		void load(Data * data);

	public:

		ImageData(Data * data);
		ImageData(love::filesystem::File * file);
		ImageData(int width, int height);
		virtual ~ImageData();

		// Implements Data.
		void * getData() const;
		int getSize() const;

		// Implements ImageData.
		int getWidth() const ;
		int getHeight() const ;
		void setPixel(int x, int y, pixel c);
		pixel getPixel(int x, int y) const;

	}; // ImageData

} // devil
} // image
} // love

#endif // LOVE_DEVIL_IMAGE_DATA_H