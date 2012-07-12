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


#ifndef LOVE_GRAPHICS_OPENGL_VERTEX_BUFFER_H
#define LOVE_GRAPHICS_OPENGL_VERTEX_BUFFER_H

// LOVE
#include "graphics/Volatile.h"

// OpenGL
#include "GLee.h"

namespace love
{
namespace graphics
{
namespace opengl
{

/**
 * VertexBuffer is an abstraction over VBOs (Vertex Buffer Objects), which
 * falls back to regular vertex arrays if VBOs are not supported.
 *
 * This allows code to take advantage of VBOs where available, but still
 * work on older systems where it's *not* available. Everyone's happy.
 *
 * The class is (for now) meant for internal use.
 */
class VertexBuffer
{
public:

	/**
	 * Create a new VertexBuffer (either a plain vertex array, or a VBO),
	 * based on what's supported on the system.
	 *
	 * If VBOs are not supported, a plain vertex array will automatically
	 * be created and returned instead.
	 *
	 * @param size The size of the VertexBuffer (in bytes).
	 * @param target GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER.
	 * @param usage GL_DYNAMIC_DRAW, etc.
	 * @return A new VertexBuffer.
	 */
	static VertexBuffer *Create(size_t size, GLenum target, GLenum usage);

	/**
	 * Constructor.
	 *
	 * @param size The size of the VertexBuffer in bytes.
	 * @param target The target VertexBuffer object, e.g. GL_ARRAY_BUFFER.
	 * @param usage Usage hint, e.g. GL_DYNAMIC_DRAW.
	 */
	VertexBuffer(size_t size, GLenum target, GLenum usage);

	/**
	 * Destructor. Does nothing, but must be declared virtual.
	 */
	virtual ~VertexBuffer();

	/**
	 * Get the size of the VertexBuffer, in bytes.
	 *
	 * @return The size of the VertexBuffer.
	 */
	size_t getSize() const
	{
		return size;
	}

	/**
	 * Get the target buffer object.
	 *
	 * @return The target buffer object, e.g. GL_ARRAY_BUFFER.
	 */
	GLenum getTarget() const
	{
		return target;
	}

	/**
	 * Get the usage hint for this VertexBuffer.
	 *
	 * @return The usage hint, e.g. GL_DYNAMIC_DRAW.
	 */
	GLenum getUsage() const
	{
		return usage;
	}

	/**
	 * Map the VertexBuffer to client memory.
	 *
	 * This can be faster for large changes to the buffer. For smaller
	 * changes, see fill().
	 *
	 * The VertexBuffer must be bound to use this function.
	 *
	 * @return A pointer to memory which represents the buffer.
	 */
	virtual void *map() = 0;

	/**
	 * Unmap a previously mapped VertexBuffer. The buffer must be unmapped
	 * when used to draw elements.
	 *
	 * The VertexBuffer must be bound to use this function.
	 */
	virtual void unmap() = 0;

	/**
	 * Bind the VertexBuffer to the specified target.
	 * (GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER).
	 *
	 * @param target GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER.
	 */
	virtual void bind() = 0;

	/**
	 * Unbind a prevously bound VertexBuffer.
	 */
	virtual void unbind() = 0;

	/**
	 * Fill a portion of the buffer with data.
	 *
	 * The VertexBuffer must be bound to use this function.
	 *
	 * @param offset The offset in the VertexBuffer to store the data.
	 * @param size The size of the incoming data.
	 * @param data Pointer to memory to copy data from.
	 */
	virtual void fill(size_t offset, size_t size, const void *data) = 0;

	/**
	 * Get a pointer which represents the specified byte offset.
	 *
	 * @param offset The byte offset. (0 is first byte).
	 * @return A pointer which represents the offset.
	 */
	virtual const void *getPointer(size_t offset) const = 0;

	/**
	 * This helper class can bind a VertexArray temporarily, and
	 * automatically un-bind when it's destroyed.
	 */
	class Bind
	{
	public:

		/**
		 * Bind a VertexBuffer.
		 */
		Bind(VertexBuffer &buf)
			: buf(buf)
		{
			buf.bind();
		}

		/**
		 * Unbinds a VertexBuffer.
		 */
		~Bind()
		{
			buf.unbind();
		}

	private:

		// VertexBuffer to work on.
		VertexBuffer &buf;
	};

	class Mapper
	{
	public:
		/**
		 * Memory-maps a VertexBuffer.
		 */
		Mapper(VertexBuffer &buffer)
			: buf(buffer)
		{
			elems = buf.map();
		}

		/**
		 * unmaps the buffer
		 */
		~Mapper()
		{
			buf.unmap();
		}

		/**
		 * Get pointer to memory mapped region
		 */
		void *get()
		{
			return elems;
		}

	private:
		VertexBuffer &buf;
		void *elems;
	};

private:

	// The size of the buffer, in bytes.
	size_t size;

	// The target buffer object. (GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER).
	GLenum target;

	// Usage hint. GL_[DYNAMIC, STATIC, STREAM]_DRAW.
	GLenum usage;
};

/**
 * Implementation of VertexBuffer which uses plain arrays to store the data.
 *
 * This implementation should be supported everywhere, and acts as a fallback
 * on systems which do not support VBOs.
 */
class VertexArray : public VertexBuffer
{
public:

	/**
	 * @copydoc VertexBuffer(int, GLenum, GLenum)
	 */
	VertexArray(size_t size, GLenum target, GLenum usage);

	/**
	 * Frees the data we've allocated.
	 */
	virtual ~VertexArray();

	// Implements VertexBuffer.
	virtual void *map();
	virtual void unmap();
	virtual void bind();
	virtual void unbind();
	virtual void fill(size_t offset, size_t size, const void *data);
	virtual const void *getPointer(size_t offset) const ;

private:
	// Holds the data.
	char *buf;
};

/**
 * Vertex Buffer Object (VBO) implementation of VertexBuffer.
 *
 * This will be used on all systems that support it. It's in general
 * faster than vertex arrays, but especially in use-cases where there
 * is no need to update the data every frame.
 */
class VBO : public VertexBuffer, public Volatile
{
public:

	/**
	 * @copydoc VertexBuffer(size_t, GLenum, GLenum)
	 */
	VBO(size_t size, GLenum target, GLenum usage);

	/**
	 * Deletes the VBOs from OpenGL.
	 */
	virtual ~VBO();

	// Implements VertexBuffer.
	virtual void *map();
	virtual void unmap();
	virtual void bind();
	virtual void unbind();
	virtual void fill(size_t offset, size_t size, const void *data);
	virtual const void *getPointer(size_t offset) const ;

	// Implements Volatile.
	bool loadVolatile();
	void unloadVolatile();

private:

	/**
	 * Creates the VBO, and optionally restores data we saved earlier.
	 *
	 * @param restore True to restore data previously saved with 'unload'.
	 * @return True on success, false otherwise.
	 */
	bool load(bool restore);

	/**
	 * Optionally save the data in the VBO, then delete it.
	 *
	 * @param save True to save the data before deleting.
	 */
	void unload(bool save);

	// The VBO identifier. Assigned by OpenGL.
	GLuint vbo;

	// *May* contain data saved by 'unload'.
	char *buffer_copy;

	// A pointer to mapped memory. Zero if memory is currently
	// not mapped.
	void *mapped;

	// Usage hint for map()/unmap() pair. Same as `access' parameter in
	// glBufferData or 0 if not mapped.
	GLenum mapped_access;
};

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_SPRITE_BATCH_H
