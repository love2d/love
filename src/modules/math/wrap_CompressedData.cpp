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

// LOVE
#include "wrap_CompressedData.h"
#include "common/wrap_Data.h"

namespace love
{
namespace math
{

CompressedData *luax_checkcompresseddata(lua_State *L, int idx)
{
	return luax_checktype<CompressedData>(L, idx, MATH_COMPRESSED_DATA_ID);
}

int w_CompressedData_getFormat(lua_State *L)
{
	CompressedData *t = luax_checkcompresseddata(L, 1);

	const char *fname = nullptr;
	if (!Compressor::getConstant(t->getFormat(), fname))
		return luaL_error(L, "Unknown compressed data format.");

	lua_pushstring(L, fname);
	return 1;
}

static const luaL_Reg w_CompressedData_functions[] =
{
	{ "getFormat", w_CompressedData_getFormat },
	{ 0, 0 },
};


extern "C" int luaopen_compresseddata(lua_State *L)
{
	return luax_register_type(L, MATH_COMPRESSED_DATA_ID, "CompressedData", w_Data_functions, w_CompressedData_functions, nullptr);
}

} // math
} // love
