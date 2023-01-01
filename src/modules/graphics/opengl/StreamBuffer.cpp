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

// Typically this should be 3 frames, but we only do per-frame syncing right now
// so we add an extra frame to reduce the (small) chance of stalls.
static const int BUFFER_FRAMES = 4;

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
		, orphan(false)
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

	MapInfo map(size_t /*minsize*/) override
	{
		if (orphan)
		{
			orphan = false;
			frameGPUReadOffset = 0;
			gl.bindBuffer(mode, vbo);
			glBufferData(glMode, bufferSize, nullptr, GL_STREAM_DRAW);
		}

		return MapInfo(data, bufferSize - frameGPUReadOffset);
	}

	size_t unmap(size_t usedsize) override
	{
		gl.bindBuffer(mode, vbo);
		glBufferSubData(glMode, frameGPUReadOffset, usedsize, data);
		return frameGPUReadOffset;
	}

	void markUsed(size_t usedsize) override
	{
		frameGPUReadOffset += usedsize;
	}

	void nextFrame() override
	{
		// Orphan the buffer before its first use in the next frame.
		frameGPUReadOffset = 0;
		orphan = true;
	}

	ptrdiff_t getHandle() const override { return vbo; }

	bool loadVolatile() override
	{
		if (vbo != 0)
			return true;

		glGenBuffers(1, &vbo);
		gl.bindBuffer(mode, vbo);
		glBufferData(glMode, bufferSize, nullptr, GL_STREAM_DRAW);

		frameGPUReadOffset = 0;
		orphan = false;

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

	bool orphan;

}; // StreamBufferSubDataOrphan

class StreamBufferSync : public love::graphics::StreamBuffer
{
public:

	StreamBufferSync(BufferType type, size_t size)
		: love::graphics::StreamBuffer(type, size)
		, frameIndex(0)
		, syncs()
	{}

	virtual ~StreamBufferSync() {}

	void nextFrame() override
	{
		// Insert a GPU fence for this frame's section of the data, we'll wait
		// for it when we try to map that data for writing in subsequent frames.
		syncs[frameIndex].fence();

		frameIndex = (frameIndex + 1) % BUFFER_FRAMES;
		frameGPUReadOffset = 0;
	}

	void markUsed(size_t usedsize) override
	{
		// We insert a fence for all data from this frame at the end of the
		// frame (in nextFrame), rather than doing anything more fine-grained.
		frameGPUReadOffset += usedsize;
	}

protected:

	int frameIndex;
	FenceSync syncs[BUFFER_FRAMES];

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

		// Make sure this frame's section of the buffer is done being used.
		syncs[frameIndex].cpuWait();

		MapInfo info;
		info.size = bufferSize - frameGPUReadOffset;

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

	// Coherent mapping is supposedly faster on intel/nvidia aside from a couple
	// old nvidia GPUs.
	StreamBufferPersistentMapSync(BufferType type, size_t size, bool coherent = true)
		: StreamBufferSync(type, size)
		, vbo(0)
		, glMode(OpenGL::getGLBufferType(mode))
		, data(nullptr)
		, coherent(coherent)
	{
		loadVolatile();
	}

	~StreamBufferPersistentMapSync()
	{
		unloadVolatile();
	}

	MapInfo map(size_t /*minsize*/) override
	{
		// Make sure this frame's section of the buffer is done being used.
		syncs[frameIndex].cpuWait();

		MapInfo info;
		info.size = bufferSize - frameGPUReadOffset;
		info.data = data + (frameIndex * bufferSize) + frameGPUReadOffset;
		return info;
	}

	size_t unmap(size_t usedsize) override
	{
		size_t offset = (frameIndex * bufferSize) + frameGPUReadOffset;

		if (!coherent)
		{
			gl.bindBuffer(mode, vbo);
			glFlushMappedBufferRange(glMode, offset, usedsize);
		}

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
		GLbitfield mapflags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;

		storageflags |= (coherent ? GL_MAP_COHERENT_BIT : 0);
		mapflags |= (coherent ? GL_MAP_COHERENT_BIT : GL_MAP_FLUSH_EXPLICIT_BIT);

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
	bool coherent;

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

		if (!loadVolatile())
		{
			ptrdiff_t pointer = (ptrdiff_t) data;
			alignedFree(data);
			throw love::Exception("AMD Pinned Memory StreamBuffer implementation failed to create buffer (address: %p, alignment: %ld, aiigned size: %ld)", pointer, alignment, alignedSize);
		}
	}

	~StreamBufferPinnedMemory()
	{
		unloadVolatile();
		alignedFree(data);
	}

	MapInfo map(size_t /*minsize*/) override
	{
		// Make sure this frame's section of the buffer is done being used.
		syncs[frameIndex].cpuWait();

		MapInfo info;
		info.size = bufferSize - frameGPUReadOffset;
		info.data = data + (frameIndex * bufferSize) + frameGPUReadOffset;
		return info;
	}

	size_t unmap(size_t /*usedsize*/) override
	{
		size_t offset = (frameIndex * bufferSize) + frameGPUReadOffset;
		return offset;
	}

	ptrdiff_t getHandle() const override { return vbo; }

	bool loadVolatile() override
	{
		if (vbo != 0)
			return true;

		glGenBuffers(1, &vbo);

		while (glGetError() != GL_NO_ERROR)
			/* Clear errors. */;

		glBindBuffer(GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD, vbo);
		glBufferData(GL_EXTERNAL_VIRTUAL_MEMORY_BUFFER_AMD, alignedSize, data, GL_STREAM_DRAW);

		if (glGetError() != GL_NO_ERROR)
		{
			gl.deleteBuffer(vbo);
			vbo = 0;
			return false;
		}

		frameGPUReadOffset = 0;
		frameIndex = 0;

		return true;
	}

	void unloadVolatile() override
	{
		if (vbo != 0)
		{
			// Make sure the GPU has completed all work before freeing the
			// memory. glFlush+sync.cpuWait doesn't seem to be enough.
			glFinish();

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
			if (GLAD_AMD_pinned_memory && gl.getVendor() == OpenGL::VENDOR_AMD)
			{
				try
				{
					return new StreamBufferPinnedMemory(mode, size);
				}
				catch (love::Exception &)
				{
					// According to the spec, pinned memory can fail if the RAM
					// allocation can't be mapped to the GPU's address space.
					// This seems to happen in practice on Mesa + amdgpu:
					// https://bitbucket.org/rude/love/issues/1540
					// Fall through to other implementations when that happens.
				}
			}

			if (GLAD_VERSION_4_4 || GLAD_ARB_buffer_storage)
				return new StreamBufferPersistentMapSync(mode, size);

			// Most modern drivers have a separate internal thread which queues
			// GL commands for the GPU. The queue causes mapping to stall until
			// the items in the queue are flushed, which makes this approach
			// slow on most drivers. On macOS, having a separate driver thread
			// is opt-in via an API, and we don't do it, so we can use this
			// instead of the (potentially slower) SubData approach.
#ifdef LOVE_MACOSX
			return new StreamBufferMapSync(mode, size);
#endif
		}

		return new StreamBufferSubDataOrphan(mode, size);
	}
	else
		return new StreamBufferClientMemory(mode, size);
}

} // opengl
} // graphics
} // love
