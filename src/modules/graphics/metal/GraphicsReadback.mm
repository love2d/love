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

#include "GraphicsReadback.h"
#include "Buffer.h"
#include "Texture.h"
#include "Graphics.h"
#include "data/ByteData.h"

namespace love::graphics::metal
{

GraphicsReadback::GraphicsReadback(love::graphics::Graphics *gfx, ReadbackMethod method, love::graphics::Buffer *buffer, size_t offset, size_t size, data::ByteData *dest, size_t destoffset)
	: love::graphics::GraphicsReadback(gfx, method, buffer, offset, size, dest, destoffset)
	, done(false)
{ @autoreleasepool {
	auto mgfx = (Graphics *) gfx;

	// Immediate readback of readback-type buffers doesn't need a staging buffer.
	if (method != READBACK_IMMEDIATE || buffer->getDataUsage() != BUFFERDATAUSAGE_READBACK)
	{
		stagingBuffer = gfx->getTemporaryBuffer(size, DATAFORMAT_FLOAT, 0, BUFFERDATAUSAGE_READBACK);
		gfx->copyBuffer(buffer, stagingBuffer, offset, 0, size);
	}

	// use instead of get, in case this was the first command in the frame.
	cmd = mgfx->useCommandBuffer();

	auto pthis = this;
	pthis->retain();
	[cmd addCompletedHandler:^(id<MTLCommandBuffer> _Nonnull)
	{
		pthis->done = true;
		pthis->release();
	}];

	if (method == READBACK_IMMEDIATE)
	{
		wait();

		if (stagingBuffer.get())
		{
			status = readbackBuffer(stagingBuffer, 0, size);
			gfx->releaseTemporaryBuffer(stagingBuffer);
		}
		else
		{
			status = readbackBuffer(buffer, offset, size);
		}
	}
}}

GraphicsReadback::GraphicsReadback(love::graphics::Graphics *gfx, ReadbackMethod method, love::graphics::Texture *texture, int slice, int mipmap, const Rect &rect, image::ImageData *dest, int destx, int desty)
	: love::graphics::GraphicsReadback(gfx, method, texture, slice, mipmap, rect, dest, destx, desty)
	, done(false)
{ @autoreleasepool {
	auto mgfx = (Graphics *) gfx;
	size_t size = getPixelFormatSliceSize(textureFormat, rect.w, rect.h);

	stagingBuffer = gfx->getTemporaryBuffer(size, DATAFORMAT_FLOAT, 0, BUFFERDATAUSAGE_READBACK);

	gfx->copyTextureToBuffer(texture, stagingBuffer, slice, mipmap, rect, 0, 0);

	cmd = mgfx->getCommandBuffer();

	auto pthis = this;
	pthis->retain();
	[cmd addCompletedHandler:^(id<MTLCommandBuffer> _Nonnull)
	{
		pthis->done = true;
		pthis->release();
	}];

	if (method == READBACK_IMMEDIATE)
	{
		wait();
		status = readbackBuffer(stagingBuffer, 0, size);
		gfx->releaseTemporaryBuffer(stagingBuffer);
	}
}}

GraphicsReadback::~GraphicsReadback()
{ @autoreleasepool {
	cmd = nil;
}}

void GraphicsReadback::wait()
{ @autoreleasepool {
	if (status != STATUS_WAITING || cmd == nil)
		return;

	if (cmd.status == MTLCommandBufferStatusNotEnqueued)
	{
		auto gfx = Graphics::getInstance();
		gfx->submitCommandBuffer(Graphics::SUBMIT_STORE);
	}

	[cmd waitUntilCompleted];
	cmd = nil;

	update();
}}

void GraphicsReadback::update()
{
	if (status != STATUS_WAITING)
		return;

	if (done)
	{
		if (stagingBuffer.get())
			status = readbackBuffer(stagingBuffer, 0, stagingBuffer->getSize());
		else
			status = STATUS_ERROR;

		if (stagingBuffer.get())
		{
			auto gfx = Module::getInstance<love::graphics::Graphics>(Module::M_GRAPHICS);
			if (gfx != nullptr)
				gfx->releaseTemporaryBuffer(stagingBuffer);
			stagingBuffer.set(nullptr);
		}
	}
}

} // love::graphics::metal
