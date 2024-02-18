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

#include "wrap_Data.h"
#include "common/int.h"
#include "thread/threads.h"

// Put the Lua code directly into a raw string literal.
static const char data_lua[] =
#include "wrap_Data.lua"
;

namespace love
{
namespace data
{

Data *luax_checkdata(lua_State *L, int idx)
{
	return luax_checktype<Data>(L, idx);
}

int w_Data_getString(lua_State *L)
{
	Data *t = luax_checkdata(L, 1);
	int64 offset = (int64) luaL_optnumber(L, 2, 0);

	int64 size = lua_isnoneornil(L, 3)
		? ((int64) t->getSize() - offset)
		: (int64) luaL_checknumber(L, 3);

	if (size <= 0)
		return luaL_error(L, "Invalid size parameter (must be greater than 0)");

	if (offset < 0 || offset + size > (int64) t->getSize())
		return luaL_error(L, "The given offset and size parameters don't fit within the Data's size.");

	auto data = (const char *) t->getData() + offset;

	lua_pushlstring(L, data, size);
	return 1;
}

int w_Data_getPointer(lua_State *L)
{
	Data *t = luax_checkdata(L, 1);
	lua_pushlightuserdata(L, t->getData());
	return 1;
}

// Placeholder, overridden by the FFI code when the FFI is available.
int w_Data_getFFIPointer(lua_State *L)
{
	lua_pushnil(L);
	return 1;
}

int w_Data_getSize(lua_State *L)
{
	Data *t = luax_checkdata(L, 1);
	lua_pushnumber(L, (lua_Number) t->getSize());
	return 1;
}

int w_Data_performAtomic(lua_State *L)
{
	Data *t = luax_checkdata(L, 1);
	int err = 0;

	{
		love::thread::Lock lock(t->getMutex());
		// call the function, passing any user-specified arguments.
		err = lua_pcall(L, lua_gettop(L) - 2, LUA_MULTRET, 0);
	}

	// Unfortunately, this eats the stack trace, too bad.
	if (err != 0)
		return lua_error(L);

	// The function and everything after it in the stack are eaten by the pcall,
	// leaving only the Data object. Everything else is a return value.
	return lua_gettop(L) - 1;
}

template <typename T>
static int w_Data_getT(lua_State* L)
{
	Data* t = luax_checkdata(L, 1);
	int64 offset = (int64)luaL_checknumber(L, 2);
	int count = (int)luaL_optinteger(L, 3, 1);

	if (count <= 0)
		return luaL_error(L, "Invalid count parameter (must be greater than 0)");

	if (offset < 0 || offset + sizeof(T) * count > t->getSize())
		return luaL_error(L, "The given offset and count parameters don't fit within the Data's size.");

	auto data = (const T*)((uint8*)t->getData() + offset);

	for (int i = 0; i < count; i++)
		lua_pushnumber(L, (lua_Number)data[i]);

	return count;
}

int w_Data_getFloat(lua_State* L)
{
	return w_Data_getT<float>(L);
}

int w_Data_getDouble(lua_State* L)
{
	return w_Data_getT<double>(L);
}

int w_Data_getInt8(lua_State* L)
{
	return w_Data_getT<int8>(L);
}

int w_Data_getUInt8(lua_State* L)
{
	return w_Data_getT<uint8>(L);
}

int w_Data_getInt16(lua_State* L)
{
	return w_Data_getT<int16>(L);
}

int w_Data_getUInt16(lua_State* L)
{
	return w_Data_getT<uint16>(L);
}

int w_Data_getInt32(lua_State* L)
{
	return w_Data_getT<int32>(L);
}

int w_Data_getUInt32(lua_State* L)
{
	return w_Data_getT<uint32>(L);
}

// C functions in a struct, necessary for the FFI versions of Data methods.
struct FFI_Data
{
	void *(*getFFIPointer)(Proxy *p);
};

static FFI_Data ffifuncs =
{
	[](Proxy *p) -> void * // getFFIPointer
	{
		auto data = luax_ffi_checktype<Data>(p);
		return data != nullptr ? data->getData() : nullptr;
	}
};

const luaL_Reg w_Data_functions[] =
{
	{ "getString", w_Data_getString },
	{ "getPointer", w_Data_getPointer },
	{ "getFFIPointer", w_Data_getFFIPointer },
	{ "getSize", w_Data_getSize },
	{ "performAtomic", w_Data_performAtomic },
	{ "getFloat", w_Data_getFloat },
	{ "getDouble", w_Data_getDouble },
	{ "getInt8", w_Data_getInt8 },
	{ "getUInt8", w_Data_getUInt8 },
	{ "getInt16", w_Data_getInt16 },
	{ "getUInt16", w_Data_getUInt16 },
	{ "getInt32", w_Data_getInt32 },
	{ "getUInt32", w_Data_getUInt32 },
	{ 0, 0 }
};

void luax_rundatawrapper(lua_State *L, const love::Type &type)
{
	luax_runwrapper(L, data_lua, sizeof(data_lua), "Data.lua", type, &ffifuncs);
}

int luaopen_data(lua_State *L)
{
	int n = luax_register_type(L, &Data::type, w_Data_functions, nullptr);
	luax_rundatawrapper(L, Data::type);
	return n;
}

} // data
} // love
