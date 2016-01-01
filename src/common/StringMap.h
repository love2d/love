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

#ifndef LOVE_STRING_MAP_H
#define LOVE_STRING_MAP_H

#include "Exception.h"

namespace love
{

template<typename T, unsigned int SIZE>
class StringMap
{
public:

	struct Entry
	{
		const char *key;
		T value;
	};

	StringMap(const Entry *entries, unsigned int num)
	{

		for (unsigned int i = 0; i < SIZE; ++i)
			reverse[i] = nullptr;

		unsigned int n = num / sizeof(Entry);

		for (unsigned int i = 0; i < n; ++i)
			add(entries[i].key, entries[i].value);
	}

	bool streq(const char *a, const char *b)
	{
		while (*a != 0 && *b != 0)
		{
			if (*a != *b)
				return false;

			++a;
			++b;
		}

		return (*a == 0 && *b == 0);
	}

	bool find(const char *key, T &t)
	{
		unsigned int str_hash = djb2(key);

		for (unsigned int i = 0; i < MAX; ++i)
		{
			unsigned int str_i = (str_hash + i) % MAX;

			if (!records[str_i].set)
				return false;

			if (streq(records[str_i].key, key))
			{
				t = records[str_i].value;
				return true;
			}
		}

		return false;
	}

	bool find(T key, const char *&str)
	{
		unsigned int index = (unsigned int) key;

		if (index >= SIZE)
			return false;

		if (reverse[index] != nullptr)
		{
			str = reverse[index];
			return true;
		}
		else
		{
			return false;
		}
	}

	bool add(const char *key, T value)
	{
		unsigned int str_hash = djb2(key);
		bool inserted = false;

		for (unsigned int i = 0; i < MAX; ++i)
		{
			unsigned int str_i = (str_hash + i) % MAX;

			if (!records[str_i].set)
			{
				inserted = true;
				records[str_i].set = true;
				records[str_i].key = key;
				records[str_i].value = value;
				break;
			}
		}

		unsigned int index = (unsigned int) value;

		if (index >= SIZE)
		{
			printf("Constant %s out of bounds with %u!\n", key, index);
			return false;
		}

		reverse[index] = key;

		return inserted;
	}

	unsigned int djb2(const char *key)
	{
		unsigned int hash = 5381;
		int c;

		while ((c = *key++))
			hash = ((hash << 5) + hash) + c;

		return hash;
	}

private:

	struct Record
	{
		const char *key;
		T value;
		bool set;
		Record() : set(false) {}
	};

	static const unsigned int MAX = SIZE * 2;

	Record records[MAX];
	const char *reverse[SIZE];

}; // StringMap

} // love

#endif // LOVE_STRING_MAP_H
