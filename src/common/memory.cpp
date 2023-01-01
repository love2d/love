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

#include "config.h"
#include "memory.h"

#include <stdlib.h>

#ifdef LOVE_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <malloc.h>
#include <Windows.h>
#else
#include <unistd.h> // Assume POSIX support.
#endif

namespace love
{

bool alignedMalloc(void **mem, size_t size, size_t alignment)
{
#ifdef LOVE_WINDOWS
	*mem = _aligned_malloc(size, alignment);
	return *mem != nullptr;
#else
	return posix_memalign(mem, alignment, size) == 0;
#endif
}

void alignedFree(void *mem)
{
#ifdef LOVE_WINDOWS
	_aligned_free(mem);
#else
	free(mem);
#endif
}

size_t getPageSize()
{
#ifdef LOVE_WINDOWS
	static DWORD size = 0;
	if (size == 0)
	{
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		size = si.dwPageSize;
	}

	return (size_t) size;
#else
	static const long size = sysconf(_SC_PAGESIZE);
	return size > 0 ? (size_t) size : 4096;
#endif
}

size_t alignUp(size_t size, size_t alignment)
{
	return (size + alignment - 1) & (~(alignment - 1));
}

} // love
