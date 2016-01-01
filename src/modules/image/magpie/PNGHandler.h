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

#ifndef LOVE_IMAGE_MAGPIE_PNG_HANDLER_H
#define LOVE_IMAGE_MAGPIE_PNG_HANDLER_H

// LOVE
#include "FormatHandler.h"

namespace love
{
namespace image
{
namespace magpie
{

/**
 * Interface between ImageData and LodePNG.
 **/
class PNGHandler : public FormatHandler
{
public:

	// Implements FormatHandler.

	virtual bool canDecode(love::filesystem::FileData *data);
	virtual bool canEncode(ImageData::EncodedFormat format);

	virtual DecodedImage decode(love::filesystem::FileData *data);
	virtual EncodedImage encode(const DecodedImage &img, ImageData::EncodedFormat format);

	virtual void free(unsigned char *mem);

}; // PNGHandler

} // magpie
} // image
} // love

#endif // LOVE_IMAGE_MAGPIE_PNG_HANDLER_H
