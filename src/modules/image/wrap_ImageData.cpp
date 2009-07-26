/**
* Copyright (c) 2006-2009 LOVE Development Team
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

#include "wrap_ImageData.h"

#include <common/wrap_Data.h>

namespace love
{
namespace image
{
	ImageData * luax_checkimagedata(lua_State * L, int idx)
	{
		return luax_checktype<ImageData>(L, idx, "ImageData", LOVE_IMAGE_IMAGE_DATA_BITS);
	}

	int _wrap_ImageData_getWidth(lua_State * L)
	{
		ImageData * t = luax_checkimagedata(L, 1);
		lua_pushinteger(L, t->getWidth());
		return 1;
	}

	int _wrap_ImageData_getHeight(lua_State * L)
	{
		ImageData * t = luax_checkimagedata(L, 1);
		lua_pushinteger(L, t->getHeight());
		return 1;
	}

	int _wrap_ImageData_getPixel(lua_State * L)
	{
		ImageData * t = luax_checkimagedata(L, 1);
		int x = luaL_checkint(L, 2);
		int y = luaL_checkint(L, 3);
		pixel c = t->getPixel(x, y);
		lua_pushnumber(L, c.r);
		lua_pushnumber(L, c.g);
		lua_pushnumber(L, c.b);
		lua_pushnumber(L, c.a);
		return 4;
	}

	int _wrap_ImageData_setPixel(lua_State * L)
	{
		ImageData * t = luax_checkimagedata(L, 1);
		int x = luaL_checkint(L, 2);
		int y = luaL_checkint(L, 3);
		pixel c;
		c.r = luaL_checkint(L, 4);
		c.g = luaL_checkint(L, 5);
		c.b = luaL_checkint(L, 6);
		c.a = luaL_checkint(L, 7);
		t->setPixel(x, y, c);
		return 0;
	}

	int _wrap_ImageData_mapPixel(lua_State * L)
	{
		ImageData * t = luax_checkimagedata(L, 1);

		if(!lua_isfunction(L, 2))
			return luaL_error(L, "Function expected");
		
		int w = t->getWidth();
		int h = t->getHeight();

		for(int i = 0; i < w; i++)
		{
			for(int j = 0; j < h; j++)
			{
				lua_pushvalue(L, 2);
				lua_pushnumber(L, i);
				lua_pushnumber(L, j);
				pixel c = t->getPixel(i, j);
				lua_pushnumber(L, c.r);
				lua_pushnumber(L, c.g);
				lua_pushnumber(L, c.b);
				lua_pushnumber(L, c.a);
				lua_call(L, 6, 4);
				c.r = luaL_optint(L, -4, 0);
				c.g = luaL_optint(L, -3, 0);
				c.b = luaL_optint(L, -2, 0);
				c.a = luaL_optint(L, -1, 0);
				t->setPixel(i, j, c);
				lua_pop(L, 4);
			}
		}
		return 0;
	}

	int _wrap_ImageData_getString(lua_State * L)
	{
		ImageData * t = luax_checkimagedata(L, 1);
		lua_pushlstring(L, (const char *)t->getData(), t->getSize());
		return 1;
	}

	int _wrap_ImageData_paste(lua_State * L)
	{
		ImageData * t = luax_checkimagedata(L, 1);
		ImageData * src = luax_checkimagedata(L, 2);
		int dx = luaL_checkint(L, 3);
		int dy = luaL_checkint(L, 4);
		int sx = luaL_optint(L, 5, 0);
		int sy = luaL_optint(L, 6, 0);
		int sw = luaL_optint(L, 7, src->getWidth());
		int sh = luaL_optint(L, 8, src->getHeight());
		t->paste((love::image::ImageData *)src, dx, dy, sx, sy, sw, sh);
		return 0;
	}

	static const luaL_Reg wrap_ImageData_functions[] = {

		// Data
		{ "getPointer", _wrap_Data_getPointer },
		{ "getSize", _wrap_Data_getSize },

		{ "getWidth", _wrap_ImageData_getWidth },
		{ "getHeight", _wrap_ImageData_getHeight },
		{ "getPixel", _wrap_ImageData_getPixel },
		{ "setPixel", _wrap_ImageData_setPixel },
		{ "mapPixel", _wrap_ImageData_mapPixel },
		{ "getString", _wrap_ImageData_getString },
		{ "paste", _wrap_ImageData_paste },
		{ 0, 0 }
	};

	int wrap_ImageData_open(lua_State * L)
	{
		luax_register_type(L, "ImageData", wrap_ImageData_functions);
		return 0;
	}

} // image
} // love
