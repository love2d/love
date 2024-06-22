/**
 * Copyright (c) 2006-2024 LOVE Development Team
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

#pragma once

#include <stddef.h>
#include <algorithm>
#include <limits>

namespace love
{

struct Range
{
	size_t first;
	size_t last;

	Range()
		: first(std::numeric_limits<size_t>::max())
		, last(0)
	{}

	Range(size_t offset, size_t size)
		: first(offset)
		, last(offset + size - 1)
	{}

	bool isValid() const { return first <= last; }

	void invalidate()
	{
		first = std::numeric_limits<size_t>::max();
		last = 0;
	}

	size_t getMin() const { return first; }
	size_t getMax() const { return last; }

	size_t getOffset() const { return first; }
	size_t getSize() const { return (last - first) + 1; }

	bool contains(const Range &other) const
	{
		return first <= other.first && last >= other.last;
	}

	bool intersects(const Range &other) const
	{
		return !(first > other.last || last < other.first);
	}

	void intersect(const Range &other)
	{
		first = std::max(first, other.first);
		last = std::min(last, other.last);
	}

	void encapsulate(size_t index)
	{
		first = std::min(first, index);
		last = std::max(last, index);
	}

	void encapsulate(size_t offset, size_t size)
	{
		first = std::min(first, offset);
		last = std::max(last, offset + size - 1);
	}

	void encapsulate(const Range &other)
	{
		first = std::min(first, other.first);
		last = std::max(last, other.last);
	}
};

} // love
