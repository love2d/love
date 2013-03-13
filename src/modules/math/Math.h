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

// LOVE
#include "common/Module.h"
#include "common/math.h"

// STL
#include <limits>
#include <stdint.h>
#include <vector>

namespace love
{
namespace math
{

class Math : public Module
{
public:
	virtual ~Math()
	{}

	/** Set pseudo random seed.
	 *
	 * It's up to the implementation how to use this.
	 *
	 * @param seed The random seed.
	 */
	inline void randomseed(uint64_t seed)
	{
		rng_state = seed;
	}

	/** Return uniformly distributed pseudo random integer.
	 *
	 * @returns Pseudo random integer in [0,2^32).
	 */
	uint32_t rand();

	/** Get uniformly distributed pseudo random number in [0,1).
	 *
	 * @returns Pseudo random number in [0,1).
	 */
	inline double random()
	{
		return double(rand()) / (double(std::numeric_limits<uint32_t>::max()) + 1.0);
	}

	/** Get uniformly distributed pseudo random number in [0,max).
	 *
	 * @returns Pseudo random number in [0,max).
	 */
	inline double random(double max)
	{
		return random() * max;
	}

	/** Get uniformly distributed pseudo random number in [min, max).
	 *
	 * @returns Pseudo random number in [min, max).
	 */
	inline double random(double min, double max)
	{
		return random() * (max - min) + min;
	}

	/** Get normally distributed pseudo random number.
	 *
	 * @param stddev Standard deviation of the distribution.
	 * @returns Normally distributed random number with mean 0 and variance (stddev)².
	 */
	double randnormal(double stddev);

	virtual const char *getName() const
	{
		return "love.math";
	}

	/** Triangulate a simple polygon.
	 * @param polygon Polygon to triangulate. Must not intersect itself.
	 * @returns List of triangles the polygon is composed of.
	 */
	std::vector<Triangle> triangulate(const std::vector<vertex> &polygon);

	static Math instance;

private:
	Math();

	uint64_t rng_state;
	double last_randnormal;
};

} // math
} // love

#endif
