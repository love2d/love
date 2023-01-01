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

#include "wrap_Data.h"

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
	lua_pushlstring(L, (const char *) t->getData(), t->getSize());
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
