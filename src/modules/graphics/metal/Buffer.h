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

#pragma once

#include "graphics/Buffer.h"
#include "Metal.h"
#include "common/Range.h"

namespace love
{
namespace graphics
{
namespace metal
{

class Buffer final : public love::graphics::Buffer
{
public:

	Buffer(love::graphics::Graphics *gfx, id<MTLDevice> device, const Settings &settings, const std::vector<DataDeclaration> &format, const void *data, size_t size, size_t arraylength);
	virtual ~Buffer();

	void *map(MapType map, size_t offset, size_t size) override;
	void unmap(size_t usedoffset, size_t usedsize) override;
	bool fill(size_t offset, size_t size, const void *data) override;
	void copyTo(love::graphics::Buffer *dest, size_t sourceoffset, size_t destoffset, size_t size) override;

	ptrdiff_t getHandle() const override { return (ptrdiff_t) buffer; }
	ptrdiff_t getTexelBufferHandle() const override { return (ptrdiff_t) texture; }

private:

	void clearInternal(size_t offset, size_t size) override;

	id<MTLBuffer> buffer;
	id<MTLTexture> texture;

	id<MTLBuffer> mapBuffer;

	Range mappedRange;

}; // Buffer

} // metal
} // graphics
} // love
