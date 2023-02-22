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

// LOVE
#include "Polyline.h"
#include "graphics/Graphics.h"

// C++
#include <algorithm>

// treat adjacent segments with angles between their directions <5 degree as straight
static const float LINES_PARALLEL_EPS = 0.05f;

namespace love
{
namespace graphics
{

void Polyline::render(const Vector2 *coords, size_t count, size_t size_hint, float halfwidth, float pixel_size, bool draw_overdraw)
{
	static std::vector<Vector2> anchors;
	anchors.clear();
	anchors.reserve(size_hint);

	static std::vector<Vector2> normals;
	normals.clear();
	normals.reserve(size_hint);

	// prepare vertex arrays
	if (draw_overdraw)
		halfwidth -= pixel_size * 0.3f;

	// compute sleeve
	bool is_looping = (coords[0] == coords[count - 1]);
	Vector2 segment;
	if (!is_looping) // virtual starting point at second point mirrored on first point
		segment = coords[1] - coords[0];
	else // virtual starting point at last vertex
		segment = coords[0] - coords[count - 2];

	float segmentLength = segment.getLength();
	Vector2 segmentNormal = segment.getNormal(halfwidth / segmentLength);

	Vector2 pointA, pointB(coords[0]);
	for (size_t i = 0; i + 1 < count; i++)
	{
		pointA = pointB;
		pointB = coords[i + 1];
		renderEdge(anchors, normals, segment, segmentLength, segmentNormal, pointA, pointB, halfwidth);
	}

	pointA = pointB;
	pointB = is_looping ? coords[1] : pointB + segment;
	renderEdge(anchors, normals, segment, segmentLength, segmentNormal, pointA, pointB, halfwidth);

	vertex_count = normals.size();

	size_t extra_vertices = 0;

	if (draw_overdraw)
	{
		calc_overdraw_vertex_count(is_looping);

		// When drawing overdraw lines using triangle strips, we want to add an
		// extra degenerate triangle in between the core line and the overdraw
		// line in order to break up the strip into two. This will let us draw
		// everything in one draw call.
		if (triangle_mode == vertex::TriangleIndexMode::STRIP)
			extra_vertices = 2;
	}

	// Use a single linear array for both the regular and overdraw vertices.
	vertices = new Vector2[vertex_count + extra_vertices + overdraw_vertex_count];

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

void NoneJoinPolyline::renderEdge(std::vector<Vector2> &anchors, std::vector<Vector2> &normals,
                                Vector2 &segment, float &segmentLength, Vector2 &segmentNormal,
                                const Vector2 &pointA, const Vector2 &pointB, float halfWidth)
{
	//   ns1------ns2
	//    |        |
	//    q ------ r
	//    |        |
	// (-ns1)----(-ns2)

	anchors.push_back(pointA);
	anchors.push_back(pointA);
	normals.push_back(segmentNormal);
	normals.push_back(-segmentNormal);

	segment = (pointB - pointA);
	segmentLength = segment.getLength();
	segmentNormal = segment.getNormal(halfWidth / segmentLength);

	anchors.push_back(pointA);
	anchors.push_back(pointA);
	normals.push_back(segmentNormal);
	normals.push_back(-segmentNormal);
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
 * with ns,nt being the normals on the segments s = p-q and t = q-r,
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
void MiterJoinPolyline::renderEdge(std::vector<Vector2> &anchors, std::vector<Vector2> &normals,
                                   Vector2 &segment, float &segmentLength, Vector2 &segmentNormal,
                                   const Vector2 &pointA, const Vector2 &pointB, float halfwidth)
{
	Vector2 newSegment = (pointB - pointA);
	float newSegmentLength = newSegment.getLength();
	if (newSegmentLength == 0.0f)
	{
		// degenerate segment, skip it
		return;
	}

	Vector2 newSegmentNormal = newSegment.getNormal(halfwidth / newSegmentLength);

	anchors.push_back(pointA);
	anchors.push_back(pointA);

	float det = Vector2::cross(segment, newSegment);
	if (fabs(det) / (segmentLength * newSegmentLength) < LINES_PARALLEL_EPS)
	{
		// lines parallel, compute as u1 = q + ns * w/2, u2 = q - ns * w/2
		normals.push_back(segmentNormal);
		normals.push_back(-segmentNormal);

		if (Vector2::dot(segment, newSegment) < 0)
		{
			// line reverses direction; because the normal flips, the
			// triangle strip would twist here, so insert a zero-size
			// quad to contain the twist
			//  ____.___.____
			// |    |\ /|    |
			// p    q X q    r
			// |____|/ \|____|
			anchors.push_back(pointA);
			anchors.push_back(pointA);
			normals.push_back(-segmentNormal);
			normals.push_back(segmentNormal);
		}
	}
	else
	{
		// cramers rule
		float lambda = Vector2::cross((newSegmentNormal - segmentNormal), newSegment) / det;
		Vector2 d = segmentNormal + segment * lambda;
		normals.push_back(d);
		normals.push_back(-d);
	}

	segment = newSegment;
	segmentNormal = newSegmentNormal;
	segmentLength = newSegmentLength;
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
void BevelJoinPolyline::renderEdge(std::vector<Vector2> &anchors, std::vector<Vector2> &normals,
                                   Vector2 &segment, float &segmentLength, Vector2 &segmentNormal,
                                   const Vector2 &pointA, const Vector2 &pointB, float halfWidth)
{
	Vector2 newSegment = (pointB - pointA);
	float newSegmentLength = newSegment.getLength();

	float det = Vector2::cross(segment, newSegment);
	if (fabs(det) / (segmentLength * newSegmentLength) < LINES_PARALLEL_EPS)
	{
		// lines parallel, compute as u1 = q + ns * w/2, u2 = q - ns * w/2
		Vector2 newSegmentNormal = newSegment.getNormal(halfWidth / newSegmentLength);
		anchors.push_back(pointA);
		anchors.push_back(pointA);
		normals.push_back(segmentNormal);
		normals.push_back(-segmentNormal);

		if (Vector2::dot(segment, newSegment) < 0)
		{
			// line reverses direction; same as for miter
			anchors.push_back(pointA);
			anchors.push_back(pointA);
			normals.push_back(-segmentNormal);
			normals.push_back(segmentNormal);
		}

		segment = newSegment;
		segmentLength = newSegmentLength;
		segmentNormal = newSegmentNormal;
		return; // early out
	}

	// cramers rule
	Vector2 newSegmentNormal = newSegment.getNormal(halfWidth / newSegmentLength);
	float lambda = Vector2::cross((newSegmentNormal - segmentNormal), newSegment) / det;
	Vector2 d = segmentNormal + segment * lambda;

	anchors.push_back(pointA);
	anchors.push_back(pointA);
	anchors.push_back(pointA);
	anchors.push_back(pointA);
	if (det > 0) // 'left' turn -> intersection on the top
	{
		normals.push_back(d);
		normals.push_back(-segmentNormal);
		normals.push_back(d);
		normals.push_back(-newSegmentNormal);
	}
	else
	{
		normals.push_back(segmentNormal);
		normals.push_back(-d);
		normals.push_back(newSegmentNormal);
		normals.push_back(-d);
	}
	segment = newSegment;
	segmentLength = newSegmentLength;
	segmentNormal = newSegmentNormal;
}

void Polyline::calc_overdraw_vertex_count(bool is_looping)
{
	overdraw_vertex_count = 2 * vertex_count + (is_looping ? 0 : 2);
}

void Polyline::render_overdraw(const std::vector<Vector2> &normals, float pixel_size, bool is_looping)
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
		overdraw[vertex_count + i+1] = vertices[k] + normals[k] * (pixel_size / normals[k].getLength());
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
		Vector2 spacer = (overdraw[1] - overdraw[3]);
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

void NoneJoinPolyline::render_overdraw(const std::vector<Vector2> &/*normals*/, float pixel_size, bool /*is_looping*/)
{
	for (size_t i = 2; i + 3 < vertex_count; i += 4)
	{
		// v0-v2
		// | / | <- main quad line
		// v1-v3

		Vector2 s = vertices[i+0] - vertices[i+2];
		Vector2 t = vertices[i+0] - vertices[i+1];
		s.normalize(pixel_size);
		t.normalize(pixel_size);

		const size_t k = 4 * (i - 2);

		overdraw[k+0] = vertices[i+0];
		overdraw[k+1] = vertices[i+1];
		overdraw[k+2] = vertices[i+0] + s + t;
		overdraw[k+3] = vertices[i+1] + s - t;

		overdraw[k+4] = vertices[i+1];
		overdraw[k+5] = vertices[i+3];
		overdraw[k+6] = vertices[i+1] + s - t;
		overdraw[k+7] = vertices[i+3] - s - t;

		overdraw[k+ 8] = vertices[i+3];
		overdraw[k+ 9] = vertices[i+2];
		overdraw[k+10] = vertices[i+3] - s - t;
		overdraw[k+11] = vertices[i+2] - s + t;

		overdraw[k+12] = vertices[i+2];
		overdraw[k+13] = vertices[i+0];
		overdraw[k+14] = vertices[i+2] - s + t;
		overdraw[k+15] = vertices[i+0] + s + t;
	}
}

Polyline::~Polyline()
{
	if (vertices)
		delete[] vertices;
}

void Polyline::draw(love::graphics::Graphics *gfx)
{
	const Matrix4 &t = gfx->getTransform();
	bool is2D = t.isAffine2DTransform();
	Color32 curcolor = toColor32(gfx->getColor());

	int overdraw_start = (int) overdraw_vertex_start;
	int overdraw_count = (int) overdraw_vertex_count;

	int total_vertex_count = (int) vertex_count;
	if (overdraw)
		total_vertex_count = overdraw_start + overdraw_count;

	// love's automatic batching can only deal with < 65k vertices per draw.
	// uint16_max - 3 is evenly divisible by 6 (needed for quads mode).
	int maxvertices = LOVE_UINT16_MAX - 3;

	int advance = maxvertices;
	if (triangle_mode == vertex::TriangleIndexMode::STRIP)
		advance -= 2;

	for (int vertex_start = 0; vertex_start < total_vertex_count; vertex_start += advance)
	{
		const Vector2 *verts = vertices + vertex_start;

		Graphics::StreamDrawCommand cmd;
		cmd.formats[0] = vertex::getSinglePositionFormat(is2D);
		cmd.formats[1] = vertex::CommonFormat::RGBAub;
		cmd.indexMode = triangle_mode;
		cmd.vertexCount = std::min(maxvertices, total_vertex_count - vertex_start);

		Graphics::StreamVertexData data = gfx->requestStreamDraw(cmd);

		if (is2D)
			t.transformXY((Vector2 *) data.stream[0], verts, cmd.vertexCount);
		else
			t.transformXY0((Vector3 *) data.stream[0], verts, cmd.vertexCount);

		Color32 *colordata = (Color32 *) data.stream[1];

		int draw_rough_count = std::min(cmd.vertexCount, (int) vertex_count - vertex_start);

		// Constant vertex color up to the overdraw vertices.
		for (int i = 0; i < draw_rough_count; i++)
			colordata[i] = curcolor;

		if (overdraw)
		{
			int draw_remaining_count = cmd.vertexCount - draw_rough_count;

			int draw_overdraw_begin = overdraw_start - vertex_start;
			int draw_overdraw_end = draw_overdraw_begin + overdraw_count;

			draw_overdraw_begin = std::max(0, draw_overdraw_begin);

			int draw_overdraw_count = std::min(draw_remaining_count, draw_overdraw_end - draw_overdraw_begin);

			if (draw_overdraw_count > 0)
			{
				Color32 *colors = colordata + draw_overdraw_begin;
				fill_color_array(curcolor, colors, draw_overdraw_count);
			}
		}
	}
}

void Polyline::fill_color_array(Color32 constant_color, Color32 *colors, int count)
{
	for (int i = 0; i < count; ++i)
	{
		Color32 c = constant_color;
		c.a *= (i+1) % 2; // avoids branching. equiv to if (i%2 == 1) c.a = 0;
		colors[i] = c;
	}
}

void NoneJoinPolyline::fill_color_array(Color32 constant_color, Color32 *colors, int count)
{
	for (int i = 0; i < count; ++i)
	{
		Color32 c = constant_color;
		c.a *= (i & 3) < 2; // if (i % 4 == 2 || i % 4 == 3) c.a = 0
		colors[i] = c;
	}
}

} // graphics
} // love
