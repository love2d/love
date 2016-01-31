/**
 * Copyright (c) 2006-2016 LOVE Development Team
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
#include <sstream>
#include <iomanip>

// C
#include <cmath>
#include <cstdlib>

namespace love
{
namespace math
{

// Thomas Wang's 64-bit integer hashing function:
// https://web.archive.org/web/20110807030012/http://www.cris.com/%7ETtwang/tech/inthash.htm
static uint64 wangHash64(uint64 key)
{
	key = (~key) + (key << 21); // key = (key << 21) - key - 1;
	key = key ^ (key >> 24);
	key = (key + (key << 3)) + (key << 8); // key * 265
	key = key ^ (key >> 14);
	key = (key + (key << 2)) + (key << 4); // key * 21
	key = key ^ (key >> 28);
	key = key + (key << 31);
	return key;
}

// 64 bit Xorshift implementation taken from the end of Sec. 3 (page 4) in
// George Marsaglia, "Xorshift RNGs", Journal of Statistical Software, Vol.8 (Issue 14), 2003
// Use an 'Xorshift*' variant, as shown here: http://xorshift.di.unimi.it

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
	rng_state.b64 ^= (rng_state.b64 >> 12);
	rng_state.b64 ^= (rng_state.b64 << 25);
	rng_state.b64 ^= (rng_state.b64 >> 27);
	return rng_state.b64 * 2685821657736338717ULL;
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
	seed = newseed;

	// Xorshift isn't designed to give a good distribution of values across many
	// similar seeds, so we hash the state integer before using it.
	// http://www.reedbeta.com/blog/2013/01/12/quick-and-easy-gpu-random-numbers-in-d3d11/
	// Xorshift also can't handle a state value of 0, so we avoid that.
	do
	{
		newseed.b64 = wangHash64(newseed.b64);
	} while (newseed.b64 == 0);

	rng_state = newseed;
}

RandomGenerator::Seed RandomGenerator::getSeed() const
{
	return seed;
}

void RandomGenerator::setState(const std::string &statestr)
{
	// For this implementation we'll accept a hex string representing the
	// 64-bit state integer xorshift uses.

	// Hex string must start with 0x.
	if (statestr.find("0x") != 0 || statestr.size() < 3)
		throw love::Exception("Invalid random state: %s", statestr.c_str());

	Seed state = {};

	char *end = nullptr;
	state.b64 = strtoull(statestr.c_str(), &end, 16);

	if (end != nullptr && *end != 0)
		throw love::Exception("Invalid random state: %s", statestr.c_str());

	rng_state = state;
}

std::string RandomGenerator::getState() const
{
	// For this implementation we'll return a hex string representing the 64-bit
	// state integer xorshift uses.
	std::stringstream ss;
	ss << "0x" << std::setfill('0') << std::setw(16) << std::hex << rng_state.b64;

	return ss.str();
}

} // math
} // love
