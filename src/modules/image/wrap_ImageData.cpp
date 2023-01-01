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

#include "wrap_ImageData.h"

#include "data/wrap_Data.h"
#include "filesystem/File.h"
#include "filesystem/Filesystem.h"

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
	return luax_checktype<ImageData>(L, idx);
}

int w_ImageData_clone(lua_State *L)
{
	ImageData *t = luax_checkimagedata(L, 1), *c = nullptr;
	luax_catchexcept(L, [&](){ c = t->clone(); });
	luax_pushtype(L, c);
	c->release();
	return 1;
}

int w_ImageData_getFormat(lua_State *L)
{
	ImageData *t = luax_checkimagedata(L, 1);
	PixelFormat format = t->getFormat();
	const char *fstr = nullptr;

	if (!getConstant(format, fstr))
		return luaL_error(L, "Unknown pixel format.");

	lua_pushstring(L, fstr);
	return 1;
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
	int x = (int) luaL_checkinteger(L, 2);
	int y = (int) luaL_checkinteger(L, 3);

	Colorf c;
	luax_catchexcept(L, [&](){ t->getPixel(x, y, c); });

	lua_pushnumber(L, c.r);
	lua_pushnumber(L, c.g);
	lua_pushnumber(L, c.b);
	lua_pushnumber(L, c.a);
	return 4;
}

int w_ImageData_setPixel(lua_State *L)
{
	ImageData *t = luax_checkimagedata(L, 1);
	int x = (int) luaL_checkinteger(L, 2);
	int y = (int) luaL_checkinteger(L, 3);

	int components = getPixelFormatColorComponents(t->getFormat());

	Colorf c;

	if (lua_istable(L, 4))
	{
		for (int i = 1; i <= components; i++)
			lua_rawgeti(L, components, i);

		c.r = (float) luaL_checknumber(L, -components);
		if (components > 1)
			c.g = (float) luaL_checknumber(L, (-components) + 1);
		if (components > 2)
			c.b = (float) luaL_checknumber(L, (-components) + 2);
		if (components > 3)
			c.a = (float) luaL_optnumber(L, (-components) + 3, 1.0);

		lua_pop(L, components);
	}
	else
	{
		c.r = (float) luaL_checknumber(L, 4);
		if (components > 1)
			c.g = (float) luaL_checknumber(L, 5);
		if (components > 2)
			c.b = (float) luaL_checknumber(L, 6);
		if (components > 3)
			c.a = (float) luaL_optnumber(L, 7, 1.0);
	}

	luax_catchexcept(L, [&](){ t->setPixel(x, y, c); });
	return 0;
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

	int iw = t->getWidth();

	PixelFormat format = t->getFormat();
	int components = getPixelFormatColorComponents(format);

	auto pixelsetfunction = t->getPixelSetFunction();
	auto pixelgetfunction = t->getPixelGetFunction();

	uint8 *data = (uint8 *) t->getData();
	size_t pixelsize = t->getPixelSize();

	for (int y = sy; y < sy+h; y++)
	{
		for (int x = sx; x < sx+w; x++)
		{
			auto pixeldata = (ImageData::Pixel *) (data + (y * iw + x) * pixelsize);

			Colorf c;
			pixelgetfunction(pixeldata, c);

			lua_pushvalue(L, 2); // ImageData

			lua_pushnumber(L, x);
			lua_pushnumber(L, y);

			lua_pushnumber(L, c.r);
			lua_pushnumber(L, c.g);
			lua_pushnumber(L, c.b);
			lua_pushnumber(L, c.a);

			lua_call(L, 6, 4);

			c.r = (float) luaL_checknumber(L, -4);
			if (components > 1)
				c.g = (float) luaL_checknumber(L, -3);
			if (components > 2)
				c.b = (float) luaL_checknumber(L, -2);
			if (components > 3)
				c.a = (float) luaL_optnumber(L, -1, 1.0);

			pixelsetfunction(c, pixeldata);

			lua_pop(L, 4); // Pop return values.
		}
	}

	return 0;
}

int w_ImageData_paste(lua_State *L)
{
	ImageData *t = luax_checkimagedata(L, 1);
	ImageData *src = luax_checkimagedata(L, 2);
	int dx = (int) luaL_checkinteger(L, 3);
	int dy = (int) luaL_checkinteger(L, 4);
	int sx = (int) luaL_optinteger(L, 5, 0);
	int sy = (int) luaL_optinteger(L, 6, 0);
	int sw = (int) luaL_optinteger(L, 7, src->getWidth());
	int sh = (int) luaL_optinteger(L, 8, src->getHeight());
	t->paste((love::image::ImageData *)src, dx, dy, sx, sy, sw, sh);
	return 0;
}

int w_ImageData_encode(lua_State *L)
{
	ImageData *t = luax_checkimagedata(L, 1);

	FormatHandler::EncodedFormat format;
	const char *fmt = luaL_checkstring(L, 2);
	if (!ImageData::getConstant(fmt, format))
		return luax_enumerror(L, "encoded image format", ImageData::getConstants(format), fmt);

	bool hasfilename = false;

	std::string filename = "Image." + std::string(fmt);
	if (!lua_isnoneornil(L, 3))
	{
		hasfilename = true;
		filename = luax_checkstring(L, 3);
	}

	love::filesystem::FileData *filedata = nullptr;
	luax_catchexcept(L, [&](){ filedata = t->encode(format, filename.c_str(), hasfilename); });

	luax_pushtype(L, filedata);
	filedata->release();

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

	float (*float16to32)(float16 f);
	float16 (*float32to16)(float f);

	float (*float11to32)(float11 f);
	float11 (*float32to11)(float f);

	float (*float10to32)(float10 f);
	float10 (*float32to10)(float f);
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
	},

	float16to32,
	float32to16,
	float11to32,
	float32to11,
	float10to32,
	float32to10,
};

static const luaL_Reg w_ImageData_functions[] =
{
	{ "clone", w_ImageData_clone },
	{ "getFormat", w_ImageData_getFormat },
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
	int ret = luax_register_type(L, &ImageData::type, data::w_Data_functions, w_ImageData_functions, nullptr);

	love::data::luax_rundatawrapper(L, ImageData::type);
	luax_runwrapper(L, imagedata_lua, sizeof(imagedata_lua), "ImageData.lua", ImageData::type, &ffifuncs);

	return ret;
}

} // image
} // love
