/**
 * Copyright (c) 2006-2024 LOVE Development Team
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
#include "common/config.h"
#include "common/int.h"
#include "common/math.h"
#include "common/Object.h"
#include "common/StringMap.h"
#include "common/pixelformat.h"

namespace love::image
{
class ImageData;
class CompressedImageData;
}

namespace love::data
{
class ByteData;
}

namespace love
{
namespace graphics
{

class Buffer;
class Texture;
class Graphics;

enum ReadbackMethod
{
	READBACK_IMMEDIATE,
	READBACK_ASYNC,
};

class GraphicsReadback : public love::Object
{
public:

	enum Status
	{
		STATUS_WAITING,
		STATUS_COMPLETE,
		STATUS_ERROR,
		STATUS_MAX_ENUM
	};

	static love::Type type;

	GraphicsReadback(Graphics *gfx, ReadbackMethod method, Buffer *buffer, size_t offset, size_t size, love::data::ByteData *dest, size_t destoffset);
	GraphicsReadback(Graphics *gfx, ReadbackMethod method, Texture *texture, int slice, int mipmap, const Rect &rect, love::image::ImageData *dest, int destx, int desty);
	virtual ~GraphicsReadback();

	virtual void wait() = 0;
	virtual void update() = 0;

	bool isComplete() const { return status != STATUS_WAITING; }
	ReadbackMethod getMethod() const { return method; }
	bool hasError() const { return status == STATUS_ERROR; }

	love::data::ByteData *getBufferData() const;
	love::image::ImageData *getImageData() const;

protected:

	enum DataType
	{
		DATA_BUFFER,
		DATA_TEXTURE,
	};

	void *prepareReadbackDest(size_t size);
	Status readbackBuffer(Buffer *buffer, size_t offset, size_t size);

	DataType dataType;
	ReadbackMethod method;
	Status status = STATUS_WAITING;

	StrongRef<love::data::ByteData> bufferData;
	size_t bufferDataOffset = 0;

	StrongRef<love::image::ImageData> imageData;
	Rect rect = {};
	PixelFormat textureFormat = PIXELFORMAT_UNKNOWN;
	bool isFormatLinear = false;
	int imageDataX = 0;
	int imageDataY = 0;

}; // GraphicsReadback

} // graphics
} // love
