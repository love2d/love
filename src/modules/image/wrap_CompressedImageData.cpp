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

#include "wrap_CompressedImageData.h"
#include "common/wrap_Data.h"

namespace love
{
namespace image
{

CompressedImageData *luax_checkcompressedimagedata(lua_State *L, int idx)
{
	return luax_checktype<CompressedImageData>(L, idx, IMAGE_COMPRESSED_IMAGE_DATA_ID);
}

int w_CompressedImageData_getWidth(lua_State *L)
{
	CompressedImageData *t = luax_checkcompressedimagedata(L, 1);
	int miplevel = (int) luaL_optnumber(L, 2, 1);
	int width = 0;

	luax_catchexcept(L, [&](){ width = t->getWidth(miplevel - 1); });

	lua_pushinteger(L, width);
	return 1;
}

int w_CompressedImageData_getHeight(lua_State *L)
{
	CompressedImageData *t = luax_checkcompressedimagedata(L, 1);
	int miplevel = (int) luaL_optnumber(L, 2, 1);
	int height = 0;

	luax_catchexcept(L, [&](){ height = t->getHeight(miplevel - 1); });

	lua_pushinteger(L, height);
	return 1;
}

int w_CompressedImageData_getDimensions(lua_State *L)
{
	CompressedImageData *t = luax_checkcompressedimagedata(L, 1);
	int miplevel = (int) luaL_optnumber(L, 2, 1);
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

int w_CompressedImageData_getMipmapCount(lua_State *L)
{
	CompressedImageData *t = luax_checkcompressedimagedata(L, 1);
	lua_pushinteger(L, t->getMipmapCount());
	return 1;
}

int w_CompressedImageData_getFormat(lua_State *L)
{
	CompressedImageData *t = luax_checkcompressedimagedata(L, 1);

	image::CompressedImageData::Format format = t->getFormat();
	const char *str;

	if (image::CompressedImageData::getConstant(format, str))
		lua_pushstring(L, str);
	else
		lua_pushstring(L, "unknown");

	return 1;
}

static const luaL_Reg w_CompressedImageData_functions[] =
{
	{ "getWidth", w_CompressedImageData_getWidth },
	{ "getHeight", w_CompressedImageData_getHeight },
	{ "getDimensions", w_CompressedImageData_getDimensions },
	{ "getMipmapCount", w_CompressedImageData_getMipmapCount },
	{ "getFormat", w_CompressedImageData_getFormat },
	{ 0, 0 },
};

extern "C" int luaopen_compressedimagedata(lua_State *L)
{
	return luax_register_type(L, IMAGE_COMPRESSED_IMAGE_DATA_ID, "CompressedImageData", w_Data_functions, w_CompressedImageData_functions, nullptr);
}

} // image
} // love
