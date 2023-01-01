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
#include "common/int.h"
#include "vertex.h"
#include "Resource.h"

// C
#include <cstddef>

namespace love
{
namespace graphics
{

class StreamBuffer : public Resource
{
public:

	struct MapInfo
	{
		uint8 *data = nullptr;
		size_t size = 0;

		MapInfo() {}

		MapInfo(uint8 *data, size_t size)
			: data(data)
			, size(size)
		{}
	};

	virtual ~StreamBuffer() {}

	size_t getSize() const { return bufferSize; }
	BufferType getMode() const { return mode; }
	size_t getUsableSize() const { return bufferSize - frameGPUReadOffset; }

	virtual MapInfo map(size_t minsize) = 0;
	virtual size_t unmap(size_t usedsize) = 0;
	virtual void markUsed(size_t usedsize) = 0;

	virtual void nextFrame() {}

protected:

	StreamBuffer(BufferType mode, size_t size);

	size_t bufferSize;
	size_t frameGPUReadOffset;
	BufferType mode;

}; // StreamBuffer

} // graphics
} // love
