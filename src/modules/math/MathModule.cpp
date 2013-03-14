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

#include "modules/math/MathModule.h"
#include "common/math.h"

#include <cmath>
#include <list>
#include <iostream>

using namespace std;
using love::vertex;

namespace
{

	// check if an angle is oriented counter clockwise
	inline bool is_oriented_ccw(const vertex &a, const vertex &b, const vertex &c)
	{
		// return det(b-a, c-a) >= 0
		return ((b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x)) >= 0;
	}

	// check if a and b are on the same side of the line c->d
	bool on_same_side(const vertex &a, const vertex &b, const vertex &c, const vertex &d)
	{
		float px = d.x - c.x, py = d.y - c.y;
		// return det(p, a-c) * det(p, b-c) >= 0
		float l = px * (a.y - c.y) - py * (a.x - c.x);
		float m = px * (b.y - c.y) - py * (b.x - c.x);
		return l * m >= 0;
	}

	// checks is p is contained in the triangle abc
	inline bool point_in_triangle(const vertex &p, const vertex &a, const vertex &b, const vertex &c)
	{
		return on_same_side(p,a, b,c) && on_same_side(p,b, a,c) && on_same_side(p,c, a,b);
	}

	// checks if any vertex in `vertices' is in the triangle abc.
	bool any_point_in_triangle(const list<const vertex *> &vertices, const vertex &a, const vertex &b, const vertex &c)
	{
		list<const vertex *>::const_iterator it, end = vertices.end();
		for (it = vertices.begin(); it != end; ++it)
		{
			const vertex *p = *it;
			if ((p != &a) && (p != &b) && (p != &c) && point_in_triangle(*p, a,b,c)) // oh god...
				return true;
		}

		return false;
	}

	inline bool is_ear(const vertex &a, const vertex &b, const vertex &c, const list<const vertex *> &vertices)
	{
		return is_oriented_ccw(a,b,c) && !any_point_in_triangle(vertices, a,b,c);
	}
}

namespace love
{
namespace math
{

Math Math::instance;

// 64 bit Xorshift implementation taken from the end of Sec. 3 (page 4) in
// George Marsaglia, "Xorshift RNGs", Journal of Statistical Software, Vol.8 (Issue 14), 2003
Math::Math()
	: last_randnormal(numeric_limits<double>::infinity())
{
	// because it is too big for some compilers to handle ... if you know what
	// i mean
	union
	{
		uint64 b64;
		struct
		{
			uint32 a;
			uint32 b;
		} b32;
	} converter;

#ifdef LOVE_BIG_ENDIAN
	converter.b32.a = 0x0139408D;
	converter.b32.b = 0xCBBF7A44;
#else
	converter.b32.b = 0x0139408D;
	converter.b32.a = 0xCBBF7A44;
#endif
	rng_state = converter.b64;

	// prevent the runtime from free()-ing this
	retain();
}

uint64 Math::rand()
{
	rng_state ^= (rng_state << 13);
	rng_state ^= (rng_state >> 7);
	rng_state ^= (rng_state << 17);
	return rng_state;
}

// Box–Muller transform
double Math::randnormal(double stddev)
{
	// use cached number if possible
	if (last_randnormal != numeric_limits<double>::infinity())
	{
		double r = last_randnormal;
		last_randnormal = numeric_limits<double>::infinity();
		return r * stddev;
	}

	double r   = sqrt(-2.0 * log(1. - random()));
	double phi = 2.0 * LOVE_M_PI * (1. - random());

	last_randnormal = r * cos(phi);
	return r * sin(phi) * stddev;
}

vector<Triangle> Math::triangulate(const vector<vertex> &polygon)
{
	if (polygon.size() < 3)
		throw love::Exception("Not a ploygon");
	else if (polygon.size() == 3)
		return vector<Triangle>(1, Triangle(polygon[0], polygon[1], polygon[2]));

	// collect list of connections and record leftmost item to check if the polygon
	// has the expected winding
	vector<size_t> next_idx(polygon.size()), prev_idx(polygon.size());
	size_t idx_lm = 0;
	for (size_t i = 0; i < polygon.size(); ++i)
	{
		const vertex &lm = polygon[idx_lm], &p = polygon[i];
		if (p.x < lm.x || (p.x == lm.x && p.y < lm.y))
			idx_lm = i;
		next_idx[i] = i+1;
		prev_idx[i] = i-1;
	}
	next_idx[next_idx.size()-1] = 0;
	prev_idx[0] = prev_idx.size()-1;

	// check if the polygon has the expected winding and reverse polygon if needed
	if (!is_oriented_ccw(polygon[prev_idx[idx_lm]], polygon[idx_lm], polygon[next_idx[idx_lm]]))
		next_idx.swap(prev_idx);

	// collect list of concave polygons
	list<const vertex *> concave_vertices;
	for (size_t i = 0; i < polygon.size(); ++i)
	{
		if (!is_oriented_ccw(polygon[prev_idx[i]], polygon[i], polygon[next_idx[i]]))
			concave_vertices.push_back(&polygon[i]);
	}

	// triangulation according to kong
	vector<Triangle> triangles;
	size_t n_vertices = polygon.size();
	size_t current = 1, skipped = 0, next, prev;
	while (n_vertices > 3)
	{
		next = next_idx[current];
		prev = prev_idx[current];
		const vertex &a = polygon[prev], &b = polygon[current], &c = polygon[next];
		if (is_ear(a,b,c, concave_vertices))
		{
			triangles.push_back(Triangle(a,b,c));
			next_idx[prev] = next;
			prev_idx[next] = prev;
			concave_vertices.remove(&b);
			--n_vertices;
			skipped = 0;
		}
		else if (++skipped > n_vertices)
		{
			throw love::Exception("Cannot triangulate polygon.");
		}
		current = next;
	}
	next = next_idx[current];
	prev = prev_idx[current];
	triangles.push_back(Triangle(polygon[prev], polygon[current], polygon[next]));

	return triangles;
}

} // math
} // love
