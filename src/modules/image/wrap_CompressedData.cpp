/**
 * Copyright (c) 2006-2015 LOVE Development Team
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

#include "wrap_CompressedData.h"
#include "common/wrap_Data.h"

namespace love
{
namespace image
{

CompressedData *luax_checkcompresseddata(lua_State *L, int idx)
{
	return luax_checktype<CompressedData>(L, idx, "CompressedData", IMAGE_COMPRESSED_DATA_T);
}

int w_CompressedData_getWidth(lua_State *L)
{
	CompressedData *t = luax_checkcompresseddata(L, 1);
	int miplevel = luaL_optint(L, 2, 1);
	int width = 0;

	luax_catchexcept(L, [&](){ width = t->getWidth(miplevel - 1); });

	lua_pushinteger(L, width);
	return 1;
}

int w_CompressedData_getHeight(lua_State *L)
{
	CompressedData *t = luax_checkcompresseddata(L, 1);
	int miplevel = luaL_optint(L, 2, 1);
	int height = 0;

	luax_catchexcept(L, [&](){ height = t->getHeight(miplevel - 1); });

	lua_pushinteger(L, height);
	return 1;
}

int w_CompressedData_getDimensions(lua_State *L)
{
	CompressedData *t = luax_checkcompresseddata(L, 1);
	int miplevel = luaL_optint(L, 2, 1);
	int width = 0, height = 0;

	luax_catchexcept(L, [&]()
	{
		width = t->getWidth(miplevel - 1);
		height = t->getHeight(miplevel - 1);
	});

	lua_pushinteger(L, width);
	lua_pushinteger(L, height);
	return 2;
}

int w_CompressedData_getMipmapCount(lua_State *L)
{
	CompressedData *t = luax_checkcompresseddata(L, 1);
	lua_pushinteger(L, t->getMipmapCount());
	return 1;
}

int w_CompressedData_getFormat(lua_State *L)
{
	CompressedData *t = luax_checkcompresseddata(L, 1);

	image::CompressedData::Format format = t->getFormat();
	const char *str;

	if (image::CompressedData::getConstant(format, str))
		lua_pushstring(L, str);
	else
		lua_pushstring(L, "unknown");

	return 1;
}

static const luaL_Reg functions[] =
{
	// Data
	{ "getString", w_Data_getString },
	{ "getPointer", w_Data_getPointer },
	{ "getSize", w_Data_getSize },

	{ "getWidth", w_CompressedData_getWidth },
	{ "getHeight", w_CompressedData_getHeight },
	{ "getDimensions", w_CompressedData_getDimensions },
	{ "getMipmapCount", w_CompressedData_getMipmapCount },
	{ "getFormat", w_CompressedData_getFormat },
	{ 0, 0 },
};

extern "C" int luaopen_compresseddata(lua_State *L)
{
	return luax_register_type(L, "CompressedData", functions);
}

} // image
} // love
