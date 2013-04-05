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

#include "ddsinfo.h"

#include <stddef.h>
#include <stdint.h>

#include <vector>
#include <exception>

namespace dds
{

// Supported DDS formats.
enum Format
{
	FORMAT_DXT1,
	FORMAT_DXT3,
	FORMAT_DXT5,
	FORMAT_BC5s,    // Signed.
	FORMAT_BC5,     // Unsigned.
	FORMAT_BC7,
	FORMAT_BC7srgb, // sRGB color space.
	FORMAT_UNKNOWN
};

class Parser
{
public:

	enum Options
	{
		OPTIONS_NONE      = 0x00,
		OPTIONS_COPY_DATA = 0x01 // Copy texture data internally when parsing.
	};

	// Represents a single mipmap level of a texture.
	struct Image
	{
		int width;
		int height;
		size_t dataSize;
		const uint8_t *data;

		Image();
	};

	/**
	 * Determines whether the input byte data is a valid DDS representation.
	 **/
	static bool isDDS(const void *data, size_t dataSize);

	/**
	 * Constructor.
	 * Attempts to parse byte data as DDS. May throw std::bad_alloc if out of
	 * memory.
	 * @param data     The byte data to parse.
	 * @param dataSize The size in bytes of the data.
	 * @param options  Any optional settings (see above.)
	 **/
	Parser(const void *data, size_t dataSize, Options opts = OPTIONS_NONE);
	Parser(const Parser &other);

	~Parser();

	/**
	 * Gets the format of this texture.
	 **/
	Format getFormat() const;

	/**
	 * Gets the data of this texture at a mipmap level. Mipmap level 0
	 * represents the base image.
	 * @param miplevel The mipmap level to get the data of.
	 * @return Pointer to the image data, or 0 if miplevel is not within the
	 *         range of [0, numMipmaps).
	 **/
	const Image *getImageData(size_t miplevel = 0) const;

	/**
	 * Gets the number of mipmap levels in this texture.
	 * Includes the base mip level.
	 **/
	size_t getNumMipmaps() const;

	/**
	 * Gets whether this Parser is using an internal copy of the texture data.
	 * Accessing texture data from a Parser which hasn't internally copied the
	 * DDS data will result in undefined behaviour if the original data is deleted.
	 **/
	bool ownsData() const;

private:

	Format parseDDSFormat(const dxinfo::DDSPixelFormat &fmt) const;
	Format parseDX10Format(dxinfo::DXGIFormat fmt) const;
	size_t parseImageSize(Format fmt, int width, int height) const;

	bool parseTexData(const uint8_t *data, size_t dataSize, Format fmt, int w, int h, int mips);

	bool parseData(const void *data, size_t dataSize);

	// Delete any heap data created by this object.
	void clearData();

	std::vector<Image> texData;
	Format format;
	Options options;

}; // Parser

} // dds

#endif // DDS_PARSE_H
