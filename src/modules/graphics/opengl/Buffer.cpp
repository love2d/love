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

#include "Buffer.h"

#include "common/Exception.h"
#include "graphics/vertex.h"
#include "Graphics.h"

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

	if (usageFlags & BUFFERUSAGEFLAG_TEXEL)
		mapUsage = BUFFERUSAGE_TEXEL;
	else if (usageFlags & BUFFERUSAGEFLAG_VERTEX)
		mapUsage = BUFFERUSAGE_VERTEX;
	else if (usageFlags & BUFFERUSAGEFLAG_INDEX)
		mapUsage = BUFFERUSAGE_INDEX;
	else if (usageFlags & BUFFERUSAGEFLAG_SHADER_STORAGE)
		mapUsage = BUFFERUSAGE_SHADER_STORAGE;
	else if (usageFlags & BUFFERUSAGEFLAG_INDIRECT_ARGUMENTS)
		mapUsage = BUFFERUSAGE_INDIRECT_ARGUMENTS;

	target = OpenGL::getGLBufferType(mapUsage);

	if (dataUsage == BUFFERDATAUSAGE_STREAM)
		ownsMemoryMap = true;

	std::vector<uint8> emptydata;
	if (settings.zeroInitialize && data == nullptr && !GLAD_VERSION_4_3)
	{
		try
		{
			emptydata.resize(getSize());
			data = emptydata.data();
		}
		catch (std::exception &)
		{
			data = nullptr;
		}
	}

	if (!load(data))
	{
		unloadVolatile();
		throw love::Exception("Could not create buffer with %d bytes (out of VRAM?)", size);
	}

	if (settings.zeroInitialize && data == nullptr && GLAD_VERSION_4_3)
	{
		gl.bindBuffer(mapUsage, buffer);
		glClearBufferData(target, GL_R8UI, GL_RED, GL_UNSIGNED_BYTE, nullptr);
	}
}

Buffer::~Buffer()
{
	unloadVolatile();
	if (memoryMap != nullptr && ownsMemoryMap)
		free(memoryMap);
}

bool Buffer::loadVolatile()
{
	if (buffer != 0)
		return true;

	return load(nullptr);
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

bool Buffer::load(const void *initialdata)
{
	while (glGetError() != GL_NO_ERROR)
		/* Clear the error buffer. */;

	glGenBuffers(1, &buffer);
	gl.bindBuffer(mapUsage, buffer);

	GLenum gldatausage = OpenGL::getGLBufferDataUsage(getDataUsage());

	// initialdata can be null.
	glBufferData(target, (GLsizeiptr) getSize(), initialdata, gldatausage);

	if (getUsageFlags() & BUFFERUSAGEFLAG_TEXEL)
	{
		glGenTextures(1, &texture);
		gl.bindBufferTextureToUnit(texture, 0, false, true);

		GLenum glformat = getGLFormat(getDataMember(0).decl.format);

		glTexBuffer(target, glformat, buffer);
	}

	if (!debugName.empty() && (GLAD_VERSION_4_3 || GLAD_ES_VERSION_3_2))
		glObjectLabel(GL_BUFFER, buffer, -1, debugName.c_str());

	return (glGetError() == GL_NO_ERROR);
}

bool Buffer::supportsOrphan() const
{
	return dataUsage == BUFFERDATAUSAGE_STREAM || dataUsage == BUFFERDATAUSAGE_DYNAMIC;
}

void *Buffer::map(MapType map, size_t offset, size_t size)
{
	if (size == 0)
		return nullptr;

	if (map == MAP_WRITE_INVALIDATE && (isImmutable() || dataUsage == BUFFERDATAUSAGE_READBACK))
		return nullptr;

	if (map == MAP_READ_ONLY && dataUsage != BUFFERDATAUSAGE_READBACK)
		return  nullptr;

	Range r(offset, size);

	if (!Range(0, getSize()).contains(r))
		return nullptr;

	char *data = nullptr;

	if (map == MAP_READ_ONLY)
	{
		gl.bindBuffer(mapUsage, buffer);
		data = (char *) glMapBufferRange(target, offset, size, GL_MAP_READ_BIT);
	}
	else if (ownsMemoryMap)
	{
		if (memoryMap == nullptr)
			memoryMap = (char *) malloc(getSize());
		data = memoryMap;
	}
	else
	{
		auto gfx = Module::getInstance<Graphics>(Module::M_GRAPHICS);
		data = (char *) gfx->getBufferMapMemory(size);
	}

	if (data != nullptr)
	{
		mapped = true;
		mappedType = map;
		mappedRange = r;
		if (!ownsMemoryMap)
			memoryMap = data;
	}

	return data;
}

void Buffer::unmap(size_t usedoffset, size_t usedsize)
{
	Range r(usedoffset, usedsize);

	if (!mapped || !mappedRange.contains(r))
		return;

	mapped = false;

	if (mappedType == MAP_READ_ONLY)
	{
		gl.bindBuffer(mapUsage, buffer);
		glUnmapBuffer(target);
		if (!ownsMemoryMap)
			memoryMap = nullptr;
		return;
	}

	// Orphan optimization - see fill().
	if (supportsOrphan() && mappedRange.first == 0 && mappedRange.getSize() == getSize())
	{
		usedoffset = 0;
		usedsize = getSize();
	}

	char *data = memoryMap + (usedoffset - mappedRange.getOffset());

	fill(usedoffset, usedsize, data);

	if (!ownsMemoryMap)
	{
		auto gfx = Module::getInstance<Graphics>(Module::M_GRAPHICS);
		gfx->releaseBufferMapMemory(memoryMap);
		memoryMap = nullptr;
	}
}

bool Buffer::fill(size_t offset, size_t size, const void *data)
{
	if (size == 0 || isImmutable() || dataUsage == BUFFERDATAUSAGE_READBACK)
		return false;

	size_t buffersize = getSize();

	if (!Range(0, buffersize).contains(Range(offset, size)))
		return false;

	GLenum gldatausage = OpenGL::getGLBufferDataUsage(dataUsage);

	gl.bindBuffer(mapUsage, buffer);

	if (supportsOrphan() && size == buffersize)
	{
		// "orphan" current buffer to avoid implicit synchronisation on the GPU:
		// http://www.seas.upenn.edu/~pcozzi/OpenGLInsights/OpenGLInsights-AsynchronousBufferTransfers.pdf
		glBufferData(target, (GLsizeiptr) buffersize, nullptr, gldatausage);

#if LOVE_WINDOWS
		// TODO: Verify that this intel codepath is a useful optimization.
		if (gl.getVendor() == OpenGL::VENDOR_INTEL)
			glBufferData(target, (GLsizeiptr) buffersize, data, gldatausage);
		else
#endif
			glBufferSubData(target, 0, (GLsizeiptr) buffersize, data);
	}
	else
	{
		glBufferSubData(target, (GLintptr) offset, (GLsizeiptr) size, data);
	}

	return true;
}

void Buffer::clearInternal(size_t offset, size_t size)
{
	if (GLAD_VERSION_4_3)
	{
		gl.bindBuffer(mapUsage, buffer);
		glClearBufferSubData(target, GL_R8UI, offset, size, GL_RED, GL_UNSIGNED_BYTE, nullptr);
	}
	else
	{
		try
		{
			std::vector<uint8> emptydata(getSize());
			fill(0, getSize(), emptydata.data());
		}
		catch (std::exception &)
		{
			throw love::Exception("Out of memory.");
		}
	}
}

void Buffer::copyTo(love::graphics::Buffer *dest, size_t sourceoffset, size_t destoffset, size_t size)
{
	// TODO: tracked state for these bind types?
	glBindBuffer(GL_COPY_READ_BUFFER, buffer);
	glBindBuffer(GL_COPY_WRITE_BUFFER, ((Buffer *) dest)->buffer);

	glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, sourceoffset, destoffset, size);
}

} // opengl
} // graphics
} // love
