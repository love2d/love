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
#include "BezierCurve.h"
#include "common/Exception.h"

#include <cmath>

using namespace std;

namespace
{

/**
 * Subdivide Bezier polygon.
 **/
void subdivide(vector<love::Vector> &points, int k)
{
	if (k <= 0)
		return;

	// subdivision using de casteljau - subdivided control polygons are
	// on the 'edges' of the computation scheme, e.g:
	//
	// ------LEFT------->
	// b00  b10  b20  b30
	// b01  b11  b21 .---
	// b02  b12 .---'
	// b03 .---'RIGHT
	// <--'
	//
	// the subdivided control polygon is:
	// b00, b10, b20, b30, b21, b12, b03
	vector<love::Vector> left, right;
	left.reserve(points.size());
	right.reserve(points.size());

	for (size_t step = 1; step < points.size(); ++step)
	{
		left.push_back(points[0]);
		right.push_back(points[points.size() - step]);
		for (size_t i = 0; i < points.size() - step; ++i)
			points[i] = (points[i] + points[i+1]) * .5;
	}
	left.push_back(points[0]);
	right.push_back(points[0]);

	// recurse
	subdivide(left, k-1);
	subdivide(right, k-1);

	// merge (right is in reversed order)
	points.resize(left.size() + right.size() - 1);
	for (size_t i = 0; i < left.size(); ++i)
		points[i] = left[i];
	for (size_t i = 1; i < right.size(); ++i)
		points[i-1 + left.size()] = right[right.size() - i - 1];
}

}

namespace love
{
namespace math
{

BezierCurve::BezierCurve(const vector<Vector> &pts)
	: controlPoints(pts)
{
}


BezierCurve BezierCurve::getDerivative() const
{
	if (getDegree() < 1)
		throw Exception("Cannot derive a curve of degree < 1.");
	// actually we can, it just doesn't make any sense.

	vector<Vector> forward_differences(controlPoints.size()-1);
	float degree = float(getDegree());
	for (size_t i = 0; i < forward_differences.size(); ++i)
		forward_differences[i] = (controlPoints[i+1] - controlPoints[i]) * degree;

	return BezierCurve(forward_differences);
}

const Vector &BezierCurve::getControlPoint(int i) const
{
	while (i < 0)
		i += controlPoints.size();

	while ((size_t) i >= controlPoints.size())
		i -= controlPoints.size();

	return controlPoints[i];
}

void BezierCurve::setControlPoint(int i, const Vector &point)
{
	while (i < 0)
		i += controlPoints.size();

	while ((size_t) i >= controlPoints.size())
		i -= controlPoints.size();

	controlPoints[i] = point;
}

void BezierCurve::insertControlPoint(const Vector &point, int i)
{
	while (i < 0)
		i += controlPoints.size() + 1;

	while ((size_t)  i > controlPoints.size())
		throw Exception("Invalid control point index");

	controlPoints.insert(controlPoints.begin() + i, point);
}

void BezierCurve::removeControlPoint(int i)
{
	while (i < 0)
		i += controlPoints.size();
	
	while ((size_t) i >= controlPoints.size())
		i -= controlPoints.size();
	
	controlPoints.erase(controlPoints.begin() + i);
}
	
void BezierCurve::translate(const Vector &t)
{
	for (size_t i = 0; i < controlPoints.size(); ++i)
		controlPoints[i] += t;
}

void BezierCurve::rotate(double phi, const Vector &center)
{
	float c = cos(phi), s = sin(phi);
	for (size_t i = 0; i < controlPoints.size(); ++i)
	{
		Vector v = controlPoints[i] - center;
		controlPoints[i].x = c * v.x - s * v.y + center.x;
		controlPoints[i].y = s * v.x + c * v.y + center.y;
	}
}

void BezierCurve::scale(double s, const Vector &center)
{
	for (size_t i = 0; i < controlPoints.size(); ++i)
		controlPoints[i] = (controlPoints[i] - center) * s + center;
}

Vector BezierCurve::evaluate(double t) const
{
	if (t < 0 || t > 1)
		throw Exception("Invalid evaluation parameter: must be between 0 and 1");
	if (controlPoints.size() < 2)
		throw Exception("Invalid Bezier curve: Not enough control points.");

	// de casteljau
	vector<Vector> points(controlPoints);
	for (size_t step = 1; step < controlPoints.size(); ++step)
		for (size_t i = 0; i < controlPoints.size() - step; ++i)
			points[i] = points[i] * (1-t) + points[i+1] * t;
	
	return points[0];
}

vector<Vector> BezierCurve::render(size_t accuracy) const
{
	if (controlPoints.size() < 2)
		throw Exception("Invalid Bezier curve: Not enough control points.");
	vector<Vector> vertices(controlPoints);
	subdivide(vertices, accuracy);
	return vertices;
}

vector<Vector> BezierCurve::renderSegment(double start, double end, size_t accuracy) const
{
	if (controlPoints.size() < 2)
		throw Exception("Invalid Bezier curve: Not enough control points.");
	vector<Vector> vertices(controlPoints);
	subdivide(vertices, accuracy);
	if (start == end)
	{
		vertices.clear();
	}
	else if (start < end)
	{
		size_t start_idx = size_t(start * vertices.size());
		size_t end_idx = size_t(end * vertices.size() + 0.5);
		return std::vector<Vector>(vertices.begin() + start_idx, vertices.begin() + end_idx);
	}
	else if (end > start)
	{
		size_t start_idx = size_t(end * vertices.size() + 0.5);
		size_t end_idx = size_t(start * vertices.size());
		return std::vector<Vector>(vertices.begin() + start_idx, vertices.begin() + end_idx);
	}
	return vertices;
}

} // namespace math
} // namespace love
