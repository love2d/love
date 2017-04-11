/**
 * Copyright (c) 2006-2017 LOVE Development Team
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

Canvas *luax_checkcanvas(lua_State *L, int idx)
{
	return luax_checktype<Canvas>(L, idx);
}

int w_Canvas_getMSAA(lua_State *L)
{
	Canvas *canvas = luax_checkcanvas(L, 1);
	lua_pushinteger(L, canvas->getMSAA());
	return 1;
}

int w_Canvas_renderTo(lua_State *L)
{
	Graphics::RenderTarget rt(luax_checkcanvas(L, 1));

	int startidx = 2;

	if (rt.canvas->getTextureType() != TEXTURE_2D)
	{
		rt.slice = (int) luaL_checknumber(L, 2) - 1;
		startidx++;
	}

	luaL_checktype(L, startidx, LUA_TFUNCTION);

	auto graphics = Module::getInstance<Graphics>(Module::M_GRAPHICS);

	if (graphics)
	{
		// Save the current render targets so we can restore them when we're done.
		Graphics::RenderTargets oldtargets = graphics->getCanvas();

		for (auto c : oldtargets.colors)
			c.canvas->retain();

		if (oldtargets.depthStencil.canvas != nullptr)
			oldtargets.depthStencil.canvas->retain();

		luax_catchexcept(L, [&](){ graphics->setCanvas(rt); });

		lua_settop(L, 2); // make sure the function is on top of the stack
		int status = lua_pcall(L, 0, 0, 0);

		graphics->setCanvas(oldtargets);

		for (auto c : oldtargets.colors)
			c.canvas->release();

		if (oldtargets.depthStencil.canvas != nullptr)
			oldtargets.depthStencil.canvas->release();

		if (status != 0)
			return lua_error(L);
	}
	
	return 0;
}

int w_Canvas_newImageData(lua_State *L)
{
	Canvas *canvas = luax_checkcanvas(L, 1);
	love::image::Image *image = luax_getmodule<love::image::Image>(L, love::image::Image::type);

	int slice = 0;
	int x = 0;
	int y = 0;
	int w = canvas->getPixelWidth();
	int h = canvas->getPixelHeight();

	int startidx = 2;

	if (canvas->getTextureType() != TEXTURE_2D)
	{
		slice = (int) luaL_checknumber(L, startidx);
		startidx++;
	}

	if (!lua_isnoneornil(L, startidx))
	{
		x = (int) luaL_checknumber(L, startidx + 0);
		y = (int) luaL_checknumber(L, startidx + 1);
		w = (int) luaL_checknumber(L, startidx + 2);
		h = (int) luaL_checknumber(L, startidx + 3);
	}

	love::image::ImageData *img = nullptr;
	luax_catchexcept(L, [&](){ img = canvas->newImageData(image, slice, x, y, w, h); });

	luax_pushtype(L, img);
	img->release();
	return 1;
}

static const luaL_Reg w_Canvas_functions[] =
{
	{ "getMSAA", w_Canvas_getMSAA },
	{ "renderTo", w_Canvas_renderTo },
	{ "newImageData", w_Canvas_newImageData },
	{ 0, 0 }
};

extern "C" int luaopen_canvas(lua_State *L)
{
	return luax_register_type(L, &Canvas::type, w_Texture_functions, w_Canvas_functions, nullptr);
}

} // graphics
} // love
