/**
 * Copyright (c) 2006-2013 LOVE Development Team
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

#include "DevilHandler.h"

// LOVE
#include "common/Exception.h"
#include "common/math.h"
#include "thread/threads.h"

// DevIL
#include <IL/il.h>

using love::thread::Lock;

static Mutex *devilMutex = 0;

namespace love
{
namespace image
{
namespace magpie
{

static inline void ilxClearErrors()
{
	while (ilGetError() != IL_NO_ERROR);
}

void DevilHandler::init()
{
	ilInit();
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
}

void DevilHandler::quit()
{
	ilShutDown();
	if (devilMutex)
	{
		delete devilMutex;
		devilMutex = 0;
	}
}

bool DevilHandler::canDecode(love::filesystem::FileData * /*data*/)
{
	// DevIL can decode a lot of formats...
	return true;
}

bool DevilHandler::canEncode(ImageData::Format format)
{
	switch (format)
	{
	case ImageData::FORMAT_BMP:
	case ImageData::FORMAT_TGA:
	case ImageData::FORMAT_JPG:
	case ImageData::FORMAT_PNG:
		return true;
	default:
		return false;
	}

	return false;
}

DevilHandler::DecodedImage DevilHandler::decode(love::filesystem::FileData *data)
{
	if (!devilMutex)
		devilMutex = thread::newMutex();

	Lock lock(devilMutex);

	ILuint image = ilGenImage();
	ilBindImage(image);

	DecodedImage img;

	try
	{
		bool success = ilLoadL(IL_TYPE_UNKNOWN, (void *)data->getData(), (ILuint) data->getSize()) == IL_TRUE;

		if (!success)
			throw love::Exception("Could not decode image!");

		img.width = ilGetInteger(IL_IMAGE_WIDTH);
		img.height = ilGetInteger(IL_IMAGE_HEIGHT);

		// Make sure the image is in RGBA format.
		ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

		// This should always be four.
		int bpp = ilGetInteger(IL_IMAGE_BPP);
		if (bpp != sizeof(pixel))
			throw love::Exception("Could not convert image!");

		img.size = (size_t) ilGetInteger(IL_IMAGE_SIZE_OF_DATA);

		try
		{
			img.data = new ILubyte[img.size];
		}
		catch (std::bad_alloc &)
		{
			throw love::Exception("Out of memory.");
		}

		memcpy(img.data, ilGetData(), img.size);
	}
	catch (std::exception &e)
	{
		// catches love and std exceptions
		ilDeleteImage(image);
		throw love::Exception("%s", e.what());
	}

	ilDeleteImage(image);

	return img;
}

DevilHandler::EncodedImage DevilHandler::encode(const DecodedImage &img, ImageData::Format format)
{
	if (!devilMutex)
		devilMutex = thread::newMutex();

	Lock lock(devilMutex);

	ILuint tempimage = ilGenImage();
	ilBindImage(tempimage);
	ilxClearErrors();

	EncodedImage encodedimage;

	try
	{
		bool success = ilTexImage(img.width, img.height, 1, sizeof(pixel), IL_RGBA, IL_UNSIGNED_BYTE, img.data) ==  IL_TRUE;

		ILenum err = ilGetError();
		ilxClearErrors();

		if (!success)
		{
			if (err != IL_NO_ERROR)
			{
				switch (err)
				{
				case IL_ILLEGAL_OPERATION:
					throw love::Exception("Illegal operation");
				case IL_INVALID_PARAM:
					throw love::Exception("Invalid parameters");
				case IL_OUT_OF_MEMORY:
					throw love::Exception("Out of memory");
				default:
					throw love::Exception("Unknown error (%d)", (int) err);
				}
			}

			throw love::Exception("Could not create image for the encoding!");
		}

		ilRegisterOrigin(IL_ORIGIN_UPPER_LEFT);

		ILuint ilFormat;
		switch (format)
		{
		case ImageData::FORMAT_BMP:
			ilFormat = IL_BMP;
			break;
		case ImageData::FORMAT_TGA:
			ilFormat = IL_TGA;
			break;
		case ImageData::FORMAT_JPG:
			ilFormat = IL_JPG;
			break;
		case ImageData::FORMAT_PNG:
		default: // PNG is the default format
			ilFormat = IL_PNG;
			break;
		}

		encodedimage.size = ilSaveL(ilFormat, NULL, 0);
		if (!encodedimage.size)
			throw love::Exception("Could not encode image!");

		try
		{
			encodedimage.data = new ILubyte[encodedimage.size];
		}
		catch(std::bad_alloc &)
		{
			throw love::Exception("Out of memory");
		}

		ilSaveL(ilFormat, encodedimage.data, encodedimage.size);
	}
	catch (std::exception &e)
	{
		// Catches love and std exceptions.
		ilDeleteImage(tempimage);
		delete[] encodedimage.data;
		encodedimage.data = 0;
		throw love::Exception("%s", e.what());
	}

	ilDeleteImage(tempimage);

	return encodedimage;
}

} // magpie
} // image
} // love
