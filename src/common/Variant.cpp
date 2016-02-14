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

#include "Variant.h"
#include "common/StringMap.h"

namespace love
{

static love::Type extractudatatype(lua_State *L, int idx)
{
	Proxy *u = (Proxy *)lua_touserdata(L, idx);

	if (u == nullptr || u->type <= INVALID_ID || u->type >= TYPE_MAX_ENUM)
		return INVALID_ID;

	// We could get rid of the dynamic_cast for more performance, but it would
	// be less safe...
	if (dynamic_cast<Object *>(u->object) != nullptr)
		return u->type;

	return INVALID_ID;
}

Variant::Variant()
	: type(NIL)
{
}

Variant::Variant(bool boolean)
	: type(BOOLEAN)
{
	data.boolean = boolean;
}

Variant::Variant(double number)
	: type(NUMBER)
{
	data.number = number;
}

Variant::Variant(const char *string, size_t len)
{
	if (len <= MAX_SMALL_STRING_LENGTH)
	{
		type = SMALLSTRING;
		memcpy(data.smallstring.str, string, len);
		data.smallstring.len = (uint8) len;
	}
	else
	{
		type = STRING;
		data.string = new SharedString(string, len);
	}
}

Variant::Variant(void *userdata)
	: type(LUSERDATA)
{
	data.userdata = userdata;
}

Variant::Variant(love::Type udatatype, void *userdata)
	: type(FUSERDATA)
	, udatatype(udatatype)
{
	if (udatatype != INVALID_ID)
	{
		Proxy *p = (Proxy *) userdata;
		data.userdata = p->object;
		p->object->retain();
	}
	else
		data.userdata = userdata;
}

// Variant gets ownership of the vector.
Variant::Variant(std::vector<std::pair<Variant, Variant>> *table)
	: type(TABLE)
{
	data.table = new SharedTable(table);
}

Variant::Variant(const Variant &v)
	: type(v.type)
	, data(v.data)
	, udatatype(v.udatatype)
{
	if (type == STRING)
		data.string->retain();
	else if (type == FUSERDATA)
		((love::Object *) data.userdata)->retain();
	else if (type == TABLE)
		data.table->retain();
}

Variant::~Variant()
{
	switch (type)
	{
	case STRING:
		data.string->release();
		break;
	case FUSERDATA:
		((love::Object *) data.userdata)->release();
		break;
	case TABLE:
		data.table->release();
		break;
	default:
		break;
	}
}

Variant &Variant::operator = (const Variant &v)
{
	if (v.type == STRING)
		v.data.string->retain();
	else if (v.type == FUSERDATA)
		((love::Object *) v.data.userdata)->retain();
	else if (v.type == TABLE)
		v.data.table->retain();

	if (type == STRING)
		data.string->release();
	else if (type == FUSERDATA)
		((love::Object *) v.data.userdata)->release();
	else if (type == TABLE)
		data.table->release();

	type = v.type;
	data = v.data;
	udatatype = v.udatatype;

	return *this;
}

bool Variant::fromLua(lua_State *L, int n, Variant *v, bool allowTables)
{
	size_t len;
	const char *str;

	if (n < 0) // Fix the stack position, we might modify it later
		n += lua_gettop(L) + 1;

	switch (lua_type(L, n))
	{
	case LUA_TBOOLEAN:
		*v = Variant(luax_toboolean(L, n));
		return true;
	case LUA_TNUMBER:
		*v = Variant(lua_tonumber(L, n));
		return true;
	case LUA_TSTRING:
		str = lua_tolstring(L, n, &len);
		*v = Variant(str, len);
		return true;
	case LUA_TLIGHTUSERDATA:
		*v = Variant(lua_touserdata(L, n));
		return true;
	case LUA_TUSERDATA:
		*v = Variant(extractudatatype(L, n), lua_touserdata(L, n));
		return true;
	case LUA_TNIL:
		*v = Variant();
		return true;
	case LUA_TTABLE:
		if (allowTables)
		{
			bool success = true;
			std::vector<std::pair<Variant, Variant>> *table = new std::vector<std::pair<Variant, Variant>>();
			std::pair<Variant, Variant> pair;

			lua_pushnil(L);

			while (lua_next(L, n))
			{
				if (!fromLua(L, -2, &pair.first, false) || !fromLua(L, -1, &pair.second, false))
				{
					success = false;
					lua_pop(L, 2);
					break;
				}

				table->push_back(pair);
				lua_pop(L, 1);
			}

			if (success)
			{
				*v = Variant(table);
				return true;
			}
			else
				delete table;
		}
		break;
	}

	return false;
}

void Variant::toLua(lua_State *L) const
{
	switch (type)
	{
	case BOOLEAN:
		lua_pushboolean(L, data.boolean);
		break;
	case NUMBER:
		lua_pushnumber(L, data.number);
		break;
	case STRING:
		lua_pushlstring(L, data.string->str, data.string->len);
		break;
	case SMALLSTRING:
		lua_pushlstring(L, data.smallstring.str, data.smallstring.len);
		break;
	case LUSERDATA:
		lua_pushlightuserdata(L, data.userdata);
		break;
	case FUSERDATA:
		if (udatatype != INVALID_ID)
			luax_pushtype(L, udatatype, (love::Object *) data.userdata);
		else
			lua_pushlightuserdata(L, data.userdata);
		// I know this is not the same
		// sadly, however, it's the most
		// I can do (at the moment).
		break;
	case TABLE:
	{
		std::vector<std::pair<Variant, Variant>> *table = data.table->table;
		lua_createtable(L, 0, (int) table->size());

		for (size_t i = 0; i < table->size(); ++i)
		{
			std::pair<Variant, Variant> &kv = table->at(i);
			kv.first.toLua(L);
			kv.second.toLua(L);
			lua_settable(L, -3);
		}

		break;
	}
	case NIL:
	default:
		lua_pushnil(L);
		break;
	}
}

} // love
