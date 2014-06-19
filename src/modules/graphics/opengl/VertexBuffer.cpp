/**
 * Copyright (c) 2006-2014 LOVE Development Team
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

#include "VertexBuffer.h"

#include "common/Exception.h"

#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <limits>

namespace love
{
namespace graphics
{
namespace opengl
{

// VertexBuffer

VertexBuffer *VertexBuffer::Create(size_t size, GLenum target, GLenum usage, MemoryBacking backing)
{
	return new VertexBuffer(size, target, usage, backing);
}

VertexBuffer::VertexBuffer(size_t size, GLenum target, GLenum usage, MemoryBacking backing)
	: is_bound(false)
	, is_mapped(false)
	, size(size)
	, target(target)
	, usage(usage)
	, backing(backing)
	, vbo(0)
	, memory_map(nullptr)
	, is_dirty(true)
{
	if (getMemoryBacking() == BACKING_FULL)
		memory_map = (char *) malloc(getSize());

	bool ok = load(false);

	if (!ok)
	{
		free(memory_map);
		throw love::Exception("Could not load VBO.");
	}
}

VertexBuffer::~VertexBuffer()
{
	if (vbo != 0)
		unload(false);

	if (memory_map)
		free(memory_map);
}

void *VertexBuffer::map()
{
	if (is_mapped)
		return memory_map;

	if (!memory_map)
	{
		memory_map = (char *) malloc(getSize());
		if (!memory_map)
			throw love::Exception("Out of memory (oh the humanity!)");
	}

	if (is_dirty)
	{
		glGetBufferSubData(getTarget(), 0, (GLsizeiptr) getSize(), memory_map);
		is_dirty = false;
	}

	is_mapped = true;

	return memory_map;
}

void VertexBuffer::unmapStatic(size_t offset, size_t size)
{
	// Upload the mapped data to the buffer.
	glBufferSubData(getTarget(), (GLintptr) offset, (GLsizeiptr) size, memory_map + offset);
}

void VertexBuffer::unmapStream()
{
	// "orphan" current buffer to avoid implicit synchronisation on the GPU:
	// http://www.seas.upenn.edu/~pcozzi/OpenGLInsights/OpenGLInsights-AsynchronousBufferTransfers.pdf
	glBufferData(getTarget(), (GLsizeiptr) getSize(), nullptr,    getUsage());
	glBufferData(getTarget(), (GLsizeiptr) getSize(), memory_map, getUsage());
}

void VertexBuffer::unmap(size_t usedOffset, size_t usedSize)
{
	if (!is_mapped)
		return;

	usedOffset = std::min(usedOffset, getSize());
	usedSize = std::min(usedSize, getSize() - usedOffset);

	// VBO::bind is a no-op when the VBO is mapped, so we have to make sure it's
	// bound here.
	if (!is_bound)
	{
		glBindBuffer(getTarget(), vbo);
		is_bound = true;
	}

	switch (getUsage())
	{
	case GL_STATIC_DRAW:
		unmapStatic(usedOffset, usedSize);
		break;
	case GL_STREAM_DRAW:
		unmapStream();
		break;
	case GL_DYNAMIC_DRAW:
	default:
		// It's probably more efficient to treat it like a streaming buffer if
		// more than a third of its contents have been modified during the map().
		if (usedSize >= getSize() / 3)
			unmapStream();
		else
			unmapStatic(usedOffset, usedSize);
		break;
	}

	is_mapped = false;
}

void VertexBuffer::bind()
{
	if (!is_mapped)
	{
		glBindBuffer(getTarget(), vbo);
		is_bound = true;
	}
}

void VertexBuffer::unbind()
{
	if (is_bound)
		glBindBuffer(getTarget(), 0);

	is_bound = false;
}

void VertexBuffer::fill(size_t offset, size_t size, const void *data)
{
	if (is_mapped || getMemoryBacking() == BACKING_FULL)
		memcpy(memory_map + offset, data, size);

	if (!is_mapped)
	{
		glBufferSubData(getTarget(), (GLintptr) offset, (GLsizeiptr) size, data);

		if (getMemoryBacking() != BACKING_FULL)
			is_dirty = true;
	}
}

const void *VertexBuffer::getPointer(size_t offset) const
{
	return BUFFER_OFFSET(offset);
}

bool VertexBuffer::loadVolatile()
{
	return load(true);
}

void VertexBuffer::unloadVolatile()
{
	unload(true);
}

bool VertexBuffer::load(bool restore)
{
	glGenBuffers(1, &vbo);

	VertexBuffer::Bind bind(*this);

	// Copy the old buffer only if 'restore' was requested.
	const GLvoid *src = restore ? memory_map : nullptr;

	while (GL_NO_ERROR != glGetError())
		/* clear error messages */;

	// Note that if 'src' is '0', no data will be copied.
	glBufferData(getTarget(), (GLsizeiptr) getSize(), src, getUsage());
	GLenum err = glGetError();

	return (GL_NO_ERROR == err);
}

void VertexBuffer::unload(bool save)
{
	// Save data before unloading, if we need to.
	if (save && getMemoryBacking() == BACKING_PARTIAL)
	{
		VertexBuffer::Bind bind(*this);

		bool mapped = is_mapped;

		map(); // saves buffer content to memory_map.
		is_mapped = mapped;
	}

	glDeleteBuffers(1, &vbo);
	vbo = 0;
}


// VertexIndex

size_t VertexIndex::maxSize = 0;
size_t VertexIndex::elementSize = 0;
std::list<size_t> VertexIndex::sizeRefs;
VertexBuffer *VertexIndex::element_array = NULL;

VertexIndex::VertexIndex(size_t size)
	: size(size)
{
	// The upper limit is the maximum of GLuint divided by six (the number
	// of indices per size) and divided by the size of GLuint. This guarantees
	// no overflows when calculating the array size in bytes.
	// Memory issues will be handled by other exceptions.
	if (size == 0 || size > ((GLuint) -1) / 6 / sizeof(GLuint))
		throw love::Exception("Invalid size.");

	addSize(size);
}

VertexIndex::~VertexIndex()
{
	removeSize(size);
}

size_t VertexIndex::getSize() const
{
	return size;
}

size_t VertexIndex::getIndexCount(size_t elements) const
{
	return elements * 6;
}

GLenum VertexIndex::getType(size_t s) const
{
	// Calculates if unsigned short is big enough to hold all the vertex indices.
	static const GLenum type_table[] = {GL_UNSIGNED_SHORT, GL_UNSIGNED_INT};
	return type_table[s * 4 > std::numeric_limits<GLushort>::max()];
	// if buffer-size > max(GLushort) then GL_UNSIGNED_INT else GL_UNSIGNED_SHORT
}

size_t VertexIndex::getElementSize()
{
	return elementSize;
}

VertexBuffer *VertexIndex::getVertexBuffer() const
{
	return element_array;
}

const void *VertexIndex::getPointer(size_t offset) const
{
	return element_array->getPointer(offset);
}

void VertexIndex::addSize(size_t newSize)
{
	if (newSize <= maxSize)
	{
		// Current size is bigger. Append the size to list and sort.
		sizeRefs.push_back(newSize);
		sizeRefs.sort();
		return;
	}

	// Try to resize before adding it to the list because resize may throw.
	resize(newSize);
	sizeRefs.push_back(newSize);
}

void VertexIndex::removeSize(size_t oldSize)
{
	// TODO: For debugging purposes, this should check if the size was actually found.
	sizeRefs.erase(std::find(sizeRefs.begin(), sizeRefs.end(), oldSize));
	if (sizeRefs.size() == 0)
	{
		resize(0);
		return;
	}

	if (oldSize == maxSize)
	{
		// Shrink if there's a smaller size.
		size_t newSize = sizeRefs.back();
		if (newSize < maxSize)
			resize(newSize);
	}
}

void VertexIndex::resize(size_t size)
{
	if (size == 0)
	{
		delete element_array;
		element_array = NULL;
		maxSize = 0;
		return;
	}

	VertexBuffer *new_element_array;
	
	// Depending on the size, a switch to int and more memory is needed.
	GLenum target_type = getType(size);
	size_t elem_size = (target_type == GL_UNSIGNED_SHORT) ? sizeof(GLushort) : sizeof(GLuint);

	size_t array_size = elem_size * 6 * size;

	// Create may throw out-of-memory exceptions.
	// VertexIndex will propagate the exception and keep the old VertexBuffer.
	try
	{
		new_element_array = VertexBuffer::Create(array_size, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);
	}
	catch (std::bad_alloc &)
	{
		throw love::Exception("Out of memory.");
	}

	// Allocation of the new VertexBuffer succeeded.
	// The old VertexBuffer can now be deleted.
	delete element_array;
	element_array = new_element_array;
	maxSize = size;
	elementSize = elem_size;

	switch (target_type)
	{
	case GL_UNSIGNED_SHORT:
		fill<GLushort>();
		break;
	case GL_UNSIGNED_INT:
		fill<GLuint>();
		break;
	}
}

template <typename T>
void VertexIndex::fill()
{
	VertexBuffer::Bind bind(*element_array);
	VertexBuffer::Mapper mapper(*element_array);

	T *indices = (T *) mapper.get();

	for (size_t i = 0; i < maxSize; ++i)
	{
		indices[i*6+0] = i * 4 + 0;
		indices[i*6+1] = i * 4 + 1;
		indices[i*6+2] = i * 4 + 2;

		indices[i*6+3] = i * 4 + 0;
		indices[i*6+4] = i * 4 + 2;
		indices[i*6+5] = i * 4 + 3;
	}
}

} // opengl
} // graphics
} // love
