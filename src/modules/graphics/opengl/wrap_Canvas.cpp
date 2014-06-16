/**
 * Copyright (c) 2006-2014 LOVE Development Team
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

#include "wrap_Canvas.h"

namespace love
{
namespace graphics
{
namespace opengl
{

Canvas *luax_checkcanvas(lua_State *L, int idx)
{
	return luax_checktype<Canvas>(L, idx, "Canvas", GRAPHICS_CANVAS_T);
}

int w_Canvas_renderTo(lua_State *L)
{
	Canvas *canvas = luax_checkcanvas(L, 1);
	luaL_checktype(L, 2, LUA_TFUNCTION);

	// Save the current Canvas so we can restore it when we're done.
	Canvas *oldcanvas = Canvas::current;

	luax_catchexcept(L, [&](){ canvas->startGrab(); });

	lua_settop(L, 2); // make sure the function is on top of the stack
	lua_call(L, 0, 0);

	if (oldcanvas != nullptr)
		oldcanvas->startGrab(oldcanvas->getAttachedCanvases());
	else
		Canvas::bindDefaultCanvas();

	return 0;
}

int w_Canvas_getImageData(lua_State *L)
{
	Canvas *canvas = luax_checkcanvas(L, 1);
	love::image::Image *image = luax_getmodule<love::image::Image>(L, "image", MODULE_IMAGE_T);
	love::image::ImageData *img = canvas->getImageData(image);
	luax_pushtype(L, "ImageData", IMAGE_IMAGE_DATA_T, img);
	return 1;
}

int w_Canvas_getPixel(lua_State * L)
{
	Canvas * canvas = luax_checkcanvas(L, 1);
	int x = luaL_checkint(L, 2);
	int y = luaL_checkint(L, 3);
	unsigned char c[4];

	luax_catchexcept(L, [&](){ canvas->getPixel(c, x, y); });

	lua_pushnumber(L, c[0]);
	lua_pushnumber(L, c[1]);
	lua_pushnumber(L, c[2]);
	lua_pushnumber(L, c[3]);
	return 4;
}

int w_Canvas_clear(lua_State *L)
{
	Canvas *canvas = luax_checkcanvas(L, 1);
	Color c;
	if (lua_isnoneornil(L, 2))
	{
		c.set(0, 0, 0, 0);
	}
	else if (lua_istable(L, 2))
	{
		for (int i = 1; i <= 4; i++)
			lua_rawgeti(L, 2, i);

		c.r = (unsigned char)luaL_checkinteger(L, -4);
		c.g = (unsigned char)luaL_checkinteger(L, -3);
		c.b = (unsigned char)luaL_checkinteger(L, -2);
		c.a = (unsigned char)luaL_optinteger(L, -1, 255);

		lua_pop(L, 4);
	}
	else
	{
		c.r = (unsigned char)luaL_checkinteger(L, 2);
		c.g = (unsigned char)luaL_checkinteger(L, 3);
		c.b = (unsigned char)luaL_checkinteger(L, 4);
		c.a = (unsigned char)luaL_optinteger(L, 5, 255);
	}
	canvas->clear(c);

	return 0;
}

int w_Canvas_getFormat(lua_State *L)
{
	Canvas *canvas = luax_checkcanvas(L, 1);
	Canvas::Format format = canvas->getTextureFormat();
	const char *str;
	if (!Canvas::getConstant(format, str))
		return luaL_error(L, "Unknown Canvas format.");

	lua_pushstring(L, str);
	return 1;
}

int w_Canvas_getMSAA(lua_State *L)
{
	Canvas *canvas = luax_checkcanvas(L, 1);
	lua_pushinteger(L, canvas->getMSAA());
	return 1;
}

static const luaL_Reg functions[] =
{
	// From wrap_Texture.
	{ "getWidth", w_Texture_getWidth },
	{ "getHeight", w_Texture_getHeight },
	{ "getDimensions", w_Texture_getDimensions },
	{ "setFilter", w_Texture_setFilter },
	{ "getFilter", w_Texture_getFilter },
	{ "setWrap", w_Texture_setWrap },
	{ "getWrap", w_Texture_getWrap },

	{ "renderTo", w_Canvas_renderTo },
	{ "getImageData", w_Canvas_getImageData },
	{ "getPixel", w_Canvas_getPixel },
	{ "clear", w_Canvas_clear },
	{ "getFormat", w_Canvas_getFormat },
	{ "getMSAA", w_Canvas_getMSAA },
	{ 0, 0 }
};

extern "C" int luaopen_canvas(lua_State *L)
{
	return luax_register_type(L, "Canvas", functions);
}

} // opengl
} // graphics
} // love
