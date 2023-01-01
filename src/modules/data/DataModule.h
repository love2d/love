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

#include "CompressedData.h"
#include "Compressor.h"
#include "HashFunction.h"
#include "DataView.h"
#include "ByteData.h"

// LOVE
#include "common/Module.h"
#include "common/int.h"

namespace love
{
namespace data
{

enum EncodeFormat
{
	ENCODE_BASE64,
	ENCODE_HEX,
	ENCODE_MAX_ENUM
};

enum ContainerType
{
	CONTAINER_DATA,
	CONTAINER_STRING,
	CONTAINER_MAX_ENUM
};

/**
 * Compresses a block of memory using the given compression format.
 *
 * @param format The compression format to use.
 * @param rawbytes The data to compress.
 * @param rawsize The size in bytes of the data to compress.
 * @param level The amount of compression to apply (between 0 and 9.)
 *              A value of -1 indicates the default amount of compression.
 *              Specific formats may not use every level.
 * @return The newly compressed data.
 **/
CompressedData *compress(Compressor::Format format, const char *rawbytes, size_t rawsize, int level = -1);

/**
 * Decompresses existing compressed data into raw bytes.
 *
 * @param[in] data The compressed data to decompress.
 * @param[out] decompressedsize The size in bytes of the decompressed data.
 * @return The newly decompressed data (allocated with new[]).
 **/
char *decompress(CompressedData *data, size_t &decompressedsize);

/**
 * Decompresses existing compressed data into raw bytes.
 *
 * @param[in] format The compression format the data is in.
 * @param[in] cbytes The compressed data to decompress.
 * @param[in] compressedsize The size in bytes of the compressed data.
 * @param[in,out] rawsize On input, the size in bytes of the original
 *               uncompressed data, or 0 if unknown. On return, the size in
 *               bytes of the newly decompressed data.
 * @return The newly decompressed data (allocated with new[]).
 **/
char *decompress(Compressor::Format format, const char *cbytes, size_t compressedsize, size_t &rawsize);

char *encode(EncodeFormat format, const char *src, size_t srclen, size_t &dstlen, size_t linelen = 0);
char *decode(EncodeFormat format, const char *src, size_t srclen, size_t &dstlen);

/**
 * Hash the input, producing an set of bytes as output.
 *
 * @param[in] function The selected hash function.
 * @param[in] input The input data to hash.
 * @return An std::string of bytes, representing the result of the hash
 *         function.
 **/
std::string hash(HashFunction::Function function, Data *input);
std::string hash(HashFunction::Function function, const char *input, uint64_t size);
void hash(HashFunction::Function function, Data *input, HashFunction::Value &output);
void hash(HashFunction::Function function, const char *input, uint64_t size, HashFunction::Value &output);


bool getConstant(const char *in, EncodeFormat &out);
bool getConstant(EncodeFormat in, const char *&out);
std::vector<std::string> getConstants(EncodeFormat);

bool getConstant(const char *in, ContainerType &out);
bool getConstant(ContainerType in, const char *&out);
std::vector<std::string> getConstants(ContainerType);


class DataModule : public Module
{
public:

	DataModule();
	virtual ~DataModule();

	// Implements Module.
	ModuleType getModuleType() const override { return M_DATA; }
	const char *getName() const override { return "love.data"; }

	DataView *newDataView(Data *data, size_t offset, size_t size);
	ByteData *newByteData(size_t size);
	ByteData *newByteData(const void *d, size_t size);
	ByteData *newByteData(void *d, size_t size, bool own);

}; // DataModule

} // data
} // love
