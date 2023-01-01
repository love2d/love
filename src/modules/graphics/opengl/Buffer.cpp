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

#include "Buffer.h"

#include "common/Exception.h"
#include "graphics/vertex.h"

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

Buffer::Buffer(size_t size, const void *data, BufferType type, vertex::Usage usage, uint32 mapflags)
	: love::graphics::Buffer(size, type, usage, mapflags)
	, vbo(0)
	, memory_map(nullptr)
	, modified_start(std::numeric_limits<size_t>::max())
	, modified_end(0)
{
	target = OpenGL::getGLBufferType(type);

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

Buffer::~Buffer()
{
	if (vbo != 0)
		unload();

	delete[] memory_map;
}

void *Buffer::map()
{
	if (is_mapped)
		return memory_map;

	is_mapped = true;

	modified_start = std::numeric_limits<size_t>::max();
	modified_end = 0;

	return memory_map;
}

void Buffer::unmapStatic(size_t offset, size_t size)
{
	if (size == 0)
		return;

	// Upload the mapped data to the buffer.
	gl.bindBuffer(type, vbo);
	glBufferSubData(target, (GLintptr) offset, (GLsizeiptr) size, memory_map + offset);
}

void Buffer::unmapStream()
{
	GLenum glusage = OpenGL::getGLBufferUsage(getUsage());

	// "orphan" current buffer to avoid implicit synchronisation on the GPU:
	// http://www.seas.upenn.edu/~pcozzi/OpenGLInsights/OpenGLInsights-AsynchronousBufferTransfers.pdf
	gl.bindBuffer(type, vbo);
	glBufferData(target, (GLsizeiptr) getSize(), nullptr, glusage);

#if LOVE_WINDOWS
	// TODO: Verify that this codepath is a useful optimization.
	if (gl.getVendor() == OpenGL::VENDOR_INTEL)
		glBufferData(target, (GLsizeiptr) getSize(), memory_map, glusage);
	else
#endif
		glBufferSubData(target, 0, (GLsizeiptr) getSize(), memory_map);
}

void Buffer::unmap()
{
	if (!is_mapped)
		return;

	if ((map_flags & MAP_EXPLICIT_RANGE_MODIFY) != 0)
	{
		if (modified_end >= modified_start)
		{
			modified_start = std::min(modified_start, getSize() - 1);
			modified_end = std::min(modified_end, getSize() - 1);
		}
	}
	else
	{
		modified_start = 0;
		modified_end = getSize() - 1;
	}

	if (modified_end >= modified_start)
	{
		size_t modified_size = (modified_end - modified_start) + 1;
		switch (getUsage())
		{
		case vertex::USAGE_STATIC:
			unmapStatic(modified_start, modified_size);
			break;
		case vertex::USAGE_STREAM:
			unmapStream();
			break;
		case vertex::USAGE_DYNAMIC:
		default:
			// It's probably more efficient to treat it like a streaming buffer if
			// at least a third of its contents have been modified during the map().
			if (modified_size >= getSize() / 3)
				unmapStream();
			else
				unmapStatic(modified_start, modified_size);
			break;
		}
	}

	modified_start = std::numeric_limits<size_t>::max();
	modified_end = 0;

	is_mapped = false;
}

void Buffer::setMappedRangeModified(size_t offset, size_t modifiedsize)
{
	if (!is_mapped || !(map_flags & MAP_EXPLICIT_RANGE_MODIFY))
		return;

	// We're being conservative right now by internally marking the whole range
	// from the start of section a to the end of section b as modified if both
	// a and b are marked as modified.
	modified_start = std::min(modified_start, offset);
	modified_end = std::max(modified_end, offset + modifiedsize - 1);
}

void Buffer::fill(size_t offset, size_t size, const void *data)
{
	memcpy(memory_map + offset, data, size);

	if (is_mapped)
		setMappedRangeModified(offset, size);
	else
	{
		gl.bindBuffer(type, vbo);
		glBufferSubData(target, (GLintptr) offset, (GLsizeiptr) size, data);
	}
}

ptrdiff_t Buffer::getHandle() const
{
	return vbo;
}

void Buffer::copyTo(size_t offset, size_t size, love::graphics::Buffer *other, size_t otheroffset)
{
	other->fill(otheroffset, size, memory_map + offset);
}

bool Buffer::loadVolatile()
{
	return load(true);
}

void Buffer::unloadVolatile()
{
	unload();
}

bool Buffer::load(bool restore)
{
	glGenBuffers(1, &vbo);
	gl.bindBuffer(type, vbo);

	while (glGetError() != GL_NO_ERROR)
		/* Clear the error buffer. */;

	// Copy the old buffer only if 'restore' was requested.
	const GLvoid *src = restore ? memory_map : nullptr;

	// Note that if 'src' is '0', no data will be copied.
	glBufferData(target, (GLsizeiptr) getSize(), src, OpenGL::getGLBufferUsage(getUsage()));

	return (glGetError() == GL_NO_ERROR);
}

void Buffer::unload()
{
	is_mapped = false;
	gl.deleteBuffer(vbo);
	vbo = 0;
}

} // opengl
} // graphics
} // love
