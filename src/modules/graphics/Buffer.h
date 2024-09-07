/**
 * Copyright (c) 2006-2024 LOVE Development Team
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
#include "common/Optional.h"
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

	static int bufferCount;
	static int64 totalGraphicsMemory;

	static const size_t SHADER_STORAGE_BUFFER_MAX_STRIDE = 2048;

	enum MapType
	{
		MAP_WRITE_INVALIDATE,
		MAP_READ_ONLY,
	};

	struct DataDeclaration
	{
		std::string name;
		DataFormat format;
		int arrayLength;
		int bindingLocation;

		DataDeclaration(const std::string &name, DataFormat format, int arrayLength = 0, int bindingLocation = -1)
			: name(name)
			, format(format)
			, arrayLength(arrayLength)
			, bindingLocation(bindingLocation)
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
		BufferUsageFlags usageFlags;
		BufferDataUsage dataUsage;
		bool zeroInitialize;
		std::string debugName;

		Settings(uint32 usageflags, BufferDataUsage dataUsage)
			: usageFlags((BufferUsageFlags)usageflags)
			, dataUsage(dataUsage)
			, zeroInitialize(false)
			, debugName()
		{}
	};

	Buffer(Graphics *gfx, const Settings &settings, const std::vector<DataDeclaration> &format, size_t size, size_t arraylength);
	virtual ~Buffer();

	size_t getSize() const { return size; }
	BufferUsageFlags getUsageFlags() const { return usageFlags; }
	BufferDataUsage getDataUsage() const { return dataUsage; }
	bool isMapped() const { return mapped; }

	size_t getArrayLength() const { return arrayLength; }
	size_t getArrayStride() const { return arrayStride; }
	const std::vector<DataMember> &getDataMembers() const { return dataMembers; }
	const DataMember &getDataMember(int index) const { return dataMembers[index]; }
	size_t getMemberOffset(int index) const { return dataMembers[index].offset; }
	int getDataMemberIndex(const std::string &name) const;
	int getDataMemberIndex(int bindingLocation) const;
	const std::string &getDebugName() const { return debugName; }

	void setImmutable(bool immutable) { this->immutable = immutable; };
	bool isImmutable() const { return immutable; }

	/**
	 * Map a portion of the Buffer to client memory.
	 */
	virtual void *map(MapType map, size_t offset, size_t size) = 0;

	/**
	 * Unmap a previously mapped Buffer. The buffer must be unmapped when used
	 * to draw.
	 */
	virtual void unmap(size_t usedoffset, size_t usedsize) = 0;

	/**
	 * Fill a portion of the buffer with data.
	 */
	virtual bool fill(size_t offset, size_t size, const void *data) = 0;

	/**
	 * Reset the given portion of this buffer's data to 0.
	 */
	void clear(size_t offset, size_t size);

	/**
	 * Copy a portion of this Buffer's data to another buffer, using the GPU.
	 **/
	virtual void copyTo(Buffer *dest, size_t sourceoffset, size_t destoffset, size_t size) = 0;

	/**
	 * Texel buffers may use an additional texture handle as well as a buffer
	 * handle.
	 **/
	virtual ptrdiff_t getTexelBufferHandle() const = 0;

	bool hasLegacyVertexBindings() const { return legacyVertexBindings; }

	static std::vector<DataDeclaration> getCommonFormatDeclaration(CommonFormat format);

	class Mapper
	{
	public:

		Mapper(Buffer &buffer, MapType maptype = MAP_WRITE_INVALIDATE)
			: buffer(buffer)
		{
			data = buffer.map(maptype, 0, buffer.getSize());
		}

		~Mapper()
		{
			buffer.unmap(0, buffer.getSize());
		}

		Buffer &buffer;
		void *data;

	}; // Mapper

protected:

	virtual void clearInternal(size_t offset, size_t size) = 0;

	std::vector<DataMember> dataMembers;
	size_t arrayLength;
	size_t arrayStride;

	// The size of the buffer, in bytes.
	size_t size;

	// Bit flags describing how the buffer can be used.
	BufferUsageFlags usageFlags;

	// Usage hint. GL_[DYNAMIC, STATIC, STREAM]_DRAW.
	BufferDataUsage dataUsage;

	std::string debugName;

	bool mapped;
	MapType mappedType;
	bool immutable;

	bool legacyVertexBindings = false;
	
}; // Buffer

} // graphics
} // love
