/**
 * Simple DDS data parser for compressed 2D textures.
 *
 * Copyright (c) 2013 Alexander Szpakowski.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 **/

#ifndef DDS_PARSE_H
#define DDS_PARSE_H

#include <stddef.h>
#include <stdint.h>

#include <vector>

namespace dds
{

// Supported DDS formats.
// Formats with an 's' suffix have signed data.
enum Format
{
	FORMAT_DXT1,
	FORMAT_DXT3,
	FORMAT_DXT5,
	FORMAT_BC4,
	FORMAT_BC4s,
	FORMAT_BC5,
	FORMAT_BC5s,
	FORMAT_BC6H,
	FORMAT_BC6Hs,
	FORMAT_BC7,
	FORMAT_BC7srgb, // sRGB color space.
	FORMAT_UNKNOWN
};

// Represents a single mipmap level of a texture.
struct Image
{
	int width;
	int height;
	size_t dataSize;
	const uint8_t *data;

	Image() : width(0), height(0), dataSize(0), data(0)
	{}
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
	Format getFormat() const;

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

	size_t parseImageSize(Format fmt, int width, int height) const;
	bool parseTexData(const uint8_t *data, size_t dataSize, Format fmt, int w, int h, int mips);
	bool parseData(const void *data, size_t dataSize);

	std::vector<Image> texData;
	Format format;

}; // Parser

} // dds

#endif // DDS_PARSE_H
