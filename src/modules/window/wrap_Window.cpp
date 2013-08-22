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

#include "wrap_Window.h"
#include "sdl/Window.h"

namespace love
{
namespace window
{

static Window *instance = 0;

int w_checkMode(lua_State *L)
{
	int w = luaL_checkint(L, 1);
	int h = luaL_checkint(L, 2);

	bool fs = false;

	if (lua_istable(L, 3))
	{
		lua_getfield(L, 3, "fullscreen");
		fs = luax_toboolean(L, -1);
		lua_pop(L, 1);
	}
	else
		fs = luax_toboolean(L, 3);

	luax_pushboolean(L, instance->checkWindowSize(w, h, fs));
	return 1;
}

int w_setMode(lua_State *L)
{
	int w = luaL_checkint(L, 1);
	int h = luaL_checkint(L, 2);

	if (lua_isnoneornil(L, 3))
	{
		luax_pushboolean(L, instance->setWindow(w, h, 0));
		return 1;
	}

	luaL_checktype(L, 3, LUA_TTABLE);

	WindowFlags flags;

	flags.fullscreen = luax_boolflag(L, 3, "fullscreen", false);
	flags.vsync = luax_boolflag(L, 3, "vsync", true);
	flags.fsaa = luax_intflag(L, 3, "fsaa", 0);
	flags.resizable = luax_boolflag(L, 3, "resizable", false);
	flags.borderless = luax_boolflag(L, 3, "borderless", false);
	flags.centered = luax_boolflag(L, 3, "centered", true);

	try
	{
		luax_pushboolean(L, instance->setWindow(w, h, &flags));
	}
	catch (love::Exception &e)
	{
		return luaL_error(L, "%s", e.what());
	}

	return 1;
}

int w_getMode(lua_State *L)
{
	int w, h;
	WindowFlags flags;
	instance->getWindow(w, h, flags);
	lua_pushnumber(L, w);
	lua_pushnumber(L, h);

	lua_newtable(L);

	luax_pushboolean(L, flags.fullscreen);
	lua_setfield(L, -2, "fullscreen");

	luax_pushboolean(L, flags.vsync);
	lua_setfield(L, -2, "vsync");

	lua_pushnumber(L, flags.fsaa);
	lua_setfield(L, -2, "fsaa");

	luax_pushboolean(L, flags.resizable);
	lua_setfield(L, -2, "resizable");

	luax_pushboolean(L, flags.borderless);
	lua_setfield(L, -2, "borderless");

	luax_pushboolean(L, flags.centered);
	lua_setfield(L, -2, "centered");
	
	return 3;
}

int w_getModes(lua_State *L)
{
	int n;
	Window::WindowSize *modes = instance->getFullscreenSizes(n);

	if (modes == 0)
		return 0;

	lua_createtable(L, n, 0);

	for (int i = 0; i < n ; i++)
	{
		lua_pushinteger(L, i+1);
		lua_createtable(L, 0, 2);

		// Inner table attribs.

		lua_pushinteger(L, modes[i].width);
		lua_setfield(L, -2, "width");

		lua_pushinteger(L, modes[i].height);
		lua_setfield(L, -2, "height");

		// Inner table attribs end.

		lua_settable(L, -3);
	}

	delete[] modes;
	return 1;
}

int w_toggleFullscreen(lua_State *L)
{
	int width, height;
	WindowFlags flags;
	instance->getWindow(width, height, flags);
	flags.fullscreen = !flags.fullscreen;

	try
	{
		luax_pushboolean(L, instance->setWindow(width, height, &flags));
	}
	catch (love::Exception &e)
	{
		return luaL_error(L, "%s", e.what());
	}

	return 1;
}

int w_isCreated(lua_State *L)
{
	luax_pushboolean(L, instance->isCreated());
	return 1;
}

int w_getWidth(lua_State *L)
{
	lua_pushinteger(L, instance->getWidth());
	return 1;
}

int w_getHeight(lua_State *L)
{
	lua_pushinteger(L, instance->getHeight());
	return 1;
}

int w_getDimensions(lua_State *L)
{
	lua_pushinteger(L, instance->getWidth());
	lua_pushinteger(L, instance->getHeight());
	return 2;
}

int w_setIcon(lua_State *L)
{
	image::ImageData *i = luax_checktype<image::ImageData>(L, 1, "ImageData", IMAGE_IMAGE_DATA_T);
	instance->setIcon(i);
	return 0;
}

int w_getIcon(lua_State *L)
{
	image::ImageData *i = instance->getIcon();
	if (i)
	{
		i->retain();
		luax_newtype(L, "ImageData", IMAGE_IMAGE_DATA_T, (void*) i);
	}
	else
		lua_pushnil(L);
	return 1;
}

int w_setTitle(lua_State *L)
{
	std::string title = luax_checkstring(L, 1);
	instance->setWindowTitle(title);
	return 0;
}

int w_getTitle(lua_State *L)
{
	luax_pushstring(L, instance->getWindowTitle());
	return 1;
}

int w_hasFocus(lua_State *L)
{
	luax_pushboolean(L, instance->hasFocus());
	return 1;
}

int w_hasMouseFocus(lua_State *L)
{
	luax_pushboolean(L, instance->hasMouseFocus());
	return 1;
}

int w_isVisible(lua_State *L)
{
	luax_pushboolean(L, instance->isVisible());
	return 1;
}

static const luaL_Reg functions[] =
{
	{ "checkMode", w_checkMode },
	{ "setMode", w_setMode },
	{ "getMode", w_getMode },
	{ "getModes", w_getModes },
	{ "toggleFullscreen", w_toggleFullscreen },
	{ "isCreated", w_isCreated },
	{ "getWidth", w_getWidth },
	{ "getHeight", w_getHeight },
	{ "getDimensions", w_getDimensions },
	{ "setIcon", w_setIcon },
	{ "getIcon", w_getIcon },
	{ "setTitle", w_setTitle },
	{ "getTitle", w_getTitle },
	{ "hasFocus", w_hasFocus },
	{ "hasMouseFocus", w_hasMouseFocus },
	{ "isVisible", w_isVisible },
	{ 0, 0 }
};

extern "C" int luaopen_love_window(lua_State *L)
{
	try
	{
		instance = sdl::Window::getSingleton();
	}
	catch (love::Exception &e)
	{
		return luaL_error(L, "%s", e.what());
	}

	WrappedModule w;
	w.module = instance;
	w.name = "window";
	w.flags = MODULE_T;
	w.functions = functions;
	w.types = 0;

	return luax_register_module(L, w);
}

} // window
} // love
