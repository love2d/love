/**
 * Copyright (c) 2006-2018 LOVE Development Team
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

#include "common/config.h"
#include "StreamBuffer.h"
#include "OpenGL.h"
#include "FenceSync.h"
#include "graphics/Volatile.h"
#include "common/Exception.h"
#include "common/memory.h"

#include <vector>
#include <algorithm>

namespace love
{
namespace graphics
{
namespace opengl
{

static const int BUFFER_FRAMES = 3;
static const int MAX_SYNCS_PER_FRAME = 4;

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
			frameGPUReadOffset = 0;
			gl.bindBuffer(mode, vbo);
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
		frameGPUReadOffset += usedsize;
	}

	void nextFrame() override
	{
		frameGPUReadOffset = 0;
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
		frameGPUReadOffset = 0;

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

class StreamBufferSync : public love::graphics::StreamBuffer
{
public:

	StreamBufferSync(BufferType type, size_t size)
		: love::graphics::StreamBuffer(type, size)
		, syncSize((size + MAX_SYNCS_PER_FRAME - 1) / MAX_SYNCS_PER_FRAME)
		, frameIndex(0)
		, syncs()
	{}

	virtual ~StreamBufferSync() {}

	void nextFrame() override
	{
		getCurrentSync()->fence();

		frameIndex = (frameIndex + 1) % BUFFER_FRAMES;
		frameGPUReadOffset = 0;
	}

	void markUsed(size_t usedsize) override
	{
		int firstSyncIndex = frameGPUReadOffset / syncSize;
		int lastSyncIndex = std::min((frameGPUReadOffset + usedsize), bufferSize - 1) / syncSize;

		// Insert fences for all sync buckets completely filled by this section
		// of the data. The last bucket before the end of the frame will also be
		// handled by nextFrame().
		for (int i = firstSyncIndex; i < lastSyncIndex; i++)
			syncs[frameIndex * MAX_SYNCS_PER_FRAME + i].fence();

		frameGPUReadOffset += usedsize;
	}

protected:

	const size_t syncSize;

	int frameIndex;

	FenceSync syncs[MAX_SYNCS_PER_FRAME * BUFFER_FRAMES];

	FenceSync *getCurrentSync()
	{
		return &syncs[frameIndex * MAX_SYNCS_PER_FRAME + frameGPUReadOffset / syncSize];
	}

}; // StreamBufferSync

class StreamBufferMapSync final : public StreamBufferSync, public Volatile
{
public:

	StreamBufferMapSync(BufferType type, size_t size)
		: StreamBufferSync(type, size)
		, vbo(0)
		, glMode(OpenGL::getGLBufferType(mode))
	{
		loadVolatile();
	}

	~StreamBufferMapSync()
	{
		unloadVolatile();
	}

	MapInfo map(size_t /*minsize*/) override
	{
		gl.bindBuffer(mode, vbo);

		MapInfo info;
		info.size = bufferSize - frameGPUReadOffset;

		int firstSyncIndex = frameGPUReadOffset / syncSize;
		int lastSyncIndex = (bufferSize - 1) / syncSize;

		// We're mapping the full range of space left in the buffer, so we
		// need to wait on all of it...
		// FIXME: is it even worth it to have multiple sync objects per frame?
		for (int i = firstSyncIndex; i <= lastSyncIndex; i++)
			syncs[frameIndex * MAX_SYNCS_PER_FRAME + i].cpuWait();

		GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_UNSYNCHRONIZED_BIT;

		size_t mapoffset = (frameIndex * bufferSize) + frameGPUReadOffset;
		info.data = (uint8 *) glMapBufferRange(glMode, mapoffset, info.size, flags);

		return info;
	}

	size_t unmap(size_t usedsize) override
	{
		gl.bindBuffer(mode, vbo);
		glFlushMappedBufferRange(glMode, 0, usedsize);
		glUnmapBuffer(glMode);

		return (frameIndex * bufferSize) + frameGPUReadOffset;
	}

	ptrdiff_t getHandle() const override { return vbo; }

	bool loadVolatile() override
	{
		if (vbo != 0)
			return true;

		glGenBuffers(1, &vbo);
		gl.bindBuffer(mode, vbo);
		glBufferData(glMode, bufferSize * BUFFER_FRAMES, nullptr, GL_STREAM_DRAW);

		frameGPUReadOffset = 0;
		frameIndex = 0;

		return true;
	}

	void unloadVolatile() override
	{
		if (vbo != 0)
		{
			gl.deleteBuffer(vbo);
			vbo = 0;
		}

		for (FenceSync &sync : syncs)
			sync.cleanup();
	}

private:

	GLuint vbo;
	GLenum glMode;

}; // StreamBufferMapSync

class StreamBufferPersistentMapSync final : public StreamBufferSync, public Volatile
{
public:

	StreamBufferPersistentMapSync(BufferType type, size_t size)
		: StreamBufferSync(type, size)
		, vbo(0)
		, glMode(OpenGL::getGLBufferType(mode))
		, data(nullptr)
	{
		loadVolatile();
	}

	~StreamBufferPersistentMapSync()
	{
		unloadVolatile();
	}

	MapInfo map(size_t /*minsize*/) override
	{
		MapInfo info;
		info.size = bufferSize - frameGPUReadOffset;
		info.data = data + (frameIndex * bufferSize) + frameGPUReadOffset;

		int firstSyncIndex = frameGPUReadOffset / syncSize;
		int lastSyncIndex = (bufferSize - 1) / syncSize;

		// We're mapping the full range of space left in the buffer, so we
		// need to wait on all of it...
		// FIXME: is it even worth it to have multiple sync objects per frame?
		for (int i = firstSyncIndex; i <= lastSyncIndex; i++)
			syncs[frameIndex * MAX_SYNCS_PER_FRAME + i].cpuWait();

		return info;
	}

	size_t unmap(size_t usedsize) override
	{
		size_t offset = (frameIndex * bufferSize) + frameGPUReadOffset;

		gl.bindBuffer(mode, vbo);
		glFlushMappedBufferRange(glMode, offset, usedsize);

		return offset;
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

		frameGPUReadOffset = 0;
		frameIndex = 0;

		return true;
	}

	void unloadVolatile() override
	{
		if (vbo != 0)
		{
			gl.bindBuffer(mode, vbo);
			glUnmapBuffer(glMode);
			gl.deleteBuffer(vbo);
			vbo = 0;
		}

		for (FenceSync &sync : syncs)
			sync.cleanup();
	}

private:

	GLuint vbo;
	GLenum glMode;
	uint8 *data;

}; // StreamBufferPersistentMapSync

class StreamBufferPinnedMemory final : public StreamBufferSync, public Volatile
{
public:

	StreamBufferPinnedMemory(BufferType type, size_t size)
		: StreamBufferSync(type, size)
		, vbo(0)
		, glMode(OpenGL::getGLBufferType(mode))
		, data(nullptr)
		, alignedSize(0)
	{
		size_t alignment = getPageSize();
		alignedSize = alignUp(size * BUFFER_FRAMES, alignment);

		if (!alignedMalloc((void **) &data, alignedSize, alignment))
			throw love::Exception("Out of memory.");

		loadVolatile();
	}

	~StreamBufferPinnedMemory()
	{
		unloadVolatile();
		alignedFree(data);
	}

	MapInfo map(size_t /*minsize*/) override
	{
		MapInfo info;
		info.size = bufferSize - frameGPUReadOffset;
		info.data = data + (frameIndex * bufferSize) + frameGPUReadOffset;

		int firstSyncIndex = frameGPUReadOffset / syncSize;
		int lastSyncIndex = (bufferSize - 1) / syncSize;

		// We're mapping the full range of space left in the buffer, so we
		// need to wait on all of it...
		// FIXME: is it even worth it to have multiple sync objects per frame?
		for (int i = firstSyncIndex; i <= lastSyncIndex; i++)
			syncs[frameIndex * MAX_SYNCS_PER_FRAME + i].cpuWait();

		return info;
	}

	size_t unmap(size_t usedsize) override
	{
		size_t offset = (frameIndex * bufferSize) + frameGPUReadOffset;

		gl.bindBuffer(mode, vbo);
		glFlushMappedBufferRange(glMode, offset, usedsize);

		return offset;
	}

	ptrdiff_t getHandle() const override { return vbo; }

	bool loadVolatile() override
	{
		if (vbo != 0)
			return true;

		glGenBuffers(1, &vbo);

		glBindBuffer(GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD, vbo);
		glBufferData(GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD, alignedSize, data, GL_STREAM_DRAW);

		frameGPUReadOffset = 0;
		frameIndex = 0;

		return true;
	}

	void unloadVolatile() override
	{
		if (vbo != 0)
		{
			// Make sure the GPU has completed work using the memory before
			// freeing it. TODO: Do we need a full glFinish() or is this
			// sufficient?
			glFlush();
			for (FenceSync &sync : syncs)
				sync.cpuWait();

			gl.bindBuffer(mode, vbo);
			gl.deleteBuffer(vbo);
			vbo = 0;
		}

		for (FenceSync &sync : syncs)
			sync.cleanup();
	}

private:

	GLuint vbo;
	GLenum glMode;
	uint8 *data;
	size_t alignedSize;

}; // StreamBufferPinnedMemory

love::graphics::StreamBuffer *CreateStreamBuffer(BufferType mode, size_t size)
{
	if (gl.isCoreProfile())
	{
		if (!gl.bugs.clientWaitSyncStalls)
		{
			// AMD's pinned memory seems to be faster than persistent mapping,
			// on AMD GPUs.
			if (GLAD_AMD_pinned_memory)
				return new StreamBufferPinnedMemory(mode, size);
			else if (GLAD_VERSION_4_4 || GLAD_ARB_buffer_storage)
				return new StreamBufferPersistentMapSync(mode, size);
		}

		return new StreamBufferSubDataOrphan(mode, size);
	}
	else
		return new StreamBufferClientMemory(mode, size);
}

} // opengl
} // graphics
} // love
