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

// LOVE
#include "Mesh.h"
#include "common/Matrix.h"
#include "common/Exception.h"

// C++
#include <algorithm>

namespace love
{
namespace graphics
{
namespace opengl
{

Mesh::Mesh(const std::vector<Vertex> &verts, Mesh::DrawMode mode)
	: vbo(nullptr)
	, vertex_count(0)
	, ibo(nullptr)
	, element_count(0)
	, element_data_type(getGLDataTypeFromMax(verts.size()))
	, draw_mode(mode)
	, range_min(-1)
	, range_max(-1)
	, texture(nullptr)
	, colors_enabled(false)
{
	setVertices(verts);
}

Mesh::Mesh(int vertexcount, Mesh::DrawMode mode)
	: vbo(nullptr)
	, vertex_count(0)
	, ibo(nullptr)
	, element_count(0)
	, element_data_type(getGLDataTypeFromMax(vertexcount))
	, draw_mode(mode)
	, range_min(-1)
	, range_max(-1)
	, texture(nullptr)
	, colors_enabled(false)
{
	if (vertexcount < 1)
		throw love::Exception("Invalid number of vertices.");

	std::vector<Vertex> verts(vertexcount);

	// Default-initialized vertices should have a white opaque color.
	for (size_t i = 0; i < verts.size(); i++)
	{
		verts[i].r = 255;
		verts[i].g = 255;
		verts[i].b = 255;
		verts[i].a = 255;
	}

	setVertices(verts);
}

Mesh::~Mesh()
{
	delete vbo;
	delete ibo;
}

void Mesh::setVertices(const std::vector<Vertex> &verts)
{
	if (verts.size() == 0)
		throw love::Exception("At least one vertex is required.");

	size_t size = sizeof(Vertex) * verts.size();

	if (vbo && size > vbo->getSize())
	{
		delete vbo;
		vbo = nullptr;
	}

	if (!vbo)
	{
		// Full memory backing because we might access the data at any time.
		vbo = VertexBuffer::Create(size, GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, VertexBuffer::BACKING_FULL);
	}

	vertex_count = verts.size();

	VertexBuffer::Bind vbo_bind(*vbo);
	VertexBuffer::Mapper vbo_mapper(*vbo);

	// Fill the buffer with the vertices.
	memcpy(vbo_mapper.get(), &verts[0], size);
}

const Vertex *Mesh::getVertices() const
{
	if (vbo)
	{
		VertexBuffer::Bind vbo_bind(*vbo);
		return (Vertex *) vbo->map();
	}

	return nullptr;
}

void Mesh::setVertex(size_t index, const Vertex &v)
{
	if (index >= vertex_count)
		throw love::Exception("Invalid vertex index: %ld", index + 1);

	VertexBuffer::Bind vbo_bind(*vbo);

	// We unmap the vertex buffer in Mesh::draw. This lets us coalesce the
	// buffer transfer calls into just one.
	Vertex *vertices = (Vertex *) vbo->map();
	vertices[index] = v;
}

Vertex Mesh::getVertex(size_t index) const
{
	if (index >= vertex_count)
		throw love::Exception("Invalid vertex index: %ld", index + 1);

	VertexBuffer::Bind vbo_bind(*vbo);

	// We unmap the vertex buffer in Mesh::draw.
	Vertex *vertices = (Vertex *) vbo->map();
	return vertices[index];
}

size_t Mesh::getVertexCount() const
{
	return vertex_count;
}

/**
 * Copies index data from a vector to a mapped index buffer.
 **/
template <typename T>
static void copyToIndexBuffer(const std::vector<uint32> &indices, VertexBuffer::Mapper &buffermap, size_t maxval)
{
	T *elems = (T *) buffermap.get();

	for (size_t i = 0; i < indices.size(); i++)
	{
		if (indices[i] >= maxval)
			throw love::Exception("Invalid vertex map value: %d", indices[i] + 1);

		elems[i] = (T) indices[i];
	}
}

void Mesh::setVertexMap(const std::vector<uint32> &map)
{
	GLenum datatype = getGLDataTypeFromMax(vertex_count);

	// Calculate the size in bytes of the index buffer data.
	size_t size = map.size() * getGLDataTypeSize(datatype);

	if (ibo && size > ibo->getSize())
	{
		delete ibo;
		ibo = nullptr;
	}

	if (!ibo && size > 0)
	{
		// Full memory backing because we might access the data at any time.
		ibo = VertexBuffer::Create(size, GL_ELEMENT_ARRAY_BUFFER, GL_DYNAMIC_DRAW, VertexBuffer::BACKING_FULL);
	}

	element_count = map.size();

	if (!ibo || element_count == 0)
		return;

	VertexBuffer::Bind ibo_bind(*ibo);
	VertexBuffer::Mapper ibo_map(*ibo);

	// Fill the buffer with the index values from the vector.
	switch (datatype)
	{
	case GL_UNSIGNED_BYTE:
		copyToIndexBuffer<uint8>(map, ibo_map, vertex_count);
		break;
	case GL_UNSIGNED_SHORT:
		copyToIndexBuffer<uint16>(map, ibo_map, vertex_count);
		break;
	case GL_UNSIGNED_INT:
	default:
		copyToIndexBuffer<uint32>(map, ibo_map, vertex_count);
		break;
	}

	element_data_type = datatype;
}

/**
 * Copies index data from a mapped buffer to a vector.
 **/
template <typename T>
static void copyFromIndexBuffer(void *buffer, size_t count, std::vector<uint32> &indices)
{
	T *elems = (T *) buffer;
	for (size_t i = 0; i < count; i++)
		indices.push_back((uint32) elems[i]);
}

void Mesh::getVertexMap(std::vector<uint32> &map) const
{
	if (!ibo || element_count == 0)
		return;

	map.clear();
	map.reserve(element_count);

	VertexBuffer::Bind ibo_bind(*ibo);

	// We unmap the buffer in Mesh::draw and Mesh::setVertexMap.
	void *buffer = ibo->map();

	// Fill the vector from the buffer.
	switch (element_data_type)
	{
	case GL_UNSIGNED_BYTE:
		copyFromIndexBuffer<uint8>(buffer, element_count, map);
		break;
	case GL_UNSIGNED_SHORT:
		copyFromIndexBuffer<uint16>(buffer, element_count, map);
		break;
	case GL_UNSIGNED_INT:
	default:
		copyFromIndexBuffer<uint32>(buffer, element_count, map);
		break;
	}
}

size_t Mesh::getVertexMapCount() const
{
	return element_count;
}

void Mesh::setTexture(Texture *tex)
{
	texture.set(tex);
}

void Mesh::setTexture()
{
	texture.set(nullptr);
}

Texture *Mesh::getTexture() const
{
	return texture.get();
}

void Mesh::setDrawMode(Mesh::DrawMode mode)
{
	draw_mode = mode;
}

Mesh::DrawMode Mesh::getDrawMode() const
{
	return draw_mode;
}

void Mesh::setDrawRange(int min, int max)
{
	if (min < 0 || max < 0 || min > max)
		throw love::Exception("Invalid draw range.");

	range_min = min;
	range_max = max;
}

void Mesh::setDrawRange()
{
	range_min = range_max = -1;
}

void Mesh::getDrawRange(int &min, int &max) const
{
	min = range_min;
	max = range_max;
}

void Mesh::setVertexColors(bool enable)
{
	colors_enabled = enable;
}

bool Mesh::hasVertexColors() const
{
	return colors_enabled;
}

void Mesh::draw(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky)
{
	const size_t pos_offset   = offsetof(Vertex, x);
	const size_t tex_offset   = offsetof(Vertex, s);
	const size_t color_offset = offsetof(Vertex, r);

	if (vertex_count == 0)
		return;

	if (texture.get())
		texture->predraw();
	else
		gl.bindTexture(0);

	Matrix m;
	m.setTransformation(x, y, angle, sx, sy, ox, oy, kx, ky);

	OpenGL::TempTransform transform(gl);
	transform.get() *= m;

	VertexBuffer::Bind vbo_bind(*vbo);

	// Make sure the VBO isn't mapped when we draw (sends data to GPU if needed.)
	vbo->unmap();

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(2, GL_FLOAT, sizeof(Vertex), vbo->getPointer(pos_offset));
	glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), vbo->getPointer(tex_offset));

	if (hasVertexColors())
	{
		// Per-vertex colors.
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), vbo->getPointer(color_offset));
	}

	GLenum mode = getGLDrawMode(draw_mode);

	gl.prepareDraw();

	if (ibo && element_count > 0)
	{
		// Use the custom vertex map (index buffer) to draw the vertices.
		VertexBuffer::Bind ibo_bind(*ibo);

		// Make sure the index buffer isn't mapped (sends data to GPU if needed.)
		ibo->unmap();

		int max = element_count - 1;
		if (range_max >= 0)
			max = std::min(range_max, max);

		int min = 0;
		if (range_min >= 0)
			min = std::min(range_min, max);

		GLenum type = element_data_type;
		const void *indices = ibo->getPointer(min * getGLDataTypeSize(type));

		gl.drawElements(mode, max - min + 1, type, indices);
	}
	else
	{
		int max = vertex_count - 1;
		if (range_max >= 0)
			max = std::min(range_max, max);

		int min = 0;
		if (range_min >= 0)
			min = std::min(range_min, max);

		// Normal non-indexed drawing (no custom vertex map.)
		gl.drawArrays(mode, min, max - min + 1);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if (hasVertexColors())
	{
		glDisableClientState(GL_COLOR_ARRAY);
		// Using the color array leaves the GL constant color undefined.
		gl.setColor(gl.getColor());
	}

	if (texture.get())
		texture->postdraw();
}

GLenum Mesh::getGLDrawMode(DrawMode mode) const
{
	switch (mode)
	{
	case DRAW_MODE_FAN:
		return GL_TRIANGLE_FAN;
	case DRAW_MODE_STRIP:
		return GL_TRIANGLE_STRIP;
	case DRAW_MODE_TRIANGLES:
	default:
		return GL_TRIANGLES;
	case DRAW_MODE_POINTS:
		return GL_POINTS;
	}
}

GLenum Mesh::getGLDataTypeFromMax(size_t maxvalue) const
{
	if (maxvalue > LOVE_UINT16_MAX)
		return GL_UNSIGNED_INT;
	else
		return GL_UNSIGNED_SHORT;
}

size_t Mesh::getGLDataTypeSize(GLenum datatype) const
{
	switch (datatype)
	{
	case GL_UNSIGNED_BYTE:
		return sizeof(uint8);
	case GL_UNSIGNED_SHORT:
		return sizeof(uint16);
	case GL_UNSIGNED_INT:
		return sizeof(uint32);
	default:
		return 0;
	}
}

bool Mesh::getConstant(const char *in, Mesh::DrawMode &out)
{
	return drawModes.find(in, out);
}

bool Mesh::getConstant(Mesh::DrawMode in, const char *&out)
{
	return drawModes.find(in, out);
}

StringMap<Mesh::DrawMode, Mesh::DRAW_MODE_MAX_ENUM>::Entry Mesh::drawModeEntries[] =
{
	{"fan", Mesh::DRAW_MODE_FAN},
	{"strip", Mesh::DRAW_MODE_STRIP},
	{"triangles", Mesh::DRAW_MODE_TRIANGLES},
	{"points", Mesh::DRAW_MODE_POINTS},
};

StringMap<Mesh::DrawMode, Mesh::DRAW_MODE_MAX_ENUM> Mesh::drawModes(Mesh::drawModeEntries, sizeof(Mesh::drawModeEntries));

} // opengl
} // graphics
} // love
