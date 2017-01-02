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
#include "SpriteBatch.h"

// OpenGL
#include "OpenGL.h"

// LOVE
#include "GLBuffer.h"
#include "graphics/Texture.h"
#include "graphics/Graphics.h"

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

love::Type SpriteBatch::type("SpriteBatch", &Drawable::type);

SpriteBatch::SpriteBatch(Texture *texture, int size, vertex::Usage usage)
	: texture(texture)
	, size(size)
	, next(0)
	, color(0)
	, array_buf(nullptr)
	, quad_indices(size)
	, range_start(-1)
	, range_count(-1)
{
	if (size <= 0)
		throw love::Exception("Invalid SpriteBatch size.");

	size_t vertex_size = sizeof(Vertex) * 4 * size;

	array_buf = new GLBuffer(vertex_size, nullptr, BUFFER_VERTEX, usage, GLBuffer::MAP_EXPLICIT_RANGE_MODIFY);
}

SpriteBatch::~SpriteBatch()
{
	delete color;
	delete array_buf;
}

int SpriteBatch::add(const Matrix4 &m, int index /*= -1*/)
{
	if (index < -1 || index >= size)
		throw love::Exception("Invalid sprite index: %d", index + 1);

	if (index == -1 && next >= size)
		setBufferSize(size * 2);

	addv(texture->getVertices(), m, (index == -1) ? next : index);

	// Increment counter.
	if (index == -1)
		return next++;

	return index;
}

int SpriteBatch::addq(Quad *quad, const Matrix4 &m, int index /*= -1*/)
{
	if (index < -1 || index >= size)
		throw love::Exception("Invalid sprite index: %d", index + 1);

	if (index == -1 && next >= size)
		setBufferSize(size * 2);

	addv(quad->getVertices(), m, (index == -1) ? next : index);

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
	array_buf->unmap();
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
	color = nullptr;
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

	// Map the old GLBuffer to get a pointer to its data.
	void *old_data = array_buf->map();

	size_t vertex_size = sizeof(Vertex) * 4 * newsize;
	GLBuffer *new_array_buf = nullptr;

	int new_next = std::min(next, newsize);

	try
	{
		new_array_buf = new GLBuffer(vertex_size, nullptr, array_buf->getType(), array_buf->getUsage(), array_buf->getMapFlags());

		// Copy as much of the old data into the new GLBuffer as can fit.
		size_t copy_size = sizeof(Vertex) * 4 * new_next;
		memcpy(new_array_buf->map(), old_data, copy_size);
		new_array_buf->setMappedRangeModified(0, copy_size);

		quad_indices = QuadIndices(newsize);
	}
	catch (love::Exception &)
	{
		delete new_array_buf;
		throw;
	}

	// We don't need to unmap the old GLBuffer since we're deleting it.
	delete array_buf;

	array_buf = new_array_buf;
	size = newsize;

	next = new_next;
}

int SpriteBatch::getBufferSize() const
{
	return size;
}

void SpriteBatch::attachAttribute(const std::string &name, Mesh *mesh)
{
	AttachedAttribute oldattrib = {};
	AttachedAttribute newattrib = {};

	if (mesh->getVertexCount() < (size_t) next * 4)
		throw love::Exception("Mesh has too few vertices to be attached to this SpriteBatch (at least %d vertices are required)", next*4);

	auto it = attached_attributes.find(name);
	if (it != attached_attributes.end())
		oldattrib = it->second;

	newattrib.index = mesh->getAttributeIndex(name);

	if (newattrib.index < 0)
		throw love::Exception("The specified mesh does not have a vertex attribute named '%s'", name.c_str());

	newattrib.mesh = mesh;

	attached_attributes[name] = newattrib;
}

void SpriteBatch::setDrawRange(int start, int count)
{
	if (start < 0 || count <= 0)
		throw love::Exception("Invalid draw range.");

	range_start = start;
	range_count = count;
}

void SpriteBatch::setDrawRange()
{
	range_start = range_count = -1;
}

bool SpriteBatch::getDrawRange(int &start, int &count) const
{
	if (range_start < 0 || range_count <= 0)
		return false;

	start = range_start;
	count = range_count;
	return true;
}

void SpriteBatch::draw(Graphics *gfx, const Matrix4 &m)
{
	const size_t pos_offset   = offsetof(Vertex, x);
	const size_t texel_offset = offsetof(Vertex, s);
	const size_t color_offset = offsetof(Vertex, color.r);

	if (next == 0)
		return;

	gfx->flushStreamDraws();

	OpenGL::TempDebugGroup debuggroup("SpriteBatch draw");

	Graphics::TempTransform transform(gfx, m);

	gl.bindTextureToUnit(texture, 0, false);

	uint32 enabledattribs = ATTRIBFLAG_POS | ATTRIBFLAG_TEXCOORD;

	// Make sure the VBO isn't mapped when we draw (sends data to GPU if needed.)
	array_buf->unmap();

	array_buf->bind();

	// Apply per-sprite color, if a color is set.
	if (color)
	{
		enabledattribs |= ATTRIBFLAG_COLOR;
		glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), array_buf->getPointer(color_offset));
	}

	glVertexAttribPointer(ATTRIB_POS, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), array_buf->getPointer(pos_offset));
	glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), array_buf->getPointer(texel_offset));

	for (const auto &it : attached_attributes)
	{
		Mesh *mesh = it.second.mesh.get();

		// We have to do this check here as wll because setBufferSize can be
		// called after attachAttribute.
		if (mesh->getVertexCount() < (size_t) next * 4)
			throw love::Exception("Mesh with attribute '%s' attached to this SpriteBatch has too few vertices", it.first.c_str());

		int location = mesh->bindAttributeToShaderInput(it.second.index, it.first);

		if (location >= 0)
			enabledattribs |= 1u << (uint32) location;
	}

	gl.useVertexAttribArrays(enabledattribs);

	gl.prepareDraw();

	int start = std::min(std::max(0, range_start), next - 1);

	int count = next;
	if (range_count > 0)
		count = std::min(count, range_count);

	count = std::min(count, next - start);

	quad_indices.getBuffer()->bind();
	const void *indices = quad_indices.getPointer(start * quad_indices.getElementSize());

	if (count > 0)
		gl.drawElements(GL_TRIANGLES, (GLsizei) quad_indices.getIndexCount(count), quad_indices.getType(), indices);
}

void SpriteBatch::addv(const Vertex *v, const Matrix4 &m, int index)
{
	// Needed for colors.
	Vertex sprite[4] = {v[0], v[1], v[2], v[3]};
	const size_t sprite_size = 4 * sizeof(Vertex); // bytecount

	m.transform(sprite, sprite, 4);

	if (color != nullptr)
	{
		for (int i = 0; i < 4; i++)
			sprite[i].color = *color;
	}

	// Always keep the VBO mapped when adding data for now (it'll be unmapped
	// on draw.)
	array_buf->map();

	array_buf->fill(index * sprite_size, sprite_size, sprite);
}

} // opengl
} // graphics
} // love
