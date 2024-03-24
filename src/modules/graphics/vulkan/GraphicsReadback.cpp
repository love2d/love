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

namespace love
{
namespace graphics
{
namespace vulkan
{

GraphicsReadback::GraphicsReadback(love::graphics::Graphics *gfx, ReadbackMethod method, love::graphics::Buffer *buffer, size_t offset, size_t size, data::ByteData *dest, size_t destoffset)
	: graphics::GraphicsReadback(gfx, method, buffer, offset, size, dest, destoffset)
	, vgfx(dynamic_cast<Graphics*>(gfx))
{
	// Immediate readback of readback-type buffers doesn't need a staging buffer.
	if (method != READBACK_IMMEDIATE || buffer->getDataUsage() != BUFFERDATAUSAGE_READBACK)
	{
		stagingBuffer = gfx->getTemporaryBuffer(size, DATAFORMAT_FLOAT, 0, BUFFERDATAUSAGE_READBACK);
		gfx->copyBuffer(buffer, stagingBuffer, offset, 0, size);
	}

	if (method == READBACK_IMMEDIATE)
	{
		vgfx->submitGpuCommands(SUBMIT_RESTART);
		if (stagingBuffer.get()) {
			status = readbackBuffer(stagingBuffer, 0, size);
			gfx->releaseTemporaryBuffer(stagingBuffer);
		}
		else
			status = readbackBuffer(buffer, offset, size);
	}
	else
		vgfx->addReadbackCallback([&]() {
			status = readbackBuffer(stagingBuffer, 0, stagingBuffer->getSize());

			vgfx->releaseTemporaryBuffer(stagingBuffer);
			stagingBuffer.set(nullptr);
		});
}

GraphicsReadback::GraphicsReadback(love::graphics::Graphics *gfx, ReadbackMethod method, love::graphics::Texture *texture, int slice, int mipmap, const Rect &rect, image::ImageData *dest, int destx, int desty)
	: graphics::GraphicsReadback(gfx, method, texture, slice, mipmap, rect, dest, destx, desty)
	, vgfx(dynamic_cast<Graphics*>(gfx))
{
	size_t size = getPixelFormatSliceSize(textureFormat, rect.w, rect.h);

	stagingBuffer = vgfx->getTemporaryBuffer(size, DATAFORMAT_FLOAT, 0, BUFFERDATAUSAGE_READBACK);

	vgfx->copyTextureToBuffer(texture, stagingBuffer, slice, mipmap, rect, 0, 0);

	vgfx->addReadbackCallback([&]() {
		status = readbackBuffer(stagingBuffer, 0, stagingBuffer->getSize());

		vgfx->releaseTemporaryBuffer(stagingBuffer);
		stagingBuffer.set(nullptr);
	});

	if (method == READBACK_IMMEDIATE)
		vgfx->submitGpuCommands(SUBMIT_RESTART);
}

GraphicsReadback::~GraphicsReadback()
{
}

void GraphicsReadback::wait()
{
	if (status == STATUS_WAITING)
		vgfx->submitGpuCommands(SUBMIT_RESTART);
}

void GraphicsReadback::update()
{
}

} // vulkan
} // graphics
} // love
