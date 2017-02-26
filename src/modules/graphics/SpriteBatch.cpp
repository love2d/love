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

// LOVE
#include "Texture.h"
#include "Quad.h"
#include "Graphics.h"
#include "Buffer.h"

// C++
#include <algorithm>

// C
#include <stddef.h>

namespace love
{
namespace graphics
{

love::Type SpriteBatch::type("SpriteBatch", &Drawable::type);

SpriteBatch::SpriteBatch(Graphics *gfx, Texture *texture, int size, vertex::Usage usage)
	: texture(texture)
	, size(size)
	, next(0)
	, color(255, 255, 255, 255)
	, color_active(false)
	, array_buf(nullptr)
	, quad_indices(gfx, size)
	, range_start(-1)
	, range_count(-1)
{
	if (size <= 0)
		throw love::Exception("Invalid SpriteBatch size.");

	if (texture == nullptr)
		throw love::Exception("A texture must be used when creating a SpriteBatch.");

	if (texture->getTextureType() == TEXTURE_2D_ARRAY)
		vertex_format = vertex::CommonFormat::XYf_STPf_RGBAub;
	else
		vertex_format = vertex::CommonFormat::XYf_STf_RGBAub;

	format_stride = vertex::getFormatStride(vertex_format);

	size_t vertex_size = format_stride * 4 * size;
	array_buf = gfx->newBuffer(vertex_size, nullptr, BUFFER_VERTEX, usage, Buffer::MAP_EXPLICIT_RANGE_MODIFY);
}

SpriteBatch::~SpriteBatch()
{
	delete array_buf;
}

int SpriteBatch::add(const Matrix4 &m, int index /*= -1*/)
{
	return add(texture->getQuad(), m, index);
}

int SpriteBatch::add(Quad *quad, const Matrix4 &m, int index /*= -1*/)
{
	using namespace vertex;

	if (vertex_format == CommonFormat::XYf_STPf_RGBAub)
		return addLayer(quad->getLayer(), quad, m, index);

	if (index < -1 || index >= size)
		throw love::Exception("Invalid sprite index: %d", index + 1);

	if (index == -1 && next >= size)
		setBufferSize(size * 2);

	const XYf_STf *quadverts = quad->getVertices();

	// Always keep the VBO mapped when adding data (it'll be unmapped on draw.)
	size_t offset = (index == -1 ? next : index) * format_stride * 4;
	auto verts = (XYf_STf_RGBAub *) ((uint8 *) array_buf->map() + offset);

	m.transform(verts, quadverts, 4);

	for (int i = 0; i < 4; i++)
	{
		verts[i].s = quadverts[i].s;
		verts[i].t = quadverts[i].t;
		verts[i].color = color;
	}

	array_buf->setMappedRangeModified(offset, format_stride * 4);

	// Increment counter.
	if (index == -1)
		return next++;

	return index;
}

int SpriteBatch::addLayer(int layer, const Matrix4 &m, int index)
{
	return addLayer(layer, texture->getQuad(), m, index);
}

int SpriteBatch::addLayer(int layer, Quad *quad, const Matrix4 &m, int index)
{
	using namespace vertex;

	if (vertex_format != CommonFormat::XYf_STPf_RGBAub)
		throw love::Exception("addLayer can only be called on a SpriteBatch that uses an Array Texture!");

	if (index < -1 || index >= size)
		throw love::Exception("Invalid sprite index: %d", index + 1);

	if (layer < 0 || layer >= texture->getLayerCount())
		throw love::Exception("Invalid layer: %d (Texture has %d layers)", layer + 1, texture->getLayerCount());

	if (index == -1 && next >= size)
		setBufferSize(size * 2);

	const XYf_STf *quadverts = quad->getVertices();

	// Always keep the VBO mapped when adding data (it'll be unmapped on draw.)
	size_t offset = (index == -1 ? next : index) * format_stride * 4;
	auto verts = (XYf_STPf_RGBAub *) ((uint8 *) array_buf->map() + offset);

	m.transform(verts, quadverts, 4);

	for (int i = 0; i < 4; i++)
	{
		verts[i].s = quadverts[i].s;
		verts[i].t = quadverts[i].t;
		verts[i].p = (float) layer;
		verts[i].color = color;
	}

	array_buf->setMappedRangeModified(offset, format_stride * 4);

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
	if (texture->getTextureType() != newtexture->getTextureType())
		throw love::Exception("Texture must have the same texture type as the SpriteBatch's previous texture.");

	texture.set(newtexture);
}

Texture *SpriteBatch::getTexture() const
{
	return texture.get();
}

void SpriteBatch::setColor(const Color &color)
{
	color_active = true;
	this->color = color;
}

void SpriteBatch::setColor()
{
	color_active = false;
	color = Color(255, 255, 255, 255);
}

const Color &SpriteBatch::getColor(bool &active) const
{
	active = color_active;
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

	size_t vertex_size = format_stride * 4 * newsize;
	love::graphics::Buffer *new_array_buf = nullptr;

	int new_next = std::min(next, newsize);

	try
	{
		auto gfx = Module::getInstance<graphics::Graphics>(Module::M_GRAPHICS);
		new_array_buf = gfx->newBuffer(vertex_size, nullptr, array_buf->getType(), array_buf->getUsage(), array_buf->getMapFlags());

		// Copy as much of the old data into the new GLBuffer as can fit.
		size_t copy_size = format_stride * 4 * new_next;
		array_buf->copyTo(0, copy_size, new_array_buf, 0);

		quad_indices = QuadIndices(gfx, newsize);
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

} // graphics
} // love
