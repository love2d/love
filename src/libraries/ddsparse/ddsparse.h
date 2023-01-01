/**
 * Simple DDS data parser for compressed 2D textures.
 *
 * Copyright (c) 2013-2023 Sasha Szpakowski
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

#ifndef DDS_PARSE_H
#define DDS_PARSE_H

#include <stddef.h>
#include <stdint.h>

#include <vector>

#include "ddsinfo.h"

namespace dds
{

// Represents a single mipmap level of a texture.
struct Image
{
	int width = 0;
	int height = 0;
	size_t dataSize = 0;
	const uint8_t *data = nullptr;
};

/**
 * Determines whether the input byte data represents a valid DDS file.
 * Does not take into account whether the texture format is supported.
 *
 * @param data     The byte data to parse.
 * @param dataSize The size in bytes of the data.
 **/
bool isDDS(const void *data, size_t dataSize);

/**
 * Determines whether the input byte data represents a valid compressed DDS
 * file. Takes into account texture format, but not type (3D textures, etc.)
 *
 * @param data     The byte data to parse.
 * @param dataSize The size in bytes of the data.
 **/
bool isCompressedDDS(const void *data, size_t dataSize);

dxinfo::DXGIFormat getDDSPixelFormat(const void *data, size_t dataSize);

class Parser
{
public:

	/**
	 * Constructor.
	 * Attempts to parse byte data as a compressed DDS file.
	 *
	 * @param data     The byte data to parse.
	 * @param dataSize The size in bytes of the data.
	 **/
	Parser(const void *data, size_t dataSize);
	Parser(const Parser &other);
	Parser();

	Parser &operator = (const Parser &other);

	~Parser();

	/**
	 * Gets the format of this texture.
	 **/
	dxinfo::DXGIFormat getFormat() const;

	/**
	 * Gets the data of this texture at a mipmap level. Mipmap level 0
	 * represents the base image.
	 *
	 * @param miplevel The mipmap level to get the data of.
	 * @return Pointer to the image data, or NULL if miplevel is not within the
	 *         range of [0, numMipmaps).
	 **/
	const Image *getImageData(size_t miplevel = 0) const;

	/**
	 * Gets the number of mipmap levels in this texture.
	 * Includes the base mip level.
	 **/
	size_t getMipmapCount() const;

private:

	size_t parseImageSize(dxinfo::DXGIFormat fmt, int width, int height) const;
	bool parseTexData(const uint8_t *data, size_t dataSize, dxinfo::DXGIFormat fmt, int w, int h, int nb_mips);
	bool parseData(const void *data, size_t dataSize);

	std::vector<Image> texData;
	dxinfo::DXGIFormat format;

}; // Parser

} // dds

#endif // DDS_PARSE_H
