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
#include <common/config.h>

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
		catch (const love::Exception &)
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

	// VertexBuffer::Bind

	VertexBuffer::Bind::Bind(VertexBuffer &buf)
		: buf(buf)
	{
		buf.bind();
	}

	VertexBuffer::Bind::~Bind()
	{
		buf.unbind();
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

	void *VertexArray::map(GLenum/* access*/)
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
		, buffer_copy(0)
		, mapped(0)
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
	}

	void *VBO::map(GLenum access)
	{
		// Don't map twice.
		if (mapped)
			return mapped;

		mapped = glMapBufferARB(getTarget(), access);

		return mapped;
	}

	void VBO::unmap()
	{
		glUnmapBufferARB(getTarget());
		mapped = 0;
	}

	void VBO::bind()
	{
		glBindBufferARB(getTarget(), vbo);
	}

	void VBO::unbind()
	{
		glBindBufferARB(getTarget(), 0);
	}

	void VBO::fill(size_t offset, size_t size, const void *data)
	{
		if (mapped)
			memcpy(static_cast<char*>(mapped) + offset, data, size);
		else
			glBufferSubDataARB(getTarget(), offset, size, data);
	}

	const void *VBO::getPointer(size_t offset) const
	{
		return reinterpret_cast<const void*>(offset);
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
		const GLvoid *src = restore ? buffer_copy : 0;

		while (GL_NO_ERROR != glGetError())
			/* clear error messages */;

		// Note that if 'src' is '0', no data will be copied.
		glBufferDataARB(getTarget(), getSize(), src, getUsage());
		GLenum err = glGetError();

		// Clean up buffer_copy, if it exists.
		delete[] buffer_copy;
		buffer_copy = 0;

		return (GL_NO_ERROR == err);
	}

	void VBO::unload(bool save)
	{
		// Clean up buffer_copy, if it exists.
		delete[] buffer_copy;
		buffer_copy = 0;

		// Save data before unloading.
		if (save)
		{
			VertexBuffer::Bind bind(*this);

			GLint size;
			glGetBufferParameterivARB(getTarget(), GL_BUFFER_SIZE, &size);

			const char *src = static_cast<char *>(map(GL_READ_ONLY));

			if (src)
			{
				buffer_copy = new char[size];
				memcpy(buffer_copy, src, size);
				unmap();
			}
		}

		glDeleteBuffers(1, &vbo);
		vbo = 0;
	}

} // opengl
} // graphics
} // love
