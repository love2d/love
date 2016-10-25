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

#include "wrap_Window.h"
#include "sdl/Window.h"

namespace love
{
namespace window
{

#define instance() (Module::getInstance<Window>(Module::M_WINDOW))

int w_getDisplayCount(lua_State *L)
{
	lua_pushinteger(L, instance()->getDisplayCount());
	return 1;
}

int w_getDisplayName(lua_State *L)
{
	int index = (int) luaL_checknumber(L, 1) - 1;

	const char *name = nullptr;
	luax_catchexcept(L, [&](){ name = instance()->getDisplayName(index); });

	lua_pushstring(L, name);
	return 1;
}

static const char *settingName(Window::Setting setting)
{
	const char *name = nullptr;
	Window::getConstant(setting, name);
	return name;
}

int w_setMode(lua_State *L)
{
	int w = (int) luaL_checknumber(L, 1);
	int h = (int) luaL_checknumber(L, 2);

	if (lua_isnoneornil(L, 3))
	{
		luax_pushboolean(L, instance()->setWindow(w, h, nullptr));
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
		Window::Setting setting;

		if (!Window::getConstant(key, setting))
			return luaL_error(L, "Invalid window setting: %s", key);

		lua_pop(L, 1);
	}

	WindowSettings settings;

	lua_getfield(L, 3, settingName(Window::SETTING_FULLSCREEN_TYPE));
	if (!lua_isnoneornil(L, -1))
	{
		const char *typestr = luaL_checkstring(L, -1);
		if (!Window::getConstant(typestr, settings.fstype))
			return luaL_error(L, "Invalid fullscreen type: %s", typestr);
	}
	else
	{
		// Default to desktop fullscreen mode.
		settings.fstype = Window::FULLSCREEN_DESKTOP;
	}
	lua_pop(L, 1);

	settings.fullscreen = luax_boolflag(L, 3, settingName(Window::SETTING_FULLSCREEN), false);
	settings.vsync = luax_boolflag(L, 3, settingName(Window::SETTING_VSYNC), true);
	settings.msaa = luax_intflag(L, 3, settingName(Window::SETTING_MSAA), 0);
	settings.resizable = luax_boolflag(L, 3, settingName(Window::SETTING_RESIZABLE), false);
	settings.minwidth = luax_intflag(L, 3, settingName(Window::SETTING_MIN_WIDTH), 1);
	settings.minheight = luax_intflag(L, 3, settingName(Window::SETTING_MIN_HEIGHT), 1);
	settings.borderless = luax_boolflag(L, 3, settingName(Window::SETTING_BORDERLESS), false);
	settings.centered = luax_boolflag(L, 3, settingName(Window::SETTING_CENTERED), true);
	settings.display = luax_intflag(L, 3, settingName(Window::SETTING_DISPLAY), 1) - 1;
	settings.highdpi = luax_boolflag(L, 3, settingName(Window::SETTING_HIGHDPI), false);

	lua_getfield(L, 3, settingName(Window::SETTING_X));
	lua_getfield(L, 3, settingName(Window::SETTING_Y));
	settings.useposition = !(lua_isnoneornil(L, -2) && lua_isnoneornil(L, -1));
	if (settings.useposition)
	{
		settings.x = (int) luaL_optnumber(L, -2, 0);
		settings.y = (int) luaL_optnumber(L, -1, 0);
	}
	lua_pop(L, 2);

	// We don't explicitly set the refresh rate, it's "read-only".

	luax_catchexcept(L,
		[&](){ luax_pushboolean(L, instance()->setWindow(w, h, &settings)); }
	);

	return 1;
}

int w_getMode(lua_State *L)
{
	int w, h;
	WindowSettings settings;
	instance()->getWindow(w, h, settings);
	lua_pushnumber(L, w);
	lua_pushnumber(L, h);

	lua_newtable(L);

	const char *fstypestr = "desktop";
	Window::getConstant(settings.fstype, fstypestr);

	lua_pushstring(L, fstypestr);
	lua_setfield(L, -2, settingName(Window::SETTING_FULLSCREEN_TYPE));

	luax_pushboolean(L, settings.fullscreen);
	lua_setfield(L, -2, settingName(Window::SETTING_FULLSCREEN));

	luax_pushboolean(L, settings.vsync);
	lua_setfield(L, -2, settingName(Window::SETTING_VSYNC));

	lua_pushinteger(L, settings.msaa);
	lua_setfield(L, -2, settingName(Window::SETTING_MSAA));

	luax_pushboolean(L, settings.resizable);
	lua_setfield(L, -2, settingName(Window::SETTING_RESIZABLE));

	lua_pushinteger(L, settings.minwidth);
	lua_setfield(L, -2, settingName(Window::SETTING_MIN_WIDTH));

	lua_pushinteger(L, settings.minheight);
	lua_setfield(L, -2, settingName(Window::SETTING_MIN_HEIGHT));

	luax_pushboolean(L, settings.borderless);
	lua_setfield(L, -2, settingName(Window::SETTING_BORDERLESS));

	luax_pushboolean(L, settings.centered);
	lua_setfield(L, -2, settingName(Window::SETTING_CENTERED));

	// Display index is 0-based internally and 1-based in Lua.
	lua_pushinteger(L, settings.display + 1);
	lua_setfield(L, -2, settingName(Window::SETTING_DISPLAY));

	luax_pushboolean(L, settings.highdpi);
	lua_setfield(L, -2, settingName(Window::SETTING_HIGHDPI));

	lua_pushnumber(L, settings.refreshrate);
	lua_setfield(L, -2, settingName(Window::SETTING_REFRESHRATE));

	lua_pushinteger(L, settings.x);
	lua_setfield(L, -2, settingName(Window::SETTING_X));

	lua_pushinteger(L, settings.y);
	lua_setfield(L, -2, settingName(Window::SETTING_Y));

	return 3;
}

int w_getFullscreenModes(lua_State *L)
{
	int displayindex = 0;
	if (!lua_isnoneornil(L, 1))
		displayindex = (int) luaL_checknumber(L, 1) - 1;
	else
	{
		int x, y;
		instance()->getPosition(x, y, displayindex);
	}

	std::vector<Window::WindowSize> modes = instance()->getFullscreenSizes(displayindex);

	lua_createtable(L, (int) modes.size(), 0);

	for (size_t i = 0; i < modes.size(); i++)
	{
		lua_pushinteger(L, i + 1);
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
	Window::FullscreenType fstype = Window::FULLSCREEN_MAX_ENUM;

	const char *typestr = lua_isnoneornil(L, 2) ? 0 : luaL_checkstring(L, 2);
	if (typestr && !Window::getConstant(typestr, fstype))
		return luaL_error(L, "Invalid fullscreen type: %s", typestr);

	bool success = false;
	if (fstype == Window::FULLSCREEN_MAX_ENUM)
		success = instance()->setFullscreen(fullscreen);
	else
		success = instance()->setFullscreen(fullscreen, fstype);

	luax_pushboolean(L, success);
	return 1;
}

int w_getFullscreen(lua_State *L)
{
	int w, h;
	WindowSettings settings;
	instance()->getWindow(w, h, settings);

	const char *typestr;
	if (!Window::getConstant(settings.fstype, typestr))
		luaL_error(L, "Unknown fullscreen type.");

	luax_pushboolean(L, settings.fullscreen);
	lua_pushstring(L, typestr);
	return 2;
}

int w_isOpen(lua_State *L)
{
	luax_pushboolean(L, instance()->isOpen());
	return 1;
}

int w_close(lua_State * /*L*/)
{
	instance()->close();
	return 0;
}

int w_getDesktopDimensions(lua_State *L)
{
	int width = 0, height = 0;
	int displayindex = 0;
	if (!lua_isnoneornil(L, 1))
		displayindex = (int) luaL_checknumber(L, 1) - 1;
	else
	{
		int x, y;
		instance()->getPosition(x, y, displayindex);
	}
	instance()->getDesktopDimensions(displayindex, width, height);
	lua_pushinteger(L, width);
	lua_pushinteger(L, height);
	return 2;
}

int w_setPosition(lua_State *L)
{
	int x = (int) luaL_checknumber(L, 1);
	int y = (int) luaL_checknumber(L, 2);

	int displayindex = 0;
	if (!lua_isnoneornil(L, 3))
		displayindex = (int) luaL_checknumber(L, 3) - 1;
	else
	{
		int x_unused, y_unused;
		instance()->getPosition(x_unused, y_unused, displayindex);
	}

	instance()->setPosition(x, y, displayindex);
	return 0;
}

int w_getPosition(lua_State *L)
{
	int x = 0;
	int y = 0;
	int displayindex = 0;
	instance()->getPosition(x, y, displayindex);
	lua_pushinteger(L, x);
	lua_pushinteger(L, y);
	lua_pushinteger(L, displayindex + 1);
	return 3;
}

int w_setIcon(lua_State *L)
{
	image::ImageData *i = luax_checktype<image::ImageData>(L, 1, IMAGE_IMAGE_DATA_ID);
	luax_pushboolean(L, instance()->setIcon(i));
	return 1;
}

int w_getIcon(lua_State *L)
{
	image::ImageData *i = instance()->getIcon();
	luax_pushtype(L, IMAGE_IMAGE_DATA_ID, i);
	return 1;
}

int w_setDisplaySleepEnabled(lua_State *L)
{
	instance()->setDisplaySleepEnabled(luax_toboolean(L, 1));
	return 0;
}

int w_isDisplaySleepEnabled(lua_State *L)
{
	luax_pushboolean(L, instance()->isDisplaySleepEnabled());
	return 1;
}

int w_setTitle(lua_State *L)
{
	std::string title = luax_checkstring(L, 1);
	instance()->setWindowTitle(title);
	return 0;
}

int w_getTitle(lua_State *L)
{
	luax_pushstring(L, instance()->getWindowTitle());
	return 1;
}

int w_hasFocus(lua_State *L)
{
	luax_pushboolean(L, instance()->hasFocus());
	return 1;
}

int w_hasMouseFocus(lua_State *L)
{
	luax_pushboolean(L, instance()->hasMouseFocus());
	return 1;
}

int w_isVisible(lua_State *L)
{
	luax_pushboolean(L, instance()->isVisible());
	return 1;
}

int w_getPixelScale(lua_State *L)
{
	lua_pushnumber(L, instance()->getPixelScale());
	return 1;
}

int w_toPixels(lua_State *L)
{
	double wx = luaL_checknumber(L, 1);

	if (lua_isnoneornil(L, 2))
	{
		lua_pushnumber(L, instance()->toPixels(wx));
		return 1;
	}

	double wy = luaL_checknumber(L, 2);
	double px = 0.0, py = 0.0;

	instance()->toPixels(wx, wy, px, py);

	lua_pushnumber(L, px);
	lua_pushnumber(L, py);

	return 2;
}

int w_fromPixels(lua_State *L)
{
	double px = luaL_checknumber(L, 1);

	if (lua_isnoneornil(L, 2))
	{
		lua_pushnumber(L, instance()->fromPixels(px));
		return 1;
	}

	double py = luaL_checknumber(L, 2);
	double wx = 0.0, wy = 0.0;

	instance()->fromPixels(px, py, wx, wy);

	lua_pushnumber(L, wx);
	lua_pushnumber(L, wy);

	return 2;
}

int w_minimize(lua_State* /*L*/)
{
	instance()->minimize();
	return 0;
}

int w_maximize(lua_State *)
{
	instance()->maximize();
	return 0;
}

int w_isMaximized(lua_State *L)
{
	luax_pushboolean(L, instance()->isMaximized());
	return 0;
}

int w_showMessageBox(lua_State *L)
{
	Window::MessageBoxData data = {};
	data.type = Window::MESSAGEBOX_INFO;

	data.title = luaL_checkstring(L, 1);
	data.message = luaL_checkstring(L, 2);

	// If we have a table argument, we assume a list of button names, which
	// means we should use the more complex message box API.
	if (lua_istable(L, 3))
	{
		size_t numbuttons = luax_objlen(L, 3);
		if (numbuttons == 0)
			return luaL_error(L, "Must have at least one messagebox button.");

		// Array of button names.
		for (size_t i = 0; i < numbuttons; i++)
		{
			lua_rawgeti(L, 3, (int) i + 1);
			data.buttons.push_back(luax_checkstring(L, -1));
			lua_pop(L, 1);
		}

		// Optional table entry specifying the button to use when enter is pressed.
		lua_getfield(L, 3, "enterbutton");
		if (!lua_isnoneornil(L, -1))
			data.enterButtonIndex = (int) luaL_checknumber(L, -1) - 1;
		else
			data.enterButtonIndex = 0;
		lua_pop(L, 1);

		// Optional table entry specifying the button to use when esc is pressed.
		lua_getfield(L, 3, "escapebutton");
		if (!lua_isnoneornil(L, -1))
			data.escapeButtonIndex = (int) luaL_checknumber(L, -1) - 1;
		else
			data.escapeButtonIndex = (int) data.buttons.size() - 1;
		lua_pop(L, 1);

		const char *typestr = lua_isnoneornil(L, 4) ? nullptr : luaL_checkstring(L, 4);
		if (typestr && !Window::getConstant(typestr, data.type))
			return luaL_error(L, "Invalid messagebox type: %s", typestr);

		data.attachToWindow = luax_optboolean(L, 5, true);

		int pressedbutton = instance()->showMessageBox(data);
		lua_pushinteger(L, pressedbutton + 1);
	}
	else
	{
		const char *typestr = lua_isnoneornil(L, 3) ? nullptr : luaL_checkstring(L, 3);
		if (typestr && !Window::getConstant(typestr, data.type))
			return luaL_error(L, "Invalid messagebox type: %s", typestr);

		data.attachToWindow = luax_optboolean(L, 4, true);

		// Display a simple message box.
		bool success = instance()->showMessageBox(data.title, data.message, data.type, data.attachToWindow);
		luax_pushboolean(L, success);
	}

	return 1;
}

int w_requestAttention(lua_State *L)
{
	bool continuous = luax_optboolean(L, 1, false);
	instance()->requestAttention(continuous);
	return 0;
}

static const luaL_Reg functions[] =
{
	{ "getDisplayCount", w_getDisplayCount },
	{ "getDisplayName", w_getDisplayName },
	{ "setMode", w_setMode },
	{ "getMode", w_getMode },
	{ "getFullscreenModes", w_getFullscreenModes },
	{ "setFullscreen", w_setFullscreen },
	{ "getFullscreen", w_getFullscreen },
	{ "isOpen", w_isOpen },
	{ "isCreated", w_isOpen }, // For compatibility with old error handlers...
	{ "close", w_close },
	{ "getDesktopDimensions", w_getDesktopDimensions },
	{ "setPosition", w_setPosition },
	{ "getPosition", w_getPosition },
	{ "setIcon", w_setIcon },
	{ "getIcon", w_getIcon },
	{ "setDisplaySleepEnabled", w_setDisplaySleepEnabled },
	{ "isDisplaySleepEnabled", w_isDisplaySleepEnabled },
	{ "setTitle", w_setTitle },
	{ "getTitle", w_getTitle },
	{ "hasFocus", w_hasFocus },
	{ "hasMouseFocus", w_hasMouseFocus },
	{ "isVisible", w_isVisible },
	{ "getPixelScale", w_getPixelScale },
	{ "toPixels", w_toPixels },
	{ "fromPixels", w_fromPixels },
	{ "minimize", w_minimize },
	{ "maximize", w_maximize },
	{ "isMaximized", w_isMaximized },
	{ "showMessageBox", w_showMessageBox },
	{ "requestAttention", w_requestAttention },
	{ 0, 0 }
};

extern "C" int luaopen_love_window(lua_State *L)
{
	Window *instance = instance();
	if (instance == nullptr)
		luax_catchexcept(L, [&](){ instance = new love::window::sdl::Window(); });
	else
		instance->retain();

	WrappedModule w;
	w.module = instance;
	w.name = "window";
	w.type = MODULE_ID;
	w.functions = functions;
	w.types = 0;

	return luax_register_module(L, w);
}

} // window
} // love
