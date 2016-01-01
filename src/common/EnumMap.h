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

#ifndef LOVE_ENUM_MAP_H
#define LOVE_ENUM_MAP_H

#include "Exception.h"

namespace love
{

template<typename T, typename U, unsigned int PEAK>
class EnumMap
{
public:

	struct Entry
	{
		T t;
		U u;
	};

	EnumMap(const Entry *entries, unsigned int size)
	{
		unsigned int n = size / sizeof(Entry);

		for (unsigned int i = 0; i < n; ++i)
		{
			unsigned int e_t = (unsigned int) entries[i].t;
			unsigned int e_u = (unsigned int) entries[i].u;

			if (e_t < PEAK)
			{
				values_u[e_t].v = e_u;
				values_u[e_t].set = true;
			}
			if (e_u < PEAK)
			{
				values_t[e_u].v = e_t;
				values_t[e_u].set = true;
			}
		}
	}

	bool find(T t, U &u)
	{
		if ((unsigned int) t < PEAK && values_u[(unsigned int) t].set)
		{
			u = (U) values_u[(unsigned int) t].v;
			return true;
		}

		return false;
	}

	bool find(U u, T &t)
	{
		if ((unsigned int) u < PEAK && values_t[(unsigned int) u].set)
		{
			t = (T) values_t[(unsigned int) u].v;
			return true;
		}

		return false;
	}

private:

	struct Value
	{
		unsigned v;
		bool set;
		Value() : set(false) {}
	};

	Value values_t[PEAK];
	Value values_u[PEAK];

}; // EnumMap

} // love

#endif // LOVE_ENUM_MAP_H
