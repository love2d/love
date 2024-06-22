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

#include "wrap_ByteData.h"
#include "wrap_Data.h"
#include "common/config.h"

#include <algorithm>

namespace love
{
namespace data
{

ByteData *luax_checkbytedata(lua_State *L, int idx)
{
	return luax_checktype<ByteData>(L, idx);
}

int w_ByteData_clone(lua_State *L)
{
	ByteData *t = luax_checkbytedata(L, 1);
	ByteData *c = nullptr;
	luax_catchexcept(L, [&](){ c = t->clone(); });
	luax_pushtype(L, c);
	c->release();
	return 1;
}

int w_ByteData_setString(lua_State *L)
{
	Data *t = luax_checkdata(L, 1);
	size_t size = 0;
	const char *str = luaL_checklstring(L, 2, &size);
	int64 offset = (int64)luaL_optnumber(L, 3, 0);

	size = std::min(size, t->getSize());

	if (size == 0)
		return 0;

	if (offset < 0 || offset + (int64) size > (int64) t->getSize())
		return luaL_error(L, "The given string offset and size don't fit within the Data's size.");

	memcpy((char *) t->getData() + (size_t) offset, str, size);
	return 0;
}

template <typename T>
static int w_ByteData_setT(lua_State *L)
{
	ByteData *t = luax_checkbytedata(L, 1);
	int64 offset = (int64) luaL_checknumber(L, 2);

	bool istable = lua_type(L, 3) == LUA_TTABLE;
	int nargs = std::max(1, istable ? (int) luax_objlen(L, 3) : lua_gettop(L) - 2);

	if (offset < 0 || offset + sizeof(T) * nargs > t->getSize())
		return luaL_error(L, "The given offset and value parameters don't fit within the Data's size.");

	auto data = (T *)((uint8 *) t->getData() + offset);

	if (istable)
	{
		for (int i = 0; i < nargs; i++)
		{
			lua_rawgeti(L, 3, i + 1);
			data[i] = (T) luaL_checknumber(L, -1);
			lua_pop(L, 1);
		}
	}
	else
	{
		for (int i = 0; i < nargs; i++)
			data[i] = (T) luaL_checknumber(L, 3 + i);
	}

	return 0;
}

int w_ByteData_setFloat(lua_State *L)
{
	return w_ByteData_setT<float>(L);
}

int w_ByteData_setDouble(lua_State *L)
{
	return w_ByteData_setT<double>(L);
}

int w_ByteData_setInt8(lua_State *L)
{
	return w_ByteData_setT<int8>(L);
}

int w_ByteData_setUInt8(lua_State *L)
{
	return w_ByteData_setT<uint8>(L);
}

int w_ByteData_setInt16(lua_State *L)
{
	return w_ByteData_setT<int16>(L);
}

int w_ByteData_setUInt16(lua_State *L)
{
	return w_ByteData_setT<uint16>(L);
}

int w_ByteData_setInt32(lua_State *L)
{
	return w_ByteData_setT<int32>(L);
}

int w_ByteData_setUInt32(lua_State *L)
{
	return w_ByteData_setT<uint32>(L);
}

static const luaL_Reg w_ByteData_functions[] =
{
	{ "clone", w_ByteData_clone },
	{ "setString", w_ByteData_setString },
	{ "setFloat", w_ByteData_setFloat },
	{ "setDouble", w_ByteData_setDouble },
	{ "setInt8", w_ByteData_setInt8 },
	{ "setUInt8", w_ByteData_setUInt8 },
	{ "setInt16", w_ByteData_setInt16 },
	{ "setUInt16", w_ByteData_setUInt16 },
	{ "setInt32", w_ByteData_setInt32 },
	{ "setUInt32", w_ByteData_setUInt32 },
	{ 0, 0 }
};

int luaopen_bytedata(lua_State *L)
{
	luax_register_type(L, &ByteData::type, w_Data_functions, w_ByteData_functions, nullptr);
	love::data::luax_rundatawrapper(L, ByteData::type);
	return 0;
}

} // data
} // love
