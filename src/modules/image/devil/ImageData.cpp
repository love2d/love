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

#include "ImageData.h"

// STD
#include <iostream>

// LOVE
#include <common/Exception.h>

namespace love
{
namespace image
{
namespace devil
{
	void ImageData::load(Data * data)
	{
		// Generate DevIL image.
		ilGenImages(1, &image);

		// Bind the image.
		ilBindImage(image);

		// Try to load the image.
		ILboolean success = ilLoadL(IL_TYPE_UNKNOWN, (void*)data->getData(), data->getSize());

		// Check for errors
		if(!success)
		{
			throw love::Exception("Could not decode image!");
			return;
		}

		width = ilGetInteger(IL_IMAGE_WIDTH);
		height = ilGetInteger(IL_IMAGE_HEIGHT);
		origin = ilGetInteger(IL_IMAGE_ORIGIN);

		// Make sure the image is in RGBA format.
		ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

		// This should always be four.
		bpp = ilGetInteger(IL_IMAGE_BPP);

		if(bpp != 4)
		{
			std::cerr << "Bits per pixel != 4" << std::endl;
			return;
		}
	}

	ImageData::ImageData(Data * data)
	{
		load(data);
	}

	ImageData::ImageData(filesystem::File * file)
	{
		Data * data = file->read();
		load(data);
		data->release();
	}

	ImageData::ImageData(int width, int height)
		: width(width), height(height), origin(IL_ORIGIN_UPPER_LEFT), bpp(4)
	{
		// Generate DevIL image.
		ilGenImages(1, &image);

		// Bind the image.
		ilBindImage(image);	

		ilTexImage(width, height, 1, bpp, IL_RGBA, IL_UNSIGNED_BYTE, 0);
	}
	
	ImageData::ImageData(int width, int height, void *data)
	: width(width), height(height), origin(IL_ORIGIN_UPPER_LEFT), bpp(4)
	{
		// Generate DevIL image.
		ilGenImages(1, &image);
		// Bind the image.
		ilBindImage(image);
		// Try to load the data.
		bool success = (ilTexImage(width, height, 1, bpp, IL_RGBA, IL_UNSIGNED_BYTE, data) == IL_TRUE);
		int err = ilGetError();
		if (err != IL_NO_ERROR){
			switch (err) {
				case IL_ILLEGAL_OPERATION:
					throw love::Exception("Error: Illegal operation");
					break;
				case IL_INVALID_PARAM:
					throw love::Exception("Error: invalid parameters");
					break;
				case IL_OUT_OF_MEMORY:
					throw love::Exception("Error: out of memory");
					break;
				default:
					throw love::Exception("Error: unknown error");
					break;
			}
		}
		
		if(!success) {
			throw love::Exception("Could not decode image data.");
		}
	}

	ImageData::~ImageData()
	{
		ilDeleteImages(1, &image);
	}

	int ImageData::getWidth() const 
	{
		return width;
	}

	int ImageData::getHeight() const 
	{
		return height;
	}

	void * ImageData::getData() const
	{
		ilBindImage(image);
		return ilGetData();
	}

	int ImageData::getSize() const
	{
		return width*height*bpp;
	}

	void ImageData::setPixel(int x, int y, pixel c)
	{
		int tx = x > width-1 ? width-1 : x;
		int ty = y > height-1 ? height-1 : y;
		pixel * pixels = (pixel *)getData();
		pixels[y*width+x] = c;
	}

	pixel ImageData::getPixel(int x, int y) const
	{
		int tx = x > width-1 ? width-1 : x;
		int ty = y > height-1 ? height-1 : y;
		pixel * pixels = (pixel *)getData();
		return pixels[y*width+x];
	}
	
	love::image::EncodedImageData * ImageData::encodeImageData(love::image::ImageData * d, love::image::Image::ImageFormat f) {
		ILubyte * data;
		ILuint w = d->getWidth();
		ILuint h = d->getHeight();
		int headerLen, bpp;
		switch (f) {
			case Image::FORMAT_TGA: // MSVC issues warning when there's no case statement.
			default: // since we only support one format
				headerLen = 18;
				bpp = 3;
				int size = h * w * bpp;
				data = new ILubyte[size + headerLen];
				// here's the header for the Targa file format.
				data[0] = 0; // ID field size
				data[1] = 0; // colormap type
				data[2] = 2; // image type
				data[3] = data[4] = 0; // colormap start
				data[5] = data[6] = 0; // colormap length
				data[7] = 32; // colormap bits
				data[8] = data[9] = 0; // x origin
				data[10] = data[11] = 0; // y origin
				// Targa is little endian, so:
				data[12] = w & 255; // least significant byte of width
				data[13] = w >> 8; // most significant byte of width
				data[14] = h & 255; // least significant byte of height
				data[15] = h >> 8; // most significant byte of height
				data[16] = bpp * 8; // bits per pixel
				data[17] = 0; // descriptor bits
				// header done. write the pixel data to TGA:
				data += headerLen;
				d->getData(); // bind the imagedata's image
				ilCopyPixels(0,0,0,w,h,1,IL_BGR,IL_UNSIGNED_BYTE,data); // convert the pixels to BGR (remember, little-endian) and copy them to data
				
				// It's Targa, so we have to flip the image.
				int row = w * bpp;
				ILubyte * temp = new ILubyte[row];
				ILubyte * src = data - row;
				ILubyte * dst = data + size;
				for (unsigned i = 0; i < (h >> 1); i++) {
					memcpy(temp,src+=row,row);
					memcpy(src,dst-=row,row);
					memcpy(dst,temp,row);
				}
				data -= headerLen;
				delete [] temp;
				return new love::image::EncodedImageData(data, f, size + headerLen);
		}
	}

} // devil
} // image
} // love
