/**
 * Copyright (c) 2006-2022 LOVE Development Team
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

#include "common/config.h"

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

template <class Int> void getIntArray(lua_State *L, size_t size, void *data, bool swap)
{
	Int *idata = (Int *) data;
	size_t length = size / sizeof(Int);
	lua_createtable(L, (int) length, 0);
	if (swap)
	{
		for (size_t i = 1; i <= length; ++i)
		{
			Int v = *idata;
			char *c = (char *) &v;
			for (size_t b = 0; b < (sizeof(Int) / 2); ++b)
				std::swap(c[b], c[sizeof(Int) - 1 - b]);
			lua_pushinteger(L, i);
			lua_pushinteger(L, (lua_Integer) v);
			lua_settable(L, -3);
			++idata;
		}
	}
	else
	{
		for (size_t i = 1; i <= length; ++i)
		{
			Int v = *idata;
			lua_pushinteger(L, i);
			lua_pushinteger(L, (lua_Integer) v);
			lua_settable(L, -3);
			++idata;
		}
	}
}

#ifdef LOVE_BIG_ENDIAN
const char* MY_ENDIAN = "b";
#else
const char* MY_ENDIAN = "l";
#endif

int w_Data_getIntArray(lua_State *L)
{
	Data *t = luax_checkdata(L, 1);
	const char *issigned = luaL_optstring(L, 2, "s");
	int width = luaL_optint(L, 3, 4);

	const char *endian = luaL_optstring(L, 4, MY_ENDIAN);
	bool swap = width > 1 && tolower(*endian) != *MY_ENDIAN;

	if (*issigned == 'u')
	{
		switch (width)
		{
		case 1:
			getIntArray<uint8_t>(L, t->getSize(), t->getData(), swap);
			break;
		case 2:
			getIntArray<uint16_t>(L, t->getSize(), t->getData(), swap);
			break;
		case 4:
			getIntArray<uint32_t>(L, t->getSize(), t->getData(), swap);
			break;
		case 8:
			getIntArray<uint64_t>(L, t->getSize(), t->getData(), swap);
			break;
		default:
			throw love::Exception("Invalid integer size. Must be 1, 2, 4, or 8.");
		}
	}
	else
	{
		switch (width)
		{
		case 1:
			getIntArray<int8_t>(L, t->getSize(), t->getData(), swap);
			break;
		case 2:
			getIntArray<int16_t>(L, t->getSize(), t->getData(), swap);
			break;
		case 4:
			getIntArray<int32_t>(L, t->getSize(), t->getData(), swap);
			break;
		case 8:
			getIntArray<int64_t>(L, t->getSize(), t->getData(), swap);
			break;
		default:
			throw love::Exception("Invalid integer size. Must be 1, 2, 4, or 8.");
		}
	}
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
	{ "getIntArray", w_Data_getIntArray },
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
