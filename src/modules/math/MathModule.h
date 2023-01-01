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

#ifndef LOVE_MATH_MODMATH_H
#define LOVE_MATH_MODMATH_H

#include "RandomGenerator.h"

// LOVE
#include "common/Module.h"
#include "common/math.h"
#include "common/Vector.h"
#include "common/int.h"

// Noise
#include "libraries/noise1234/noise1234.h"
#include "libraries/noise1234/simplexnoise1234.h"

// STL
#include <vector>

namespace love
{
namespace math
{

class BezierCurve;
class Transform;

struct Triangle
{
	Triangle(const Vector2 &x, const Vector2 &y, const Vector2 &z)
		: a(x), b(y), c(z)
	{}
	Vector2 a, b, c;
};

/**
 * Triangulate a simple polygon.
 *
 * @param polygon Polygon to triangulate. Must not intersect itself.
 * @return List of triangles the polygon is composed of.
 **/
std::vector<Triangle> triangulate(const std::vector<love::Vector2> &polygon);

/**
 * Checks whether a polygon is convex.
 *
 * @param polygon Polygon to test.
 * @return True if the polygon is convex, false otherwise.
 **/
bool isConvex(const std::vector<love::Vector2> &polygon);

/**
 * Converts a value from the sRGB (gamma) colorspace to linear RGB.
 **/
float gammaToLinear(float c);

/**
 * Converts a value from linear RGB to the sRGB (gamma) colorspace.
 **/
float linearToGamma(float c);

/**
 * Calculate noise for the specified coordinate(s).
 *
 * @return Noise value in the range of [0, 1].
 **/
static float noise1(float x);
static float noise2(float x, float y);
static float noise3(float x, float y, float z);
static float noise4(float x, float y, float z, float w);


class Math : public Module
{
public:

	Math();
	virtual ~Math();

	RandomGenerator *getRandomGenerator()
	{
		return &rng;
	}

	/**
	 * Create a new random number generator.
	 **/
	RandomGenerator *newRandomGenerator();

	/**
	 * Creates a new bezier curve.
	 **/
	BezierCurve *newBezierCurve(const std::vector<Vector2> &points);

	Transform *newTransform();
	Transform *newTransform(float x, float y, float a, float sx, float sy, float ox, float oy, float kx, float ky);

	// Implements Module.
	virtual ModuleType getModuleType() const
	{
		return M_MATH;
	}

	virtual const char *getName() const
	{
		return "love.math";
	}

private:

	RandomGenerator rng;

}; // Math


static inline float noise1(float x)
{
	return SimplexNoise1234::noise(x) * 0.5f + 0.5f;
}

static inline float noise2(float x, float y)
{
	return SimplexNoise1234::noise(x, y) * 0.5f + 0.5f;
}

// Perlin noise is used instead of Simplex noise in the 3D and 4D cases to avoid
// patent issues.

static inline float noise3(float x, float y, float z)
{
	return Noise1234::noise(x, y, z) * 0.5f + 0.5f;
}

static inline float noise4(float x, float y, float z, float w)
{
	return Noise1234::noise(x, y, z, w) * 0.5f + 0.5f;
}

} // math
} // love

#endif
