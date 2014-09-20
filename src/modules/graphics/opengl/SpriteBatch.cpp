/**
 * Copyright (c) 2006-2014 LOVE Development Team
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
#include "SpriteBatch.h"

// OpenGL
#include "OpenGL.h"

// LOVE
#include "VertexBuffer.h"
#include "Texture.h"

// C++
#include <algorithm>

// C
#include <stddef.h>

namespace love
{
namespace graphics
{
namespace opengl
{

SpriteBatch::SpriteBatch(Texture *texture, int size, int usage)
	: texture(texture)
	, size(size)
	, next(0)
	, color(0)
	, array_buf(nullptr)
	, element_buf(size)
	, buffer_used_offset(0)
	, buffer_used_size(0)
{
	if (size <= 0)
		throw love::Exception("Invalid SpriteBatch size.");

	GLenum gl_usage;
	switch (usage)
	{
	default:
	case USAGE_DYNAMIC:
		gl_usage = GL_DYNAMIC_DRAW;
		break;
	case USAGE_STATIC:
		gl_usage = GL_STATIC_DRAW;
		break;
	case USAGE_STREAM:
		gl_usage = GL_STREAM_DRAW;
		break;
	}

	const size_t vertex_size = sizeof(Vertex) * 4 * size;

	try
	{
		array_buf = VertexBuffer::Create(vertex_size, GL_ARRAY_BUFFER, gl_usage);
	}
	catch (love::Exception &)
	{
		delete array_buf;
		throw;
	}
	catch (std::bad_alloc &)
	{
		delete array_buf;
		throw love::Exception("Out of memory.");
	}
}

SpriteBatch::~SpriteBatch()
{
	delete color;
	delete array_buf;
}

int SpriteBatch::add(float x, float y, float a, float sx, float sy, float ox, float oy, float kx, float ky, int index /*= -1*/)
{
	// Only do this if there's a free slot.
	if ((index == -1 && next >= size) || index < -1 || index >= size)
		return -1;

	Matrix t(x, y, a, sx, sy, ox, oy, kx, ky);

	addv(texture->getVertices(), t, (index == -1) ? next : index);

	// Increment counter.
	if (index == -1)
		return next++;

	return index;
}

int SpriteBatch::addq(Quad *quad, float x, float y, float a, float sx, float sy, float ox, float oy, float kx, float ky, int index /*= -1*/)
{
	// Only do this if there's a free slot.
	if ((index == -1 && next >= size) || index < -1 || index >= next)
		return -1;

	Matrix t(x, y, a, sx, sy, ox, oy, kx, ky);

	addv(quad->getVertices(), t, (index == -1) ? next : index);

	// Increment counter.
	if (index == -1)
		return next++;

	return index;
}

void SpriteBatch::clear()
{
	// Reset the position of the next index.
	next = 0;
}

void SpriteBatch::flush()
{
	VertexBuffer::Bind bind(*array_buf);
	array_buf->unmap(buffer_used_offset, buffer_used_size);

	buffer_used_offset = buffer_used_size = 0;
}

void SpriteBatch::setTexture(Texture *newtexture)
{
	texture.set(newtexture);
}

Texture *SpriteBatch::getTexture() const
{
	return texture.get();
}

void SpriteBatch::setColor(const Color &color)
{
	if (!this->color)
		this->color = new Color(color);
	else
		*(this->color) = color;
}

void SpriteBatch::setColor()
{
	delete color;
	color = 0;
}

const Color *SpriteBatch::getColor() const
{
	return color;
}

int SpriteBatch::getCount() const
{
	return next;
}

void SpriteBatch::setBufferSize(int newsize)
{
	if (newsize <= 0)
		throw love::Exception("Invalid SpriteBatch size.");

	if (newsize == size)
		return;

	// Map the old VertexBuffer to get a pointer to its data.
	void *old_data = nullptr;
	{
		VertexBuffer::Bind bind(*array_buf);
		old_data = array_buf->map();
	}

	size_t vertex_size = sizeof(Vertex) * 4 * newsize;
	VertexBuffer *new_array_buf = nullptr;

	try
	{
		new_array_buf = VertexBuffer::Create(vertex_size, array_buf->getTarget(), array_buf->getUsage());

		// Copy as much of the old data into the new VertexBuffer as can fit.
		VertexBuffer::Bind bind(*new_array_buf);
		void *new_data = new_array_buf->map();
		memcpy(new_data, old_data, sizeof(Vertex) * 4 * std::min(newsize, size));

		element_buf = VertexIndex(newsize);
	}
	catch (love::Exception &)
	{
		delete new_array_buf;
		throw;
	}

	// We don't need to unmap the old VertexBuffer since we're deleting it.
	delete array_buf;

	array_buf = new_array_buf;
	size = newsize;

	next = std::min(next, newsize);
}

int SpriteBatch::getBufferSize() const
{
	return size;
}

void SpriteBatch::draw(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky)
{
	const size_t pos_offset   = offsetof(Vertex, x);
	const size_t texel_offset = offsetof(Vertex, s);
	const size_t color_offset = offsetof(Vertex, r);

	if (next == 0)
		return;

	Matrix t(x, y, angle, sx, sy, ox, oy, kx, ky);

	OpenGL::TempTransform transform(gl);
	transform.get() *= t;

	texture->predraw();

	VertexBuffer::Bind array_bind(*array_buf);
	VertexBuffer::Bind element_bind(*element_buf.getVertexBuffer());

	// Make sure the VBO isn't mapped when we draw (sends data to GPU if needed.)
	array_buf->unmap(buffer_used_offset, buffer_used_size);
	buffer_used_offset = buffer_used_size = 0;

	Color curcolor = gl.getColor();

	// Apply per-sprite color, if a color is set.
	if (color)
	{
		glEnableVertexAttribArray(ATTRIB_COLOR);
		glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), array_buf->getPointer(color_offset));
	}

	glEnableVertexAttribArray(ATTRIB_POS);
	glVertexAttribPointer(ATTRIB_POS, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), array_buf->getPointer(pos_offset));

	glEnableVertexAttribArray(ATTRIB_TEXCOORD);
	glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), array_buf->getPointer(texel_offset));

	gl.prepareDraw();
	gl.drawElements(GL_TRIANGLES, element_buf.getIndexCount(next), element_buf.getType(), element_buf.getPointer(0));

	glDisableVertexAttribArray(ATTRIB_TEXCOORD);
	glDisableVertexAttribArray(ATTRIB_POS);

	if (color)
	{
		glDisableVertexAttribArray(ATTRIB_COLOR);
		gl.setColor(curcolor);
	}

	texture->postdraw();
}

void SpriteBatch::addv(const Vertex *v, const Matrix &m, int index)
{
	// Needed for colors.
	Vertex sprite[4] = {v[0], v[1], v[2], v[3]};
	const size_t sprite_size = 4 * sizeof(Vertex); // bytecount

	m.transform(sprite, sprite, 4);

	if (color)
		setColorv(sprite, *color);

	VertexBuffer::Bind bind(*array_buf);

	// Always keep the VBO mapped when adding data for now (it'll be unmapped
	// on draw.)
	array_buf->map();

	array_buf->fill(index * sprite_size, sprite_size, sprite);

	buffer_used_offset = std::min(buffer_used_offset, index * sprite_size);
	buffer_used_size = std::max(buffer_used_size, (index + 1) * sprite_size - buffer_used_offset);
}

void SpriteBatch::setColorv(Vertex *v, const Color &color)
{
	for (size_t i = 0; i < 4; ++i)
	{
		v[i].r = color.r;
		v[i].g = color.g;
		v[i].b = color.b;
		v[i].a = color.a;
	}
}

bool SpriteBatch::getConstant(const char *in, UsageHint &out)
{
	return usageHints.find(in, out);
}

bool SpriteBatch::getConstant(UsageHint in, const char *&out)
{
	return usageHints.find(in, out);
}

StringMap<SpriteBatch::UsageHint, SpriteBatch::USAGE_MAX_ENUM>::Entry SpriteBatch::usageHintEntries[] =
{
	{"dynamic", SpriteBatch::USAGE_DYNAMIC},
	{"static", SpriteBatch::USAGE_STATIC},
	{"stream", SpriteBatch::USAGE_STREAM},
};

StringMap<SpriteBatch::UsageHint, SpriteBatch::USAGE_MAX_ENUM> SpriteBatch::usageHints(usageHintEntries, sizeof(usageHintEntries));

} // opengl
} // graphics
} // love
