/**
 * Copyright (c) 2006-2013 LOVE Development Team
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

#include "common/wrap_Data.h"
#include "filesystem/File.h"

namespace love
{
namespace image
{

ImageData *luax_checkimagedata(lua_State *L, int idx)
{
	return luax_checktype<ImageData>(L, idx, "ImageData", IMAGE_IMAGE_DATA_T);
}

int w_ImageData_getWidth(lua_State *L)
{
	ImageData *t = luax_checkimagedata(L, 1);
	lua_pushinteger(L, t->getWidth());
	return 1;
}

int w_ImageData_getHeight(lua_State *L)
{
	ImageData *t = luax_checkimagedata(L, 1);
	lua_pushinteger(L, t->getHeight());
	return 1;
}

int w_ImageData_getDimensions(lua_State *L)
{
	ImageData *t = luax_checkimagedata(L, 1);
	lua_pushinteger(L, t->getWidth());
	lua_pushinteger(L, t->getHeight());
	return 2;
}

int w_ImageData_getPixel(lua_State *L)
{
	ImageData *t = luax_checkimagedata(L, 1);
	int x = luaL_checkint(L, 2);
	int y = luaL_checkint(L, 3);
	pixel c;
	try
	{
		c = t->getPixel(x, y);
	}
	catch(love::Exception &e)
	{
		return luaL_error(L, "%s", e.what());
	}
	lua_pushnumber(L, c.r);
	lua_pushnumber(L, c.g);
	lua_pushnumber(L, c.b);
	lua_pushnumber(L, c.a);
	return 4;
}

int w_ImageData_setPixel(lua_State *L)
{
	ImageData *t = luax_checkimagedata(L, 1);
	int x = luaL_checkint(L, 2);
	int y = luaL_checkint(L, 3);
	pixel c;

	if (lua_istable(L, 4))
	{
		for (int i = 1; i <= 4; i++)
			lua_rawgeti(L, 4, i);

		c.r = (unsigned char)luaL_checkinteger(L, -4);
		c.g = (unsigned char)luaL_checkinteger(L, -3);
		c.b = (unsigned char)luaL_checkinteger(L, -2);
		c.a = (unsigned char)luaL_optinteger(L, -1, 255);

		lua_pop(L, 4);
	}
	else
	{
		c.r = (unsigned char)luaL_checkinteger(L, 4);
		c.g = (unsigned char)luaL_checkinteger(L, 5);
		c.b = (unsigned char)luaL_checkinteger(L, 6);
		c.a = (unsigned char)luaL_optinteger(L, 7, 255);
	}

	try
	{
		t->setPixel(x, y, c);
	}
	catch(love::Exception &e)
	{
		return luaL_error(L, "%s", e.what());
	}
	return 0;
}

// Gets the result of luaL_where as a string.
static std::string luax_getwhere(lua_State *L, int level)
{
	luaL_where(L, level);

	const char *str = lua_tostring(L, -1);
	std::string where;
	if (str)
		where = str;

	lua_pop(L, 1);
	return where;
}

// Generates a Lua error with a nice error string when a return value of a
// called function is not a number.
static int luax_retnumbererror(lua_State *L, int level, int retnum, int ttype)
{
	if (ttype == LUA_TNUMBER)
		return 0;

	const char *where = luax_getwhere(L, level).c_str();
	const char *ttypename = lua_typename(L, ttype);

	return luaL_error(L, "%sbad return value #%d (number expected, got %s)",
	                     where, retnum, ttypename);
}

// ImageData:mapPixel. Not thread-safe! See the wrapper function below.
static int w_ImageData_mapPixelUnsafe(lua_State *L)
{
	ImageData *t = luax_checkimagedata(L, 1);
	luaL_checktype(L, 2, LUA_TFUNCTION);

	// No optints because we assume they're done in the wrapper function.
	int sx = (int) lua_tonumber(L, 3);
	int sy = (int) lua_tonumber(L, 4);
	int w  = (int) lua_tonumber(L, 5);
	int h  = (int) lua_tonumber(L, 6);

	if (!(t->inside(sx, sy) && t->inside(sx+w-1, sy+h-1)))
		return luaL_error(L, "Invalid rectangle dimensions.");

	// Default pixel component values (r, g, b, a.)
	const unsigned char pixel_defaults[4] = {0, 0, 0, 255};

	// Cache-friendlier loop. :)
	for (int y = sy; y < sy+h; y++)
	{
		for (int x = sx; x < sx+w; x++)
		{
			lua_pushvalue(L, 2);
			lua_pushnumber(L, x);
			lua_pushnumber(L, y);
			pixel c = t->getPixel(x, y);
			lua_pushnumber(L, c.r);
			lua_pushnumber(L, c.g);
			lua_pushnumber(L, c.b);
			lua_pushnumber(L, c.a);
			lua_call(L, 6, 4);

			// If we used luaL_checkX / luaL_optX then we would get messy error
			// messages (e.g. Error: bad argument #-1 to '?'), so while this is
			// messier code, at least the errors are a bit more descriptive.

			// Treat the pixel as an array for less code duplication. :(
			unsigned char *parray = (unsigned char *) &c;
			for (int i = 0; i < 4; i++)
			{
				int ttype = lua_type(L, -4 + i);
				switch (ttype)
				{
				case LUA_TNUMBER:
					parray[i] = (unsigned char) lua_tonumber(L, -4 + i);
					break;
				case LUA_TNONE:
				case LUA_TNIL:
					parray[i] = pixel_defaults[i];
					break;
						
				default:
					// Level 2 because this is function will be wrapped.
					return luax_retnumbererror(L, 2, i + 1, ttype);
				}
			}

			lua_pop(L, 4);

			// We're locking the entire function, instead of each setPixel call.
			t->setPixelUnsafe(x, y, c);
		}
	}
	return 0;
}

// Thread-safe wrapper for the above function.
int w_ImageData_mapPixel(lua_State *L)
{
	ImageData *t = luax_checkimagedata(L, 1);
	luaL_checktype(L, 2, LUA_TFUNCTION);
	int sx = luaL_optint(L, 3, 0);
	int sy = luaL_optint(L, 4, 0);
	int w = luaL_optint(L, 5, t->getWidth());
	int h = luaL_optint(L, 6, t->getHeight());

	lua_pushcfunction(L, w_ImageData_mapPixelUnsafe);
	lua_pushvalue(L, 1);
	lua_pushvalue(L, 2);
	lua_pushinteger(L, sx);
	lua_pushinteger(L, sy);
	lua_pushinteger(L, w);
	lua_pushinteger(L, h);

	int ret = 0;

	// Lock this ImageData's mutex during the entire mapPixel. We pcall instead
	// of call because lua_error longjmp's without calling object destructors.
	{
		love::thread::Lock lock(t->getMutex());
		ret = lua_pcall(L, 6, 0, 0);
	}

	if (ret != 0)
		return lua_error(L);

	return 0;
}

int w_ImageData_paste(lua_State *L)
{
	ImageData *t = luax_checkimagedata(L, 1);
	ImageData *src = luax_checkimagedata(L, 2);
	int dx = luaL_checkint(L, 3);
	int dy = luaL_checkint(L, 4);
	int sx = luaL_optint(L, 5, 0);
	int sy = luaL_optint(L, 6, 0);
	int sw = luaL_optint(L, 7, src->getWidth());
	int sh = luaL_optint(L, 8, src->getHeight());
	t->paste((love::image::ImageData *)src, dx, dy, sx, sy, sw, sh);
	return 0;
}

int w_ImageData_encode(lua_State *L)
{
	std::string ext;
	const char *fmt;
	ImageData::Format format = ImageData::FORMAT_MAX_ENUM;
	ImageData *t = luax_checkimagedata(L, 1);

	if (lua_isstring(L, 2))
		luax_convobj(L, 2, "filesystem", "newFile");
	love::filesystem::File *file = luax_checktype<love::filesystem::File>(L, 2, "File", FILESYSTEM_FILE_T);

	if (lua_isnoneornil(L, 3))
	{
		ext = file->getExtension();
		fmt = ext.c_str();
		if (!ImageData::getConstant(fmt, format))
			return luaL_error(L, "Invalid image format '%s'.", fmt);
	}
	else
	{
		fmt = luaL_checkstring(L, 3);
		if (!ImageData::getConstant(fmt, format))
			return luaL_error(L, "Invalid image format '%s'.", fmt);
	}

	try
	{
		t->encode(file, format);
	}
	catch(love::Exception &e)
	{
		return luaL_error(L, e.what());
	}
	return 0;
}

static const luaL_Reg functions[] =
{
	// Data
	{ "getString", w_Data_getString },
	{ "getSize", w_Data_getSize },

	{ "getWidth", w_ImageData_getWidth },
	{ "getHeight", w_ImageData_getHeight },
	{ "getDimensions", w_ImageData_getDimensions },
	{ "getPixel", w_ImageData_getPixel },
	{ "setPixel", w_ImageData_setPixel },
	{ "mapPixel", w_ImageData_mapPixel },
	{ "paste", w_ImageData_paste },
	{ "encode", w_ImageData_encode },
	{ 0, 0 }
};

extern "C" int luaopen_imagedata(lua_State *L)
{
	return luax_register_type(L, "ImageData", functions);
}

} // image
} // love
