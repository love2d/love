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

// LOVE
#include "FormatHandler.h"
#include "common/Exception.h"

namespace love
{
namespace image
{
namespace magpie
{

FormatHandler::FormatHandler()
{
}

FormatHandler::~FormatHandler()
{
}

bool FormatHandler::canDecode(love::filesystem::FileData* /*data*/)
{
	return false;
}

bool FormatHandler::canEncode(ImageData::EncodedFormat /*format*/)
{
	return false;
}

FormatHandler::DecodedImage FormatHandler::decode(love::filesystem::FileData* /*data*/)
{
	throw love::Exception("Image decoding is not implemented for this format backend.");
}

FormatHandler::EncodedImage FormatHandler::encode(const DecodedImage& /*img*/, ImageData::EncodedFormat /*format*/)
{
	throw love::Exception("Image encoding is not implemented for this format backend.");
}

void FormatHandler::free(unsigned char *mem)
{
	delete[] mem;
}

} // magpie
} // image
} // love
