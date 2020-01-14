/**
 * Copyright (c) 2006-2020 LOVE Development Team
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

Buffer::Buffer(size_t size, const void *data, BufferTypeFlags typeflags, BufferUsage usage, uint32 mapflags)
	: love::graphics::Buffer(size, typeflags, usage, mapflags)
	, vbo(0)
	, memoryMap(nullptr)
	, modifiedOffset(0)
	, modifiedSize(0)
{
	if (typeflags & BUFFERFLAG_VERTEX)
		mapType = BUFFER_VERTEX;
	else if (typeflags & BUFFERFLAG_INDEX)
		mapType = BUFFER_INDEX;
	else if (mapflags & BUFFERFLAG_UNIFORM)
		mapType = BUFFER_UNIFORM;
	else if (mapflags & BUFFERFLAG_SHADER_STORAGE)
		mapType = BUFFER_SHADER_STORAGE;

	target = OpenGL::getGLBufferType(mapType);

	try
	{
		memoryMap = new char[size];
	}
	catch (std::bad_alloc &)
	{
		throw love::Exception("Out of memory.");
	}

	if (data != nullptr)
		memcpy(memoryMap, data, size);

	if (!load(data != nullptr))
	{
		delete[] memoryMap;
		throw love::Exception("Could not load vertex buffer (out of VRAM?)");
	}
}

Buffer::~Buffer()
{
	if (vbo != 0)
		unload();

	delete[] memoryMap;
}

void *Buffer::map()
{
	if (mapped)
		return memoryMap;

	mapped = true;

	modifiedOffset = 0;
	modifiedSize = 0;

	return memoryMap;
}

void Buffer::unmapStatic(size_t offset, size_t size)
{
	if (size == 0)
		return;

	// Upload the mapped data to the buffer.
	gl.bindBuffer(mapType, vbo);
	glBufferSubData(target, (GLintptr) offset, (GLsizeiptr) size, memoryMap + offset);
}

void Buffer::unmapStream()
{
	GLenum glusage = OpenGL::getGLBufferUsage(getUsage());

	// "orphan" current buffer to avoid implicit synchronisation on the GPU:
	// http://www.seas.upenn.edu/~pcozzi/OpenGLInsights/OpenGLInsights-AsynchronousBufferTransfers.pdf
	gl.bindBuffer(mapType, vbo);
	glBufferData(target, (GLsizeiptr) getSize(), nullptr, glusage);

#if LOVE_WINDOWS
	// TODO: Verify that this codepath is a useful optimization.
	if (gl.getVendor() == OpenGL::VENDOR_INTEL)
		glBufferData(target, (GLsizeiptr) getSize(), memoryMap, glusage);
	else
#endif
		glBufferSubData(target, 0, (GLsizeiptr) getSize(), memoryMap);
}

void Buffer::unmap()
{
	if (!mapped)
		return;

	if ((mapFlags & MAP_EXPLICIT_RANGE_MODIFY) != 0)
	{
		modifiedOffset = std::min(modifiedOffset, getSize() - 1);
		modifiedSize = std::min(modifiedSize, getSize() - modifiedOffset);
	}
	else
	{
		modifiedOffset = 0;
		modifiedSize = getSize();
	}

	if (modifiedSize > 0)
	{
		switch (getUsage())
		{
		case BUFFERUSAGE_STATIC:
			unmapStatic(modifiedOffset, modifiedSize);
			break;
		case BUFFERUSAGE_STREAM:
			unmapStream();
			break;
		case BUFFERUSAGE_DYNAMIC:
		default:
			// It's probably more efficient to treat it like a streaming buffer if
			// at least a third of its contents have been modified during the map().
			if (modifiedSize >= getSize() / 3)
				unmapStream();
			else
				unmapStatic(modifiedOffset, modifiedSize);
			break;
		}
	}

	modifiedOffset = 0;
	modifiedSize = 0;

	mapped = false;
}

void Buffer::setMappedRangeModified(size_t offset, size_t modifiedsize)
{
	if (!mapped || !(mapFlags & MAP_EXPLICIT_RANGE_MODIFY))
		return;

	// We're being conservative right now by internally marking the whole range
	// from the start of section a to the end of section b as modified if both
	// a and b are marked as modified.

	size_t oldrangeend = modifiedOffset + modifiedSize;
	modifiedOffset = std::min(modifiedOffset, offset);

	size_t newrangeend = std::max(offset + modifiedsize, oldrangeend);
	modifiedSize = newrangeend - modifiedOffset;
}

void Buffer::fill(size_t offset, size_t size, const void *data)
{
	memcpy(memoryMap + offset, data, size);

	if (mapped)
		setMappedRangeModified(offset, size);
	else
	{
		gl.bindBuffer(mapType, vbo);
		glBufferSubData(target, (GLintptr) offset, (GLsizeiptr) size, data);
	}
}

ptrdiff_t Buffer::getHandle() const
{
	return vbo;
}

void Buffer::copyTo(size_t offset, size_t size, love::graphics::Buffer *other, size_t otheroffset)
{
	other->fill(otheroffset, size, memoryMap + offset);
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
	gl.bindBuffer(mapType, vbo);

	while (glGetError() != GL_NO_ERROR)
		/* Clear the error buffer. */;

	// Copy the old buffer only if 'restore' was requested.
	const GLvoid *src = restore ? memoryMap : nullptr;

	// Note that if 'src' is '0', no data will be copied.
	glBufferData(target, (GLsizeiptr) getSize(), src, OpenGL::getGLBufferUsage(getUsage()));

	return (glGetError() == GL_NO_ERROR);
}

void Buffer::unload()
{
	mapped = false;
	gl.deleteBuffer(vbo);
	vbo = 0;
}

} // opengl
} // graphics
} // love
