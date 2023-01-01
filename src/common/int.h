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

#ifndef LOVE_INT_H
#define LOVE_INT_H

// C standard sized integer types.
#include <stdint.h>

#define LOVE_INT8_MAX   0x7F
#define LOVE_UINT8_MAX  0xFF
#define LOVE_INT16_MAX  0x7FFF
#define LOVE_UINT16_MAX 0xFFFF
#define LOVE_INT32_MAX  0x7FFFFFFF
#define LOVE_UINT32_MAX 0xFFFFFFFF
#define LOVE_INT64_MAX  0x7FFFFFFFFFFFFFFF
#define LOVE_UINT64_MAX 0xFFFFFFFFFFFFFFFF

namespace love
{

typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;

static inline uint16 swapuint16(uint16 x)
{
	return (x >> 8) | (x << 8);
}

static inline uint32 swapuint32(uint32 x)
{
	return ((x & 0x000000FF) << 24) |
	       ((x & 0x0000FF00) <<  8) |
	       ((x & 0x00FF0000) >>  8) |
	       ((x & 0xFF000000) >> 24);
}

static inline uint64 swapuint64(uint64 x)
{
	return ((x << 56) & 0xFF00000000000000ULL) | ((x << 40) & 0x00FF000000000000ULL) |
	       ((x << 24) & 0x0000FF0000000000ULL) | ((x <<  8) & 0x000000FF00000000ULL) |
	       ((x >>  8) & 0x00000000FF000000ULL) | ((x >> 24) & 0x0000000000FF0000ULL) |
	       ((x >> 40) & 0x000000000000FF00ULL) | ((x >> 56) & 0x00000000000000FFULL);
}

} // love

#endif // LOVE_INT_H
