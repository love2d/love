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

#pragma once

// LOVE
#include "common/config.h"
#include "common/int.h"
#include "vertex.h"

// C
#include <stddef.h>

namespace love
{
namespace graphics
{

class Graphics;

/**
 * A block of GPU-owned memory. Currently meant for internal use.
 **/
class Buffer
{
public:

	enum MapFlags
	{
		MAP_EXPLICIT_RANGE_MODIFY = (1 << 0), // see setMappedRangeModified.
		MAP_READ = (1 << 1),
	};

	Buffer(size_t size, BufferType type, vertex::Usage usage, uint32 mapflags);
	virtual ~Buffer();

	size_t getSize() const
	{
		return size;
	}

	BufferType getType() const
	{
		return type;
	}

	vertex::Usage getUsage() const
	{
		return usage;
	}

	bool isMapped() const
	{
		return is_mapped;
	}

	/**
	 * Map the Buffer to client memory.
	 *
	 * This can be faster for large changes to the buffer. For smaller
	 * changes, see fill().
	 */
	virtual void *map() = 0;

	/**
	 * Unmap a previously mapped Buffer. The buffer must be unmapped when used
	 * to draw.
	 */
	virtual void unmap() = 0;

	/**
	 * Marks a range of mapped data as modified.
	 * NOTE: Buffer::fill calls this internally for you.
	 **/
	virtual void setMappedRangeModified(size_t offset, size_t size) = 0;

	/**
	 * Gets the backend-specific handle for this Buffer.
	 **/
	virtual ptrdiff_t getHandle() const = 0;

	/**
	 * Fill a portion of the buffer with data and marks the range as modified.
	 *
	 * @param offset The offset in the GLBuffer to store the data.
	 * @param size The size of the incoming data.
	 * @param data Pointer to memory to copy data from.
	 */
	virtual void fill(size_t offset, size_t size, const void *data) = 0;

	/**
	 * Copy the contents of this Buffer to another Buffer object.
	 **/
	virtual void copyTo(size_t offset, size_t size, Buffer *other, size_t otheroffset) = 0;

	uint32 getMapFlags() const
	{
		return map_flags;
	}

	class Mapper
	{
	public:

		/**
		 * Memory-maps a Buffer.
		 */
		Mapper(Buffer &buffer)
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

		Buffer &buf;
		void *elems;

	}; // Mapper

protected:

	// The size of the buffer, in bytes.
	size_t size;

	// The type of the buffer object.
	BufferType type;

	// Usage hint. GL_[DYNAMIC, STATIC, STREAM]_DRAW.
	vertex::Usage usage;
	
	uint32 map_flags;

	bool is_mapped;
	
}; // Buffer

/**
 * QuadIndices manages one shared Buffer that stores the indices for an element
 * array. Vertex arrays using the vertex structure (or anything else that can
 * use the pattern below) can request a size and use it for the indexed draw
 * call.
 *
 *  indices[i*6 + 0] = i*4 + 0;
 *  indices[i*6 + 1] = i*4 + 1;
 *  indices[i*6 + 2] = i*4 + 2;
 *
 *  indices[i*6 + 3] = i*4 + 2;
 *  indices[i*6 + 4] = i*4 + 1;
 *  indices[i*6 + 5] = i*4 + 3;
 *
 * There will always be a large enough Buffer around until all QuadIndices
 * instances have been deleted.
 *
 * Q: Why have something like QuadIndices?
 * A: The indices for the SpriteBatch do not change, only the array size varies.
 * Using one Buffer for all element arrays removes this duplicated data and
 * saves some memory.
 */
class QuadIndices
{
public:
	/**
	 * Adds an entry to the list of sizes and resizes the Buffer
	 * if needed. A size of 1 allocates a group of 6 indices for 4 vertices
	 * creating 1 face.
	 */
	QuadIndices(Graphics *gfx, size_t size);

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
	 */
	size_t getSize() const;

	/**
	 * Returns the number of indices that the passed element count will have.
	 * Use QuadIndices::getSize to get the full index count for that
	 * QuadIndices instance.
	 */
	size_t getIndexCount(size_t elements) const;

	/**
	 * Returns the integer type of the element array.
	 * If an optional nonzero size argument is passed, the function returns
	 * the integer type of the element array of that size.
	 */
	IndexDataType getType(size_t s) const;
	inline IndexDataType getType() const
	{
		return getType(maxSize);
	}

	/**
	 * Returns the size in bytes of an element in the element array.
	 * Can be used with getPointer to calculate an offset into the array based
	 * on a number of elements.
	 **/
	size_t getElementSize() const;

	/**
	 * Returns the pointer to the Buffer.
	 * The pointer will change if a new size request or removal causes a Buffer
	 * resize. It is recommended to retrieve the pointer value directly before
	 * the drawing call.
	 */
	Buffer *getBuffer() const;

	/**
	 * Returns a direct pointer to the index data.
	 *
	 * At least one graphics driver (the one for Kepler nvidia GPUs in OS X)
	 * fails to render geometry if the vertex data was a direct CPU pointer but
	 * the index data came from an Index Buffer.
	 * So the direct pointer to the index buffer should be used instead of the
	 * index buffer when rendering using client-side vertex arrays.
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

	// The current GLBuffer size. 0 means no Buffer.
	static size_t maxSize;

	static size_t objectCount;

	// The Buffer for the element array. Can be null.
	static Buffer *indexBuffer;
	
	// The array of indices that will also be stored in the index buffer.
	static char *indices;

}; // QuadIndices

} // graphics
} // love
