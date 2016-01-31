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

#include "wrap_Canvas.h"
#include "Graphics.h"

namespace love
{
namespace graphics
{
namespace opengl
{

Canvas *luax_checkcanvas(lua_State *L, int idx)
{
	return luax_checktype<Canvas>(L, idx, GRAPHICS_CANVAS_ID);
}

int w_Canvas_renderTo(lua_State *L)
{
	Canvas *canvas = luax_checkcanvas(L, 1);
	luaL_checktype(L, 2, LUA_TFUNCTION);

	auto graphics = Module::getInstance<Graphics>(Module::M_GRAPHICS);

	if (graphics)
	{
		// Save the current Canvas so we can restore it when we're done.
		std::vector<Canvas *> oldcanvases = graphics->getCanvas();

		for (Canvas *c : oldcanvases)
			c->retain();

		luax_catchexcept(L, [&](){ graphics->setCanvas(canvas); });

		lua_settop(L, 2); // make sure the function is on top of the stack
		int status = lua_pcall(L, 0, 0, 0);

		graphics->setCanvas(oldcanvases);

		for (Canvas *c : oldcanvases)
			c->release();

		if (status != 0)
			return lua_error(L);
	}

	return 0;
}

int w_Canvas_newImageData(lua_State *L)
{
	Canvas *canvas = luax_checkcanvas(L, 1);
	love::image::Image *image = luax_getmodule<love::image::Image>(L, MODULE_IMAGE_ID);
	int x = (int) luaL_optnumber(L, 2, 0);
	int y = (int) luaL_optnumber(L, 3, 0);
	int w = (int) luaL_optnumber(L, 4, canvas->getWidth());
	int h = (int) luaL_optnumber(L, 5, canvas->getHeight());

	love::image::ImageData *img = nullptr;
	luax_catchexcept(L, [&](){ img = canvas->newImageData(image, x, y, w, h); });

	luax_pushtype(L, IMAGE_IMAGE_DATA_ID, img);
	img->release();
	return 1;
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

static const luaL_Reg w_Canvas_functions[] =
{
	{ "renderTo", w_Canvas_renderTo },
	{ "newImageData", w_Canvas_newImageData },
	{ "getFormat", w_Canvas_getFormat },
	{ "getMSAA", w_Canvas_getMSAA },
	{ 0, 0 }
};

extern "C" int luaopen_canvas(lua_State *L)
{
	return luax_register_type(L, GRAPHICS_CANVAS_ID, "Canvas", w_Texture_functions, w_Canvas_functions, nullptr);
}

} // opengl
} // graphics
} // love
