#pragma once

#include "graphics/GraphicsReadback.h"

namespace love
{
namespace graphics
{
namespace vulkan
{

class Graphics;

class GraphicsReadback : public graphics::GraphicsReadback
{
public:
	GraphicsReadback(love::graphics::Graphics *gfx, ReadbackMethod method, love::graphics::Buffer *buffer, size_t offset, size_t size, data::ByteData *dest, size_t destoffset);
	GraphicsReadback(love::graphics::Graphics *gfx, ReadbackMethod method, love::graphics::Texture *texture, int slice, int mipmap, const Rect &rect, image::ImageData *dest, int destx, int desty);
	virtual ~GraphicsReadback();

	void wait() override;
	void update() override;

private:

	Graphics *vgfx = nullptr;
	StrongRef<love::graphics::Buffer> stagingBuffer;
};

} // vulkan
} // graphics
} // love
