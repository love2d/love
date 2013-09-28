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
	: draw_mode(mode)
	, image(0)
{
	setVertices(verts);
}

void Mesh::setVertices(const std::vector<Vertex> &verts)
{
	if (verts.size() < 3)
		throw love::Exception("At least 3 vertices are required.");

	vertices = verts;
}

const std::vector<Vertex> &Mesh::getVertices() const
{
	return vertices;
}

void Mesh::setVertex(size_t i, Vertex v)
{
	if (i >= vertices.size())
		throw love::Exception("Invalid index.");

	vertices[i] = v;
}

Vertex Mesh::getVertex(size_t i) const
{
	if (i >= vertices.size())
		throw love::Exception("Invalid index.");

	return vertices[i];
}

size_t Mesh::getVertexCount() const
{
	return vertices.size();
}

void Mesh::setVertexMap(const std::vector<uint16> &map)
{
	for (size_t i = 0; i < map.size(); i++)
	{
		if (map[i] >= vertices.size())
			throw love::Exception("Invalid vertex map value: %d", map[i]);
	}

	vertex_map = map;
}

const std::vector<uint16> &Mesh::getVertexMap() const
{
	return vertex_map;
}

Mesh::~Mesh()
{
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

	image = 0;
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

void Mesh::draw(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky) const
{
	if (vertices.size() == 0)
		return;

	if (image)
		image->bind();
	else
		gl.bindTexture(0);

	Matrix m;
	m.setTransformation(x, y, angle, sx, sy, ox, oy, kx, ky);

	glPushMatrix();
	glMultMatrixf(m.getElements());

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &(vertices[0].x));
	glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &(vertices[0].s));

	{
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), &(vertices[0].r));
	}

	GLenum gl_draw_mode = getGLDrawMode(draw_mode);

	if (vertex_map.size() > 0)
		glDrawElements(gl_draw_mode, vertex_map.size(), GL_UNSIGNED_SHORT, &vertex_map[0]);
	else
		glDrawArrays(gl_draw_mode, 0, vertices.size());

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	{
		glDisableClientState(GL_COLOR_ARRAY);
		gl.setColor(gl.getColor());
	}

	glPopMatrix();
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
