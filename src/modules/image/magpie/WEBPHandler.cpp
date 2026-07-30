/**
 * Copyright (c) 2006-2026 LOVE Development Team
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

#include "WEBPHandler.h"

 // LOVE
#include "common/Exception.h"

// C++
#include <new>

// Simple WebP
#define SIMPLEWEBP_IMPLEMENTATION
#include "simplewebp/simplewebp.h"


namespace love
{
namespace image
{
namespace magpie
{

bool WEBPHandler::canDecode(Data *data)
{
	simplewebp *swebp = nullptr;
	simplewebp_error err = simplewebp_load_from_memory(data->getData(), data->getSize(), nullptr, &swebp);

	if (err == SIMPLEWEBP_NO_ERROR)
		simplewebp_unload(swebp);

	return swebp != nullptr;
}

WEBPHandler::DecodedImage WEBPHandler::decode(Data *data)
{
	simplewebp *swebp = nullptr;
	simplewebp_error err = simplewebp_load_from_memory(data->getData(), data->getSize(), nullptr, &swebp);

	if (err != SIMPLEWEBP_NO_ERROR)
		throw love::Exception("Could not decode WebP image (%s)", simplewebp_get_error_text(err));

	size_t width, height;
	simplewebp_get_dimensions(swebp, &width, &height);
	size_t imagesize = width * height * 4;

	unsigned char *buf = new (std::nothrow) unsigned char[imagesize];
	if (!buf)
		throw love::Exception("Could not decode WebP image (not enough memory)");

	err = simplewebp_decode(swebp, buf, nullptr);
	simplewebp_unload(swebp);
	if (err != SIMPLEWEBP_NO_ERROR)
	{
		delete[] buf;
		throw love::Exception("Could not decode WebP image (%s)", simplewebp_get_error_text(err));
	}

	WEBPHandler::DecodedImage output;
	output.data = buf;
	output.width = (int) width;
	output.height = (int) height;
	output.size = imagesize;
	output.format = PIXELFORMAT_RGBA8_UNORM;
	return output;
}

void WEBPHandler::freeRawPixels(unsigned char *mem)
{
	delete[] mem;
}

} // magpie
} // image
} // love
