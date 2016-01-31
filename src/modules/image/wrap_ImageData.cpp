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

#include "wrap_ImageData.h"

#include "common/wrap_Data.h"
#include "filesystem/File.h"

// Shove the wrap_ImageData.lua code directly into a raw string literal.
static const char imagedata_lua[] =
#include "wrap_ImageData.lua"
;

namespace love
{
namespace image
{

/**
 * NOTE: Additional wrapper code is in wrap_ImageData.lua. Be sure to keep it
 * in sync with any changes made to this file!
 **/

ImageData *luax_checkimagedata(lua_State *L, int idx)
{
	return luax_checktype<ImageData>(L, idx, IMAGE_IMAGE_DATA_ID);
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
	int x = (int) luaL_checknumber(L, 2);
	int y = (int) luaL_checknumber(L, 3);
	pixel c;

	luax_catchexcept(L, [&](){ c = t->getPixel(x, y); });

	lua_pushnumber(L, c.r);
	lua_pushnumber(L, c.g);
	lua_pushnumber(L, c.b);
	lua_pushnumber(L, c.a);
	return 4;
}

int w_ImageData_setPixel(lua_State *L)
{
	ImageData *t = luax_checkimagedata(L, 1);
	int x = (int) luaL_checknumber(L, 2);
	int y = (int) luaL_checknumber(L, 3);
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

	luax_catchexcept(L, [&](){ t->setPixel(x, y, c); });
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

// ImageData:mapPixel. Not thread-safe! See wrap_ImageData.lua for the thread-
// safe wrapper function.
int w_ImageData__mapPixelUnsafe(lua_State *L)
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

				if (ttype == LUA_TNUMBER)
					parray[i] = (unsigned char) lua_tonumber(L, -4 + i);
				else if (i == 3 && (ttype == LUA_TNONE || ttype == LUA_TNIL))
					parray[i] = 255; // Alpha component defaults to 255.
				else
					// Error (level 2 because this is function will be wrapped.)
					return luax_retnumbererror(L, 2, i + 1, ttype);
			}

			// Pop return values.
			lua_pop(L, 4);

			// We're locking the entire function, instead of each setPixel call.
			t->setPixelUnsafe(x, y, c);
		}
	}

	return 0;
}

int w_ImageData_paste(lua_State *L)
{
	ImageData *t = luax_checkimagedata(L, 1);
	ImageData *src = luax_checkimagedata(L, 2);
	int dx = (int) luaL_checknumber(L, 3);
	int dy = (int) luaL_checknumber(L, 4);
	int sx = (int) luaL_optnumber(L, 5, 0);
	int sy = (int) luaL_optnumber(L, 6, 0);
	int sw = (int) luaL_optnumber(L, 7, src->getWidth());
	int sh = (int) luaL_optnumber(L, 8, src->getHeight());
	t->paste((love::image::ImageData *)src, dx, dy, sx, sy, sw, sh);
	return 0;
}

int w_ImageData_encode(lua_State *L)
{
	ImageData *t = luax_checkimagedata(L, 1);

	ImageData::EncodedFormat format;
	const char *fmt = luaL_checkstring(L, 2);
	if (!ImageData::getConstant(fmt, format))
		return luaL_error(L, "Invalid encoded image format '%s'.", fmt);

	bool hasfilename = false;

	std::string filename = "Image." + std::string(fmt);
	if (!lua_isnoneornil(L, 3))
	{
		hasfilename = true;
		filename = luax_checkstring(L, 3);
	}

	love::filesystem::FileData *filedata = nullptr;
	luax_catchexcept(L, [&](){ filedata = t->encode(format, filename.c_str()); });

	luax_pushtype(L, FILESYSTEM_FILE_DATA_ID, filedata);
	filedata->release();

	if (hasfilename)
	{
		luax_getfunction(L, "filesystem", "write");
		lua_pushvalue(L, 3); // filename
		lua_pushvalue(L, -3); // FileData
		lua_call(L, 2, 0);
	}

	return 1;
}

int w_ImageData__performAtomic(lua_State *L)
{
	ImageData *t = luax_checkimagedata(L, 1);
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
	// leaving only the ImageData object. Everything else is a return value.
	return lua_gettop(L) - 1;
}

// C functions in a struct, necessary for the FFI versions of ImageData methods.
struct FFI_ImageData
{
	void (*lockMutex)(Proxy *p);
	void (*unlockMutex)(Proxy *p);
};

static FFI_ImageData ffifuncs =
{
	[](Proxy *p) // lockMutex
	{
		// We don't do any type-checking for the Proxy here since these functions
		// are always called from code which has already done type checking.
		ImageData *i = (ImageData *) p->object;
		i->getMutex()->lock();
	},

	[](Proxy *p) // unlockMutex
	{
		ImageData *i = (ImageData *) p->object;
		i->getMutex()->unlock();
	}
};

static const luaL_Reg w_ImageData_functions[] =
{
	{ "getWidth", w_ImageData_getWidth },
	{ "getHeight", w_ImageData_getHeight },
	{ "getDimensions", w_ImageData_getDimensions },
	{ "getPixel", w_ImageData_getPixel },
	{ "setPixel", w_ImageData_setPixel },
	{ "paste", w_ImageData_paste },
	{ "encode", w_ImageData_encode },

	// Used in the Lua wrapper code.
	{ "_mapPixelUnsafe", w_ImageData__mapPixelUnsafe },
	{ "_performAtomic", w_ImageData__performAtomic },

	{ 0, 0 }
};

extern "C" int luaopen_imagedata(lua_State *L)
{
	int ret = luax_register_type(L, IMAGE_IMAGE_DATA_ID, "ImageData", w_Data_functions, w_ImageData_functions, nullptr);

	luax_gettypemetatable(L, IMAGE_IMAGE_DATA_ID);

	// Load and execute ImageData.lua, sending the metatable and the ffi
	// functions struct pointer as arguments.
	if (lua_istable(L, -1))
	{
		luaL_loadbuffer(L, imagedata_lua, sizeof(imagedata_lua), "ImageData.lua");
		lua_pushvalue(L, -2);
		lua_pushlightuserdata(L, &ffifuncs);
		lua_call(L, 2, 0);
	}

	// Pop the metatable.
	lua_pop(L, 1);

	return ret;
}

} // image
} // love
