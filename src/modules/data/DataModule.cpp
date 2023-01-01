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
#include "DataModule.h"
#include "common/b64.h"
#include "common/int.h"
#include "common/StringMap.h"

// STL
#include <cmath>
#include <list>
#include <iostream>

namespace
{

static const char hexchars[] = "0123456789abcdef";

char *bytesToHex(const love::uint8 *src, size_t srclen, size_t &dstlen)
{
	dstlen = srclen * 2;

	if (dstlen == 0)
		return nullptr;

	char *dst = nullptr;
	try
	{
		dst = new char[dstlen + 1];
	}
	catch (std::exception &)
	{
		throw love::Exception("Out of memory.");
	}

	for (size_t i = 0; i < srclen; i++)
	{
		love::uint8 b = src[i];
		dst[i * 2 + 0] = hexchars[b >> 4];
		dst[i * 2 + 1] = hexchars[b & 0xF];
	}

	dst[dstlen] = '\0';
	return dst;
}

love::uint8 nibble(char c)
{
	if (c >= '0' && c <= '9')
		return (love::uint8) (c - '0');

	if (c >= 'A' && c <= 'F')
		return (love::uint8) (c - 'A' + 0x0a);

	if (c >= 'a' && c <= 'f')
		return (love::uint8) (c - 'a' + 0x0a);

	return 0;
}

love::uint8 *hexToBytes(const char *src, size_t srclen, size_t &dstlen)
{
	if (srclen >= 2 && src[0] == '0' && (src[1] == 'x' || src[1] == 'X'))
	{
		src += 2;
		srclen -= 2;
	}

	dstlen = (srclen + 1) / 2;

	if (dstlen == 0)
		return nullptr;

	love::uint8 *dst = nullptr;
	try
	{
		dst = new love::uint8[dstlen];
	}
	catch (std::exception &)
	{
		throw love::Exception("Out of memory.");
	}

	for (size_t i = 0; i < dstlen; i++)
	{
		dst[i] = nibble(src[i * 2]) << 4;

		if (i * 2 + 1 < srclen)
			dst[i] |= nibble(src[i * 2 + 1]);
	}

	return dst;
}

} // anonymous namespace

namespace love
{
namespace data
{

CompressedData *compress(Compressor::Format format, const char *rawbytes, size_t rawsize, int level)
{
	Compressor *compressor = Compressor::getCompressor(format);

	if (compressor == nullptr)
		throw love::Exception("Invalid compression format.");

	size_t compressedsize = 0;
	char *cbytes = compressor->compress(format, rawbytes, rawsize, level, compressedsize);

	CompressedData *data = nullptr;

	try
	{
		data = new CompressedData(format, cbytes, compressedsize, rawsize, true);
	}
	catch (love::Exception &)
	{
		delete[] cbytes;
		throw;
	}

	return data;
}

char *decompress(CompressedData *data, size_t &decompressedsize)
{
	size_t rawsize = data->getDecompressedSize();

	char *rawbytes = decompress(data->getFormat(), (const char *) data->getData(),
	                            data->getSize(), rawsize);

	decompressedsize = rawsize;
	return rawbytes;
}

char *decompress(Compressor::Format format, const char *cbytes, size_t compressedsize, size_t &rawsize)
{
	Compressor *compressor = Compressor::getCompressor(format);

	if (compressor == nullptr)
		throw love::Exception("Invalid compression format.");

	return compressor->decompress(format, cbytes, compressedsize, rawsize);
}

char *encode(EncodeFormat format, const char *src, size_t srclen, size_t &dstlen, size_t linelen)
{
	switch (format)
	{
	case ENCODE_BASE64:
	default:
		return b64_encode(src, srclen, linelen, dstlen);
	case ENCODE_HEX:
		return bytesToHex((const uint8 *) src, srclen, dstlen);
	}
}

char *decode(EncodeFormat format, const char *src, size_t srclen, size_t &dstlen)
{
	switch (format)
	{
	case ENCODE_BASE64:
	default:
		return b64_decode(src, srclen, dstlen);
	case ENCODE_HEX:
		return (char *) hexToBytes(src, srclen, dstlen);
	}
}

std::string hash(HashFunction::Function function, Data *input)
{
	return hash(function, (const char*) input->getData(), input->getSize());
}

std::string hash(HashFunction::Function function, const char *input, uint64_t size)
{
	HashFunction::Value output;
	hash(function, input, size, output);
	return std::string(output.data, output.size);
}

void hash(HashFunction::Function function, Data *input, HashFunction::Value &output)
{
	hash(function, (const char*) input->getData(), input->getSize(), output);
}

void hash(HashFunction::Function function, const char *input, uint64_t size, HashFunction::Value &output)
{
	HashFunction *hashfunction = HashFunction::getHashFunction(function);
	if (hashfunction == nullptr)
		throw love::Exception("Invalid hash function.");

	hashfunction->hash(function, input, size, output);
}

DataModule::DataModule()
{
}

DataModule::~DataModule()
{
}

DataView *DataModule::newDataView(Data *data, size_t offset, size_t size)
{
	return new DataView(data, offset, size);
}

ByteData *DataModule::newByteData(size_t size)
{
	return new ByteData(size);
}

ByteData *DataModule::newByteData(const void *d, size_t size)
{
	return new ByteData(d, size);
}

ByteData *DataModule::newByteData(void *d, size_t size, bool own)
{
	return new ByteData(d, size, own);
}

static StringMap<EncodeFormat, ENCODE_MAX_ENUM>::Entry encoderEntries[] =
{
	{ "base64", ENCODE_BASE64 },
	{ "hex",    ENCODE_HEX    },
};

static StringMap<EncodeFormat, ENCODE_MAX_ENUM> encoders(encoderEntries, sizeof(encoderEntries));

static StringMap<ContainerType, CONTAINER_MAX_ENUM>::Entry containerEntries[] =
{
	{ "data",   CONTAINER_DATA   },
	{ "string", CONTAINER_STRING },
};

static StringMap<ContainerType, CONTAINER_MAX_ENUM> containers(containerEntries, sizeof(containerEntries));

bool getConstant(const char *in, EncodeFormat &out)
{
	return encoders.find(in, out);
}

bool getConstant(EncodeFormat in, const char *&out)
{
	return encoders.find(in, out);
}

std::vector<std::string> getConstants(EncodeFormat)
{
	return encoders.getNames();
}

bool getConstant(const char *in, ContainerType &out)
{
	return containers.find(in, out);
}

bool getConstant(ContainerType in, const char *&out)
{
	return containers.find(in, out);
}

std::vector<std::string> getConstants(ContainerType)
{
	return containers.getNames();
}

} // data
} // love
