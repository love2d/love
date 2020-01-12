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

#pragma once

// LOVE
#include "common/config.h"
#include "common/int.h"
#include "vertex.h"
#include "Resource.h"

// C
#include <stddef.h>
#include <string>
#include <vector>

namespace love
{
namespace graphics
{

class Graphics;

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

	enum DataType
	{
		DATA_FLOAT,
		DATA_FLOAT_VEC2,
		DATA_FLOAT_VEC3,
		DATA_FLOAT_VEC4,

		DATA_FLOAT_MAT2X2,
		DATA_FLOAT_MAT2X3,
		DATA_FLOAT_MAT2X4,

		DATA_FLOAT_MAT3X2,
		DATA_FLOAT_MAT3X3,
		DATA_FLOAT_MAT3X4,

		DATA_FLOAT_MAT4X2,
		DATA_FLOAT_MAT4X3,
		DATA_FLOAT_MAT4X4,

		DATA_INT32,
		DATA_INT32_VEC2,
		DATA_INT32_VEC3,
		DATA_INT32_VEC4,

		DATA_UINT32,
		DATA_UINT32_VEC2,
		DATA_UINT32_VEC3,
		DATA_UINT32_VEC4,

		DATA_SNORM8_VEC4,

		DATA_UNORM8_VEC4,

		DATA_INT8_VEC4,

		DATA_UINT8_VEC4,

		DATA_SNORM16,
		DATA_SNORM16_VEC2,
		DATA_SNORM16_VEC4,

		DATA_UNORM16,
		DATA_UNORM16_VEC2,
		DATA_UNORM16_VEC4,

		DATA_INT16,
		DATA_INT16_VEC2,
		DATA_INT16_VEC4,

		DATA_UINT16,
		DATA_UINT16_VEC2,
		DATA_UINT16_VEC4,

		DATA_BOOL,
		DATA_BOOL_VEC2,
		DATA_BOOL_VEC3,
		DATA_BOOL_VEC4,

		DATA_MAX_ENUM
	};

	struct DataMember
	{
		std::string name;
		DataType type;
		int arraySize;
	};

	struct Settings
	{
		BufferTypeFlags typeFlags;
		MapFlags mapFlags;
		vertex::Usage usage;
	};

	Buffer(size_t size, BufferTypeFlags typeflags, vertex::Usage usage, uint32 mapflags);
	Buffer(Graphics *gfx, const Settings &settings, const std::vector<DataMember> &format, size_t arraylength);
	virtual ~Buffer();

	size_t getSize() const { return size; }
	BufferTypeFlags getTypeFlags() const { return typeFlags; }
	vertex::Usage getUsage() const { return usage; }
	bool isMapped() const { return mapped; }
	uint32 getMapFlags() const { return mapFlags; }

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

	class Mapper
	{
	public:

		Mapper(Buffer &buffer)
			: buf(buffer)
		{
			elems = buf.map();
		}

		~Mapper()
		{
			buf.unmap();
		}

		void *get() { return elems; }

	private:

		Buffer &buf;
		void *elems;

	}; // Mapper

//	static size_t getDataTypeSize(DataType type, bool uniform)

protected:

	// The size of the buffer, in bytes.
	size_t size;

	// The type of the buffer object.
	BufferTypeFlags typeFlags;

	// Usage hint. GL_[DYNAMIC, STATIC, STREAM]_DRAW.
	vertex::Usage usage;
	
	uint32 mapFlags;

	bool mapped;
	
}; // Buffer

} // graphics
} // love
