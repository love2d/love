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
#include "common/Object.h"
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
 * A block of GPU-owned memory.
 **/
class Buffer : public love::Object, public Resource
{
public:

	static love::Type type;

	enum MapFlags
	{
		MAP_NONE = 0,
		MAP_EXPLICIT_RANGE_MODIFY = (1 << 0), // see setMappedRangeModified.
		MAP_READ = (1 << 1),
	};

	enum TypeFlags
	{
		TYPEFLAG_NONE = 0,
		TYPEFLAG_VERTEX = 1 << BUFFERTYPE_VERTEX,
		TYPEFLAG_INDEX = 1 << BUFFERTYPE_INDEX,
	};

	struct DataDeclaration
	{
		std::string name;
		DataFormat format;
		int arrayLength;

		DataDeclaration(const std::string &name, DataFormat format, int arrayLength = 0)
			: name(name)
			, format(format)
			, arrayLength(arrayLength)
		{}
	};

	struct DataMember
	{
		DataDeclaration decl;
		DataFormatInfo info;
		size_t offset;
		size_t size;

		DataMember(const DataDeclaration &decl)
			: decl(decl)
			, info(getDataFormatInfo(decl.format))
			, offset(0)
			, size(0)
		{}
	};

	struct Settings
	{
		TypeFlags typeFlags;
		MapFlags mapFlags;
		BufferUsage usage;

		Settings(uint32 typeflags, uint32 mapflags, BufferUsage usage)
			: typeFlags((TypeFlags)typeflags)
			, mapFlags((MapFlags)mapflags)
			, usage(usage)
		{}
	};

	Buffer(Graphics *gfx, const Settings &settings, const std::vector<DataDeclaration> &format, size_t size, size_t arraylength);
	virtual ~Buffer();

	size_t getSize() const { return size; }
	TypeFlags getTypeFlags() const { return typeFlags; }
	BufferUsage getUsage() const { return usage; }
	bool isMapped() const { return mapped; }
	uint32 getMapFlags() const { return mapFlags; }

	size_t getArrayLength() const { return arrayLength; }
	size_t getArrayStride() const { return arrayStride; }
	const std::vector<DataMember> &getDataMembers() const { return dataMembers; }
	const DataMember &getDataMember(int index) const { return dataMembers[index]; }
	size_t getMemberOffset(int index) const { return dataMembers[index].offset; }
	int getDataMemberIndex(const std::string &name) const;

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

	static std::vector<DataDeclaration> getCommonFormatDeclaration(CommonFormat format);

	class Mapper
	{
	public:

		Mapper(Buffer &buffer)
			: buffer(buffer)
		{
			data = buffer.map();
		}

		~Mapper()
		{
			if (buffer.getMapFlags() & MAP_EXPLICIT_RANGE_MODIFY)
				buffer.setMappedRangeModified(0, buffer.getSize());
			buffer.unmap();
		}

		Buffer &buffer;
		void *data;

	}; // Mapper

protected:

	std::vector<DataMember> dataMembers;
	size_t arrayLength;
	size_t arrayStride;

	// The size of the buffer, in bytes.
	size_t size;

	// The type of the buffer object.
	TypeFlags typeFlags;

	// Usage hint. GL_[DYNAMIC, STATIC, STREAM]_DRAW.
	BufferUsage usage;
	
	uint32 mapFlags;

	bool mapped;
	
}; // Buffer

} // graphics
} // love
