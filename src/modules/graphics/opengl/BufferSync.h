/**
 * Copyright (c) 2006-2017 LOVE Development Team
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

// LOVE
#include "OpenGL.h"

// C
#include <stddef.h>
#include <vector>

#pragma once

namespace love
{
namespace graphics
{
namespace opengl
{

class BufferSync
{
public:

	~BufferSync();

	void lock(size_t start, size_t length);
	void wait(size_t start, size_t length);
	void cleanup();

private:

	struct Range
	{
		size_t offset;
		size_t length;

		bool overlaps(const Range &other) const
		{
			return offset < (other.offset + other.length)
				&& other.offset < (offset + length);
		}
	};

	struct Lock
	{
		Range range;
		GLsync sync;

		Lock(const Range &range, GLsync sync)
			: range(range)
			, sync(sync)
		{}
	};

	void syncWait(GLsync sync);

	std::vector<Lock> locks;

}; // BufferSync

} // opengl
} // graphics
} // love
