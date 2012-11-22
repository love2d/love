/**
 * Copyright (c) 2006-2012 LOVE Development Team
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
#include "common/config.h"

#include <cstdlib>
#include <cstring>

namespace love
{
namespace graphics
{
namespace opengl
{

// VertexBuffer

VertexBuffer *VertexBuffer::Create(size_t size, GLenum target, GLenum usage)
{
	try
	{
		// Try to create a VBO.
		return new VBO(size, target, usage);
	}
	catch(const love::Exception &)
	{
		// VBO not supported ... create regular array.
		return new VertexArray(size, target, usage);
	}
}

VertexBuffer::VertexBuffer(size_t size, GLenum target, GLenum usage)
	: size(size)
	, target(target)
	, usage(usage)
{
}

VertexBuffer::~VertexBuffer()
{
}

// VertexArray

VertexArray::VertexArray(size_t size, GLenum target, GLenum usage)
	: VertexBuffer(size, target, usage)
	, buf(new char[size])
{
}

VertexArray::~VertexArray()
{
	delete [] buf;
}

void *VertexArray::map()
{
	return buf;
}

void VertexArray::unmap()
{
}

void VertexArray::bind()
{
}

void VertexArray::unbind()
{
}

void VertexArray::fill(size_t offset, size_t size, const void *data)
{
	memcpy(buf + offset, data, size);
}

const void *VertexArray::getPointer(size_t offset) const
{
	return buf + offset;
}

// VBO

VBO::VBO(size_t size, GLenum target, GLenum usage)
	: VertexBuffer(size, target, usage)
	, vbo(0)
	, memory_map(0)
	, is_mapped(false)
	, is_dirty(true)
{
	if (!(GLEE_ARB_vertex_buffer_object || GLEE_VERSION_1_5))
		throw love::Exception("Not supported");

	bool ok = load(false);

	if (!ok)
		throw love::Exception("Could not load VBO.");
}

VBO::~VBO()
{
	if (vbo != 0)
		unload(false);

	if (memory_map)
		free(memory_map);
}

void *VBO::map()
{
	if (is_mapped)
		return memory_map;

	if (!memory_map)
	{
		memory_map = malloc(getSize());
		if (!memory_map)
			throw love::Exception("Out of memory (oh the humanity!)");
	}

	if (is_dirty)
		glGetBufferSubDataARB(getTarget(), 0, getSize(), memory_map);

	is_mapped = true;
	is_dirty = false;

	return memory_map;
}

void VBO::unmap()
{
	if (!is_mapped)
		return;

	// "orphan" current buffer to avoid implicit synchronisation on the gpu:
	// http://www.seas.upenn.edu/~pcozzi/OpenGLInsights/OpenGLInsights-AsynchronousBufferTransfers.pdf)
	glBufferDataARB(getTarget(), getSize(), NULL, getUsage());
	glBufferSubDataARB(getTarget(), 0, getSize(), memory_map);
	is_mapped = false;
}

void VBO::bind()
{
	if (!is_mapped)
		glBindBufferARB(getTarget(), vbo);
}

void VBO::unbind()
{
	if (!is_mapped)
		glBindBufferARB(getTarget(), 0);
}

void VBO::fill(size_t offset, size_t size, const void *data)
{
	if (is_mapped)
	{
		memcpy(static_cast<char *>(memory_map) + offset, data, size);
	}
	else
	{
		glBufferSubDataARB(getTarget(), offset, size, data);
		is_dirty = true;
	}
}

const void *VBO::getPointer(size_t offset) const
{
	return reinterpret_cast<const void *>(offset);
}

bool VBO::loadVolatile()
{
	return load(true);
}

void VBO::unloadVolatile()
{
	unload(true);
}

bool VBO::load(bool restore)
{
	glGenBuffersARB(1, &vbo);

	VertexBuffer::Bind bind(*this);

	// Copy the old buffer only if 'restore' was requested.
	const GLvoid *src = restore ? memory_map : 0;

	while (GL_NO_ERROR != glGetError())
		/* clear error messages */;

	// Note that if 'src' is '0', no data will be copied.
	glBufferDataARB(getTarget(), getSize(), src, getUsage());
	GLenum err = glGetError();

	return (GL_NO_ERROR == err);
}

void VBO::unload(bool save)
{
	// Save data before unloading.
	if (save)
	{
		VertexBuffer::Bind bind(*this);

		GLint size;
		glGetBufferParameterivARB(getTarget(), GL_BUFFER_SIZE, &size);

		map(); // saves buffer content to memory_map.
		unmap();
	}

	glDeleteBuffers(1, &vbo);
	vbo = 0;
}

} // opengl
} // graphics
} // love
