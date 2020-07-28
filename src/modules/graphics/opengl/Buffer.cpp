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

#include "Buffer.h"

#include "common/Exception.h"
#include "graphics/vertex.h"

#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <limits>

namespace love
{
namespace graphics
{
namespace opengl
{

static GLenum getGLFormat(DataFormat format)
{
	switch (format)
	{
		case DATAFORMAT_FLOAT: return GL_R32F;
		case DATAFORMAT_FLOAT_VEC2: return GL_RG32F;
		case DATAFORMAT_FLOAT_VEC3: return GL_RGB32F;
		case DATAFORMAT_FLOAT_VEC4: return GL_RGBA32F;
		case DATAFORMAT_INT32: return GL_R32I;
		case DATAFORMAT_INT32_VEC2: return GL_RG32I;
		case DATAFORMAT_INT32_VEC3: return GL_RGB32I;
		case DATAFORMAT_INT32_VEC4: return GL_RGBA32I;
		case DATAFORMAT_UINT32: return GL_R32UI;
		case DATAFORMAT_UINT32_VEC2: return GL_RG32UI;
		case DATAFORMAT_UINT32_VEC3: return GL_RGB32UI;
		case DATAFORMAT_UINT32_VEC4: return GL_RGBA32UI;
		case DATAFORMAT_UNORM8_VEC4: return GL_RGBA8;
		case DATAFORMAT_INT8_VEC4: return GL_RGBA8I;
		case DATAFORMAT_UINT8_VEC4: return GL_RGBA8UI;
		case DATAFORMAT_UNORM16_VEC2: return GL_RG16;
		case DATAFORMAT_UNORM16_VEC4: return GL_RGBA16;
		case DATAFORMAT_INT16_VEC2: return GL_RG16I;
		case DATAFORMAT_INT16_VEC4: return GL_RGBA16I;
		case DATAFORMAT_UINT16: return GL_R16UI;
		case DATAFORMAT_UINT16_VEC2: return GL_RG16UI;
		case DATAFORMAT_UINT16_VEC4: return GL_RGBA16UI;
		default: return GL_ZERO;
	}
}

Buffer::Buffer(love::graphics::Graphics *gfx, const Settings &settings, const std::vector<DataDeclaration> &format, const void *data, size_t size, size_t arraylength)
	: love::graphics::Buffer(gfx, settings, format, size, arraylength)
{
	size = getSize();
	arraylength = getArrayLength();

	if (typeFlags & TYPEFLAG_TEXEL)
		mapType = BUFFERTYPE_TEXEL;
	else if (typeFlags & TYPEFLAG_VERTEX)
		mapType = BUFFERTYPE_VERTEX;
	else if (typeFlags & TYPEFLAG_INDEX)
		mapType = BUFFERTYPE_INDEX;

	target = OpenGL::getGLBufferType(mapType);

	try
	{
		memoryMap = new char[size];
	}
	catch (std::bad_alloc &)
	{
		throw love::Exception("Out of memory.");
	}

	if (data != nullptr)
		memcpy(memoryMap, data, size);

	if (!load(data != nullptr))
	{
		unloadVolatile();
		delete[] memoryMap;
		throw love::Exception("Could not create buffer (out of VRAM?)");
	}
}

Buffer::~Buffer()
{
	unloadVolatile();
	delete[] memoryMap;
}

bool Buffer::loadVolatile()
{
	if (buffer != 0)
		return true;

	return load(true);
}

void Buffer::unloadVolatile()
{
	mapped = false;
	if (buffer != 0)
		gl.deleteBuffer(buffer);
	buffer = 0;
	if (texture != 0)
		gl.deleteTexture(texture);
	texture = 0;
}

bool Buffer::load(bool restore)
{
	while (glGetError() != GL_NO_ERROR)
		/* Clear the error buffer. */;

	glGenBuffers(1, &buffer);
	gl.bindBuffer(mapType, buffer);

	// Copy the old buffer only if 'restore' was requested.
	const GLvoid *src = restore ? memoryMap : nullptr;

	// Note that if 'src' is '0', no data will be copied.
	glBufferData(target, (GLsizeiptr) getSize(), src, OpenGL::getGLBufferUsage(getUsage()));

	if (getTypeFlags() & TYPEFLAG_TEXEL)
	{
		glGenTextures(1, &texture);
		gl.bindBufferTextureToUnit(texture, 0, false, true);

		glTexBuffer(target, getGLFormat(getDataMember(0).decl.format), buffer);
	}

	return (glGetError() == GL_NO_ERROR);
}

void *Buffer::map()
{
	if (mapped)
		return memoryMap;

	mapped = true;

	modifiedOffset = 0;
	modifiedSize = 0;
	isMappedDataModified = false;

	return memoryMap;
}

void Buffer::unmapStatic(size_t offset, size_t size)
{
	if (size == 0)
		return;

	// Upload the mapped data to the buffer.
	gl.bindBuffer(mapType, buffer);
	glBufferSubData(target, (GLintptr) offset, (GLsizeiptr) size, memoryMap + offset);
}

void Buffer::unmapStream()
{
	GLenum glusage = OpenGL::getGLBufferUsage(getUsage());

	// "orphan" current buffer to avoid implicit synchronisation on the GPU:
	// http://www.seas.upenn.edu/~pcozzi/OpenGLInsights/OpenGLInsights-AsynchronousBufferTransfers.pdf
	gl.bindBuffer(mapType, buffer);
	glBufferData(target, (GLsizeiptr) getSize(), nullptr, glusage);

#if LOVE_WINDOWS
	// TODO: Verify that this codepath is a useful optimization.
	if (gl.getVendor() == OpenGL::VENDOR_INTEL)
		glBufferData(target, (GLsizeiptr) getSize(), memoryMap, glusage);
	else
#endif
		glBufferSubData(target, 0, (GLsizeiptr) getSize(), memoryMap);
}

void Buffer::unmap()
{
	if (!mapped)
		return;

	mapped = false;

	if ((mapFlags & MAP_EXPLICIT_RANGE_MODIFY) != 0)
	{
		if (!isMappedDataModified)
			return;

		modifiedOffset = std::min(modifiedOffset, getSize() - 1);
		modifiedSize = std::min(modifiedSize, getSize() - modifiedOffset);
	}
	else
	{
		modifiedOffset = 0;
		modifiedSize = getSize();
	}

	if (modifiedSize > 0)
	{
		switch (getUsage())
		{
		case BUFFERUSAGE_STATIC:
			unmapStatic(modifiedOffset, modifiedSize);
			break;
		case BUFFERUSAGE_STREAM:
			unmapStream();
			break;
		case BUFFERUSAGE_DYNAMIC:
		default:
			// It's probably more efficient to treat it like a streaming buffer if
			// at least a third of its contents have been modified during the map().
			if (modifiedSize >= getSize() / 3)
				unmapStream();
			else
				unmapStatic(modifiedOffset, modifiedSize);
			break;
		}
	}

	modifiedOffset = 0;
	modifiedSize = 0;
}

void Buffer::setMappedRangeModified(size_t offset, size_t modifiedsize)
{
	if (!mapped || !(mapFlags & MAP_EXPLICIT_RANGE_MODIFY))
		return;

	if (!isMappedDataModified)
	{
		modifiedOffset = offset;
		modifiedSize = modifiedsize;
		isMappedDataModified = true;
		return;
	}

	// We're being conservative right now by internally marking the whole range
	// from the start of section a to the end of section b as modified if both
	// a and b are marked as modified.

	size_t oldrangeend = modifiedOffset + modifiedSize;
	modifiedOffset = std::min(modifiedOffset, offset);

	size_t newrangeend = std::max(offset + modifiedsize, oldrangeend);
	modifiedSize = newrangeend - modifiedOffset;
}

void Buffer::fill(size_t offset, size_t size, const void *data)
{
	memcpy(memoryMap + offset, data, size);

	if (mapped)
		setMappedRangeModified(offset, size);
	else
	{
		gl.bindBuffer(mapType, buffer);
		glBufferSubData(target, (GLintptr) offset, (GLsizeiptr) size, data);
	}
}

void Buffer::copyTo(size_t offset, size_t size, love::graphics::Buffer *other, size_t otheroffset)
{
	other->fill(otheroffset, size, memoryMap + offset);
}

} // opengl
} // graphics
} // love
