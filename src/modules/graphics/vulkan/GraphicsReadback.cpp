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
		vgfx->submitGpuCommands(false);
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
		vgfx->submitGpuCommands(false);
}

GraphicsReadback::~GraphicsReadback()
{
}

void GraphicsReadback::wait()
{
	if (status == STATUS_WAITING)
		vgfx->submitGpuCommands(false);
}

void GraphicsReadback::update()
{
}

} // vulkan
} // graphics
} // love
