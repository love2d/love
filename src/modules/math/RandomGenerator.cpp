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
	: last_randnormal(std::numeric_limits<double>::infinity())
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
}

uint64 RandomGenerator::rand()
{
	rng_state ^= (rng_state << 13);
	rng_state ^= (rng_state >> 7);
	rng_state ^= (rng_state << 17);
	return rng_state;
}

// Box–Muller transform
double RandomGenerator::randnormal(double stddev)
{
	// use cached number if possible
	if (last_randnormal != std::numeric_limits<double>::infinity())
	{
		double r = last_randnormal;
		last_randnormal = std::numeric_limits<double>::infinity();
		return r * stddev;
	}

	double r   = sqrt(-2.0 * log(1. - random()));
	double phi = 2.0 * LOVE_M_PI * (1. - random());

	last_randnormal = r * cos(phi);
	return r * sin(phi) * stddev;
}

} // math
} // love
