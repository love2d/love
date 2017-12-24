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

#include "common/config.h"
#include "Buffer.h"
#include "Graphics.h"
#include "common/Exception.h"

#include <algorithm> // std::min

namespace love
{
namespace graphics
{

Buffer::Buffer(size_t size, BufferType type, vertex::Usage usage, uint32 mapflags)
	: size(size)
	, type(type)
	, usage(usage)
	, map_flags(mapflags)
	, is_mapped(false)
{
}

Buffer::~Buffer()
{
}

// QuadIndices

static const int MAX_VERTICES_PER_DRAW = LOVE_UINT16_MAX;
static const int MAX_QUADS_PER_DRAW    = MAX_VERTICES_PER_DRAW / 4;

size_t QuadIndices::objectCount = 0;

Buffer *QuadIndices::indexBuffer = nullptr;

QuadIndices::QuadIndices(Graphics *gfx)
{
	if (indexBuffer == nullptr)
	{
		size_t buffersize = sizeof(uint16) * MAX_QUADS_PER_DRAW * 6;

		indexBuffer = gfx->newBuffer(buffersize, nullptr, BUFFER_INDEX, vertex::USAGE_STATIC, 0);

		Buffer::Mapper map(*indexBuffer);
		vertex::fillIndices(vertex::TriangleIndexMode::QUADS, 0, MAX_VERTICES_PER_DRAW, (uint16 *) map.get());
	}

	objectCount++;
}

QuadIndices::QuadIndices(const QuadIndices &)
{
	objectCount++;
}

QuadIndices &QuadIndices::operator = (const QuadIndices &)
{
	return *this;
}

QuadIndices::~QuadIndices()
{
	--objectCount;

	// Delete the buffer if we were the last living QuadIndices object.
	if (objectCount <= 0)
	{
		delete indexBuffer;
		indexBuffer = nullptr;
	}
}

static inline void advanceVertexOffsets(const vertex::Attributes &attributes, vertex::Buffers &buffers, int vertexcount)
{
	// TODO: Figure out a better way to avoid touching the same buffer multiple
	// times, if multiple attributes share the buffer.
	uint32 touchedbuffers = 0;

	for (unsigned int i = 0; i < vertex::Attributes::MAX; i++)
	{
		if (!attributes.isEnabled(i))
			continue;

		auto &attrib = attributes.attribs[i];

		uint32 bufferbit = 1u << attrib.bufferindex;
		if ((touchedbuffers & bufferbit) == 0)
		{
			touchedbuffers |= bufferbit;
			buffers.info[attrib.bufferindex].offset += attrib.stride * vertexcount;
		}
	}
}

void QuadIndices::draw(Graphics *gfx, int quadstart, int quadcount, const vertex::Attributes &attributes, vertex::Buffers buffers, Texture *texture)
{
	Graphics::DrawIndexedCommand cmd(&attributes, &buffers, indexBuffer);
	cmd.primitiveType = PRIMITIVE_TRIANGLES;
	cmd.indexBufferOffset = 0;
	cmd.indexType = INDEX_UINT16;
	cmd.texture = texture;

	// TODO: We can use glDrawElementsBaseVertex when supported, instead of
	// advancing the vertex offset.
	if (quadstart > 0)
		advanceVertexOffsets(attributes, buffers, quadstart * 4);

	for (int quadindex = 0; quadindex < quadcount; quadindex += MAX_QUADS_PER_DRAW)
	{
		int quaddrawcount = std::min(MAX_QUADS_PER_DRAW, quadcount - quadindex);

		if (quadindex > 0)
			advanceVertexOffsets(attributes, buffers, quaddrawcount * 4);

		cmd.indexCount = quaddrawcount * 6;
		gfx->draw(cmd);
	}
}

Buffer *QuadIndices::getBuffer() const
{
	return indexBuffer;
}

} // graphics
} // love
