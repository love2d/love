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

#include "StreamBuffer.h"
#include "Metal.h"
#include "Graphics.h"
#include "common/int.h"

#include <dispatch/semaphore.h>

namespace love
{
namespace graphics
{
namespace metal
{

static const int BUFFER_FRAMES = 3;

class StreamBuffer final : public love::graphics::StreamBuffer
{
public:

	StreamBuffer(id<MTLDevice> device, BufferUsage usage, size_t size)
		: love::graphics::StreamBuffer(usage, size)
		, frameIndex(0)
		, mappedFrames()
	{ @autoreleasepool {
		MTLResourceOptions opts = MTLResourceStorageModeShared;
		buffer = [device newBufferWithLength:size * BUFFER_FRAMES options:opts];
		if (buffer == nil)
			throw love::Exception("Out of graphics memory.");

		buffer.label = [NSString stringWithFormat:@"StreamBuffer (usage: %d, size: %ld)", usage, size];

		data = (uint8 *) buffer.contents;

		for (int i = 0; i < BUFFER_FRAMES; i++)
			frameSemaphores[i] = dispatch_semaphore_create(1);
	}}

	virtual ~StreamBuffer()
	{ @autoreleasepool {
		buffer = nil;
		for (int i = 0; i < BUFFER_FRAMES; i++)
		{
			if (mappedFrames[i])
				dispatch_semaphore_signal(frameSemaphores[i]);
			frameSemaphores[i] = nil;
		}
	}}

	size_t getGPUReadOffset() const override
	{
		return (frameIndex * bufferSize) + frameGPUReadOffset;
	}

	MapInfo map(size_t /*minsize*/) override
	{
		// Make sure this frame's section of the buffer is done being used.
		if (!mappedFrames[frameIndex])
		{
			dispatch_semaphore_wait(frameSemaphores[frameIndex], DISPATCH_TIME_FOREVER);
			mappedFrames[frameIndex] = true;
		}

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

	void nextFrame() override
	{
		id<MTLCommandBuffer> cmd = Graphics::getInstance()->getCommandBuffer();

		// Insert a GPU fence for this frame's section of the data, we'll wait
		// for it when we try to map that data for writing in subsequent frames.
		if (mappedFrames[frameIndex])
		{
			/*__weak*/ dispatch_semaphore_t semaphore = frameSemaphores[frameIndex];
			StreamBuffer *pthis = this;
			pthis->retain();
			[cmd addCompletedHandler:^(id<MTLCommandBuffer> _Nonnull)
			{
				dispatch_semaphore_signal(semaphore);
				pthis->release();
			}];
		}

		mappedFrames[frameIndex] = false;
		frameIndex = (frameIndex + 1) % BUFFER_FRAMES;
		frameGPUReadOffset = 0;
	}

	void markUsed(size_t usedsize) override
	{
		// We insert a fence for all data from this frame at the end of the
		// frame (in nextFrame), rather than doing anything more fine-grained.
		frameGPUReadOffset += usedsize;
	}

	ptrdiff_t getHandle() const override { return (ptrdiff_t)buffer; }

private:

	id<MTLBuffer> buffer;
	uint8 *data;

	int frameIndex;
	dispatch_semaphore_t frameSemaphores[BUFFER_FRAMES];
	bool mappedFrames[BUFFER_FRAMES];

}; // StreamBuffer

love::graphics::StreamBuffer *CreateStreamBuffer(id<MTLDevice> device, BufferUsage usage, size_t size)
{
	return new StreamBuffer(device, usage, size);
}

} // metal
} // graphics
} // love
