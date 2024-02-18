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
#include "graphics/Graphics.h"
#include "data/ByteData.h"

namespace love
{
namespace graphics
{
namespace opengl
{

GraphicsReadback::GraphicsReadback(love::graphics::Graphics *gfx, ReadbackMethod method, love::graphics::Buffer *buffer, size_t offset, size_t size, data::ByteData *dest, size_t destoffset)
	: love::graphics::GraphicsReadback(gfx, method, buffer, offset, size, dest, destoffset)
{
	// Immediate readback of readback-type buffers doesn't need a staging buffer.
	if (method != READBACK_IMMEDIATE || buffer->getDataUsage() != BUFFERDATAUSAGE_READBACK)
	{
		stagingBuffer = gfx->getTemporaryBuffer(size, DATAFORMAT_FLOAT, 0, BUFFERDATAUSAGE_READBACK);
		gfx->copyBuffer(buffer, stagingBuffer, offset, 0, size);
	}

	if (method == READBACK_IMMEDIATE)
	{
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
	else
	{
		sync.fence();
	}
}

GraphicsReadback::GraphicsReadback(love::graphics::Graphics *gfx, ReadbackMethod method, love::graphics::Texture *texture, int slice, int mipmap, const Rect &rect, image::ImageData *dest, int destx, int desty)
	: love::graphics::GraphicsReadback(gfx, method, texture, slice, mipmap, rect, dest, destx, desty)
{
	size_t size = getPixelFormatSliceSize(textureFormat, rect.w, rect.h);

	if (method == READBACK_IMMEDIATE)
	{
		void *dest = prepareReadbackDest(size);

		// Direct readback without copying avoids the need for a staging buffer,
		// and lowers the system requirements of immediate RT readback.
		Texture *t = (Texture *) texture;
		t->readbackInternal(slice, mipmap, rect, imageData->getWidth(), size, dest);

		status = STATUS_COMPLETE;
	}
	else
	{
		stagingBuffer = gfx->getTemporaryBuffer(size, DATAFORMAT_FLOAT, 0, BUFFERDATAUSAGE_READBACK);

		gfx->copyTextureToBuffer(texture, stagingBuffer, slice, mipmap, rect, 0, 0);
		sync.fence();
	}
}

GraphicsReadback::~GraphicsReadback()
{
}

void GraphicsReadback::wait()
{
	if (status != STATUS_WAITING)
		return;

	sync.cpuWait();
	update();
}

void GraphicsReadback::update()
{
	if (status != STATUS_WAITING)
		return;

	if (sync.isComplete())
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

} // opengl
} // graphics
} // love
