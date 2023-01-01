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

#pragma once

// LOVE
#include "common/config.h"
#include "common/int.h"
#include "vertex.h"
#include "Resource.h"

// C
#include <stddef.h>

namespace love
{
namespace graphics
{

/**
 * A block of GPU-owned memory. Currently meant for internal use.
 **/
class Buffer : public Resource
{
public:

	enum MapFlags
	{
		MAP_EXPLICIT_RANGE_MODIFY = (1 << 0), // see setMappedRangeModified.
		MAP_READ = (1 << 1),
	};

	Buffer(size_t size, BufferType type, vertex::Usage usage, uint32 mapflags);
	virtual ~Buffer();

	size_t getSize() const { return size; }

	BufferType getType() const { return type; }

	vertex::Usage getUsage() const { return usage; }

	bool isMapped() const { return is_mapped; }

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

	uint32 getMapFlags() const { return map_flags; }

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

} // graphics
} // love
