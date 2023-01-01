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

#include <memory>

#include "Variant.h"
#include "common/StringMap.h"

namespace love
{

static Proxy *tryextractproxy(lua_State *L, int idx)
{
	Proxy *u = (Proxy *)lua_touserdata(L, idx);

	if (u == nullptr || u->type == nullptr)
		return nullptr;

	// We could get rid of the dynamic_cast for more performance, but it would
	// be less safe...
	if (dynamic_cast<Object *>(u->object) != nullptr)
		return u;

	return nullptr;
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

Variant::Variant(const char *str, size_t len)
{
	if (len <= MAX_SMALL_STRING_LENGTH)
	{
		type = SMALLSTRING;
		memcpy(data.smallstring.str, str, len);
		data.smallstring.len = (uint8) len;
	}
	else
	{
		type = STRING;
		data.string = new SharedString(str, len);
	}
}

Variant::Variant(const std::string &str)
	: Variant(str.c_str(), str.length())
{
}

Variant::Variant(void *lightuserdata)
	: type(LUSERDATA)
{
	data.userdata = lightuserdata;
}

Variant::Variant(love::Type *lovetype, love::Object *object)
	: type(LOVEOBJECT)
{
	data.objectproxy.type = lovetype;
	data.objectproxy.object = object;

	if (data.objectproxy.object != nullptr)
		data.objectproxy.object->retain();
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
{
	if (type == STRING)
		data.string->retain();
	else if (type == LOVEOBJECT && data.objectproxy.object != nullptr)
		data.objectproxy.object->retain();
	else if (type == TABLE)
		data.table->retain();
}

Variant::Variant(Variant &&v)
	: type(std::move(v.type))
	, data(std::move(v.data))
{
	v.type = NIL;
}

Variant::~Variant()
{
	if (type == STRING)
		data.string->release();
	else if (type == LOVEOBJECT && data.objectproxy.object != nullptr)
		data.objectproxy.object->release();
	else if (type == TABLE)
		data.table->release();
}

Variant &Variant::operator = (const Variant &v)
{
	if (v.type == STRING)
		v.data.string->retain();
	else if (v.type == LOVEOBJECT && v.data.objectproxy.object != nullptr)
		v.data.objectproxy.object->retain();
	else if (v.type == TABLE)
		v.data.table->retain();

	if (type == STRING)
		data.string->release();
	else if (type == LOVEOBJECT && data.objectproxy.object != nullptr)
		data.objectproxy.object->release();
	else if (type == TABLE)
		data.table->release();

	type = v.type;
	data = v.data;

	return *this;
}

Variant Variant::fromLua(lua_State *L, int n, std::set<const void*> *tableSet)
{
	size_t len;
	const char *str;
	Proxy *p = nullptr;

	if (n < 0) // Fix the stack position, we might modify it later
		n += lua_gettop(L) + 1;

	switch (lua_type(L, n))
	{
	case LUA_TBOOLEAN:
		return Variant(luax_toboolean(L, n));
	case LUA_TNUMBER:
		return Variant(lua_tonumber(L, n));
	case LUA_TSTRING:
		str = lua_tolstring(L, n, &len);
		return Variant(str, len);
	case LUA_TLIGHTUSERDATA:
		return Variant(lua_touserdata(L, n));
	case LUA_TUSERDATA:
		p = tryextractproxy(L, n);
		if (p != nullptr)
			return Variant(p->type, p->object);
		else
		{
			luax_typerror(L, n, "love type");
			return Variant();
		}
	case LUA_TNIL:
		return Variant();
	case LUA_TTABLE:
		{
			bool success = true;
			std::set<const void *> topTableSet;
			std::vector<std::pair<Variant, Variant>> *table = new std::vector<std::pair<Variant, Variant>>();

			// We can use a pointer to a stack-allocated variable because it's
			// never used after the stack-allocated variable is destroyed.
			if (tableSet == nullptr)
				tableSet = &topTableSet;

			// Now make sure this table wasn't already serialised
			const void *tablePointer = lua_topointer(L, n);
			{
				auto result = tableSet->insert(tablePointer);
				if (!result.second) // insertion failed
					throw love::Exception("Cycle detected in table");
			}

			size_t len = luax_objlen(L, -1);
			if (len > 0)
				table->reserve(len);

			lua_pushnil(L);

			while (lua_next(L, n))
			{
				table->emplace_back(fromLua(L, -2, tableSet), fromLua(L, -1, tableSet));
				lua_pop(L, 1);

				const auto &p = table->back();
				if (p.first.getType() == UNKNOWN || p.second.getType() == UNKNOWN)
				{
					success = false;
					break;
				}
			}

			// And remove the table from the set again
			tableSet->erase(tablePointer);

			if (success)
				return Variant(table);
			else
				delete table;
		}
		break;
	}

	Variant v;
	v.type = UNKNOWN;
	return v;
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
	case LOVEOBJECT:
		luax_pushtype(L, *data.objectproxy.type, data.objectproxy.object);
		break;
	case TABLE:
	{
		std::vector<std::pair<Variant, Variant>> *table = data.table->table;
		int tsize = (int) table->size();

		lua_createtable(L, 0, tsize);

		for (int i = 0; i < tsize; ++i)
		{
			std::pair<Variant, Variant> &kv = (*table)[i];
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
