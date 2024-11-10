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

SpriteBatch::SpriteBatch(Graphics *gfx, Texture *texture, int size, BufferDataUsage usage)
	: texture(texture)
	, size(size)
	, next(0)
	, color(255, 255, 255, 255)
	, colorf(1.0f, 1.0f, 1.0f, 1.0f)
	, attributesID()
	, array_buf(nullptr)
	, vertex_data(nullptr)
	, modified_sprites()
	, range_start(-1)
	, range_count(-1)
{
	if (size <= 0)
		throw love::Exception("Invalid SpriteBatch size.");

	if (texture == nullptr)
		throw love::Exception("A texture must be used when creating a SpriteBatch.");

	if (texture->getTextureType() == TEXTURE_2D_ARRAY)
		vertex_format = CommonFormat::XYf_STPf_RGBAub;
	else
		vertex_format = CommonFormat::XYf_STf_RGBAub;

	vertex_stride = getFormatStride(vertex_format);

	size_t vertex_size = vertex_stride * 4 * size;

	vertex_data = (uint8 *) malloc(vertex_size);
	if (vertex_data == nullptr)
		throw love::Exception("Out of memory.");

	memset(vertex_data, 0, vertex_size);

	Buffer::Settings settings(BUFFERUSAGEFLAG_VERTEX, usage);
	auto decl = Buffer::getCommonFormatDeclaration(vertex_format);

	array_buf.set(gfx->newBuffer(settings, decl, nullptr, vertex_size, 0), Acquire::NORETAIN);
}

SpriteBatch::~SpriteBatch()
{
	free(vertex_data);
}

int SpriteBatch::add(const Matrix4 &m, int index /*= -1*/)
{
	return add(texture->getQuad(), m, index);
}

int SpriteBatch::add(Quad *quad, const Matrix4 &m, int index /*= -1*/)
{
	if (vertex_format == CommonFormat::XYf_STPf_RGBAub)
		return addLayer(quad->getLayer(), quad, m, index);

	if (index < -1 || index >= size)
		throw love::Exception("Invalid sprite index: %d", index + 1);

	if (index == -1 && next >= size)
		setBufferSize(size * 2);

	const Vector2 *quadpositions = quad->getVertexPositions();
	const Vector2 *quadtexcoords = quad->getVertexTexCoords();

	int spriteindex = (index == -1 ? next : index);

	size_t offset = spriteindex * vertex_stride * 4;
	auto verts = (XYf_STf_RGBAub *) (vertex_data + offset);

	m.transformXY(verts, quadpositions, 4);

	for (int i = 0; i < 4; i++)
	{
		verts[i].s = quadtexcoords[i].x;
		verts[i].t = quadtexcoords[i].y;
		verts[i].color = color;
	}

	modified_sprites.encapsulate(spriteindex);

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
	if (vertex_format != CommonFormat::XYf_STPf_RGBAub)
		throw love::Exception("addLayer can only be called on a SpriteBatch that uses an Array Texture.");

	if (index < -1 || index >= size)
		throw love::Exception("Invalid sprite index: %d", index + 1);

	if (layer < 0 || layer >= texture->getLayerCount())
		throw love::Exception("Invalid layer: %d (Texture has %d layers)", layer + 1, texture->getLayerCount());

	if (index == -1 && next >= size)
		setBufferSize(size * 2);

	const Vector2 *quadpositions = quad->getVertexPositions();
	const Vector2 *quadtexcoords = quad->getVertexTexCoords();

	int spriteindex = (index == -1 ? next : index);

	size_t offset = spriteindex * vertex_stride * 4;
	auto verts = (XYf_STPf_RGBAub *) (vertex_data + offset);

	m.transformXY(verts, quadpositions, 4);

	for (int i = 0; i < 4; i++)
	{
		verts[i].s = quadtexcoords[i].x;
		verts[i].t = quadtexcoords[i].y;
		verts[i].p = (float) layer;
		verts[i].color = color;
	}

	modified_sprites.encapsulate(spriteindex);

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
	if (modified_sprites.isValid())
	{
		size_t offset = modified_sprites.getOffset() * vertex_stride * 4;
		size_t size = modified_sprites.getSize() * vertex_stride * 4;

		if (array_buf->getDataUsage() == BUFFERDATAUSAGE_STREAM)
			array_buf->fill(0, array_buf->getSize(), vertex_data);
		else
			array_buf->fill(offset, size, vertex_data + offset);

		modified_sprites.invalidate();
	}
}

void SpriteBatch::setTexture(Texture *newtexture)
{
	if (texture->getTextureType() != newtexture->getTextureType())
		throw love::Exception("Texture must have the same type as the SpriteBatch's previous texture.");

	texture.set(newtexture);
}

Texture *SpriteBatch::getTexture() const
{
	return texture.get();
}

void SpriteBatch::setColor(const Colorf &c)
{
	colorf.r = std::min(std::max(c.r, 0.0f), 1.0f);
	colorf.g = std::min(std::max(c.g, 0.0f), 1.0f);
	colorf.b = std::min(std::max(c.b, 0.0f), 1.0f);
	colorf.a = std::min(std::max(c.a, 0.0f), 1.0f);

	color = toColor32(colorf);
}

Colorf SpriteBatch::getColor() const
{
	return colorf;
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

	int new_next = std::min(next, newsize);

	void *new_vertex_data = realloc(vertex_data, vertex_size);
	if (new_vertex_data == nullptr)
		throw love::Exception("Out of memory.");

	auto gfx = Module::getInstance<graphics::Graphics>(Module::M_GRAPHICS);
	Buffer::Settings settings(array_buf->getUsageFlags(), array_buf->getDataUsage());
	auto decl = Buffer::getCommonFormatDeclaration(vertex_format);

	array_buf.set(gfx->newBuffer(settings, decl, nullptr, vertex_size, 0), Acquire::NORETAIN);

	array_buf->fill(0, vertex_stride * 4 * new_next, new_vertex_data);

	vertex_data = (uint8 *) new_vertex_data;

	size = newsize;
	next = new_next;

	attributesID.invalidate();
}

int SpriteBatch::getBufferSize() const
{
	return size;
}

void SpriteBatch::attachAttribute(const std::string &name, Buffer *buffer, Mesh *mesh)
{
	if ((buffer->getUsageFlags() & BUFFERUSAGEFLAG_VERTEX) == 0)
		throw love::Exception("GraphicsBuffer must be created with vertex buffer support to be used as a SpriteBatch vertex attribute.");

	AttachedAttribute oldattrib = {};
	AttachedAttribute newattrib = {};

	if (buffer->getArrayLength() < (size_t) next * 4)
		throw love::Exception("Buffer has too few vertices to be attached to this SpriteBatch (at least %d vertices are required)", next*4);

	auto it = attached_attributes.find(name);
	if (it != attached_attributes.end())
		oldattrib = it->second;

	newattrib.index = buffer->getDataMemberIndex(name);

	if (newattrib.index < 0)
		throw love::Exception("The specified Buffer does not have a vertex attribute named '%s'", name.c_str());

	newattrib.buffer = buffer;
	newattrib.mesh = mesh;
	newattrib.bindingIndex = buffer->getDataMember(newattrib.index).decl.bindingLocation;

	BuiltinVertexAttribute builtinattrib;
	if (newattrib.bindingIndex < 0 && getConstant(name.c_str(), builtinattrib))
		newattrib.bindingIndex = (int)builtinattrib;

	attached_attributes[name] = newattrib;

	attributesID.invalidate();
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

void SpriteBatch::updateVertexAttributes(Graphics *gfx)
{
	VertexAttributes attributes;
	BufferBindings &buffers = bufferBindings;

	buffers.set(0, array_buf, 0);
	attributes.setCommonFormat(vertex_format, 0);

	int activebuffers = 1;

	for (const auto &it : attached_attributes)
	{
		Buffer *buffer = it.second.buffer.get();

		int bindingindex = it.second.bindingIndex;

		// If the attribute is one of the LOVE-defined ones, use the constant
		// attribute index for it, otherwise query the index from the shader.
		if (bindingindex < 0 && Shader::current)
			bindingindex = Shader::current->getVertexAttributeIndex(it.first);

		if (bindingindex >= 0)
		{
			const auto &member = buffer->getDataMember(it.second.index);

			uint16 offset = (uint16) buffer->getMemberOffset(it.second.index);
			uint16 stride = (uint16) buffer->getArrayStride();

			int bufferindex = activebuffers;

			for (int i = 1; i < activebuffers; i++)
			{
				if (buffers.info[i].buffer == buffer && attributes.bufferLayouts[i].stride == stride)
				{
					bufferindex = i;
					break;
				}
			}

			attributes.set(bindingindex, member.decl.format, offset, bufferindex);
			attributes.setBufferLayout(bufferindex, stride);

			buffers.set(bufferindex, buffer, 0);

			activebuffers = std::max(activebuffers, bufferindex + 1);
		}
	}

	attributesID = gfx->registerVertexAttributes(attributes);
}

void SpriteBatch::draw(Graphics *gfx, const Matrix4 &m)
{
	if (next == 0)
		return;

	gfx->flushBatchedDraws();

	if (texture.get())
	{
		if (Shader::isDefaultActive())
		{
			Shader::StandardShader defaultshader = Shader::STANDARD_DEFAULT;
			if (texture->getTextureType() == TEXTURE_2D_ARRAY)
				defaultshader = Shader::STANDARD_ARRAY;

			Shader::attachDefault(defaultshader);
		}
	}

	if (Shader::current)
		Shader::current->validateDrawState(PRIMITIVE_TRIANGLES, texture);

	flush(); // Upload any modified sprite data to the GPU.

	bool attributesIDneedsupdate = !attributesID.isValid();

	for (const auto &it : attached_attributes)
	{
		Buffer *buffer = it.second.buffer.get();

		// We have to do this check here as wll because setBufferSize can be
		// called after attachAttribute.
		if (buffer->getArrayLength() < (size_t) next * 4)
			throw love::Exception("Buffer with attribute '%s' attached to this SpriteBatch has too few vertices", it.first.c_str());

		// If the attribute is one of the LOVE-defined ones, use the constant
		// attribute index for it, otherwise query the index from the shader.
		if (it.second.bindingIndex < 0)
			attributesIDneedsupdate = true;

		if (it.second.mesh.get())
			it.second.mesh->flush();
	}

	if (attributesIDneedsupdate)
		updateVertexAttributes(gfx);

	Graphics::TempTransform transform(gfx, m);

	int start = std::min(std::max(0, range_start), next - 1);

	int count = next;
	if (range_count > 0)
		count = std::min(count, range_count);

	count = std::min(count, next - start);

	if (count > 0)
	{
		Texture *tex = gfx->getTextureOrDefaultForActiveShader(texture);
		gfx->drawQuads(start, count, attributesID, bufferBindings, tex);
	}
}

} // graphics
} // love
