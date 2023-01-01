/**
 * Copyright (c) 2006-2023 LOVE Development Team
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

	vertex_stride = vertex::getFormatStride(vertex_format);

	size_t vertex_size = vertex_stride * 4 * size;
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

	const Vector2 *quadpositions = quad->getVertexPositions();
	const Vector2 *quadtexcoords = quad->getVertexTexCoords();

	// Always keep the buffer mapped when adding data (it'll be unmapped on draw.)
	size_t offset = (index == -1 ? next : index) * vertex_stride * 4;
	auto verts = (XYf_STf_RGBAub *) ((uint8 *) array_buf->map() + offset);

	m.transformXY(verts, quadpositions, 4);

	for (int i = 0; i < 4; i++)
	{
		verts[i].s = quadtexcoords[i].x;
		verts[i].t = quadtexcoords[i].y;
		verts[i].color = color;
	}

	array_buf->setMappedRangeModified(offset, vertex_stride * 4);

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

	const Vector2 *quadpositions = quad->getVertexPositions();
	const Vector2 *quadtexcoords = quad->getVertexTexCoords();

	// Always keep the buffer mapped when adding data (it'll be unmapped on draw.)
	size_t offset = (index == -1 ? next : index) * vertex_stride * 4;
	auto verts = (XYf_STPf_RGBAub *) ((uint8 *) array_buf->map() + offset);

	m.transformXY(verts, quadpositions, 4);

	for (int i = 0; i < 4; i++)
	{
		verts[i].s = quadtexcoords[i].x;
		verts[i].t = quadtexcoords[i].y;
		verts[i].p = (float) layer;
		verts[i].color = color;
	}

	array_buf->setMappedRangeModified(offset, vertex_stride * 4);

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

void SpriteBatch::setColor(const Colorf &c)
{
	color_active = true;

	Colorf cclamped;
	cclamped.r = std::min(std::max(c.r, 0.0f), 1.0f);
	cclamped.g = std::min(std::max(c.g, 0.0f), 1.0f);
	cclamped.b = std::min(std::max(c.b, 0.0f), 1.0f);
	cclamped.a = std::min(std::max(c.a, 0.0f), 1.0f);

	this->color = toColor32(cclamped);
}

void SpriteBatch::setColor()
{
	color_active = false;
	color = Color32(255, 255, 255, 255);
}

Colorf SpriteBatch::getColor(bool &active) const
{
	active = color_active;
	return toColorf(color);
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

	size_t vertex_size = vertex_stride * 4 * newsize;
	love::graphics::Buffer *new_array_buf = nullptr;

	int new_next = std::min(next, newsize);

	try
	{
		auto gfx = Module::getInstance<graphics::Graphics>(Module::M_GRAPHICS);
		new_array_buf = gfx->newBuffer(vertex_size, nullptr, array_buf->getType(), array_buf->getUsage(), array_buf->getMapFlags());

		// Copy as much of the old data into the new GLBuffer as can fit.
		size_t copy_size = vertex_stride * 4 * new_next;
		array_buf->copyTo(0, copy_size, new_array_buf, 0);
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
	using namespace vertex;

	if (next == 0)
		return;

	gfx->flushStreamDraws();

	if (texture.get())
	{
		if (Shader::isDefaultActive())
		{
			Shader::StandardShader defaultshader = Shader::STANDARD_DEFAULT;
			if (texture->getTextureType() == TEXTURE_2D_ARRAY)
				defaultshader = Shader::STANDARD_ARRAY;

			Shader::attachDefault(defaultshader);
		}

		if (Shader::current)
			Shader::current->checkMainTexture(texture);
	}

	// Make sure the buffer isn't mapped when we draw (sends data to GPU if needed.)
	array_buf->unmap();

	Attributes attributes;
	BufferBindings buffers;

	{
		buffers.set(0, array_buf, 0);
		attributes.setCommonFormat(vertex_format, 0);

		if (!color_active)
			attributes.disable(ATTRIB_COLOR);
	}

	int activebuffers = 1;

	for (const auto &it : attached_attributes)
	{
		Mesh *mesh = it.second.mesh.get();

		// We have to do this check here as wll because setBufferSize can be
		// called after attachAttribute.
		if (mesh->getVertexCount() < (size_t) next * 4)
			throw love::Exception("Mesh with attribute '%s' attached to this SpriteBatch has too few vertices", it.first.c_str());

		int attributeindex = -1;

		// If the attribute is one of the LOVE-defined ones, use the constant
		// attribute index for it, otherwise query the index from the shader.
		BuiltinVertexAttribute builtinattrib;
		if (vertex::getConstant(it.first.c_str(), builtinattrib))
			attributeindex = (int) builtinattrib;
		else if (Shader::current)
			attributeindex = Shader::current->getVertexAttributeIndex(it.first);

		if (attributeindex >= 0)
		{
			// Make sure the buffer isn't mapped (sends data to GPU if needed.)
			mesh->vertexBuffer->unmap();

			const auto &formats = mesh->getVertexFormat();
			const auto &format = formats[it.second.index];

			uint16 offset = (uint16) mesh->getAttributeOffset(it.second.index);
			uint16 stride = (uint16) mesh->getVertexStride();

			attributes.set(attributeindex, format.type, (uint8) format.components, offset, activebuffers);
			attributes.setBufferLayout(activebuffers, stride);

			// TODO: We should reuse buffer bindings with the same buffer+stride+step.
			buffers.set(activebuffers, mesh->vertexBuffer, 0);
			activebuffers++;
		}
	}

	Graphics::TempTransform transform(gfx, m);

	int start = std::min(std::max(0, range_start), next - 1);

	int count = next;
	if (range_count > 0)
		count = std::min(count, range_count);

	count = std::min(count, next - start);

	if (count > 0)
		gfx->drawQuads(start, count, attributes, buffers, texture);
}

} // graphics
} // love
