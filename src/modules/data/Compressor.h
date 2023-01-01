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
#include "common/StringMap.h"

namespace love
{
namespace data
{

/**
 * Base class for backends for different compression formats.
 **/
class Compressor
{
public:

	enum Format
	{
		FORMAT_LZ4,
		FORMAT_ZLIB,
		FORMAT_GZIP,
		FORMAT_DEFLATE,
		FORMAT_MAX_ENUM
	};

	/**
	 * Gets a Compressor that can compress and decompress a specific format.
	 * Returns null if there are no supported compressors for the given format.
	 **/
	static Compressor *getCompressor(Format format);

	virtual ~Compressor() {}

	/**
	 * Compresses input data, and returns the compressed result.
	 *
	 * @param[in] format The format to compress to.
	 * @param[in] data The input (uncompressed) data.
	 * @param[in] dataSize The size in bytes of the input data.
	 * @param[in] level The amount of compression to apply (between 0 and 9.)
	 *            A value of -1 indicates the default amount of compression.
	 *            Specific formats may not use every level.
	 * @param[out] compressedSize The size in bytes of the compressed result.
	 *
	 * @return The newly compressed data (allocated with new[]).
	 **/
	virtual char *compress(Format format, const char *data, size_t dataSize, int level, size_t &compressedSize) = 0;

	/**
	 * Decompresses compressed data, and returns the decompressed result.
	 *
	 * @param[in] format The format the compressed data is in.
	 * @param[in] data The input (compressed) data.
	 * @param[in] dataSize The size in bytes of the compressed data.
	 * @param[in,out] decompressedSize On input, the size in bytes of the
	 *               original uncompressed data, or 0 if unknown. On return, the
	 *               size in bytes of the decompressed data.
	 *
	 * @return The decompressed data (allocated with new[]).
	 **/
	virtual char *decompress(Format format, const char *data, size_t dataSize, size_t &decompressedSize) = 0;

	/**
	 * Gets whether a specific format is supported by this backend.
	 **/
	virtual bool isSupported(Format format) const = 0;

	static bool getConstant(const char *in, Format &out);
	static bool getConstant(Format in, const char *&out);
	static std::vector<std::string> getConstants(Format);

protected:

	Compressor() {}

private:

	static StringMap<Format, FORMAT_MAX_ENUM>::Entry formatEntries[];
	static StringMap<Format, FORMAT_MAX_ENUM> formatNames;

}; // Compressor

} // data
} // love
