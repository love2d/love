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

// LOVE
#include "ASTCHandler.h"
#include "common/int.h"
#include "common/Exception.h"

namespace love
{
namespace image
{
namespace magpie
{

namespace
{

static const uint32 ASTC_IDENTIFIER = 0x5CA1AB13;

#pragma pack(push, 1)
struct ASTCHeader
{
	uint8 identifier[4];
	uint8 blockdimX;
	uint8 blockdimY;
	uint8 blockdimZ;
	uint8 sizeX[3];
	uint8 sizeY[3];
	uint8 sizeZ[3];
};
#pragma pack(pop)

static PixelFormat convertFormat(uint32 blockX, uint32 blockY, uint32 blockZ)
{
	if (blockZ > 1)
		return PIXELFORMAT_UNKNOWN;

	if (blockX == 4 && blockY == 4)
		return PIXELFORMAT_ASTC_4x4;
	else if (blockX == 5 && blockY == 4)
		return PIXELFORMAT_ASTC_5x4;
	else if (blockX == 5 && blockY == 5)
		return PIXELFORMAT_ASTC_5x5;
	else if (blockX == 6 && blockY == 5)
		return PIXELFORMAT_ASTC_6x5;
	else if (blockX == 6 && blockY == 6)
		return PIXELFORMAT_ASTC_6x6;
	else if (blockX == 8 && blockY == 5)
		return PIXELFORMAT_ASTC_8x5;
	else if (blockX == 8 && blockY == 6)
		return PIXELFORMAT_ASTC_8x6;
	else if (blockX == 8 && blockY == 8)
		return PIXELFORMAT_ASTC_8x8;
	else if (blockX == 10 && blockY == 5)
		return PIXELFORMAT_ASTC_10x5;
	else if (blockX == 10 && blockY == 6)
		return PIXELFORMAT_ASTC_10x6;
	else if (blockX == 10 && blockY == 8)
		return PIXELFORMAT_ASTC_10x8;
	else if (blockX == 10 && blockY == 10)
		return PIXELFORMAT_ASTC_10x10;
	else if (blockX == 12 && blockY == 10)
		return PIXELFORMAT_ASTC_12x10;
	else if (blockX == 12 && blockY == 12)
		return PIXELFORMAT_ASTC_12x12;

	return PIXELFORMAT_UNKNOWN;
}

} // Anonymous namespace.

bool ASTCHandler::canParseCompressed(Data *data)
{
	if (data->getSize() <= sizeof(ASTCHeader))
		return false;

	const ASTCHeader *header = (const ASTCHeader *) data->getData();

	uint32 identifier =  (uint32) header->identifier[0]
	                  + ((uint32) header->identifier[1] << 8)
	                  + ((uint32) header->identifier[2] << 16)
	                  + ((uint32) header->identifier[3] << 24);

	if (identifier != ASTC_IDENTIFIER)
		return false;

	return true;
}

StrongRef<CompressedMemory> ASTCHandler::parseCompressed(Data *filedata, std::vector<StrongRef<CompressedSlice>> &images, PixelFormat &format, bool &sRGB)
{
	if (!canParseCompressed(filedata))
		throw love::Exception("Could not decode compressed data (not an .astc file?)");

	ASTCHeader header = *(const ASTCHeader *) filedata->getData();

	PixelFormat cformat = convertFormat(header.blockdimX, header.blockdimY, header.blockdimZ);

	if (cformat == PIXELFORMAT_UNKNOWN)
		throw love::Exception("Could not parse .astc file: unsupported ASTC format %dx%dx%d.", header.blockdimX, header.blockdimY, header.blockdimZ);

	uint32 sizeX = header.sizeX[0] + (header.sizeX[1] << 8) + (header.sizeX[2] << 16);
	uint32 sizeY = header.sizeY[0] + (header.sizeY[1] << 8) + (header.sizeY[2] << 16);
	uint32 sizeZ = header.sizeZ[0] + (header.sizeZ[1] << 8) + (header.sizeZ[2] << 16);

	uint32 blocksX = (sizeX + header.blockdimX - 1) / header.blockdimX;
	uint32 blocksY = (sizeY + header.blockdimY - 1) / header.blockdimY;
	uint32 blocksZ = (sizeZ + header.blockdimZ - 1) / header.blockdimZ;

	size_t totalsize = blocksX * blocksY * blocksZ * 16;

	if (totalsize + sizeof(header) > filedata->getSize())
		throw love::Exception("Could not parse .astc file: file is too small.");

	StrongRef<CompressedMemory> memory(new CompressedMemory(totalsize), Acquire::NORETAIN);

	// .astc files only store a single mipmap level.
	memcpy(memory->data, (uint8 *) filedata->getData() + sizeof(ASTCHeader), totalsize);

	images.emplace_back(new CompressedSlice(cformat, sizeX, sizeY, memory, 0, totalsize), Acquire::NORETAIN);

	format = cformat;
	sRGB = false;

	return memory;
}

} // magpie
} // image
} // love

