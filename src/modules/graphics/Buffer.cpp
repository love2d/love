/**
 * Copyright (c) 2006-2017 LOVE Development Team
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

#include "Buffer.h"
#include "Graphics.h"
#include "common/Exception.h"

namespace love
{
namespace graphics
{

Buffer::Buffer(size_t size, BufferType type, vertex::Usage usage, uint32 mapflags)
	: size(size)
	, type(type)
	, usage(usage)
	, map_flags(mapflags)
	, is_mapped(false)
{
}

Buffer::~Buffer()
{
}

// QuadIndices

size_t QuadIndices::maxSize = 0;
size_t QuadIndices::elementSize = 0;
size_t QuadIndices::objectCount = 0;

Buffer *QuadIndices::indexBuffer = nullptr;
char *QuadIndices::indices = nullptr;

QuadIndices::QuadIndices(Graphics *gfx, size_t size)
	: size(size)
{
	// The upper limit is the maximum of uint32 divided by six (the number
	// of indices per size) and divided by the size of uint32. This guarantees
	// no overflows when calculating the array size in bytes.
	if (size == 0 || size > ((uint32) -1) / 6 / sizeof(uint32))
		throw love::Exception("Invalid number of quads.");

	// Create a new / larger buffer if needed.
	if (indexBuffer == nullptr || size > maxSize)
	{
		Buffer *newbuffer = nullptr;
		char *newindices = nullptr;

		// Depending on the size, a switch to int and more memory is needed.
		IndexDataType targettype = getType(size);
		size_t elemsize = vertex::getIndexDataSize(targettype);

		size_t buffersize = elemsize * 6 * size;

		try
		{
			newbuffer = gfx->newBuffer(buffersize, nullptr, BUFFER_INDEX, vertex::USAGE_STATIC, 0);
			newindices = new char[buffersize];
		}
		catch (std::bad_alloc &)
		{
			delete newbuffer;
			delete[] newindices;
			throw love::Exception("Out of memory.");
		}

		// Allocation of the new Buffer succeeded.
		// The old Buffer can now be deleted.
		delete indexBuffer;
		indexBuffer = newbuffer;

		delete[] indices;
		indices = newindices;

		maxSize = size;
		elementSize = elemsize;

		switch (targettype)
		{
		case INDEX_UINT16:
			fill<uint16>();
			break;
		case INDEX_UINT32:
			fill<uint32>();
			break;
		case INDEX_MAX_ENUM:
			break;
		}
	}

	objectCount++;
}

QuadIndices::QuadIndices(const QuadIndices &other)
: size(other.size)
{
	objectCount++;
}

QuadIndices &QuadIndices::operator = (const QuadIndices &other)
{
	size = other.size;
	return *this;
}

QuadIndices::~QuadIndices()
{
	--objectCount;

	// Delete the buffer if we were the last living QuadIndices object.
	if (objectCount <= 0)
	{
		delete indexBuffer;
		indexBuffer = nullptr;

		delete[] indices;
		indices = nullptr;
	}
}

size_t QuadIndices::getSize() const
{
	return size;
}

size_t QuadIndices::getIndexCount(size_t elements) const
{
	return elements * 6;
}

IndexDataType QuadIndices::getType(size_t s) const
{
	return vertex::getIndexDataTypeFromMax(getIndexCount(s));
}

size_t QuadIndices::getElementSize() const
{
	return elementSize;
}

Buffer *QuadIndices::getBuffer() const
{
	return indexBuffer;
}

const void *QuadIndices::getIndices(size_t offset) const
{
	return indices + offset;
}

template <typename T>
void QuadIndices::fill()
{
	vertex::fillIndices(vertex::TriangleIndexMode::QUADS, 0, maxSize * 4, (T *) indices);
	indexBuffer->fill(0, indexBuffer->getSize(), indices);
}

} // graphics
} // love
