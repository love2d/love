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

#include "Geometry.h"
#include "common/Exception.h"
#include "common/Vector.h"
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

Geometry::Geometry(const std::vector<vertex> &polygon)
	: vertexArray(NULL)
	, x_min(std::numeric_limits<float>::max())
	, x_max(std::numeric_limits<float>::min())
	, y_min(std::numeric_limits<float>::max())
	, y_max(std::numeric_limits<float>::min())
	, vertexColors(false)
{
	if (!Math::instance.isConvex(polygon))
		throw love::Exception("Geometry must be convex");

	vertexCount = polygon.size();
	vertexArray = new vertex[vertexCount];
	for (size_t i = 0; i < vertexCount; ++i)
	{
		const vertex &v = polygon[i];
		vertexArray[i] = v;
		x_min = v.x < x_min ? v.x : x_min;
		x_max = v.x > x_max ? v.x : x_max;
		y_min = v.y < y_min ? v.y : y_min;
		y_max = v.y > y_max ? v.y : y_max;
	}
}

Geometry::Geometry(float x, float y, float w, float h, float sw, float sh)
	: vertexArray(NULL)
	, vertexCount(4)
	, x_min(x)
	, x_max(x+w)
	, y_min(y)
	, y_max(y+h)
	, vertexColors(false)
{
	vertexArray = new vertex[4];
	float s0 = x/sw, s1 = (x+w)/sw, t0 = y/sh, t1 = (y+h)/sh;
	vertexArray[0] = vertex(0,0, s0,t0);
	vertexArray[1] = vertex(w,0, s1,t0);
	vertexArray[2] = vertex(w,h, s1,t1);
	vertexArray[3] = vertex(0,h, s0,t1);
}

Geometry::Geometry(const Geometry &other)
	: vertexCount(other.vertexCount)
	, x_min(other.x_min)
	, x_max(other.x_max)
	, y_min(other.y_min)
	, y_max(other.y_max)
	, vertexColors(other.vertexColors)
{
	vertexArray = new vertex[vertexCount];
	memcpy(vertexArray, other.vertexArray, vertexCount * sizeof(vertex));
}

Geometry &Geometry::operator=(const Geometry &other)
{
	if (this != &other)
	{
		Geometry temp(other);
		std::swap(vertexArray, temp.vertexArray);

		vertexCount = temp.vertexCount;
		x_min       = temp.x_min;
		x_max       = temp.x_max;
		y_min       = temp.y_min;
		y_max       = temp.y_max;

		vertexColors = other.vertexColors;
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
	if (i >= vertexCount)
		throw Exception("Invalid vertex index");
	return vertexArray[i];
}

void Geometry::setVertex(size_t i, const vertex &v)
{
	if (i >= vertexCount)
		throw Exception("Invalid vertex index");

	if (vertexArray[i].x != v.x || vertexArray[i].y != v.y)
	{
		x_min = v.x < x_min ? v.x : x_min;
		x_max = v.x > x_max ? v.x : x_max;
		y_min = v.y < y_min ? v.y : y_min;
		y_max = v.y > y_max ? v.y : y_max;
	}

	vertexArray[i] = v;
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

void Geometry::setVertexColors(bool on)
{
	vertexColors = on;
}

} // graphics
} // love
