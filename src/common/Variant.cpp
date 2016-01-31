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
	Type t = INVALID_ID;
	if (!lua_isuserdata(L, idx))
		return t;
	if (luaL_getmetafield(L, idx, "type") == 0)
		return t;
	lua_pushvalue(L, idx);
	int result = lua_pcall(L, 1, 1, 0);
	if (result == 0)
		getTypeName(lua_tostring(L, -1), t);
	if (result == 0 || result == LUA_ERRRUN)
		lua_pop(L, 1);
	return t;
}

static inline void delete_table(std::vector<std::pair<Variant*, Variant*> > *table)
{
	while (!table->empty())
	{
		std::pair<Variant*, Variant*> &kv = table->back();
		kv.first->release();
		kv.second->release();
		table->pop_back();
	}
	delete table;
}

Variant::Variant()
	: type(NIL)
	, data()
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
	: type(STRING)
{
	char *buf = new char[len+1];
	memset(buf, 0, len+1);
	memcpy(buf, string, len);
	data.string.str = buf;
	data.string.len = len;
}

Variant::Variant(char c)
	: type(CHARACTER)
{
	data.character = c;
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
Variant::Variant(std::vector<std::pair<Variant*, Variant*> > *table)
	: type(TABLE)
{
	data.table = table;
}

Variant::~Variant()
{
	switch (type)
	{
	case STRING:
		delete[] data.string.str;
		break;
	case FUSERDATA:
		((love::Object *) data.userdata)->release();
		break;
	case TABLE:
		delete_table(data.table);
	default:
		break;
	}
}

Variant *Variant::fromLua(lua_State *L, int n, bool allowTables)
{
	Variant *v = nullptr;
	size_t len;
	const char *str;
	if (n < 0) // Fix the stack position, we might modify it later
		n += lua_gettop(L) + 1;

	switch (lua_type(L, n))
	{
	case LUA_TBOOLEAN:
		v = new Variant(luax_toboolean(L, n));
		break;
	case LUA_TNUMBER:
		v = new Variant(lua_tonumber(L, n));
		break;
	case LUA_TSTRING:
		str = lua_tolstring(L, n, &len);
		v = new Variant(str, len);
		break;
	case LUA_TLIGHTUSERDATA:
		v = new Variant(lua_touserdata(L, n));
		break;
	case LUA_TUSERDATA:
		v = new Variant(extractudatatype(L, n), lua_touserdata(L, n));
		break;
	case LUA_TNIL:
		v = new Variant();
		break;
	case LUA_TTABLE:
		if (allowTables)
		{
			bool success = true;
			std::vector<std::pair<Variant*, Variant*>> *table = new std::vector<std::pair<Variant*, Variant*>>();
			lua_pushnil(L);
			while (lua_next(L, n))
			{
				Variant *key = fromLua(L, -2, false);
				if (!key)
				{
					success = false;
					lua_pop(L, 2);
					break;
				}

				Variant *value = fromLua(L, -1, false);
				if (!value)
				{
					delete key;
					success = false;
					lua_pop(L, 2);
					break;
				}

				table->push_back(std::make_pair(key, value));

				lua_pop(L, 1);
			}

			if (success)
				v = new Variant(table);
			else
				delete_table(table);
		}
		break;
	}
	return v;
}

void Variant::toLua(lua_State *L)
{
	switch (type)
	{
	case BOOLEAN:
		lua_pushboolean(L, data.boolean);
		break;
	case CHARACTER:
		lua_pushlstring(L, &data.character, 1);
		break;
	case NUMBER:
		lua_pushnumber(L, data.number);
		break;
	case STRING:
		lua_pushlstring(L, data.string.str, data.string.len);
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
		lua_createtable(L, 0, (int) data.table->size());
		for (size_t i = 0; i < data.table->size(); ++i)
		{
			std::pair<Variant*, Variant*> &kv = data.table->at(i);
			kv.first->toLua(L);
			kv.second->toLua(L);
			lua_settable(L, -3);
		}
		break;
	case NIL:
	default:
		lua_pushnil(L);
		break;
	}
}

} // love
