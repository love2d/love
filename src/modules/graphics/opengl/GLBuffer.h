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


#ifndef LOVE_GRAPHICS_OPENGL_GL_BUFFER_H
#define LOVE_GRAPHICS_OPENGL_GL_BUFFER_H

// LOVE
#include "common/config.h"
#include "graphics/Volatile.h"

// OpenGL
#include "OpenGL.h"

// C
#include <stddef.h>

namespace love
{
namespace graphics
{
namespace opengl
{

/**
 * GLBuffer is a thin abstraction over OpenGL's Buffer Objects.
 * The class is meant for internal use.
 */
class GLBuffer : public Volatile
{
public:

	enum MapFlags
	{
		MAP_EXPLICIT_RANGE_MODIFY = 0x01, // see setMappedRangeModified.
	};

	/**
	 * Constructor.
	 *
	 * @param size The size of the GLBuffer in bytes.
	 * @param target The target GLBuffer object, e.g. GL_ARRAY_BUFFER.
	 * @param usage Usage hint, e.g. GL_DYNAMIC_DRAW.
	 */
	GLBuffer(size_t size, const void *data, GLenum target, GLenum usage, uint32 mapflags = 0);

	/**
	 * Destructor.
	 */
	virtual ~GLBuffer();

	/**
	 * Get the size of the GLBuffer, in bytes.
	 *
	 * @return The size of the GLBuffer.
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
	 * Get the usage hint for this GLBuffer.
	 *
	 * @return The usage hint, e.g. GL_DYNAMIC_DRAW.
	 */
	GLenum getUsage() const
	{
		return usage;
	}

	bool isBound() const
	{
		return is_bound;
	}

	bool isMapped() const
	{
		return is_mapped;
	}

	/**
	 * Map the GLBuffer to client memory.
	 *
	 * This can be faster for large changes to the buffer. For smaller
	 * changes, see fill().
	 *
	 * The GLBuffer must be bound to use this function.
	 *
	 * @return A pointer to memory which represents the buffer.
	 */
	void *map();

	/**
	 * Unmap a previously mapped GLBuffer. The buffer must be unmapped
	 * when used to draw elements.
	 *
	 * The GLBuffer must be bound to use this function.
	 */
	void unmap();

	/**
	 * Marks a range of mapped data as modified.
	 * NOTE: GLBuffer::fill calls this internally for you.
	 **/
	void setMappedRangeModified(size_t offset, size_t size);

	/**
	 * Bind the GLBuffer to its specified target.
	 * (GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, etc).
	 */
	void bind();

	/**
	 * Unbind a prevously bound GLBuffer.
	 */
	void unbind();

	/**
	 * Fill a portion of the buffer with data and marks the range as modified.
	 *
	 * The GLBuffer must be bound to use this function.
	 *
	 * @param offset The offset in the GLBuffer to store the data.
	 * @param size The size of the incoming data.
	 * @param data Pointer to memory to copy data from.
	 */
	void fill(size_t offset, size_t size, const void *data);

	/**
	 * Get a pointer which represents the specified byte offset.
	 *
	 * @param offset The byte offset. (0 is first byte).
	 * @return A pointer which represents the offset.
	 */
	const void *getPointer(size_t offset) const;

	uint32 getMapFlags() const
	{
		return map_flags;
	}

	// Implements Volatile.
	bool loadVolatile() override;
	void unloadVolatile() override;

	/**
	 * This helper class can bind a GLBuffer temporarily, and
	 * automatically un-bind when it's destroyed.
	 */
	class Bind
	{
	public:

		/**
		 * Bind a GLBuffer.
		 */
		Bind(GLBuffer &buf)
			: buf(buf)
		{
			buf.bind();
		}

		/**
		 * Unbinds a GLBuffer.
		 */
		~Bind()
		{
			buf.unbind();
		}

	private:

		// GLBuffer to work on.
		GLBuffer &buf;

	}; // Bind

	class Mapper
	{
	public:

		/**
		 * Memory-maps a GLBuffer.
		 */
		Mapper(GLBuffer &buffer)
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

		GLBuffer &buf;
		void *elems;

	}; // Mapper

private:

	/**
	 * Creates the VBO, and optionally restores data we saved earlier.
	 *
	 * @param restore True to restore data previously saved with 'unload'.
	 * @return True on success, false otherwise.
	 */
	bool load(bool restore);
	void unload();

	void unmapStatic(size_t offset, size_t size);
	void unmapStream();

	// Whether the buffer is currently bound.
	bool is_bound;

	// Whether the buffer is currently mapped to main memory.
	bool is_mapped;

	// The size of the buffer, in bytes.
	size_t size;

	// The target buffer object. (GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER).
	GLenum target;

	// Usage hint. GL_[DYNAMIC, STATIC, STREAM]_DRAW.
	GLenum usage;

	// The VBO identifier. Assigned by OpenGL.
	GLuint vbo;

	// A pointer to mapped memory.
	char *memory_map;

	size_t modified_offset;
	size_t modified_size;

	uint32 map_flags;

}; // GLBuffer


/**
 * QuadIndices manages one shared GLBuffer that stores the indices for an
 * element array. Vertex arrays using the vertex structure (or anything else
 * that can use the pattern below) can request a size and use it for the
 * drawElements call.
 *
 *  indices[i*6 + 0] = i*4 + 0;
 *  indices[i*6 + 1] = i*4 + 1;
 *  indices[i*6 + 2] = i*4 + 2;
 *
 *  indices[i*6 + 3] = i*4 + 2;
 *  indices[i*6 + 4] = i*4 + 1;
 *  indices[i*6 + 5] = i*4 + 3;
 *
 * There will always be a large enough GLBuffer around until all
 * QuadIndices instances have been deleted.
 *
 * Q: Why have something like QuadIndices?
 * A: The indices for the SpriteBatch do not change, only the array size
 * varies. Using one GLBuffer for all element arrays removes this
 * duplicated data and saves some memory.
 */
class QuadIndices
{
public:
	/**
	 * Adds an entry to the list of sizes and resizes the GLBuffer
	 * if needed. A size of 1 allocates a group of 6 indices for 4 vertices
	 * creating 1 face.
	 *
	 * @param size The requested size in groups of 6 indices.
	 */
	QuadIndices(size_t size);

	QuadIndices(const QuadIndices &other);
	QuadIndices &operator = (const QuadIndices &other);

	/**
	 * Removes an entry from the list of sizes and resizes the GLBuffer
	 * if needed.
	 */
	~QuadIndices();

	/**
	 * Returns the number of index groups.
	 * This can be used for getIndexCount to get the full count of indices.
	 *
	 * @return The number of index groups.
	 */
	size_t getSize() const;

	/**
	 * Returns the number of indices that the passed element count will have.
	 * Use QuadIndices::getSize to get the full index count for that
	 * QuadIndices instance.
	 *
	 * @param elements The number of elements to calculate the index count for.
	 * @return The index count.
	 */
	size_t getIndexCount(size_t elements) const;

	/**
	 * Returns the integer type of the element array.
	 * If an optional nonzero size argument is passed, the function returns
	 * the integer type of the element array of that size.
	 *
	 * @param s The size of the array to calculated the integer type of.
	 * @return The element array integer type.
	 */
	GLenum getType(size_t s) const;
	inline GLenum getType() const
	{
		return getType(maxSize);
	}

	/**
	 * Returns the size in bytes of an element in the element array.
	 * Can be used with getPointer to calculate an offset into the array based
	 * on a number of elements.
	 *
	 * @return The size of an element in bytes.
	 **/
	size_t getElementSize();

	/**
	 * Returns the pointer to the GLBuffer.
	 * The pointer will change if a new size request or removal causes
	 * a GLBuffer resize. It is recommended to retrieve the pointer
	 * value directly before the drawing call.
	 *
	 * @return The pointer to the GLBuffer.
	 */
	GLBuffer *getBuffer() const;

	/**
	 * Returns a pointer which represents the specified byte offset.
	 *
	 * @param offset The offset in bytes.
	 * @return A pointer which represents the offset.
	 */
	const void *getPointer(size_t offset) const;

	/**
	 * Returns a direct pointer to the index data.
	 *
	 * At least one graphics driver (the one for Kepler nvidia GPUs in OS X)
	 * fails to render geometry if the vertex data was a direct CPU pointer but
	 * the index data came from an Index Buffer.
	 * So the direct pointer to the index buffer should be used instead of the
	 * index buffer when rendering using client-side vertex arrays.
	 *
	 * @param offset An offset in bytes into the index data.
	 * @return A direct pointer to the index data at the specified offset.
	 **/
	const void *getIndices(size_t offset) const;

private:

	/**
	 * Adds all indices to the array with the type T.
	 * There are no checks for the correct types or overflows. The calling
	 * function should check for that.
	 */
	template <typename T> void fill();

	// The size of the array requested by this instance.
	size_t size;

	// The size in bytes of an element in the element array.
	static size_t elementSize;

	// The current GLBuffer size. 0 means no GLBuffer.
	static size_t maxSize;

	static size_t objectCount;

	// The GLBuffer for the element array. Can be null.
	static GLBuffer *indexBuffer;

	// The array of indices that will also be stored in the index buffer.
	static char *indices;
};

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_GL_BUFFER_H
