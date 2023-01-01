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
	int index = (int) luaL_checkinteger(L, 1) - 1;

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

static int readWindowSettings(lua_State *L, int idx, WindowSettings &settings)
{
	luax_checktablefields<Window::Setting>(L, idx, "window setting", Window::getConstant);

	lua_getfield(L, idx, settingName(Window::SETTING_FULLSCREEN_TYPE));
	if (!lua_isnoneornil(L, -1))
	{
		const char *typestr = luaL_checkstring(L, -1);
		if (!Window::getConstant(typestr, settings.fstype))
			return luax_enumerror(L, "fullscreen type", Window::getConstants(settings.fstype), typestr);
	}
	lua_pop(L, 1);

	settings.fullscreen = luax_boolflag(L, idx, settingName(Window::SETTING_FULLSCREEN), settings.fullscreen);
	settings.msaa = luax_intflag(L, idx, settingName(Window::SETTING_MSAA), settings.msaa);
	settings.stencil = luax_boolflag(L, idx, settingName(Window::SETTING_STENCIL), settings.stencil);
	settings.depth = luax_intflag(L, idx, settingName(Window::SETTING_DEPTH), settings.depth);
	settings.resizable = luax_boolflag(L, idx, settingName(Window::SETTING_RESIZABLE), settings.resizable);
	settings.minwidth = luax_intflag(L, idx, settingName(Window::SETTING_MIN_WIDTH), settings.minwidth);
	settings.minheight = luax_intflag(L, idx, settingName(Window::SETTING_MIN_HEIGHT), settings.minheight);
	settings.borderless = luax_boolflag(L, idx, settingName(Window::SETTING_BORDERLESS), settings.borderless);
	settings.centered = luax_boolflag(L, idx, settingName(Window::SETTING_CENTERED), settings.centered);
	settings.display = luax_intflag(L, idx, settingName(Window::SETTING_DISPLAY), settings.display+1) - 1;
	settings.highdpi = luax_boolflag(L, idx, settingName(Window::SETTING_HIGHDPI), settings.highdpi);
	settings.usedpiscale = luax_boolflag(L, idx, settingName(Window::SETTING_USE_DPISCALE), settings.usedpiscale);

	lua_getfield(L, idx, settingName(Window::SETTING_VSYNC));
	if (lua_isnumber(L, -1))
		settings.vsync = (int) lua_tointeger(L, -1);
	else if (lua_isboolean(L, -1))
		settings.vsync = lua_toboolean(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, idx, settingName(Window::SETTING_X));
	lua_getfield(L, idx, settingName(Window::SETTING_Y));
	settings.useposition = !(lua_isnoneornil(L, -2) && lua_isnoneornil(L, -1));
	if (settings.useposition)
	{
		settings.x = (int) luaL_optinteger(L, -2, 0);
		settings.y = (int) luaL_optinteger(L, -1, 0);
	}
	lua_pop(L, 2);

	// We don't explicitly set the refresh rate, it's "read-only".
	return 0;
}

int w_setMode(lua_State *L)
{
	int w = (int) luaL_checkinteger(L, 1);
	int h = (int) luaL_checkinteger(L, 2);

	if (lua_isnoneornil(L, 3))
	{
		luax_catchexcept(L, [&](){ luax_pushboolean(L, instance()->setWindow(w, h, nullptr)); });
		return 1;
	}

	// Defaults come from WindowSettings itself.
	WindowSettings settings;

	readWindowSettings(L, 3, settings);

	luax_catchexcept(L,
		[&](){ luax_pushboolean(L, instance()->setWindow(w, h, &settings)); }
	);

	return 1;
}

int w_updateMode(lua_State *L)
{
	int w, h;
	WindowSettings settings;
	instance()->getWindow(w, h, settings);

	if (lua_gettop(L) == 0)
		return luaL_error(L, "Expected at least one argument");

	int idx = 1;
	if (lua_isnumber(L, 1))
	{
		idx = 3;
		w = (int) luaL_checkinteger(L, 1);
		h = (int) luaL_checkinteger(L, 2);
	}

	if (!lua_isnoneornil(L, idx))
		readWindowSettings(L, idx, settings);

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

	if (lua_istable(L, 1))
		lua_pushvalue(L, 1);
	else
		lua_newtable(L);

	const char *fstypestr = "desktop";
	Window::getConstant(settings.fstype, fstypestr);

	lua_pushstring(L, fstypestr);
	lua_setfield(L, -2, settingName(Window::SETTING_FULLSCREEN_TYPE));

	luax_pushboolean(L, settings.fullscreen);
	lua_setfield(L, -2, settingName(Window::SETTING_FULLSCREEN));

	lua_pushinteger(L, settings.vsync);
	lua_setfield(L, -2, settingName(Window::SETTING_VSYNC));

	lua_pushinteger(L, settings.msaa);
	lua_setfield(L, -2, settingName(Window::SETTING_MSAA));

	luax_pushboolean(L, settings.stencil);
	lua_setfield(L, -2, settingName(Window::SETTING_STENCIL));

	lua_pushinteger(L, settings.depth);
	lua_setfield(L, -2, settingName(Window::SETTING_DEPTH));

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

	luax_pushboolean(L, settings.usedpiscale);
	lua_setfield(L, -2, settingName(Window::SETTING_USE_DPISCALE));

	lua_pushnumber(L, settings.refreshrate);
	lua_setfield(L, -2, settingName(Window::SETTING_REFRESHRATE));

	lua_pushinteger(L, settings.x);
	lua_setfield(L, -2, settingName(Window::SETTING_X));

	lua_pushinteger(L, settings.y);
	lua_setfield(L, -2, settingName(Window::SETTING_Y));

	return 3;
}

int w_getDisplayOrientation(lua_State *L)
{
	int displayindex = 0;
	if (!lua_isnoneornil(L, 1))
		displayindex = (int) luaL_checkinteger(L, 1) - 1;
	else
	{
		int x, y;
		instance()->getPosition(x, y, displayindex);
	}

	const char *orientationstr = nullptr;
	if (!Window::getConstant(instance()->getDisplayOrientation(displayindex), orientationstr))
		return luaL_error(L, "Unknown display orientation type.");

	lua_pushstring(L, orientationstr);
	return 1;
}

int w_getFullscreenModes(lua_State *L)
{
	int displayindex = 0;
	if (!lua_isnoneornil(L, 1))
		displayindex = (int) luaL_checkinteger(L, 1) - 1;
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
	bool fullscreen = luax_checkboolean(L, 1);
	Window::FullscreenType fstype = Window::FULLSCREEN_MAX_ENUM;

	const char *typestr = lua_isnoneornil(L, 2) ? 0 : luaL_checkstring(L, 2);
	if (typestr && !Window::getConstant(typestr, fstype))
		return luax_enumerror(L, "fullscreen type", Window::getConstants(fstype), typestr);

	bool success = false;
	luax_catchexcept(L, [&]() {
		if (fstype == Window::FULLSCREEN_MAX_ENUM)
			success = instance()->setFullscreen(fullscreen);
		else
			success = instance()->setFullscreen(fullscreen, fstype);
	});

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

int w_close(lua_State *L)
{
	luax_catchexcept(L, [&]() { instance()->close(); });
	return 0;
}

int w_getDesktopDimensions(lua_State *L)
{
	int width = 0, height = 0;
	int displayindex = 0;
	if (!lua_isnoneornil(L, 1))
		displayindex = (int) luaL_checkinteger(L, 1) - 1;
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
	int x = (int) luaL_checkinteger(L, 1);
	int y = (int) luaL_checkinteger(L, 2);

	int displayindex = 0;
	if (!lua_isnoneornil(L, 3))
		displayindex = (int) luaL_checkinteger(L, 3) - 1;
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

int w_getSafeArea(lua_State *L)
{
	Rect area = instance()->getSafeArea();
	lua_pushnumber(L, area.x);
	lua_pushnumber(L, area.y);
	lua_pushnumber(L, area.w);
	lua_pushnumber(L, area.h);
	return 4;
}

int w_setIcon(lua_State *L)
{
	image::ImageData *i = luax_checktype<image::ImageData>(L, 1);
	bool success = false;
	luax_catchexcept(L, [&]() { success = instance()->setIcon(i); });
	luax_pushboolean(L, success);
	return 1;
}

int w_getIcon(lua_State *L)
{
	image::ImageData *i = instance()->getIcon();
	luax_pushtype(L, i);
	return 1;
}

int w_setVSync(lua_State *L)
{
	int vsync = 0;
	if (lua_type(L, 1) == LUA_TBOOLEAN)
		vsync = lua_toboolean(L, 1);
	else
		vsync = (int)luaL_checkinteger(L, 1);
	instance()->setVSync(vsync);
	return 0;
}

int w_getVSync(lua_State *L)
{
	lua_pushinteger(L, instance()->getVSync());
	return 1;
}

int w_setDisplaySleepEnabled(lua_State *L)
{
	instance()->setDisplaySleepEnabled(luax_checkboolean(L, 1));
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

int w_getDPIScale(lua_State *L)
{
	lua_pushnumber(L, instance()->getDPIScale());
	return 1;
}

int w_getNativeDPIScale(lua_State *L)
{
	lua_pushnumber(L, instance()->getNativeDPIScale());
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

int w_restore(lua_State *)
{
	instance()->restore();
	return 0;
}

int w_isMaximized(lua_State *L)
{
	luax_pushboolean(L, instance()->isMaximized());
	return 1;
}

int w_isMinimized(lua_State *L)
{
	luax_pushboolean(L, instance()->isMinimized());
	return 1;
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
			data.enterButtonIndex = (int) luaL_checkinteger(L, -1) - 1;
		else
			data.enterButtonIndex = 0;
		lua_pop(L, 1);

		// Optional table entry specifying the button to use when esc is pressed.
		lua_getfield(L, 3, "escapebutton");
		if (!lua_isnoneornil(L, -1))
			data.escapeButtonIndex = (int) luaL_checkinteger(L, -1) - 1;
		else
			data.escapeButtonIndex = (int) data.buttons.size() - 1;
		lua_pop(L, 1);

		const char *typestr = lua_isnoneornil(L, 4) ? nullptr : luaL_checkstring(L, 4);
		if (typestr && !Window::getConstant(typestr, data.type))
			return luax_enumerror(L, "messagebox type", Window::getConstants(data.type), typestr);

		data.attachToWindow = luax_optboolean(L, 5, true);

		int pressedbutton = instance()->showMessageBox(data);
		lua_pushinteger(L, pressedbutton + 1);
	}
	else
	{
		const char *typestr = lua_isnoneornil(L, 3) ? nullptr : luaL_checkstring(L, 3);
		if (typestr && !Window::getConstant(typestr, data.type))
			return luax_enumerror(L, "messagebox type", Window::getConstants(data.type), typestr);

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
	{ "updateMode", w_updateMode },
	{ "getMode", w_getMode },
	{ "getDisplayOrientation", w_getDisplayOrientation },
	{ "getFullscreenModes", w_getFullscreenModes },
	{ "setFullscreen", w_setFullscreen },
	{ "getFullscreen", w_getFullscreen },
	{ "isOpen", w_isOpen },
	{ "close", w_close },
	{ "getDesktopDimensions", w_getDesktopDimensions },
	{ "setPosition", w_setPosition },
	{ "getPosition", w_getPosition },
	{ "getSafeArea", w_getSafeArea },
	{ "setIcon", w_setIcon },
	{ "getIcon", w_getIcon },
	{ "setVSync", w_setVSync },
	{ "getVSync", w_getVSync },
	{ "setDisplaySleepEnabled", w_setDisplaySleepEnabled },
	{ "isDisplaySleepEnabled", w_isDisplaySleepEnabled },
	{ "setTitle", w_setTitle },
	{ "getTitle", w_getTitle },
	{ "hasFocus", w_hasFocus },
	{ "hasMouseFocus", w_hasMouseFocus },
	{ "isVisible", w_isVisible },
	{ "getDPIScale", w_getDPIScale },
	{ "getNativeDPIScale", w_getNativeDPIScale },
	{ "toPixels", w_toPixels },
	{ "fromPixels", w_fromPixels },
	{ "minimize", w_minimize },
	{ "maximize", w_maximize },
	{ "restore", w_restore },
	{ "isMaximized", w_isMaximized },
	{ "isMinimized", w_isMinimized },
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
	w.type = &Module::type;
	w.functions = functions;
	w.types = 0;

	return luax_register_module(L, w);
}

} // window
} // love
