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
#include "common/Data.h"
#include "common/StringMap.h"
#include "common/int.h"
#include "common/pixelformat.h"
#include "common/floattypes.h"
#include "common/Color.h"
#include "filesystem/FileData.h"
#include "thread/threads.h"
#include "ImageDataBase.h"
#include "FormatHandler.h"

using love::thread::Mutex;

namespace love
{
namespace image
{

/**
 * Represents raw pixel data.
 **/
class ImageData : public ImageDataBase
{
public:

	union Pixel
	{
		uint8   rgba8[4];
		uint16  rgba16[4];
		float16 rgba16f[4];
		float   rgba32f[4];
		uint16  packed16;
		uint32  packed32;
	};

	typedef void (*PixelSetFunction)(const Colorf &c, Pixel *p);
	typedef void (*PixelGetFunction)(const Pixel *p, Colorf &c);

	static love::Type type;

	ImageData(Data *data);
	ImageData(int width, int height, PixelFormat format = PIXELFORMAT_RGBA8);
	ImageData(int width, int height, PixelFormat format, void *data, bool own);
	ImageData(const ImageData &c);
	virtual ~ImageData();

	/**
	 * Paste part of one ImageData onto another. The subregion defined by the top-left
	 * corner (sx, sy) and the size (sw,sh) will be pasted to (dx,dy) in this ImageData.
	 * @param dx The destination x-coordinate.
	 * @param dy The destination y-coordinate.
	 * @param sx The source x-coordinate.
	 * @param sy The source y-coordinate.
	 * @param sw The source width.
	 * @param sh The source height.
	 **/
	void paste(ImageData *src, int dx, int dy, int sx, int sy, int sw, int sh);

	/**
	 * Checks whether a position is inside this ImageData. Useful for checking bounds.
	 * @param x The position along the x-axis.
	 * @param y The position along the y-axis.
	 **/
	bool inside(int x, int y) const;

	/**
	 * Sets the pixel at location (x,y).
	 * @param x The location along the x-axis.
	 * @param y The location along the y-axis.
	 * @param p The color to use for the given location.
	 **/
	void setPixel(int x, int y, const Colorf &p);

	/**
	 * Gets the pixel at location (x,y).
	 * @param x The location along the x-axis.
	 * @param y The location along the y-axis.
	 * @return The color for the given location.
	 **/
	void getPixel(int x, int y, Colorf &c) const;
	Colorf getPixel(int x, int y) const;

	/**
	 * Encodes raw pixel data into a given format.
	 * @param f The file to save the encoded image data to.
	 * @param format The format of the encoded data.
	 **/
	love::filesystem::FileData *encode(FormatHandler::EncodedFormat format, const char *filename, bool writefile) const;

	love::thread::Mutex *getMutex() const;

	// Implements ImageDataBase.
	ImageData *clone() const override;
	void *getData() const override;
	size_t getSize() const override;
	bool isSRGB() const override;

	size_t getPixelSize() const;

	PixelSetFunction getPixelSetFunction() const { return pixelSetFunction; }
	PixelGetFunction getPixelGetFunction() const { return pixelGetFunction; }

	static bool validPixelFormat(PixelFormat format);
	static bool canPaste(PixelFormat src, PixelFormat dst);

	static PixelSetFunction getPixelSetFunction(PixelFormat format);
	static PixelGetFunction getPixelGetFunction(PixelFormat format);

	static bool getConstant(const char *in, FormatHandler::EncodedFormat &out);
	static bool getConstant(FormatHandler::EncodedFormat in, const char *&out);
	static std::vector<std::string> getConstants(FormatHandler::EncodedFormat);

private:

	// Create imagedata. Initialize with data if not null.
	void create(int width, int height, PixelFormat format, void *data = nullptr);

	// Decode and load an encoded format.
	void decode(Data *data);

	// The actual data.
	unsigned char *data = nullptr;

	love::thread::MutexRef mutex;

	// The format handler that was used to decode the ImageData. We need to know
	// this so we can properly delete memory allocated by the decoder.
	StrongRef<FormatHandler> decodeHandler;

	PixelSetFunction pixelSetFunction;
	PixelGetFunction pixelGetFunction;

	static StringMap<FormatHandler::EncodedFormat, FormatHandler::ENCODED_MAX_ENUM>::Entry encodedFormatEntries[];
	static StringMap<FormatHandler::EncodedFormat, FormatHandler::ENCODED_MAX_ENUM> encodedFormats;

}; // ImageData

} // image
} // love
