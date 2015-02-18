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

#ifndef LOVE_IMAGE_MAGPIE_JPEG_HANDLER_H
#define LOVE_IMAGE_MAGPIE_JPEG_HANDLER_H

#include "common/config.h"

#ifndef LOVE_NO_TURBOJPEG

// LOVE
#include "FormatHandler.h"
#include "thread/threads.h"

// libjpeg-turbo
#ifdef LOVE_MACOSX_USE_FRAMEWORKS
#include <jpeg-turbo/turbojpeg.h>
#else
#include <turbojpeg.h>
#endif

namespace love
{
namespace image
{
namespace magpie
{

/**
 * Interface between ImageData and TurboJPEG.
 **/
class JPEGHandler : public FormatHandler
{
public:

	// Implements FormatHandler.

	JPEGHandler();
	virtual ~JPEGHandler();

	virtual bool canDecode(love::filesystem::FileData *data);
	virtual bool canEncode(ImageData::Format format);

	virtual DecodedImage decode(love::filesystem::FileData *data);
	virtual EncodedImage encode(const DecodedImage &img, ImageData::Format format);

private:

	Mutex *mutex;

	tjhandle decompressor;
	tjhandle compressor;

	static const int COMPRESS_QUALITY = 90;

}; // JPEGHandler

} // magpie
} // image
} // love

#endif // LOVE_NO_TURBOJPEG

#endif // LOVE_IMAGE_MAGPIE_JPEG_HANDLER_H
