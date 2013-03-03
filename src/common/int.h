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

#ifndef LOVE_INT_H
#define LOVE_INT_H

#include "common/config.h"

#ifndef LOVE_WINDOWS
#include <stdint.h>
#endif

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

// Blame Microsoft
#ifdef LOVE_WINDOWS
	typedef __int8 int8;
	typedef unsigned __int8 uint8;
	typedef __int16 int16;
	typedef unsigned __int16 uint16;
	typedef __int32 int32;
	typedef unsigned __int32 uint32;
	typedef __int64 int64;
	typedef unsigned __int64 uint64;
#else // LOVE_WINDOWS
	typedef int8_t int8;
	typedef uint8_t uint8;
	typedef int16_t int16;
	typedef uint16_t uint16;
	typedef int32_t int32;
	typedef uint32_t uint32;
	typedef int64_t int64;
	typedef uint64_t uint64;
#endif // LOVE_WINDOWS

} // love

#endif // LOVE_INT_H
