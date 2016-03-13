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

#ifndef LOVE_VARIANT_H
#define LOVE_VARIANT_H

#include "common/runtime.h"
#include "common/Object.h"
#include "common/int.h"

#include <cstring>
#include <vector>

namespace love
{

class Variant
{
public:

	enum Type
	{
		UNKNOWN = 0,
		BOOLEAN,
		NUMBER,
		STRING,
		SMALLSTRING,
		LUSERDATA,
		FUSERDATA,
		NIL,
		TABLE
	};

	Variant();
	Variant(bool boolean);
	Variant(double number);
	Variant(const char *string, size_t len);
	Variant(void *userdata);
	Variant(love::Type udatatype, void *userdata);
	Variant(std::vector<std::pair<Variant, Variant>> *table);
	Variant(const Variant &v);
	Variant(Variant &&v);
	~Variant();

	Variant &operator = (const Variant &v);

	Type getType() const { return type; }

	static Variant fromLua(lua_State *L, int n, bool allowTables = true);
	void toLua(lua_State *L) const;

private:

	class SharedString : public love::Object
	{
	public:

		SharedString(const char *string, size_t len)
			: len(len)
		{
			str = new char[len+1];
			memcpy(str, string, len);
		}
		virtual ~SharedString() { delete[] str; }

		char *str;
		size_t len;
	};

	class SharedTable : public love::Object
	{
	public:

		SharedTable(std::vector<std::pair<Variant, Variant>> *table)
			: table(table)
		{
		}

		virtual ~SharedTable() { delete table; }

		std::vector<std::pair<Variant, Variant>> *table;
	};

	static const int MAX_SMALL_STRING_LENGTH = 15;

	Type type;
	love::Type udatatype;

	union Data
	{
		bool boolean;
		double number;
		SharedString *string;
		void *userdata;
		SharedTable *table;
		struct
		{
			char str[MAX_SMALL_STRING_LENGTH];
			uint8 len;
		} smallstring;
	} data;

}; // Variant
} // love

#endif // LOVE_VARIANT_H
