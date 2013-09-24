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

#ifndef LOVE_MATH_MODMATH_H
#define LOVE_MATH_MODMATH_H

#include "RandomGenerator.h"

// LOVE
#include "common/Module.h"
#include "common/math.h"
#include "common/Vector.h"
#include "common/int.h"

// Noise
#include "libraries/noise1234/simplexnoise1234.h"

// STL
#include <vector>

namespace love
{
namespace math
{

class BezierCurve;

class Math : public Module
{
private:

	RandomGenerator rng;

public:

	virtual ~Math()
	{}

	inline void setRandomState(RandomGenerator::State state)
	{
		rng.setState(state);
	}

	inline void setRandomState(uint32 low, uint32 high)
	{
		rng.setState(low, high);
	}

	inline RandomGenerator::State getRandomState() const
	{
		return rng.getState();
	}

	inline void getRandomState(uint32 &low, uint32 &high) const
	{
		rng.getState(low, high);
	}

	/**
	 * @copydoc RandomGenerator::random()
	 **/
	inline double random()
	{
		return rng.random();
	}

	/**
	 * @copydoc RandomGenerator::random(double)
	 **/
	inline double random(double max)
	{
		return rng.random(max);
	}

	/**
	 * @copydoc RandomGenerator::random(double,double)
	 **/
	inline double random(double min, double max)
	{
		return rng.random(min, max);
	}

	/**
	 * @copydoc RandomGenerator::randomNormal()
	 **/
	inline double randomNormal(double stddev)
	{
		return rng.randomNormal(stddev);
	}

	/**
	 * Create a new random number generator.
	 **/
	RandomGenerator *newRandomGenerator();

	/**
	 * Creates a new bezier curve.
	 **/
	BezierCurve *newBezierCurve(const std::vector<Vector> &points);

	virtual const char *getName() const
	{
		return "love.math";
	}

	/**
	 * Triangulate a simple polygon.
	 *
	 * @param polygon Polygon to triangulate. Must not intersect itself.
	 * @return List of triangles the polygon is composed of.
	 **/
	std::vector<Triangle> triangulate(const std::vector<Vertex> &polygon);

	/**
	 * Checks whether a polygon is convex.
	 *
	 * @param polygon Polygon to test.
	 * @return True if the polygon is convex, false otherwise.
	 **/
	bool isConvex(const std::vector<Vertex> &polygon);

	/**
	 * Calculate Simplex noise for the specified coordinate(s).
	 *
	 * @return Noise value in the range of [0, 1].
	 **/
	float noise(float x) const;
	float noise(float x, float y) const;
	float noise(float x, float y, float z) const;
	float noise(float x, float y, float z, float w) const;

	static Math instance;

private:

	Math();

}; // Math

inline float Math::noise(float x) const
{
	return SimplexNoise1234::noise(x) * 0.5f + 0.5f;
}

inline float Math::noise(float x, float y) const
{
	return SimplexNoise1234::noise(x, y) * 0.5f + 0.5f;
}

inline float Math::noise(float x, float y, float z) const
{
	return SimplexNoise1234::noise(x, y, z) * 0.5f + 0.5f;
}

inline float Math::noise(float x, float y, float z, float w) const
{
	return SimplexNoise1234::noise(x, y, z, w) * 0.5f + 0.5f;
}

} // math
} // love

#endif
