/**
 * Copyright (c) 2006-2016 LOVE Development Team
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

#include "GLBuffer.h"

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

GLBuffer::GLBuffer(size_t size, const void *data, GLenum target, GLenum usage, uint32 mapflags)
	: is_bound(false)
	, is_mapped(false)
	, size(size)
	, target(target)
	, usage(usage)
	, vbo(0)
	, memory_map(nullptr)
	, modified_offset(0)
	, modified_size(0)
	, map_flags(mapflags)
{
	try
	{
		memory_map = new char[size];
	}
	catch (std::bad_alloc &)
	{
		throw love::Exception("Out of memory.");
	}

	if (data != nullptr)
		memcpy(memory_map, data, size);

	if (!load(data != nullptr))
	{
		delete[] memory_map;
		throw love::Exception("Could not load vertex buffer (out of VRAM?)");
	}
}

GLBuffer::~GLBuffer()
{
	if (vbo != 0)
		unload();

	delete[] memory_map;
}

void *GLBuffer::map()
{
	if (is_mapped)
		return memory_map;

	is_mapped = true;

	modified_offset = 0;
	modified_size = 0;

	return memory_map;
}

void GLBuffer::unmapStatic(size_t offset, size_t size)
{
	if (size == 0)
		return;

	// Upload the mapped data to the buffer.
	glBufferSubData(getTarget(), (GLintptr) offset, (GLsizeiptr) size, memory_map + offset);
}

void GLBuffer::unmapStream()
{
	// "orphan" current buffer to avoid implicit synchronisation on the GPU:
	// http://www.seas.upenn.edu/~pcozzi/OpenGLInsights/OpenGLInsights-AsynchronousBufferTransfers.pdf
	glBufferData(getTarget(), (GLsizeiptr) getSize(), nullptr,    getUsage());
	glBufferData(getTarget(), (GLsizeiptr) getSize(), memory_map, getUsage());
}

void GLBuffer::unmap()
{
	if (!is_mapped)
		return;

	if ((map_flags & MAP_EXPLICIT_RANGE_MODIFY) != 0)
	{
		modified_offset = std::min(modified_offset, getSize() - 1);
		modified_size = std::min(modified_size, getSize() - modified_offset);
	}
	else
	{
		modified_offset = 0;
		modified_size = getSize();
	}

	// VBO::bind is a no-op when the VBO is mapped, so we have to make sure it's
	// bound here.
	if (!is_bound)
	{
		glBindBuffer(getTarget(), vbo);
		is_bound = true;
	}

	if (modified_size > 0)
	{
		switch (getUsage())
		{
		case GL_STATIC_DRAW:
			unmapStatic(modified_offset, modified_size);
			break;
		case GL_STREAM_DRAW:
			unmapStream();
			break;
		case GL_DYNAMIC_DRAW:
		default:
			// It's probably more efficient to treat it like a streaming buffer if
			// at least a third of its contents have been modified during the map().
			if (modified_size >= getSize() / 3)
				unmapStream();
			else
				unmapStatic(modified_offset, modified_size);
			break;
		}
	}

	modified_offset = 0;
	modified_size = 0;

	is_mapped = false;
}

void GLBuffer::setMappedRangeModified(size_t offset, size_t modifiedsize)
{
	if (!is_mapped || !(map_flags & MAP_EXPLICIT_RANGE_MODIFY))
		return;

	// We're being conservative right now by internally marking the whole range
	// from the start of section a to the end of section b as modified if both
	// a and b are marked as modified.

	size_t old_range_end = modified_offset + modified_size;
	modified_offset = std::min(modified_offset, offset);

	size_t new_range_end = std::max(offset + modifiedsize, old_range_end);
	modified_size = new_range_end - modified_offset;
}

void GLBuffer::bind()
{
	if (!is_mapped)
	{
		glBindBuffer(getTarget(), vbo);
		is_bound = true;
	}
}

void GLBuffer::unbind()
{
	if (is_bound)
		glBindBuffer(getTarget(), 0);

	is_bound = false;
}

void GLBuffer::fill(size_t offset, size_t size, const void *data)
{
	memcpy(memory_map + offset, data, size);

	if (is_mapped)
		setMappedRangeModified(offset, size);
	else
		glBufferSubData(getTarget(), (GLintptr) offset, (GLsizeiptr) size, data);
}

const void *GLBuffer::getPointer(size_t offset) const
{
	return BUFFER_OFFSET(offset);
}

bool GLBuffer::loadVolatile()
{
	return load(true);
}

void GLBuffer::unloadVolatile()
{
	unload();
}

bool GLBuffer::load(bool restore)
{
	glGenBuffers(1, &vbo);

	GLBuffer::Bind bind(*this);

	while (glGetError() != GL_NO_ERROR)
		/* Clear the error buffer. */;

	// Copy the old buffer only if 'restore' was requested.
	const GLvoid *src = restore ? memory_map : nullptr;

	// Note that if 'src' is '0', no data will be copied.
	glBufferData(getTarget(), (GLsizeiptr) getSize(), src, getUsage());

	return (glGetError() == GL_NO_ERROR);
}

void GLBuffer::unload()
{
	is_mapped = false;

	glDeleteBuffers(1, &vbo);
	vbo = 0;
}


// QuadIndices

size_t QuadIndices::maxSize = 0;
size_t QuadIndices::elementSize = 0;
size_t QuadIndices::objectCount = 0;

GLBuffer *QuadIndices::indexBuffer = nullptr;
char *QuadIndices::indices = nullptr;

QuadIndices::QuadIndices(size_t size)
	: size(size)
{
	// The upper limit is the maximum of GLuint divided by six (the number
	// of indices per size) and divided by the size of GLuint. This guarantees
	// no overflows when calculating the array size in bytes.
	if (size == 0 || size > ((GLuint) -1) / 6 / sizeof(GLuint))
		throw love::Exception("Invalid number of quads.");

	// Create a new / larger buffer if needed.
	if (indexBuffer == nullptr || size > maxSize)
	{
		GLBuffer *newbuffer = nullptr;
		char *newindices = nullptr;

		// Depending on the size, a switch to int and more memory is needed.
		GLenum targettype = getType(size);
		size_t elemsize = (targettype == GL_UNSIGNED_SHORT) ? sizeof(GLushort) : sizeof(GLuint);

		size_t buffersize = elemsize * 6 * size;

		// Create may throw out-of-memory exceptions.
		// QuadIndices will propagate the exception and keep the old GLBuffer.
		try
		{
			newbuffer = new GLBuffer(buffersize, nullptr, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);
			newindices = new char[buffersize];
		}
		catch (std::bad_alloc &)
		{
			delete newbuffer;
			delete[] newindices;
			throw love::Exception("Out of memory.");
		}

		// Allocation of the new GLBuffer succeeded.
		// The old GLBuffer can now be deleted.
		delete indexBuffer;
		indexBuffer = newbuffer;

		delete[] indices;
		indices = newindices;

		maxSize = size;
		elementSize = elemsize;

		switch (targettype)
		{
		case GL_UNSIGNED_SHORT:
			fill<GLushort>();
			break;
		case GL_UNSIGNED_INT:
			fill<GLuint>();
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

GLenum QuadIndices::getType(size_t s) const
{
	// Calculates if unsigned short is big enough to hold all the vertex indices.
	static const GLenum types[] = {GL_UNSIGNED_SHORT, GL_UNSIGNED_INT};
	return types[s * 4 > std::numeric_limits<GLushort>::max()];
	// if buffer-size > max(GLushort) then GL_UNSIGNED_INT else GL_UNSIGNED_SHORT
}

size_t QuadIndices::getElementSize()
{
	return elementSize;
}

GLBuffer *QuadIndices::getBuffer() const
{
	return indexBuffer;
}

const void *QuadIndices::getPointer(size_t offset) const
{
	return indexBuffer->getPointer(offset);
}

const void *QuadIndices::getIndices(size_t offset) const
{
	return indices + offset;
}

template <typename T>
void QuadIndices::fill()
{
	T *inds = (T *) indices;

	// 0----2
	// |  / |
	// | /  |
	// 1----3
	for (size_t i = 0; i < maxSize; ++i)
	{
		inds[i*6+0] = T(i * 4 + 0);
		inds[i*6+1] = T(i * 4 + 1);
		inds[i*6+2] = T(i * 4 + 2);

		inds[i*6+3] = T(i * 4 + 2);
		inds[i*6+4] = T(i * 4 + 1);
		inds[i*6+5] = T(i * 4 + 3);
	}

	GLBuffer::Bind bind(*indexBuffer);
	indexBuffer->fill(0, indexBuffer->getSize(), indices);
}

} // opengl
} // graphics
} // love
