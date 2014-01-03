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

#include "wrap_Window.h"
#include "sdl/Window.h"

namespace love
{
namespace window
{

static Window *instance = 0;

int w_getDisplayCount(lua_State *L)
{
	lua_pushinteger(L, instance->getDisplayCount());
	return 1;
}

static const char *attribName(Window::Attribute attrib)
{
	const char *name = nullptr;
	Window::getConstant(attrib, name);
	return name;
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

	// We want to error for invalid / misspelled window attributes.
	lua_pushnil(L);
	while (lua_next(L, 3))
	{
		if (lua_type(L, -2) != LUA_TSTRING)
			return luax_typerror(L, -2, "string");

		const char *key = luaL_checkstring(L, -2);
		Window::Attribute attrib;

		if (!Window::getConstant(key, attrib))
			return luaL_error(L, "Invalid window attribute: %s", key);

		lua_pop(L, 1);
	}

	WindowAttributes attribs;

	lua_getfield(L, 3, attribName(Window::ATTRIB_FULLSCREEN_TYPE));
	if (!lua_isnoneornil(L, -1))
	{
		const char *typestr = luaL_checkstring(L, -1);

		if (!Window::getConstant(typestr, attribs.fstype))
			return luaL_error(L, "Invalid fullscreen type: %s", typestr);
	}
	else
	{
		// Default to "normal" fullscreen.
		attribs.fstype = Window::FULLSCREEN_TYPE_NORMAL;
	}
	lua_pop(L, 1);

	attribs.fullscreen = luax_boolflag(L, 3, attribName(Window::ATTRIB_FULLSCREEN), false);
	attribs.vsync = luax_boolflag(L, 3, attribName(Window::ATTRIB_VSYNC), true);
	attribs.fsaa = luax_intflag(L, 3, attribName(Window::ATTRIB_FSAA), 0);
	attribs.resizable = luax_boolflag(L, 3, attribName(Window::ATTRIB_RESIZABLE), false);
	attribs.minwidth = luax_intflag(L, 3, attribName(Window::ATTRIB_MIN_WIDTH), 1);
	attribs.minheight = luax_intflag(L, 3, attribName(Window::ATTRIB_MIN_HEIGHT), 1);
	attribs.borderless = luax_boolflag(L, 3, attribName(Window::ATTRIB_BORDERLESS), false);
	attribs.centered = luax_boolflag(L, 3, attribName(Window::ATTRIB_CENTERED), true);
	attribs.display = luax_intflag(L, 3, attribName(Window::ATTRIB_DISPLAY), 1);

	// Display index is 1-based in Lua and 0-based internally.
	attribs.display--;

	EXCEPT_GUARD(luax_pushboolean(L, instance->setWindow(w, h, &attribs));)
	return 1;
}

int w_getMode(lua_State *L)
{
	int w, h;
	WindowAttributes attribs;
	instance->getWindow(w, h, attribs);
	lua_pushnumber(L, w);
	lua_pushnumber(L, h);

	lua_newtable(L);

	const char *fstypestr = "normal";
	Window::getConstant(attribs.fstype, fstypestr);

	lua_pushstring(L, fstypestr);
	lua_setfield(L, -2, attribName(Window::ATTRIB_FULLSCREEN_TYPE));

	luax_pushboolean(L, attribs.fullscreen);
	lua_setfield(L, -2, attribName(Window::ATTRIB_FULLSCREEN));

	luax_pushboolean(L, attribs.vsync);
	lua_setfield(L, -2, attribName(Window::ATTRIB_VSYNC));

	lua_pushinteger(L, attribs.fsaa);
	lua_setfield(L, -2, attribName(Window::ATTRIB_FSAA));

	luax_pushboolean(L, attribs.resizable);
	lua_setfield(L, -2, attribName(Window::ATTRIB_RESIZABLE));

	lua_pushinteger(L, attribs.minwidth);
	lua_setfield(L, -2, attribName(Window::ATTRIB_MIN_WIDTH));

	lua_pushinteger(L, attribs.minheight);
	lua_setfield(L, -2, attribName(Window::ATTRIB_MIN_HEIGHT));

	luax_pushboolean(L, attribs.borderless);
	lua_setfield(L, -2, attribName(Window::ATTRIB_BORDERLESS));

	luax_pushboolean(L, attribs.centered);
	lua_setfield(L, -2, attribName(Window::ATTRIB_CENTERED));

	// Display index is 0-based internally and 1-based in Lua.
	lua_pushinteger(L, attribs.display + 1);
	lua_setfield(L, -2, attribName(Window::ATTRIB_DISPLAY));

	return 3;
}

int w_getFullscreenModes(lua_State *L)
{
	int displayindex = luaL_optint(L, 1, 1) - 1;

	std::vector<Window::WindowSize> modes = instance->getFullscreenSizes(displayindex);

	lua_createtable(L, modes.size(), 0);

	for (size_t i = 0; i < modes.size(); i++)
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

	return 1;
}

int w_setFullscreen(lua_State *L)
{
	bool fullscreen = luax_toboolean(L, 1);
	Window::FullscreenType fstype = Window::FULLSCREEN_TYPE_MAX_ENUM;

	const char *typestr = lua_isnoneornil(L, 2) ? 0 : luaL_checkstring(L, 2);
	if (typestr && !Window::getConstant(typestr, fstype))
		return luaL_error(L, "Invalid fullscreen type: %s", typestr);

	bool success = false;
	if (fstype == Window::FULLSCREEN_TYPE_MAX_ENUM)
		success = instance->setFullscreen(fullscreen);
	else
		success = instance->setFullscreen(fullscreen, fstype);

	luax_pushboolean(L, success);
	return 1;
}

int w_getFullscreen(lua_State *L)
{
	int w, h;
	WindowAttributes attribs;
	instance->getWindow(w, h, attribs);

	const char *typestr;
	if (!Window::getConstant(attribs.fstype, typestr))
		luaL_error(L, "Unknown fullscreen type.");

	luax_pushboolean(L, attribs.fullscreen);
	lua_pushstring(L, typestr);
	return 2;
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

int w_getDesktopDimensions(lua_State *L)
{
	int width = 0, height = 0;
	int displayindex = luaL_optint(L, 1, 1) - 1;
	instance->getDesktopDimensions(displayindex, width, height);
	lua_pushinteger(L, width);
	lua_pushinteger(L, height);
	return 2;
}

int w_setIcon(lua_State *L)
{
	image::ImageData *i = luax_checktype<image::ImageData>(L, 1, "ImageData", IMAGE_IMAGE_DATA_T);
	luax_pushboolean(L, instance->setIcon(i));
	return 1;
}

int w_getIcon(lua_State *L)
{
	image::ImageData *i = instance->getIcon();
	if (i)
	{
		i->retain();
		luax_pushtype(L, "ImageData", IMAGE_IMAGE_DATA_T, i);
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
	{ "getDisplayCount", w_getDisplayCount },
	{ "setMode", w_setMode },
	{ "getMode", w_getMode },
	{ "getFullscreenModes", w_getFullscreenModes },
	{ "setFullscreen", w_setFullscreen },
	{ "getFullscreen", w_getFullscreen },
	{ "isCreated", w_isCreated },
	{ "getWidth", w_getWidth },
	{ "getHeight", w_getHeight },
	{ "getDimensions", w_getDimensions },
	{ "getDesktopDimensions", w_getDesktopDimensions },
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
	EXCEPT_GUARD(instance = sdl::Window::createSingleton();)

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
