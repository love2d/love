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

#pragma once

// LOVE
#include "image/FormatHandler.h"

// STL
#include <string>

namespace love
{
namespace image
{
namespace magpie
{

/**
 * Interface between CompressedImageData and the ddsparse library.
 **/
class DDSHandler : public FormatHandler
{
public:

	virtual ~DDSHandler() {}

	// Implements FormatHandler.
	bool canDecode(Data *data) override;
	DecodedImage decode(Data *data) override;
	bool canParseCompressed(Data *data) override;
	StrongRef<CompressedMemory> parseCompressed(Data *filedata,
	        std::vector<StrongRef<CompressedSlice>> &images,
	        PixelFormat &format, bool &sRGB) override;

}; // DDSHandler

} // magpie
} // image
} // love
