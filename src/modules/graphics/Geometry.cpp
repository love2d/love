/**
 * Copyright (c) 2006-2012 LOVE Development Team
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

#include "Geometry.h"
#include "common/Exception.h"
#include "modules/math/MathModule.h"

using love::math::Math;

// STD
#include <limits>
#include <algorithm> // for std::swap()
#include <cstring> // For memcpy

namespace love
{
namespace graphics
{


Geometry::Geometry(const std::vector<vertex> &p)
	: polygon(p)
	, vertexArray(NULL)
	, x_min(std::numeric_limits<float>::max())
	, x_max(std::numeric_limits<float>::min())
	, y_min(std::numeric_limits<float>::max())
	, y_max(std::numeric_limits<float>::min())
{
	for (size_t i = 0; i < polygon.size(); ++i)
	{
		const vertex &v = polygon[i];
		x_min = v.x < x_min ? v.x : x_min;
		x_max = v.x > x_max ? v.x : x_max;
		y_min = v.y < y_min ? v.y : y_min;
		y_max = v.y > y_max ? v.y : y_max;
	}

	triangulate();
}

Geometry::Geometry(float x, float y, float w, float h, float sw, float sh)
	: vertexArray(NULL)
	, x_min(x)
	, x_max(x+w)
	, y_min(y)
	, y_max(y+h)
{
	float s0 = x/sw, s1 = (x+w)/sw, t0 = y/sh, t1 = (y+h)/sh;
	polygon.resize(4);
	polygon[0] = vertex(0,0, s0,t0);
	polygon[1] = vertex(w,0, s1,t0);
	polygon[2] = vertex(w,h, s1,t1);
	polygon[3] = vertex(0,h, s0,t1);

	triangulate();
}

Geometry::Geometry(const Geometry &other)
	: polygon(other.polygon)
	, vertexCount(other.vertexCount)
	, x_min(other.x_min)
	, x_max(other.x_max)
	, y_min(other.y_min)
	, y_max(other.y_max)
{
	vertexArray = new vertex[vertexCount];
	memcpy(vertexArray, other.vertexArray, vertexCount * sizeof(vertex));
}

Geometry &Geometry::operator=(const Geometry &other)
{
	if (this != &other)
	{
		Geometry temp(other);
		std::swap(polygon, temp.polygon);
		std::swap(vertexArray, temp.vertexArray);

		vertexCount = temp.vertexCount;
		x_min       = temp.x_min;
		x_max       = temp.x_max;
		y_min       = temp.y_min;
		y_max       = temp.y_max;
	}
	return *this;
}

Geometry::~Geometry()
{
	if (vertexArray)
	{
		delete[] vertexArray;
		vertexArray = NULL;
	}
}

const vertex &Geometry::getVertex(size_t i) const
{
	if (i >= polygon.size())
		throw Exception("Invalid vertex index");
	return polygon[i];
}

void Geometry::setVertex(size_t i, const vertex &v)
{
	if (i >= polygon.size())
		throw Exception("Invalid vertex index");

	polygon[i] = v;
	x_min = v.x < x_min ? v.x : x_min;
	x_max = v.x > x_max ? v.x : x_max;
	y_min = v.y < y_min ? v.y : y_min;
	y_max = v.y > y_max ? v.y : y_max;

	triangulate();
}

void Geometry::flip(bool x, bool y)
{
	for (size_t i = 0; i < vertexCount; ++i)
	{
		vertex &v = vertexArray[i];
		if (x) v.x = x_max + x_min - v.x;
		if (y) v.y = y_max + y_min - v.y;
	}
}

void Geometry::triangulate()
{
	const std::vector<Triangle> triangles = Math::instance.triangulate(polygon);

	if (vertexArray)
		delete[] vertexArray;

	vertexCount = triangles.size() * 3;
	vertexArray = new vertex[vertexCount];

	for (size_t i = 0; i < triangles.size(); ++i)
	{
		const Triangle &t = triangles[i];
		vertexArray[i*3]   = t.a;
		vertexArray[i*3+1] = t.b;
		vertexArray[i*3+2] = t.c;
	}
}

} // graphics
} // love
