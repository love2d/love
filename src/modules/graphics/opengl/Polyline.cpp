/**
 * Copyright (c) 2006-2016 LOVE Development Team
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
#include "Polyline.h"

// OpenGL
#include "OpenGL.h"

// C++
#include <algorithm>

// treat adjacent segments with angles between their directions <5 degree as straight
static const float LINES_PARALLEL_EPS = 0.05f;

namespace love
{
namespace graphics
{
namespace opengl
{

void Polyline::render(const float *coords, size_t count, size_t size_hint, float halfwidth, float pixel_size, bool draw_overdraw)
{
	static std::vector<Vector> anchors;
	anchors.clear();
	anchors.reserve(size_hint);

	static std::vector<Vector> normals;
	normals.clear();
	normals.reserve(size_hint);

	// prepare vertex arrays
	if (draw_overdraw)
		halfwidth -= pixel_size * 0.3f;

	// compute sleeve
	bool is_looping = (coords[0] == coords[count - 2]) && (coords[1] == coords[count - 1]);
	Vector s;
	if (!is_looping) // virtual starting point at second point mirrored on first point
		s = Vector(coords[2] - coords[0], coords[3] - coords[1]);
	else // virtual starting point at last vertex
		s = Vector(coords[0] - coords[count - 4], coords[1] - coords[count - 3]);

	float len_s = s.getLength();
	Vector ns = s.getNormal(halfwidth / len_s);

	Vector q, r(coords[0], coords[1]);
	for (size_t i = 0; i + 3 < count; i += 2)
	{
		q = r;
		r = Vector(coords[i + 2], coords[i + 3]);
		renderEdge(anchors, normals, s, len_s, ns, q, r, halfwidth);
	}

	q = r;
	r = is_looping ? Vector(coords[2], coords[3]) : r + s;
	renderEdge(anchors, normals, s, len_s, ns, q, r, halfwidth);

	vertex_count = normals.size();

	size_t extra_vertices = 0;

	if (draw_overdraw)
	{
		calc_overdraw_vertex_count(is_looping);

		// When drawing overdraw lines using triangle strips, we want to add an
		// extra degenerate triangle in between the core line and the overdraw
		// line in order to break up the strip into two. This will let us draw
		// everything in one draw call.
		if (draw_mode == GL_TRIANGLE_STRIP)
			extra_vertices = 2;
	}

	// Use a single linear array for both the regular and overdraw vertices.
	vertices = new Vector[vertex_count + extra_vertices + overdraw_vertex_count];

	for (size_t i = 0; i < vertex_count; ++i)
		vertices[i] = anchors[i] + normals[i];

	if (draw_overdraw)
	{
		overdraw = vertices + vertex_count + extra_vertices;
		overdraw_vertex_start = vertex_count + extra_vertices;
		render_overdraw(normals, pixel_size, is_looping);
	}

	// Add the degenerate triangle strip.
	if (extra_vertices)
	{
		vertices[vertex_count + 0] = vertices[vertex_count - 1];
		vertices[vertex_count + 1] = vertices[overdraw_vertex_start];
	}
}

void NoneJoinPolyline::renderEdge(std::vector<Vector> &anchors, std::vector<Vector> &normals,
                                Vector &s, float &len_s, Vector &ns,
                                const Vector &q, const Vector &r, float hw)
{
	anchors.push_back(q);
	anchors.push_back(q);
	normals.push_back(ns);
	normals.push_back(-ns);

	s     = (r - q);
	len_s = s.getLength();
	ns    = s.getNormal(hw / len_s);

	anchors.push_back(q);
	anchors.push_back(q);
	normals.push_back(-ns);
	normals.push_back(ns);
}


/** Calculate line boundary points.
 *
 * Sketch:
 *
 *              u1
 * -------------+---...___
 *              |         ```'''--  ---
 * p- - - - - - q- - . _ _           | w/2
 *              |          ` ' ' r   +
 * -------------+---...___           | w/2
 *              u2         ```'''-- ---
 *
 * u1 and u2 depend on four things:
 *   - the half line width w/2
 *   - the previous line vertex p
 *   - the current line vertex q
 *   - the next line vertex r
 *
 * u1/u2 are the intersection points of the parallel lines to p-q and q-r,
 * i.e. the point where
 *
 *    (q + w/2 * ns) + lambda * (q - p) = (q + w/2 * nt) + mu * (r - q)   (u1)
 *    (q - w/2 * ns) + lambda * (q - p) = (q - w/2 * nt) + mu * (r - q)   (u2)
 *
 * with nt,nt being the normals on the segments s = p-q and t = q-r,
 *
 *    ns = perp(s) / |s|
 *    nt = perp(t) / |t|.
 *
 * Using the linear equation system (similar for u2)
 *
 *         q + w/2 * ns + lambda * s - (q + w/2 * nt + mu * t) = 0                 (u1)
 *    <=>  q-q + lambda * s - mu * t                          = (nt - ns) * w/2
 *    <=>  lambda * s   - mu * t                              = (nt - ns) * w/2
 *
 * the intersection points can be efficiently calculated using Cramer's rule.
 */
void MiterJoinPolyline::renderEdge(std::vector<Vector> &anchors, std::vector<Vector> &normals,
                                   Vector &s, float &len_s, Vector &ns,
                                   const Vector &q, const Vector &r, float hw)
{
	Vector t    = (r - q);
	float len_t = t.getLength();
	Vector nt   = t.getNormal(hw / len_t);

	anchors.push_back(q);
	anchors.push_back(q);

	float det = s ^ t;
	if (fabs(det) / (len_s * len_t) < LINES_PARALLEL_EPS && s * t > 0)
	{
		// lines parallel, compute as u1 = q + ns * w/2, u2 = q - ns * w/2
		normals.push_back(ns);
		normals.push_back(-ns);
	}
	else
	{
		// cramers rule
		float lambda = ((nt - ns) ^ t) / det;
		Vector d = ns + s * lambda;
		normals.push_back(d);
		normals.push_back(-d);
	}

	s     = t;
	ns    = nt;
	len_s = len_t;
}

/** Calculate line boundary points.
 *
 * Sketch:
 *
 *     uh1___uh2
 *      .'   '.
 *    .'   q   '.
 *  .'   '   '   '.
 *.'   '  .'.  '   '.
 *   '  .' ul'.  '
 * p  .'       '.  r
 *
 *
 * ul can be found as above, uh1 and uh2 are much simpler:
 *
 * uh1 = q + ns * w/2, uh2 = q + nt * w/2
 */
void BevelJoinPolyline::renderEdge(std::vector<Vector> &anchors, std::vector<Vector> &normals,
                                   Vector &s, float &len_s, Vector &ns,
                                   const Vector &q, const Vector &r, float hw)
{
	Vector t    = (r - q);
	float len_t = t.getLength();

	float det = s ^ t;
	if (fabs(det) / (len_s * len_t) < LINES_PARALLEL_EPS && s * t > 0)
	{
		// lines parallel, compute as u1 = q + ns * w/2, u2 = q - ns * w/2
		Vector n = t.getNormal(hw / len_t);
		anchors.push_back(q);
		anchors.push_back(q);
		normals.push_back(n);
		normals.push_back(-n);
		s     = t;
		len_s = len_t;
		return; // early out
	}

	// cramers rule
	Vector nt= t.getNormal(hw / len_t);
	float lambda = ((nt - ns) ^ t) / det;
	Vector d = ns + s * lambda;

	anchors.push_back(q);
	anchors.push_back(q);
	anchors.push_back(q);
	anchors.push_back(q);
	if (det > 0) // 'left' turn -> intersection on the top
	{
		normals.push_back(d);
		normals.push_back(-ns);
		normals.push_back(d);
		normals.push_back(-nt);
	}
	else
	{
		normals.push_back(ns);
		normals.push_back(-d);
		normals.push_back(nt);
		normals.push_back(-d);
	}
	s     = t;
	len_s = len_t;
	ns    = nt;
}

void Polyline::calc_overdraw_vertex_count(bool is_looping)
{
	overdraw_vertex_count = 2 * vertex_count + (is_looping ? 0 : 2);
}

void Polyline::render_overdraw(const std::vector<Vector> &normals, float pixel_size, bool is_looping)
{
	// upper segment
	for (size_t i = 0; i + 1 < vertex_count; i += 2)
	{
		overdraw[i]   = vertices[i];
		overdraw[i+1] = vertices[i] + normals[i] * (pixel_size / normals[i].getLength());
	}
	// lower segment
	for (size_t i = 0; i + 1 < vertex_count; i += 2)
	{
		size_t k = vertex_count - i - 1;
		overdraw[vertex_count + i]   = vertices[k];
		overdraw[vertex_count + i+1] = vertices[k] + normals[k] * (pixel_size / normals[i].getLength());
	}

	// if not looping, the outer overdraw vertices need to be displaced
	// to cover the line endings, i.e.:
	// +- - - - //- - +         +- - - - - //- - - +
	// +-------//-----+         : +-------//-----+ :
	// | core // line |   -->   : | core // line | :
	// +-----//-------+         : +-----//-------+ :
	// +- - //- - - - +         +- - - //- - - - - +
	if (!is_looping)
	{
		// left edge
		Vector spacer = (overdraw[1] - overdraw[3]);
		spacer.normalize(pixel_size);
		overdraw[1] += spacer;
		overdraw[overdraw_vertex_count - 3] += spacer;

		// right edge
		spacer = (overdraw[vertex_count-1] - overdraw[vertex_count-3]);
		spacer.normalize(pixel_size);
		overdraw[vertex_count-1] += spacer;
		overdraw[vertex_count+1] += spacer;

		// we need to draw two more triangles to close the
		// overdraw at the line start.
		overdraw[overdraw_vertex_count-2] = overdraw[0];
		overdraw[overdraw_vertex_count-1] = overdraw[1];
	}
}

void NoneJoinPolyline::calc_overdraw_vertex_count(bool /*is_looping*/)
{
	overdraw_vertex_count = 4 * (vertex_count-2); // less than ideal
}

void NoneJoinPolyline::render_overdraw(const std::vector<Vector> &/*normals*/, float pixel_size, bool /*is_looping*/)
{
	for (size_t i = 2; i + 3 < vertex_count; i += 4)
	{
		Vector s = vertices[i] - vertices[i+3];
		Vector t = vertices[i] - vertices[i+1];
		s.normalize(pixel_size);
		t.normalize(pixel_size);

		const size_t k = 4 * (i - 2);
		overdraw[k  ] = vertices[i];
		overdraw[k+1] = vertices[i]   + s + t;
		overdraw[k+2] = vertices[i+1] + s - t;
		overdraw[k+3] = vertices[i+1];

		overdraw[k+4] = vertices[i+1];
		overdraw[k+5] = vertices[i+1] + s - t;
		overdraw[k+6] = vertices[i+2] - s - t;
		overdraw[k+7] = vertices[i+2];

		overdraw[k+8]  = vertices[i+2];
		overdraw[k+9]  = vertices[i+2] - s - t;
		overdraw[k+10] = vertices[i+3] - s + t;
		overdraw[k+11] = vertices[i+3];

		overdraw[k+12] = vertices[i+3];
		overdraw[k+13] = vertices[i+3] - s + t;
		overdraw[k+14] = vertices[i]   + s + t;
		overdraw[k+15] = vertices[i];
	}
}

Polyline::~Polyline()
{
	if (vertices)
		delete[] vertices;
}

void Polyline::draw()
{
	OpenGL::TempDebugGroup debuggroup("Line draw");

	GLushort *indices = nullptr;
	Color *colors = nullptr;

	size_t total_vertex_count = vertex_count;
	if (overdraw)
		total_vertex_count = overdraw_vertex_start + overdraw_vertex_count;

	// TODO: We should probably be using a reusable index buffer.
	if (use_quad_indices)
	{
		size_t numindices = (total_vertex_count / 4) * 6;

		try
		{
			indices = new GLushort[numindices];
		}
		catch (std::bad_alloc &)
		{
			throw love::Exception("Out of memory.");
		}

		// Fill the index array to make 2 triangles from each quad.
		// NOTE: The triangle vertex ordering here is important!
		for (size_t i = 0; i < numindices / 6; i++)
		{
			// First triangle.
			indices[i * 6 + 0] = GLushort(i * 4 + 0);
			indices[i * 6 + 1] = GLushort(i * 4 + 1);
			indices[i * 6 + 2] = GLushort(i * 4 + 2);

			// Second triangle.
			indices[i * 6 + 3] = GLushort(i * 4 + 0);
			indices[i * 6 + 4] = GLushort(i * 4 + 2);
			indices[i * 6 + 5] = GLushort(i * 4 + 3);
		}
	}

	gl.prepareDraw();

	gl.bindTexture(gl.getDefaultTexture());

	uint32 enabledattribs = ATTRIBFLAG_POS;

	if (overdraw)
	{
		// Prepare per-vertex colors. Set the core to white, and the overdraw
		// line's colors to white on one side and transparent on the other.
		colors = new Color[total_vertex_count];
		memset(colors, 255, overdraw_vertex_start * sizeof(Color));
		fill_color_array(colors + overdraw_vertex_start);

		glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, colors);

		enabledattribs |= ATTRIBFLAG_COLOR;
	}

	gl.useVertexAttribArrays(enabledattribs);

	glVertexAttribPointer(ATTRIB_POS, 2, GL_FLOAT, GL_FALSE, 0, vertices);

	// Draw the core line and the overdraw in a single draw call. We can do this
	// because the vertex array contains both the core line and the overdraw
	// vertices.
	if (use_quad_indices)
		gl.drawElements(draw_mode, (int) (total_vertex_count / 4) * 6, GL_UNSIGNED_SHORT, indices);
	else
		gl.drawArrays(draw_mode, 0, (int) total_vertex_count);

	if (overdraw)
		delete[] colors;

	if (indices)
		delete[] indices;
}

void Polyline::fill_color_array(Color *colors)
{
	for (size_t i = 0; i < overdraw_vertex_count; ++i)
	{
		// avoids branching. equiv to if (i%2 == 1) colors[i].a = 0;
		colors[i] = {255, 255, 255, GLubyte(255 * ((i+1) % 2))};
	}
}

void NoneJoinPolyline::fill_color_array(Color *colors)
{
	for (size_t i = 0; i < overdraw_vertex_count; ++i)
	{
		// if (i % 4 == 1 || i % 4 == 2) colors[i].a = 0
		colors[i] = {255, 255, 255, GLubyte(255 * ((i+1) % 4 < 2))};
	}
}

} // opengl
} // graphics
} // love
