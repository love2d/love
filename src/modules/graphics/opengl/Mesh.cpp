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

// LOVE
#include "Mesh.h"
#include "common/Matrix.h"
#include "common/Exception.h"

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
	, draw_mode(mode)
	, image(nullptr)
	, colors_enabled(false)
{
	setVertices(verts);
}

Mesh::~Mesh()
{
	delete vbo;
	delete ibo;
}

void Mesh::setVertices(const std::vector<Vertex> &verts)
{
	if (verts.size() < 3)
		throw love::Exception("At least 3 vertices are required.");

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

void Mesh::setVertexMap(const std::vector<uint32> &map)
{
	for (size_t i = 0; i < map.size(); i++)
	{
		if (map[i] >= vertex_count)
			throw love::Exception("Invalid vertex map value: %d", map[i] + 1);
	}

	size_t size = sizeof(uint32) * map.size();

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

	if (ibo && element_count > 0)
	{
		VertexBuffer::Bind ibo_bind(*ibo);
		VertexBuffer::Mapper ibo_map(*ibo);

		// Fill the buffer.
		memcpy(ibo_map.get(), &map[0], size);
	}
}

const uint32 *Mesh::getVertexMap() const
{
	if (ibo && element_count > 0)
	{
		VertexBuffer::Bind ibo_bind(*ibo);

		// We unmap the buffer in Mesh::draw and Mesh::setVertexMap.
		return (uint32 *) ibo->map();
	}

	return 0;
}

size_t Mesh::getVertexMapCount() const
{
	return element_count;
}

void Mesh::setImage(Image *img)
{
	img->retain();

	if (image)
		image->release();

	image = img;
}

void Mesh::setImage()
{
	if (image)
		image->release();

	image = nullptr;
}

Image *Mesh::getImage() const
{
	return image;
}

void Mesh::setDrawMode(Mesh::DrawMode mode)
{
	draw_mode = mode;
}

Mesh::DrawMode Mesh::getDrawMode() const
{
	return draw_mode;
}

void Mesh::setVertexColors(bool enable)
{
	colors_enabled = enable;
}

bool Mesh::hasVertexColors() const
{
	return colors_enabled;
}

void Mesh::draw(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky) const
{
	const size_t pos_offset   = offsetof(Vertex, x);
	const size_t tex_offset   = offsetof(Vertex, s);
	const size_t color_offset = offsetof(Vertex, r);

	if (vertex_count == 0)
		return;

	if (image)
		image->predraw();
	else
		gl.bindTexture(0);

	Matrix m;
	m.setTransformation(x, y, angle, sx, sy, ox, oy, kx, ky);

	glPushMatrix();
	glMultMatrixf(m.getElements());

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

	if (ibo && element_count > 0)
	{
		VertexBuffer::Bind ibo_bind(*ibo);

		// Make sure the index buffer isn't mapped (sends data to GPU if needed.)
		ibo->unmap();

		// Use the custom vertex map to draw the vertices.
		glDrawElements(mode, element_count, GL_UNSIGNED_INT, ibo->getPointer(0));
	}
	else
	{
		// Normal non-indexed drawing (no custom vertex map.)
		glDrawArrays(mode, 0, vertex_count);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if (hasVertexColors())
	{
		glDisableClientState(GL_COLOR_ARRAY);
		// Using the color array leaves the GL constant color undefined.
		gl.setColor(gl.getColor());
	}

	glPopMatrix();

	if (image)
		image->postdraw();
}

GLenum Mesh::getGLDrawMode(Mesh::DrawMode mode) const
{
	switch (mode)
	{
	case DRAW_MODE_FAN:
		return GL_TRIANGLE_FAN;
	case DRAW_MODE_STRIP:
		return GL_TRIANGLE_STRIP;
	case DRAW_MODE_TRIANGLES:
		return GL_TRIANGLES;
	case DRAW_MODE_POINTS:
		return GL_POINTS;
	default:
		break;
	}

	return GL_TRIANGLES;
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
