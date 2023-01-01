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
#include "Compressor.h"
#include "common/config.h"
#include "common/int.h"

#include "libraries/lz4/lz4.h"
#include "libraries/lz4/lz4hc.h"

#include <zlib.h>

namespace love
{
namespace data
{

class LZ4Compressor : public Compressor
{
public:

	char *compress(Format format, const char *data, size_t dataSize, int level, size_t &compressedSize) override
	{
		if (format != FORMAT_LZ4)
			throw love::Exception("Invalid format (expecting LZ4)");

		if (dataSize > LZ4_MAX_INPUT_SIZE)
			throw love::Exception("Data is too large for LZ4 compressor.");

		// We use a custom header to store some info with the compressed data.
		const size_t headersize = sizeof(uint32);

		int maxdestsize = LZ4_compressBound((int) dataSize);
		size_t maxsize = headersize + (size_t) maxdestsize;
		char *compressedbytes = nullptr;

		try
		{
			compressedbytes = new char[maxsize];
		}
		catch (std::bad_alloc &)
		{
			throw love::Exception("Out of memory.");
		}

		// Store the size of the uncompressed data as a header.
#ifdef LOVE_BIG_ENDIAN
		// Make sure it's little-endian for storage.
		*(uint32 *) compressedbytes = swapuint32((uint32) dataSize);
#else
		*(uint32 *) compressedbytes = (uint32) dataSize;
#endif

		// Use LZ4-HC for compression level 9 and higher.
		int csize = 0;
		if (level > 8)
			csize = LZ4_compress_HC(data, compressedbytes + headersize, (int) dataSize, maxdestsize, LZ4HC_CLEVEL_DEFAULT);
		else
			csize = LZ4_compress_default(data, compressedbytes + headersize, (int) dataSize, maxdestsize);

		if (csize <= 0)
		{
			delete[] compressedbytes;
			throw love::Exception("Could not LZ4-compress data.");
		}

		// We allocated space for the maximum possible amount of data, but the
		// actual compressed size might be much smaller, so we should shrink the
		// data buffer if so.
		if ((double) maxsize / (double) (csize + headersize) >= 1.2)
		{
			char *cbytes = new (std::nothrow) char[csize + headersize];
			if (cbytes)
			{
				memcpy(cbytes, compressedbytes, csize + headersize);
				delete[] compressedbytes;
				compressedbytes = cbytes;
			}
		}

		compressedSize = (size_t) csize + headersize;
		return compressedbytes;
	}

	char *decompress(Format format, const char *data, size_t dataSize, size_t &decompressedSize) override
	{
		if (format != FORMAT_LZ4)
			throw love::Exception("Invalid format (expecting LZ4)");

		const size_t headersize = sizeof(uint32);
		char *rawbytes = nullptr;

		if (dataSize < headersize)
			throw love::Exception("Invalid LZ4-compressed data size.");

		// Extract the original uncompressed size (stored in our custom header.)
#ifdef LOVE_BIG_ENDIAN
		// Convert from stored little-endian to big-endian.
		uint32 rawsize = swapuint32(*(uint32 *) data);
#else
		uint32 rawsize = *(uint32 *) data;
#endif

		try
		{
			rawbytes = new char[rawsize];
		}
		catch (std::bad_alloc &)
		{
			throw love::Exception("Out of memory.");
		}

		// If the uncompressed size is passed in as an argument (non-zero) and
		// it matches the header's stored size, then we assume it's 100% accurate
		// and we use a more efficient decompression function.
		if (decompressedSize > 0 && decompressedSize == (size_t) rawsize)
		{
			// We don't use the header here, but we need to account for its size.
			if (LZ4_decompress_fast(data + headersize, rawbytes, (int) decompressedSize) < 0)
			{
				delete[] rawbytes;
				throw love::Exception("Could not decompress LZ4-compressed data.");
			}
		}
		else
		{
			// Account for our custom header's size in the decompress arguments.
			int result = LZ4_decompress_safe(data + headersize, rawbytes,
			                                 (int) (dataSize - headersize), rawsize);

			if (result < 0)
			{
				delete[] rawbytes;
				throw love::Exception("Could not decompress LZ4-compressed data.");
			}

			decompressedSize = (size_t) result;
		}

		return rawbytes;
	}

	bool isSupported(Format format) const override
	{
		return format == FORMAT_LZ4;
	}

}; // LZ4Compressor


class zlibCompressor : public Compressor
{
private:

	// The following three functions are mostly copied from the zlib source
	// (compressBound, compress2, and uncompress), but modified to support both
	// zlib and gzip.

	uLong zlibCompressBound(Format format, uLong sourceLen)
	{
		uLong size = sourceLen + (sourceLen >> 12) + (sourceLen >> 14) + (sourceLen >> 25) + 13;

		// The gzip header is slightly larger than the zlib header.
		if (format == FORMAT_GZIP)
			size += 18 - 6;

		return size;
	}

	int zlibCompress(Format format, Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen, int level)
	{
		z_stream stream = {};

		stream.next_in = (Bytef *) source;
		stream.avail_in = (uInt) sourceLen;

		stream.next_out = dest;
		stream.avail_out = (uInt) (*destLen);

		int windowbits = 15;
		if (format == FORMAT_GZIP)
			windowbits += 16; // This tells zlib to use a gzip header.
		else if (format == FORMAT_DEFLATE)
			windowbits = -windowbits;

		int err = deflateInit2(&stream, level, Z_DEFLATED, windowbits, 8, Z_DEFAULT_STRATEGY);

		if (err != Z_OK)
			return err;

		err = deflate(&stream, Z_FINISH);

		if (err != Z_STREAM_END)
		{
			deflateEnd(&stream);
			return err == Z_OK ? Z_BUF_ERROR : err;
		}

		*destLen = stream.total_out;

		return deflateEnd(&stream);
	}

	int zlibDecompress(Format format, Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen)
	{
		z_stream stream = {};

		stream.next_in = (Bytef *) source;
		stream.avail_in = (uInt) sourceLen;

		stream.next_out = dest;
		stream.avail_out = (uInt) (*destLen);

		// 15 is the default. Adding 32 makes zlib auto-detect the header type.
		int windowbits = 15 + 32;

		if (format == FORMAT_DEFLATE)
			windowbits = -15;

		int err = inflateInit2(&stream, windowbits);

		if (err != Z_OK)
			return err;

		err = inflate(&stream, Z_FINISH);

		if (err != Z_STREAM_END)
		{
			inflateEnd(&stream);
			if (err == Z_NEED_DICT || (err == Z_BUF_ERROR && stream.avail_in == 0))
				return Z_DATA_ERROR;
			return err;
		}

		*destLen = stream.total_out;

		return inflateEnd(&stream);
	}

public:

	char *compress(Format format, const char *data, size_t dataSize, int level, size_t &compressedSize) override
	{
		if (!isSupported(format))
			throw love::Exception("Invalid format (expecting zlib or gzip)");

		if (level < 0)
			level = Z_DEFAULT_COMPRESSION;
		else if (level > 9)
			level = 9;

		uLong maxsize = zlibCompressBound(format, (uLong) dataSize);
		char *compressedbytes = nullptr;

		try
		{
			compressedbytes = new char[maxsize];
		}
		catch (std::bad_alloc &)
		{
			throw love::Exception("Out of memory.");
		}

		uLongf destlen = maxsize;
		int status = zlibCompress(format, (Bytef *) compressedbytes, &destlen, (const Bytef *) data, (uLong) dataSize, level);

		if (status != Z_OK)
		{
			delete[] compressedbytes;
			throw love::Exception("Could not zlib/gzip-compress data.");
		}

		// We allocated space for the maximum possible amount of data, but the
		// actual compressed size might be much smaller, so we should shrink the
		// data buffer if so.
		if ((double) maxsize / (double) destlen >= 1.3)
		{
			char *cbytes = new (std::nothrow) char[destlen];
			if (cbytes)
			{
				memcpy(cbytes, compressedbytes, destlen);
				delete[] compressedbytes;
				compressedbytes = cbytes;
			}
		}

		compressedSize = (size_t) destlen;
		return compressedbytes;
	}

	char *decompress(Format format, const char *data, size_t dataSize, size_t &decompressedSize) override
	{
		if (!isSupported(format))
			throw love::Exception("Invalid format (expecting zlib or gzip)");

		char *rawbytes = nullptr;

		// We might know the output size before decompression. If not, we guess.
		size_t rawsize = decompressedSize > 0 ? decompressedSize : dataSize * 2;

		// Repeatedly try to decompress with an increasingly large output buffer.
		while (true)
		{
			try
			{
				rawbytes = new char[rawsize];
			}
			catch (std::bad_alloc &)
			{
				throw love::Exception("Out of memory.");
			}

			uLongf destLen = (uLongf) rawsize;
			int status = zlibDecompress(format, (Bytef *) rawbytes, &destLen, (const Bytef *) data, (uLong) dataSize);

			if (status == Z_OK)
			{
				decompressedSize = (size_t) destLen;
				break;
			}
			else if (status != Z_BUF_ERROR)
			{
				// For any error other than "not enough room", throw an exception.
				delete[] rawbytes;
				throw love::Exception("Could not decompress zlib/gzip-compressed data.");
			}

			// Not enough room in the output buffer: try again with a larger size.
			delete[] rawbytes;
			rawsize *= 2;
		}

		return rawbytes;
	}

	bool isSupported(Format format) const override
	{
		return format == FORMAT_ZLIB || format == FORMAT_GZIP || format == FORMAT_DEFLATE;
	}

}; // zlibCompressor

Compressor *Compressor::getCompressor(Format format)
{
	static LZ4Compressor lz4compressor;
	static zlibCompressor zlibcompressor;

	Compressor *compressors[] = {&lz4compressor, &zlibcompressor};

	for (Compressor *c : compressors)
	{
		if (c->isSupported(format))
			return c;
	}

	return nullptr;
}

bool Compressor::getConstant(const char *in, Format &out)
{
	return formatNames.find(in, out);
}

bool Compressor::getConstant(Format in, const char *&out)
{
	return formatNames.find(in, out);
}

std::vector<std::string> Compressor::getConstants(Format)
{
	return formatNames.getNames();
}

StringMap<Compressor::Format, Compressor::FORMAT_MAX_ENUM>::Entry Compressor::formatEntries[] =
{
	{ "lz4",     FORMAT_LZ4     },
	{ "zlib",    FORMAT_ZLIB    },
	{ "gzip",    FORMAT_GZIP    },
	{ "deflate", FORMAT_DEFLATE },
};

StringMap<Compressor::Format, Compressor::FORMAT_MAX_ENUM> Compressor::formatNames(Compressor::formatEntries, sizeof(Compressor::formatEntries));

} // data
} // love
