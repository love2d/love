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

#ifndef LOVE_DEVIL_COMPRESSED_DATA_H
#define LOVE_DEVIL_COMPRESSED_DATA_H

// LOVE
#include "filesystem/File.h"
#include "image/CompressedData.h"

// dds parser
#include "ddsparse/ddsparse.h"

namespace love
{
namespace image
{
namespace devil
{

class CompressedData : public love::image::CompressedData
{
public:

	CompressedData(love::filesystem::File *file);
	CompressedData(Data *data);

	virtual ~CompressedData();

	static bool isCompressed(const Data *data);

private:

	bool convertFormat(dds::Format ddsformat);
	void load(Data *data);

}; // CompressedData

} // devil
} // image
} // love

#endif // LOVE_DEVIL_COMPRESSED_DATA_H
