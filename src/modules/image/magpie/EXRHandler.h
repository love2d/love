/**
 * Copyright (c) 2006-2024 LOVE Development Team
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

#pragma once

#include "image/FormatHandler.h"

namespace love
{
namespace image
{
namespace magpie
{

/**
 * Interface between ImageData and TinyEXR library, for decoding exr files.
 **/
class EXRHandler : public FormatHandler
{
public:

	virtual ~EXRHandler() {}

	// Implements FormatHandler.

	bool canDecode(Data *data) override;
	bool canEncode(PixelFormat rawFormat, EncodedFormat encodedFormat) override;

	DecodedImage decode(Data *data) override;
	EncodedImage encode(const DecodedImage &img, EncodedFormat format) override;

	void freeRawPixels(unsigned char *mem) override;
	void freeEncodedImage(unsigned char *mem) override;

}; // EXRHandler

} // magpie
} // image
} // love
