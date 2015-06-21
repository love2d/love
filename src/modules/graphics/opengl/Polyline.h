/**
 * Copyright (c) 2006-2015 LOVE Development Team
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

#ifndef LOVE_GRAPHICS_OPENGL_POLYLINE_H
#define LOVE_GRAPHICS_OPENGL_POLYLINE_H

// LOVE
#include "common/config.h"
#include "common/Vector.h"

// OpenGL
#include "OpenGL.h"

// C++
#include <vector>

namespace love
{
namespace graphics
{
namespace opengl
{

/**
 * Abstract base class for a chain of segments.
 * @author Matthias Richter
 **/
class Polyline
{
public:
	Polyline(GLenum mode = GL_TRIANGLE_STRIP, bool quadindices = false)
		: vertices(NULL)
		, overdraw(NULL)
		, vertex_count(0)
		, overdraw_vertex_count(0)
		, draw_mode(mode)
		, use_quad_indices(quadindices)
	{}
	virtual ~Polyline();

	/**
	 * @param vertices      Vertices defining the core line segments
	 * @param count         Number of coordinates (= size of the array vertices)
	 * @param size_hint     Expected number of vertices of the rendering sleeve around the core line.
	 * @param halfwidth     linewidth / 2.
	 * @param pixel_size    Dimension of one pixel on the screen in world coordinates.
	 * @param draw_overdraw Fake antialias the line.
	 */
	void render(const float *vertices, size_t count, size_t size_hint, float halfwidth, float pixel_size, bool draw_overdraw);

	/** Draws the line on the screen
	 */
	void draw();

protected:
	virtual void render_overdraw(const std::vector<Vector> &normals, float pixel_size, bool is_looping);
	virtual void fill_color_array(Color *colors);

	/** Calculate line boundary points.
	 *
	 * @param[out]    anchors Anchor points defining the core line.
	 * @param[out]    normals Normals defining the edge of the sleeve.
	 * @param[in,out] s       Direction of segment pq (updated to the segment qr).
	 * @param[in,out] len_s   Length of segment pq (updated to the segment qr).
	 * @param[in,out] ns      Normal on the segment pq (updated to the segment qr).
	 * @param[in]     q       Current point on the line.
	 * @param[in]     r       Next point on the line.
	 * @param[in]     hw      Half line width (see Polyline.render()).
	 */
	virtual void renderEdge(std::vector<Vector> &anchors, std::vector<Vector> &normals,
	                        Vector &s, float &len_s, Vector &ns,
	                        const Vector &q, const Vector &r, float hw) = 0;

	Vector *vertices;
	Vector *overdraw;
	size_t vertex_count;
	size_t overdraw_vertex_count;
	GLenum draw_mode;
	bool use_quad_indices;

}; // Polyline


/**
 * A Polyline whose segments are not connected.
 * @author Matthias Richter
 */
class NoneJoinPolyline : public Polyline
{
public:
	NoneJoinPolyline()
		: Polyline(GL_TRIANGLES, true)
	{}

	void render(const float *vertices, size_t count, float halfwidth, float pixel_size, bool draw_overdraw)
	{
		Polyline::render(vertices, count, 2 * count - 4, halfwidth, pixel_size, draw_overdraw);
		// discard the first and last two vertices. (these are redundant)
		for (size_t i = 0; i < vertex_count - 2; ++i)
			this->vertices[i] = this->vertices[i+2];
		vertex_count -= 2;
	}

protected:
	virtual void render_overdraw(const std::vector<Vector> &normals, float pixel_size, bool is_looping);
	virtual void fill_color_array(Color *colors);
	virtual void renderEdge(std::vector<Vector> &anchors, std::vector<Vector> &normals,
	                        Vector &s, float &len_s, Vector &ns,
	                        const Vector &q, const Vector &r, float hw);
};


/**
 * A Polyline whose segments are connected by a sharp edge.
 * @author Matthias Richter
 */
class MiterJoinPolyline : public Polyline
{
public:
	void render(const float *vertices, size_t count, float halfwidth, float pixel_size, bool draw_overdraw)
	{
		Polyline::render(vertices, count, count, halfwidth, pixel_size, draw_overdraw);
	}

protected:
	virtual void renderEdge(std::vector<Vector> &anchors, std::vector<Vector> &normals,
	                        Vector &s, float &len_s, Vector &ns,
	                        const Vector &q, const Vector &r, float hw);
};


/**
 * A Polyline whose segments are connected by a flat edge.
 * @author Matthias Richter
 */
class BevelJoinPolyline : public Polyline
{
public:
	void render(const float *vertices, size_t count, float halfwidth, float pixel_size, bool draw_overdraw)
	{
		Polyline::render(vertices, count, 2 * count - 4, halfwidth, pixel_size, draw_overdraw);
	}

protected:
	virtual void renderEdge(std::vector<Vector> &anchors, std::vector<Vector> &normals,
	                        Vector &s, float &len_s, Vector &ns,
	                        const Vector &q, const Vector &r, float hw);
};

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_POLYLINE_H
