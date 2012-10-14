#include "math.h"
#include <limits>
#include <cmath>

namespace
{
	// The Box–Muller transform generates two random numbers, one of which we
	// cache here. A value of +infinity is used to signal the cache is invalid
	// and that new numbers have to be generated.
	float last_randnormal = std::numeric_limits<float>::infinity();
}

namespace love
{

float random_normal(float o)
{
	// number in cache?
	if (last_randnormal != std::numeric_limits<float>::infinity())
	{
		float r = last_randnormal;
		last_randnormal = std::numeric_limits<float>::infinity();
		return r * o;
	}

	// else: generate numbers using the Box-Muller transform
	float a = sqrt(-2.0f * log(random()));
	float b = float(LOVE_M_PI) * 2.0f * random();
	last_randnormal = a * cos(b);
	return a * sin(b) * o;
}

} // namespace love
