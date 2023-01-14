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

#pragma once

// LOVE
#include "common/config.h"
#include "common/Vector.h"
#include "graphics/vertex.h"

// C++
#include <vector>
#include <string.h>

namespace love
{
namespace graphics
{

class Graphics;

/**
 * Abstract base class for a chain of segments.
 * @author Matthias Richter
 **/
class Polyline
{
public:

	Polyline(vertex::TriangleIndexMode mode = vertex::TriangleIndexMode::STRIP)
		: vertices(nullptr)
		, overdraw(nullptr)
		, vertex_count(0)
		, overdraw_vertex_count(0)
		, triangle_mode(mode)
		, overdraw_vertex_start(0)
	{}

	virtual ~Polyline();

	/**
	 * @param vertices      Vertices defining the core line segments
	 * @param count         Number of vertices
	 * @param size_hint     Expected number of vertices of the rendering sleeve around the core line.
	 * @param halfwidth     linewidth / 2.
	 * @param pixel_size    Dimension of one pixel on the screen in world coordinates.
	 * @param draw_overdraw Fake antialias the line.
	 */
	void render(const Vector2 *vertices, size_t count, size_t size_hint, float halfwidth, float pixel_size, bool draw_overdraw);

	/** Draws the line on the screen
	 */
	void draw(love::graphics::Graphics *gfx);

protected:

	virtual void calc_overdraw_vertex_count(bool is_looping);
	virtual void render_overdraw(const std::vector<Vector2> &normals, float pixel_size, bool is_looping);
	virtual void fill_color_array(Color32 constant_color, Color32 *colors, int count);

	/** Calculate line boundary points.
	 *
	 * @param[out]    anchors       Anchor points defining the core line.
	 * @param[out]    normals       Normals defining the edge of the sleeve.
	 * @param[in,out] segment       Direction of segment pq (updated to the segment qr).
	 * @param[in,out] segmentLength Length of segment pq (updated to the segment qr).
	 * @param[in,out] segmentNormal Normal on the segment pq (updated to the segment qr).
	 * @param[in]     pointA        Current point on the line (q).
	 * @param[in]     pointB        Next point on the line (r).
	 * @param[in]     halfWidth     Half line width (see Polyline.render()).
	 */
	virtual void renderEdge(std::vector<Vector2> &anchors, std::vector<Vector2> &normals,
	                        Vector2 &segment, float &segmentLength, Vector2 &segmentNormal,
	                        const Vector2 &pointA, const Vector2 &pointB, float halfWidth) = 0;

	Vector2 *vertices;
	Vector2 *overdraw;
	size_t vertex_count;
	size_t overdraw_vertex_count;
	vertex::TriangleIndexMode triangle_mode;
	size_t overdraw_vertex_start;

}; // Polyline


/**
 * A Polyline whose segments are not connected.
 * @author Matthias Richter
 */
class NoneJoinPolyline : public Polyline
{
public:

	NoneJoinPolyline()
		: Polyline(vertex::TriangleIndexMode::QUADS)
	{}

	void render(const Vector2 *vertices, size_t count, float halfwidth, float pixel_size, bool draw_overdraw)
	{
		Polyline::render(vertices, count, 4 * count - 4, halfwidth, pixel_size, draw_overdraw);

		// discard the first and last two vertices. (these are redundant)
		for (size_t i = 0; i < vertex_count - 4; ++i)
			this->vertices[i] = this->vertices[i+2];

		// The last quad is now garbage, so zero it out to make sure it doesn't
		// get rasterized. These vertices are in between the core line vertices
		// and the overdraw vertices in the combined vertex array, so they still
		// get "rendered" since we draw everything with one draw call.
		memset(&this->vertices[vertex_count - 4], 0, sizeof(love::Vector2) * 4);

		vertex_count -= 4;
	}

protected:

	void calc_overdraw_vertex_count(bool is_looping) override;
	void render_overdraw(const std::vector<Vector2> &normals, float pixel_size, bool is_looping) override;
	void fill_color_array(Color32 constant_color, Color32 *colors, int count) override;
	void renderEdge(std::vector<Vector2> &anchors, std::vector<Vector2> &normals,
	                Vector2 &s, float &len_s, Vector2 &ns, const Vector2 &q,
	                const Vector2 &r, float hw) override;

}; // NoneJoinPolyline


/**
 * A Polyline whose segments are connected by a sharp edge.
 * @author Matthias Richter
 */
class MiterJoinPolyline : public Polyline
{
public:

	void render(const Vector2 *vertices, size_t count, float halfwidth, float pixel_size, bool draw_overdraw)
	{
		Polyline::render(vertices, count, 2 * count, halfwidth, pixel_size, draw_overdraw);
	}

protected:

	void renderEdge(std::vector<Vector2> &anchors, std::vector<Vector2> &normals,
	                Vector2 &s, float &len_s, Vector2 &ns, const Vector2 &q,
	                const Vector2 &r, float hw) override;

}; // MiterJoinPolyline


/**
 * A Polyline whose segments are connected by a flat edge.
 * @author Matthias Richter
 */
class BevelJoinPolyline : public Polyline
{
public:

	void render(const Vector2 *vertices, size_t count, float halfwidth, float pixel_size, bool draw_overdraw)
	{
		Polyline::render(vertices, count, 4 * count - 4, halfwidth, pixel_size, draw_overdraw);
	}

protected:

	void renderEdge(std::vector<Vector2> &anchors, std::vector<Vector2> &normals,
	                Vector2 &s, float &len_s, Vector2 &ns, const Vector2 &q,
	                const Vector2 &r, float hw) override;

}; // BevelJoinPolyline

} // graphics
} // love
