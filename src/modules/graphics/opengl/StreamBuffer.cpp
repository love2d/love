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

#include "StreamBuffer.h"
#include "OpenGL.h"
#include "BufferSync.h"
#include "graphics/Volatile.h"
#include "common/Exception.h"

#include <vector>

namespace love
{
namespace graphics
{
namespace opengl
{

static int BUFFER_FRAMES = 3;

class StreamBufferClientMemory final : public love::graphics::StreamBuffer
{
public:

	StreamBufferClientMemory(BufferType mode, size_t size)
		: love::graphics::StreamBuffer(mode, size)
		, data(nullptr)
	{
		try
		{
			data = new uint8[size];
		}
		catch (std::exception &)
		{
			throw love::Exception("Out of memory.");
		}
	}

	virtual ~StreamBufferClientMemory()
	{
		delete[] data;
	}

	MapInfo map(size_t /*minsize*/) override
	{
		return MapInfo(data, bufferSize);
	}

	size_t unmap(size_t /*usedsize*/) override
	{
		return (size_t) data;
	}

	void markUsed(size_t /*usedsize*/) override { }
	ptrdiff_t getHandle() const override { return 0; }

private:

	uint8 *data;

}; // StreamBufferClientMemory

class StreamBufferSubDataOrphan final : public love::graphics::StreamBuffer, public Volatile
{
public:

	StreamBufferSubDataOrphan(BufferType mode, size_t size)
		: love::graphics::StreamBuffer(mode, size)
		, vbo(0)
		, glMode(OpenGL::getGLBufferType(mode))
		, data(nullptr)
		, offset(0)
	{
		try
		{
			data = new uint8[size];
		}
		catch (std::exception &)
		{
			throw love::Exception("Out of memory.");
		}

		loadVolatile();
	}

	virtual ~StreamBufferSubDataOrphan()
	{
		unloadVolatile();
		delete[] data;
	}

	MapInfo map(size_t minsize) override
	{
		if (offset + minsize > bufferSize)
		{
			offset = 0;
			glBufferData(glMode, bufferSize, nullptr, GL_STREAM_DRAW);
		}

		return MapInfo(data, bufferSize - offset);
	}

	size_t unmap(size_t usedsize) override
	{
		gl.bindBuffer(mode, vbo);
		glBufferSubData(glMode, offset, usedsize, data);
		return offset;
	}

	void markUsed(size_t usedsize) override
	{
		offset += usedsize;
	}

	ptrdiff_t getHandle() const override { return vbo; }

	bool loadVolatile() override
	{
		if (vbo != 0)
			return true;

		glGenBuffers(1, &vbo);
		gl.bindBuffer(mode, vbo);
		glBufferData(glMode, bufferSize, nullptr, GL_STREAM_DRAW);

		offset = 0;

		return true;
	}

	void unloadVolatile() override
	{
		if (vbo == 0)
			return;

		gl.deleteBuffer(vbo);
		vbo = 0;
	}

protected:

	GLuint vbo;
	GLenum glMode;

	uint8 *data;

	size_t offset;

}; // StreamBufferSubDataOrphan

class StreamBufferMapSync final : public love::graphics::StreamBuffer, public Volatile
{
public:

	StreamBufferMapSync(BufferType type, size_t size)
		: love::graphics::StreamBuffer(type, size)
		, vbo(0)
		, gpuReadOffset(0)
		, glMode(OpenGL::getGLBufferType(mode))
	{
		loadVolatile();
	}

	~StreamBufferMapSync()
	{
		unloadVolatile();
	}

	MapInfo map(size_t minsize) override
	{
		gl.bindBuffer(mode, vbo);

		if (gpuReadOffset + minsize > bufferSize * BUFFER_FRAMES)
			gpuReadOffset = 0;

		MapInfo info;
		info.size = bufferSize - (gpuReadOffset % bufferSize);

		sync.wait(gpuReadOffset, info.size);

		GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_UNSYNCHRONIZED_BIT;

		info.data = (uint8 *) glMapBufferRange(glMode, gpuReadOffset, info.size, flags);
		return info;
	}

	size_t unmap(size_t usedsize) override
	{
		gl.bindBuffer(mode, vbo);
		glFlushMappedBufferRange(glMode, 0, usedsize);
		glUnmapBuffer(glMode);

		return gpuReadOffset;
	}

	void markUsed(size_t usedsize) override
	{
		sync.lock(gpuReadOffset, usedsize);
		gpuReadOffset += usedsize;
	}

	ptrdiff_t getHandle() const override { return vbo; }

	bool loadVolatile() override
	{
		if (vbo != 0)
			return true;

		glGenBuffers(1, &vbo);
		gl.bindBuffer(mode, vbo);
		glBufferData(glMode, bufferSize * BUFFER_FRAMES, nullptr, GL_STREAM_DRAW);

		gpuReadOffset = 0;

		return true;
	}

	void unloadVolatile() override
	{
		if (vbo == 0)
			return;

		gl.deleteBuffer(vbo);
		vbo = 0;

		sync.cleanup();
	}

private:

	GLuint vbo;
	size_t gpuReadOffset;
	GLenum glMode;

	BufferSync sync;

}; // StreamBufferMapSync

class StreamBufferPersistentMapSync final : public love::graphics::StreamBuffer, public Volatile
{
public:

	StreamBufferPersistentMapSync(BufferType type, size_t size)
		: love::graphics::StreamBuffer(type, size)
		, vbo(0)
		, gpuReadOffset(0)
		, glMode(OpenGL::getGLBufferType(mode))
		, data(nullptr)
	{
		loadVolatile();
	}

	~StreamBufferPersistentMapSync()
	{
		unloadVolatile();
	}

	MapInfo map(size_t minsize) override
	{
		if (gpuReadOffset + minsize > bufferSize * BUFFER_FRAMES)
			gpuReadOffset = 0;

		MapInfo info;
		info.size = bufferSize - (gpuReadOffset % bufferSize);
		info.data = data + gpuReadOffset;

		sync.wait(gpuReadOffset, info.size);

		return info;
	}

	size_t unmap(size_t usedsize) override
	{
		gl.bindBuffer(mode, vbo);
		glFlushMappedBufferRange(glMode, gpuReadOffset, usedsize);

		return gpuReadOffset;
	}

	void markUsed(size_t usedsize) override
	{
		sync.lock(gpuReadOffset, usedsize);
		gpuReadOffset += usedsize;
	}

	ptrdiff_t getHandle() const override { return vbo; }

	bool loadVolatile() override
	{
		if (vbo != 0)
			return true;

		glGenBuffers(1, &vbo);
		gl.bindBuffer(mode, vbo);

		GLbitfield storageflags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;
		GLbitfield mapflags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_FLUSH_EXPLICIT_BIT;

		glBufferStorage(glMode, bufferSize * BUFFER_FRAMES, nullptr, storageflags);
		data = (uint8 *) glMapBufferRange(glMode, 0, bufferSize * BUFFER_FRAMES, mapflags);

		gpuReadOffset = 0;

		return true;
	}

	void unloadVolatile() override
	{
		if (vbo == 0)
			return;

		gl.bindBuffer(mode, vbo);
		glUnmapBuffer(glMode);
		gl.deleteBuffer(vbo);
		vbo = 0;

		sync.cleanup();
	}

private:

	GLuint vbo;
	size_t gpuReadOffset;
	GLenum glMode;
	uint8 *data;

	BufferSync sync;

}; // StreamBufferPersistentMapSync

love::graphics::StreamBuffer *CreateStreamBuffer(BufferType mode, size_t size)
{
	if (gl.isCoreProfile())
	{
		// FIXME: This is disabled until more efficient manual syncing can be
		// implemented.
#if 0
		if (GLAD_VERSION_4_4 || GLAD_ARB_buffer_storage)
			return new StreamBufferPersistentMapSync(mode, size);
		else
#endif
			return new StreamBufferSubDataOrphan(mode, size);
	}
	else
		return new StreamBufferClientMemory(mode, size);
}

} // opengl
} // graphics
} // love
