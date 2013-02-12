/**
 * Copyright (c) 2006-2013 LOVE Development Team
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

#include "SpriteBatch.h"

// STD
#include <algorithm> // std::find

// OpenGL
#include "OpenGL.h"

// LOVE
#include "Image.h"
#include "Quad.h"
#include "VertexBuffer.h"

namespace love
{
namespace graphics
{
namespace opengl
{

SpriteBatch::SpriteBatch(Image *image, int size, int usage)
	: image(image)
	, size(size)
	, next(0)
	, color(0)
	, array_buf(0)
	, element_buf(0)
{
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

	const size_t vertex_size = sizeof(vertex) * 4 * size;

	try
	{
		array_buf = VertexBuffer::Create(vertex_size, GL_ARRAY_BUFFER, gl_usage);
		element_buf = new VertexIndex(size);
	}
	catch (love::Exception &)
	{
		delete array_buf;
		delete element_buf;
		throw;
	}
	catch (std::bad_alloc &)
	{
		delete array_buf;
		delete element_buf;
		throw love::Exception("Out of memory.");
	}

	image->retain();
}

SpriteBatch::~SpriteBatch()
{
	image->release();

	delete color;
	delete array_buf;
	delete element_buf;
}

int SpriteBatch::add(float x, float y, float a, float sx, float sy, float ox, float oy, float kx, float ky, int index /*= -1*/)
{
	// Only do this if there's a free slot.
	if ((index == -1 && next >= size && gaps.size() == 0) || index < -1 || index >= size)
		return -1;

	// Determine where in the vertex buffer to insert into, using the gap list
	// if possible.
	int realindex;
	if (index == -1 && gaps.size() > 0)
	{
		realindex = gaps.front();
		gaps.pop_front();
	}
	else
	{
		realindex = (index == -1) ? next : index;
		std::deque<int>::iterator it = std::find(gaps.begin(), gaps.end(), realindex);
		if (it != gaps.end())
			gaps.erase(it); // This index is no longer a gap.
	}

	// Needed for colors.
	memcpy(sprite, image->getVertices(), sizeof(vertex)*4);

	// Transform.
	Matrix t;
	t.setTransformation(x, y, a, sx, sy, ox, oy, kx, ky);
	t.transform(sprite, sprite, 4);

	if (color)
		setColorv(sprite, *color);
	

	addv(sprite, realindex);

	// Increment counter.
	if (index == -1 && realindex == next)
		return next++;

	return realindex;
}

int SpriteBatch::addq(Quad *quad, float x, float y, float a, float sx, float sy, float ox, float oy, float kx, float ky, int index /*= -1*/)
{
	// Only do this if there's a free slot.
	if ((index == -1 && next >= size && gaps.size() == 0) || index < -1 || index >= next)
		return -1;

	// Determine where in the vertex buffer to insert into, using the gap list
	// if possible.
	int realindex;
	if (index == -1 && gaps.size() > 0)
	{
		realindex = gaps.front();
		gaps.pop_front();
	}
	else
	{
		realindex = (index == -1) ? next : index;
		std::deque<int>::iterator it = std::find(gaps.begin(), gaps.end(), realindex);
		if (it != gaps.end())
			gaps.erase(it); // This index is no longer a gap.
	}

	// Needed for colors.
	memcpy(sprite, quad->getVertices(), sizeof(vertex)*4);

	// Transform.
	Matrix t;
	t.setTransformation(x, y, a, sx, sy, ox, oy, kx, ky);
	t.transform(sprite, sprite, 4);

	if (color)
		setColorv(sprite, *color);

	addv(sprite, realindex);

	// Increment counter.
	if (index == -1 && realindex == next)
		return next++;

	return realindex;
}

void SpriteBatch::remove(int index)
{
	if (index < 0 || index >= next || next <= 0)
		return;

	// If this is the last index in the sprite list, decrease the total sprite
	// count instead of adding a dummy sprite.
	if (index == next - 1)
	{
		int spritecount = index;

		// Remove all consecutive gaps at the end of the sprite list.
		std::deque<int>::iterator it;
		while ((it = std::find(gaps.begin(), gaps.end(), --spritecount)) != gaps.end())
			gaps.erase(it);

		next = spritecount + 1;
		return;
	}
	else if (std::find(gaps.begin(), gaps.end(), index) != gaps.end())
		return; // Don't add the same index to the gap list twice.


	// OpenGL won't render any primitive whose vertices are all identical.
	for (int i = 0; i < 4; i++)
		sprite[i].x = sprite[i].y = 0;

	// Replace the existing sprite at this index with the dummy sprite.
	addv(sprite, index);

	gaps.push_back(index);
}

void SpriteBatch::clear()
{
	// Reset the position of the next index.
	next = 0;
	gaps.clear();
}

void *SpriteBatch::lock()
{
	VertexBuffer::Bind bind(*array_buf);

	return array_buf->map();
}

void SpriteBatch::unlock()
{
	VertexBuffer::Bind bind(*array_buf);

	array_buf->unmap();
}

void SpriteBatch::setImage(Image *newimage)
{
	image->release();
	image = newimage;
	image->retain();
}

Image *SpriteBatch::getImage()
{
	return image;
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

void SpriteBatch::draw(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky) const
{
	const int color_offset = 0;
	const int vertex_offset = sizeof(unsigned char) * 4;
	const int texel_offset = sizeof(unsigned char) * 4 + sizeof(float) * 2;

	static Matrix t;

	glPushMatrix();

	t.setTransformation(x, y, angle, sx, sy, ox, oy, kx, ky);
	glMultMatrixf((const GLfloat *)t.getElements());

	image->bind();

	VertexBuffer::Bind array_bind(*array_buf);
	VertexBuffer::Bind element_bind(*element_buf->getVertexBuffer());

	// Apply per-sprite color, if a color is set.
	if (color)
	{
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(vertex), array_buf->getPointer(color_offset));
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, sizeof(vertex), array_buf->getPointer(vertex_offset));

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), array_buf->getPointer(texel_offset));

	glDrawElements(GL_TRIANGLES, element_buf->getIndexCount(next), element_buf->getType(), element_buf->getPointer(0));

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if (color)
		glDisableClientState(GL_COLOR_ARRAY);

	glPopMatrix();
}

void SpriteBatch::addv(const vertex *v, int index)
{
	int sprite_size = sizeof(vertex) * 4;

	VertexBuffer::Bind bind(*array_buf);

	array_buf->fill(index * sprite_size, sprite_size, v);
}

void SpriteBatch::setColorv(vertex *v, const Color &color)
{
	v[0].r = color.r;
	v[0].g = color.g;
	v[0].b = color.b;
	v[0].a = color.a;
	v[1].r = color.r;
	v[1].g = color.g;
	v[1].b = color.b;
	v[1].a = color.a;
	v[2].r = color.r;
	v[2].g = color.g;
	v[2].b = color.b;
	v[2].a = color.a;
	v[3].r = color.r;
	v[3].g = color.g;
	v[3].b = color.b;
	v[3].a = color.a;
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
