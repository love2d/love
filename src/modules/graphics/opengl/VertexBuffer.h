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


#ifndef LOVE_GRAPHICS_OPENGL_VERTEX_BUFFER_H
#define LOVE_GRAPHICS_OPENGL_VERTEX_BUFFER_H

// LOVE
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

	// Different guarantees for VertexBuffer data storage.
	enum MemoryBacking
	{
		// The VertexBuffer is will have a valid copy of its data in main memory
		// at all times.
		BACKING_FULL,

		// The VertexBuffer will have a valid copy of its data in main memory
		// when it needs to be reloaded and when it's mapped.
		BACKING_PARTIAL
	};

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
	 * @param backing Determines what guarantees are placed on the data.
	 * @return A new VertexBuffer.
	 */
	static VertexBuffer *Create(size_t size, GLenum target, GLenum usage, MemoryBacking backing = BACKING_PARTIAL);

	/**
	 * Constructor.
	 *
	 * @param size The size of the VertexBuffer in bytes.
	 * @param target The target VertexBuffer object, e.g. GL_ARRAY_BUFFER.
	 * @param usage Usage hint, e.g. GL_DYNAMIC_DRAW.
	 * @param backing Determines what guarantees are placed on the data.
	 */
	VertexBuffer(size_t size, GLenum target, GLenum usage, MemoryBacking backing = BACKING_PARTIAL);

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

	bool isBound() const
	{
		return is_bound;
	}

	bool isMapped() const
	{
		return is_mapped;
	}

	MemoryBacking getMemoryBacking() const
	{
		return backing;
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
	 *
	 * @param usedOffset The offset into the mapped buffer indicating the
	 *                   sub-range of data modified. Optional.
	 * @param usedSize   The size of the sub-range of modified data. Optional.
	 */
	virtual void unmap(size_t usedOffset = 0, size_t usedSize = -1) = 0;

	/**
	 * Bind the VertexBuffer to its specified target.
	 * (GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, etc).
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

protected:

	// Whether the buffer is currently bound.
	bool is_bound;

	// Whether the buffer is currently mapped to main memory.
	bool is_mapped;

private:

	// The size of the buffer, in bytes.
	size_t size;

	// The target buffer object. (GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER).
	GLenum target;

	// Usage hint. GL_[DYNAMIC, STATIC, STREAM]_DRAW.
	GLenum usage;

	//
	MemoryBacking backing;
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
	 * @copydoc VertexBuffer(int, GLenum, GLenum, Backing)
	 */
	VertexArray(size_t size, GLenum target, GLenum usage, MemoryBacking backing);

	/**
	 * Frees the data we've allocated.
	 */
	virtual ~VertexArray();

	// Implements VertexBuffer.
	virtual void *map();
	virtual void unmap(size_t usedOffset = 0, size_t usedSize = -1);
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
 **/
class VBO : public VertexBuffer, public Volatile
{
public:

	/**
	 * @copydoc VertexBuffer(size_t, GLenum, GLenum, Backing)
	 **/
	VBO(size_t size, GLenum target, GLenum usage, MemoryBacking backing);

	/**
	 * Deletes the VBOs from OpenGL.
	 **/
	virtual ~VBO();

	// Implements VertexBuffer.
	virtual void *map();
	virtual void unmap(size_t usedOffset = 0, size_t usedSize = -1);
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

	void unmapStatic(size_t offset, size_t size);
	void unmapStream();

	// The VBO identifier. Assigned by OpenGL.
	GLuint vbo;

	// A pointer to mapped memory. Will be inialized on the first
	// call to map().
	char *memory_map;

	// Set if the buffer was modified while operating on gpu memory
	// and needs to be synchronized.
	bool is_dirty;
};

/**
 * VertexIndex manages one shared VertexBuffer that stores the indices for an
 * element array. Vertex arrays using the vertex structure (or anything else
 * that can use the pattern below) can request a size and use it for the
 * drawElements call.
 *
 *  indices[i*6 + 0] = i*4 + 0;
 *  indices[i*6 + 1] = i*4 + 1;
 *  indices[i*6 + 2] = i*4 + 2;
 *
 *  indices[i*6 + 3] = i*4 + 0;
 *  indices[i*6 + 4] = i*4 + 2;
 *  indices[i*6 + 5] = i*4 + 3;
 *
 * There will always be a large enough VertexBuffer around until all
 * VertexIndex instances have been deleted.
 *
 * Q: Why have something like VertexIndex?
 * A: The indices for the SpriteBatch do not change, only the array size
 * varies. Using one VertexBuffer for all element arrays removes this
 * duplicated data and saves some memory.
 */
class VertexIndex
{
public:
	/**
	 * Adds an entry to the list of sizes and resizes the VertexBuffer
	 * if needed. A size of 1 allocates a group of 6 indices for 4 vertices
	 * creating 1 face.
	 *
	 * @param size The requested size in groups of 6 indices.
	 */
	VertexIndex(size_t size);

	/**
	 * Removes an entry from the list of sizes and resizes the VertexBuffer
	 * if needed.
	 */
	~VertexIndex();

	/**
	 * Returns the number of index groups.
	 * This can be used for getIndexCount to get the full count of indices.
	 *
	 * @return The number of index groups.
	 */
	size_t getSize() const;

	/**
	 * Returns the number of indices that the passed element count will have.
	 * Use VertexIndex::getSize to get the full index count for that
	 * VertexIndex instance.
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
	 * Returns the pointer to the VertexBuffer.
	 * The pointer will change if a new size request or removal causes
	 * a VertexBuffer resize. It is recommended to retrieve the pointer
	 * value directly before the drawing call.
	 *
	 * @return The pointer to the VertexBuffer.
	 */
	VertexBuffer *getVertexBuffer() const;

	/**
	 * Returns a pointer which represents the specified byte offset.
	 *
	 * @param offset The offset in bytes.
	 * @return A pointer which represents the offset.
	 */
	const void *getPointer(size_t offset) const;

private:

	/**
	 * Adds a new size to the size list, then sorts and resizes it if needed.
	 *
	 * @param newSize The new size to be added.
	 */
	void addSize(size_t newSize);

	/**
	 * Removes a size from the size list, then sorts and resizes it if needed.
	 *
	 * @param oldSize The old size to be removed.
	 */
	void removeSize(size_t oldSize);

	/**
	 * Resizes the VertexBuffer to the requested size.
	 * This function takes care of choosing the correct integer type and
	 * allocating and deleting the VertexBuffer instance. It also has some
	 * fallback logic in case the memory ran out.
	 *
	 * @param size The requested VertexBuffer size. Passing 0 deletes the VertexBuffer without allocating a new one.
	 */
	void resize(size_t size);

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
	// The current VertexBuffer size. 0 means no VertexBuffer.
	static size_t maxSize;
	// The list of sizes. Needs to be kept sorted in ascending order.
	static std::list<size_t> sizeRefs;
	// The VertexBuffer for the element array. Can be NULL.
	static VertexBuffer *element_array;
};

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_SPRITE_BATCH_H
