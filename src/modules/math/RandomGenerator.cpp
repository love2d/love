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

#include "RandomGenerator.h"

// STL
#include <cmath>

namespace love
{
namespace math
{

// 64 bit Xorshift implementation taken from the end of Sec. 3 (page 4) in
// George Marsaglia, "Xorshift RNGs", Journal of Statistical Software, Vol.8 (Issue 14), 2003

RandomGenerator::RandomGenerator()
	: last_randomnormal(std::numeric_limits<double>::infinity())
{
	// because it is too big for some compilers to handle ... if you know what
	// i mean
#ifdef LOVE_BIG_ENDIAN
	seed.b32.a = 0x0139408D;
	seed.b32.b = 0xCBBF7A44;
#else
	seed.b32.b = 0x0139408D;
	seed.b32.a = 0xCBBF7A44;
#endif

	rng_state = seed;
}

void RandomGenerator::setSeed(RandomGenerator::Seed newseed)
{
	// 0 xor 0 is still 0, so Xorshift can't generate new numbers.
	if (newseed.b64 == 0)
		throw love::Exception("Invalid random seed.");

	seed = newseed;
	rng_state = seed;
}

uint64 RandomGenerator::rand()
{
	rng_state.b64 ^= (rng_state.b64 << 13);
	rng_state.b64 ^= (rng_state.b64 >> 7);
	rng_state.b64 ^= (rng_state.b64 << 17);
	return rng_state.b64;
}

// Box–Muller transform
double RandomGenerator::randomNormal(double stddev)
{
	// use cached number if possible
	if (last_randomnormal != std::numeric_limits<double>::infinity())
	{
		double r = last_randomnormal;
		last_randomnormal = std::numeric_limits<double>::infinity();
		return r * stddev;
	}

	double r   = sqrt(-2.0 * log(1. - random()));
	double phi = 2.0 * LOVE_M_PI * (1. - random());

	last_randomnormal = r * cos(phi);
	return r * sin(phi) * stddev;
}

} // math
} // love
