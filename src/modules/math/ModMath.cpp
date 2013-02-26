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

#include "ModMath.h"
#include "common/math.h"

#include <cmath>

namespace love
{
namespace math
{

// 64 bit Xorshift implementation taken from the end of Sec. 3 (page 4) in
// George Marsaglia, "Xorshift RNGs", Journal of Statistical Software, Vol.8 (Issue 14), 2003
ModMath::ModMath()
{
	RNGState.seed = 0x0139408DCBBF7A44;
	RNGState.last_randnormal = std::numeric_limits<double>::infinity();
}

uint32_t ModMath::rand()
{
	uint64_t &x = RNGState.seed;
	x ^= (x << 13);
	x ^= (x >> 7);
	x ^= (x << 17);
	return x;
}

// Box–Muller transform
double ModMath::randnormal(double stddev)
{
	if (RNGState.last_randnormal != std::numeric_limits<double>::infinity())
	{
		double r = RNGState.last_randnormal;
		RNGState.last_randnormal = std::numeric_limits<double>::infinity();
		return r * stddev;
	}

	double r   = sqrt(-2.0 * log(1. - random()));
	double phi = 2.0 * LOVE_M_PI * (1. - random());

	RNGState.last_randnormal = r * cos(phi);
	return r * sin(phi) * stddev;
}

} // math
} // love
