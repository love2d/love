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

#include "RandomGenerator.h"

// C++
#include <cmath>
#include <sstream>
#include <iomanip>

// C
#include <cstdlib>

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
	Seed newseed;
	newseed.b32.low = 0xCBBF7A44;
	newseed.b32.high = 0x0139408D;
	setSeed(newseed);
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

void RandomGenerator::setSeed(RandomGenerator::Seed newseed)
{
	// 0 xor 0 is still 0, so Xorshift can't generate new numbers.
	if (newseed.b64 == 0)
		throw love::Exception("Random seed cannot be 0.");

	seed = newseed;
	rng_state = seed;
}

RandomGenerator::Seed RandomGenerator::getSeed() const
{
	return seed;
}

void RandomGenerator::setState(const std::string &statestr)
{
	// For this implementation we'll accept a hex string representing the
	// 64-bit state integer xorshift uses.

	Seed state = {};

	// Hex string must start with 0x.
	if (statestr.find("0x") != 0 || statestr.size() < 3)
		throw love::Exception("Invalid random state.");

	// standardized strtoull (or 64 bit integer support for stringstream)
	// requires C++11's standard library, which we can't use yet.
	// I use strtol like this not because it's the best solution, but because
	// it's "good enough".

	// Convert the hex string to the state integer character-by-character.
	for (size_t i = 2; i < statestr.size(); i++)
	{
		char hex[2] = {statestr[i], 0};
		char *end = nullptr;

		// Convert the current hex character to a number.
		int nibble = strtol(hex, &end, 16);

		// Check if strtol failed to convert it.
		if (end != nullptr && *end != 0)
			throw love::Exception("Invalid random state.");

		state.b64 = (state.b64 << 4) + nibble;
	}

	rng_state = state;
}

std::string RandomGenerator::getState() const
{
	// For this implementation we'll return a hex string representing the 64-bit
	// state integer xorshift uses.

	std::stringstream ss;

	ss << "0x";

	// Again with the stringstream not dealing with 64 bit integers...
	ss << std::setfill('0') << std::setw(8) << std::hex << rng_state.b32.high;
	ss << std::setfill('0') << std::setw(8) << std::hex << rng_state.b32.low;

	return ss.str();
}

} // math
} // love
