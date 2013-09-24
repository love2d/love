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

#ifndef LOVE_MATH_BEZIER_CURVE_H
#define LOVE_MATH_BEZIER_CURVE_H

// LOVE
#include "common/Object.h"
#include "common/Vector.h"
#include <vector>

namespace love
{
namespace math
{

class BezierCurve : public Object
{
public:

	/**
	 * @param controlPoints Control polygon of the curve.
	 **/
	BezierCurve(const std::vector<Vector> &controlPoints);

	/**
	 * @returns Degree of the curve
	 **/
	size_t getDegree() const
	{
		return controlPoints.size() - 1;
	}

	/**
	 * @returns First derivative of the curve.
	 */
	BezierCurve getDerivative() const;

	/**
	 * @returns i'th control point.
	 **/
	const Vector &getControlPoint(int i) const;

	/**
	 * Sets the i'th control point.
	 * @param i Control point to change.
	 * @param point New control point.
	 **/
	void setControlPoint(int i, const Vector &point);

	/**
	 * Insert a new control point before the i'th control point.
	 * If i < 0, Lua string indexing rules apply.
	 * @param point Control point to insert.
	 * @param pos Position to insert.
	 **/
	void insertControlPoint(const Vector &point, int pos = -1);

	/**
	 * Move the curve.
	 * @param t Translation vector.
	 */
	void translate(const Vector &t);

	/**
	 * Rotate the curve.
	 * @param phi Rotation angle (radians).
	 * @param center Rotation center.
	 */
	void rotate(double phi, const Vector &center);

	/**
	 * Scale the curve.
	 * @param phi Scale factor.
	 * @param center Scale center.
	 */
	void scale(double phi, const Vector &center);

	/**
	 * Evaluates the curve at time t.
	 * @param t Curve parameter, must satisfy 0 <= t <= 1.
	 **/
	Vector evaluate(double t) const;

	/**
	 * Renders the curve by subdivision.
	 * @param accuracy The 'fineness' of the curve.
	 * @returns A polygon chain that approximates the bezier curve.
	 **/
	std::vector<Vector> render(size_t accuracy = 4) const;

private:
	std::vector<Vector> controlPoints;
};

}
}

#endif
