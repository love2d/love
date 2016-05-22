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

int w_ImageData_getFormat(lua_State *L)
{
	ImageData *t = luax_checkimagedata(L, 1);
	ImageData::Format format = t->getFormat();
	const char *fstr = nullptr;

	if (!ImageData::getConstant(format, fstr))
		return luaL_error(L, "Unknown ImageData format.");

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

// TODO: rgba16f

static void luax_checkpixel_rgba8(lua_State *L, int startidx, Pixel &p)
{
	for (int i = 0; i < 3; i++)
		p.rgba8[i] = (uint8) (luaL_checknumber(L, startidx + i) * 255.0);

	p.rgba8[3] = (uint8) (luaL_optnumber(L, startidx + 3, 1.0) * 255.0);
}

static void luax_checkpixel_rgba16(lua_State *L, int startidx, Pixel &p)
{
	for (int i = 0; i < 3; i++)
		p.rgba16[i] = (uint16) (luaL_checknumber(L, startidx + i) * 65535.0);

	p.rgba16[3] = (uint16) (luaL_optnumber(L, startidx + 3, 1.0) * 65535.0);
}

static void luax_checkpixel_rgba16f(lua_State *L, int startidx, Pixel &p)
{
	for (int i = 0; i < 3; i++)
		p.rgba16f[i] = floatToHalf((float) luaL_checknumber(L, startidx + i));

	p.rgba16f[3] = floatToHalf((float) luaL_optnumber(L, startidx + 3, 1.0));
}

static void luax_checkpixel_rgba32f(lua_State *L, int startidx, Pixel &p)
{
	for (int i = 0; i < 3; i++)
		p.rgba32f[i] = (float) luaL_checknumber(L, startidx + i);

	p.rgba32f[3] = (float) luaL_optnumber(L, startidx + 3, 1.0);
}

static int luax_pushpixel_rgba8(lua_State *L, const Pixel &p)
{
	for (int i = 0; i < 4; i++)
		lua_pushnumber(L, (lua_Number) p.rgba8[i] / 255.0);
	return 4;
}

static int luax_pushpixel_rgba16(lua_State *L, const Pixel &p)
{
	for (int i = 0; i < 4; i++)
		lua_pushnumber(L, (lua_Number) p.rgba16[i] / 65535.0);
	return 4;
}

static int luax_pushpixel_rgba16f(lua_State *L, const Pixel &p)
{
	for (int i = 0; i < 4; i++)
		lua_pushnumber(L, (lua_Number) halfToFloat(p.rgba16f[i]));
	return 4;
}

static int luax_pushpixel_rgba32f(lua_State *L, const Pixel &p)
{
	for (int i = 0; i < 4; i++)
		lua_pushnumber(L, (lua_Number) p.rgba32f[i]);
	return 4;
}

typedef void(*checkpixel)(lua_State *L, int startidx, Pixel &p);
typedef int(*pushpixel)(lua_State *L, const Pixel &p);

static checkpixel checkFormats[ImageData::FORMAT_MAX_ENUM] =
{
	luax_checkpixel_rgba8,
	luax_checkpixel_rgba16,
	luax_checkpixel_rgba16f,
	luax_checkpixel_rgba32f,
};

static pushpixel pushFormats[ImageData::FORMAT_MAX_ENUM] =
{
	luax_pushpixel_rgba8,
	luax_pushpixel_rgba16,
	luax_pushpixel_rgba16f,
	luax_pushpixel_rgba32f,
};

int w_ImageData_getPixel(lua_State *L)
{
	ImageData *t = luax_checkimagedata(L, 1);
	int x = (int) luaL_checknumber(L, 2);
	int y = (int) luaL_checknumber(L, 3);

	ImageData::Format format = t->getFormat();

	Pixel p;
	luax_catchexcept(L, [&](){ t->getPixel(x, y, p); });

	return pushFormats[format](L, p);
}

int w_ImageData_setPixel(lua_State *L)
{
	ImageData *t = luax_checkimagedata(L, 1);
	int x = (int) luaL_checknumber(L, 2);
	int y = (int) luaL_checknumber(L, 3);

	ImageData::Format format = t->getFormat();

	Pixel p;

	if (lua_istable(L, 4))
	{
		for (int i = 1; i <= 4; i++)
			lua_rawgeti(L, 4, i);

		checkFormats[format](L, -4, p);

		lua_pop(L, 4);
	}
	else
		checkFormats[format](L, 4, p);

	luax_catchexcept(L, [&](){ t->setPixel(x, y, p); });
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

	ImageData::Format format = t->getFormat();

	auto checkpixel = checkFormats[format];
	auto pushpixel = pushFormats[format];

	uint8 *data = (uint8 *) t->getData();
	size_t pixelsize = t->getPixelSize();

	for (int y = sy; y < sy+h; y++)
	{
		for (int x = sx; x < sx+w; x++)
		{
			Pixel *pixeldata = (Pixel *) (data + (y * iw + x) * pixelsize);

			lua_pushvalue(L, 2); // ImageData
			lua_pushnumber(L, x);
			lua_pushnumber(L, y);

			pushpixel(L, *pixeldata);

			lua_call(L, 6, 4);

			checkpixel(L, -4, *pixeldata);
			lua_pop(L, 4); // Pop return values.
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

	float (*halfToFloat)(half h);
	half (*floatToHalf)(float f);
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

	halfToFloat,
	floatToHalf,
};

static const luaL_Reg w_ImageData_functions[] =
{
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
