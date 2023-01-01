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

#include "floattypes.h"

#include <limits>
#include <cmath>

namespace love
{

// Code from ftp://www.fox-toolkit.org/pub/fasthalffloatconversion.pdf

static bool halfInitialized = false;

// tables for half -> float conversions
static uint32 mantissatable[2048];
static uint16 offsettable[64];
static uint32 exponenttable[64];

// tables for float -> half conversions
static uint16 basetable[512];
static uint8 shifttable[512];


static uint32 convertMantissa(uint32 i)
{
	uint32 m = i << 13; // Zero pad mantissa bits
	uint32 e = 0; // Zero exponent

	while (!(m & 0x00800000)) // While not normalized
	{
		e -= 0x00800000; // Decrement exponent (1<<23)
		m <<= 1; // Shift mantissa
	}

	m &= ~(0x00800000); // Clear leading 1 bit
	e += 0x38800000; // Adjust bias ((127-14)<<23)

	return m | e; // Return combined number
}

void float16Init()
{
	if (halfInitialized)
		return;

	halfInitialized = true;


	// tables for float16 -> float32 conversions.

	mantissatable[0] = 0;

	for (uint32 i = 1; i < 1024; i++)
		mantissatable[i] = convertMantissa(i);

	for (uint32 i = 1024; i < 2048; i++)
		mantissatable[i] = 0x38000000 + ((i - 1024) << 13);

	exponenttable[0] = 0;
	exponenttable[32] = 0x80000000;

	for (uint32 i = 0; i < 31; i++)
		exponenttable[i] = i << 23;

	for (uint32 i = 33; i < 63; i++)
		exponenttable[i] = 0x80000000 + ((i - 32) << 23);

	exponenttable[31] = 0x47800000;
	exponenttable[63] = 0xC7800000;

	for (int i = 0; i < 64; i++)
	{
		if (i == 0 || i == 32)
			offsettable[i] = 0;
		else
			offsettable[i] = 1024;
	}


	// tables for float32 -> float16 conversions.

	for (uint32 i = 0; i < 256; i++)
	{
		int e = (int) i - 127;

		if (e < -24) // Very small numbers map to zero
		{
			basetable[i | 0x000] = 0x0000;
			basetable[i | 0x100] = 0x8000;
			shifttable[i | 0x000] = 24;
			shifttable[i | 0x100] = 24;
		}
		else if (e < -14) // Small numbers map to denorms
		{
			basetable[i | 0x000] = (0x0400 >> (-e - 14));
			basetable[i | 0x100] = (0x0400 >> (-e - 14)) | 0x8000;
			shifttable[i | 0x000] = -e - 1;
			shifttable[i | 0x100] = -e - 1;
		}
		else if (e <= 15) // Normal numbers just lose precision
		{
			basetable[i | 0x000] = ((e + 15) << 10);
			basetable[i | 0x100] = ((e + 15) << 10) | 0x8000;
			shifttable[i | 0x000] = 13;
			shifttable[i | 0x100] = 13;
		}
		else if (e < 128) // Large numbers map to Infinity
		{
			basetable[i | 0x000] = 0x7C00;
			basetable[i | 0x100] = 0xFC00;
			shifttable[i | 0x000] = 24;
			shifttable[i | 0x100] = 24;
		}
		else // Infinity and NaN's stay Infinity and NaN's
		{
			basetable[i | 0x000] = 0x7C00;
			basetable[i | 0x100] = 0xFC00;
			shifttable[i | 0x000] = 13;
			shifttable[i | 0x100] = 13;
		}
	}
}

static inline uint32 asuint32(float f)
{
	union { float f; uint32 u; } conv;
	conv.f = f;
	return conv.u;
}

static inline float asfloat32(uint32 u)
{
	union { float f; uint32 u; } conv;
	conv.u = u;
	return conv.f;
}

float float16to32(float16 f)
{
	return asfloat32(mantissatable[offsettable[f >> 10] + (f & 0x3FF)] + exponenttable[f >> 10]);
}

float16 float32to16(float f)
{
	uint32 u = asuint32(f);
	return basetable[(u >> 23) & 0x1FF] + ((u & 0x007FFFFF) >> shifttable[(u >> 23) & 0x1FF]);
}

// Adapted from https://stackoverflow.com/questions/41532085/how-to-pack-unpack-11-and-10-bit-floats-in-javascript-for-webgl2

float float11to32(float11 f)
{
	uint16 exponent = f >> 6;
	uint16 mantissa = f & 0x3F;

	if (exponent == 0)
		return mantissa == 0 ? 0 : powf(2.0f, -14.0f) * (mantissa / 64.0f);

	if (exponent < 31)
		return powf(2.0f, exponent - 15) * (1.0f + mantissa / 64.0f);

	return mantissa == 0 ? std::numeric_limits<float>::infinity() : std::numeric_limits<float>::quiet_NaN();
}

float11 float32to11(float f)
{
	const uint16 EXPONENT_BITS = 0x1F;
	const uint16 EXPONENT_SHIFT = 6;
	const uint16 EXPONENT_BIAS = 15;
	const uint16 MANTISSA_BITS = 0x3F;
	const uint16 MANTISSA_SHIFT = (23 - EXPONENT_SHIFT);
	const uint16 MAX_EXPONENT = (EXPONENT_BITS << EXPONENT_SHIFT);

	uint32 u = asuint32(f);

	if (u & 0x80000000)
		return 0; // Negative values go to 0.

	// Map exponent to the range [-127,128]
	int32 exponent = (int32)((u >> 23) & 0xFF) - 127;
	uint32 mantissa = u & 0x007FFFFF;

	if (exponent > 15) // Infinity or NaN
		return MAX_EXPONENT | (exponent == 128 ? (mantissa & MANTISSA_BITS) : 0);
	else if (exponent <= -15)
		return 0;

	exponent += EXPONENT_BIAS;

	return ((uint16)exponent << EXPONENT_SHIFT) | (mantissa >> MANTISSA_SHIFT);
}

float float10to32(float10 f)
{
	uint16 exponent = f >> 5;
	uint16 mantissa = f & 0x1F;

	if (exponent == 0)
		return mantissa == 0 ? 0 : powf(2.0f, -14.0f) * (mantissa / 32.0f);

	if (exponent < 31)
		return powf(2.0f, exponent - 15) * (1.0f + mantissa / 32.0f);

	return mantissa == 0 ? std::numeric_limits<float>::infinity() : std::numeric_limits<float>::quiet_NaN();
}

float10 float32to10(float f)
{
	const uint16 EXPONENT_BITS = 0x1F;
	const uint16 EXPONENT_SHIFT = 5;
	const uint16 EXPONENT_BIAS = 15;
	const uint16 MANTISSA_BITS = 0x1F;
	const uint16 MANTISSA_SHIFT = (23 - EXPONENT_SHIFT);
	const uint16 MAX_EXPONENT = (EXPONENT_BITS << EXPONENT_SHIFT);

	uint32 u = asuint32(f);

	if (u & 0x80000000)
		return 0; // Negative values go to 0.

	// Map exponent to the range [-127,128]
	int32 exponent = (int32)((u >> 23) & 0xFF) - 127;
	uint32 mantissa = u & 0x007FFFFF;

	if (exponent > 15) // Infinity or NaN
		return MAX_EXPONENT | (exponent == 128 ? (mantissa & MANTISSA_BITS) : 0);
	else if (exponent <= -15)
		return 0;

	exponent += EXPONENT_BIAS;

	return ((uint16)exponent << EXPONENT_SHIFT) | (mantissa >> MANTISSA_SHIFT);
}

} // love
