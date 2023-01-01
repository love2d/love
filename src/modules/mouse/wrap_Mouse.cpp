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

// LOVE
#include "wrap_Mouse.h"
#include "wrap_Cursor.h"
#include "common/config.h"

#include "sdl/Mouse.h"
#include "filesystem/File.h"

namespace love
{
namespace mouse
{

#define instance() (Module::getInstance<Mouse>(Module::M_MOUSE))

int w_newCursor(lua_State *L)
{
	Cursor *cursor = nullptr;

	if (lua_isstring(L, 1) || luax_istype(L, 1, love::filesystem::File::type) || luax_istype(L, 1, love::filesystem::FileData::type))
		luax_convobj(L, 1, "image", "newImageData");

	love::image::ImageData *data = luax_checktype<love::image::ImageData>(L, 1);
	int hotx = (int) luaL_optinteger(L, 2, 0);
	int hoty = (int) luaL_optinteger(L, 3, 0);

	luax_catchexcept(L, [&](){ cursor = instance()->newCursor(data, hotx, hoty); });

	luax_pushtype(L, cursor);
	cursor->release();
	return 1;
}

int w_getSystemCursor(lua_State *L)
{
	const char *str = luaL_checkstring(L, 1);
	Cursor::SystemCursor systemCursor;

	if (!Cursor::getConstant(str, systemCursor))
		return luax_enumerror(L, "system cursor type", str);

	Cursor *cursor = 0;
	luax_catchexcept(L, [&](){ cursor = instance()->getSystemCursor(systemCursor); });

	luax_pushtype(L, cursor);
	return 1;
}

int w_setCursor(lua_State *L)
{
	// Revert to the default system cursor if no argument is given.
	if (lua_isnoneornil(L, 1))
	{
		instance()->setCursor();
		return 0;
	}

	Cursor *cursor = luax_checkcursor(L, 1);
	instance()->setCursor(cursor);
	return 0;
}

int w_getCursor(lua_State *L)
{
	Cursor *cursor = instance()->getCursor();

	if (cursor)
		luax_pushtype(L, cursor);
	else
		lua_pushnil(L);

	return 1;
}

int w_isCursorSupported(lua_State *L)
{
	luax_pushboolean(L, instance()->isCursorSupported());
	return 1;
}

int w_getX(lua_State *L)
{
	lua_pushnumber(L, instance()->getX());
	return 1;
}

int w_getY(lua_State *L)
{
	lua_pushnumber(L, instance()->getY());
	return 1;
}

int w_getPosition(lua_State *L)
{
	double x, y;
	instance()->getPosition(x, y);
	lua_pushnumber(L, x);
	lua_pushnumber(L, y);
	return 2;
}

int w_setX(lua_State *L)
{
	double x = luaL_checknumber(L, 1);
	instance()->setX(x);
	return 0;
}

int w_setY(lua_State *L)
{
	double y = luaL_checknumber(L, 1);
	instance()->setY(y);
	return 0;
}

int w_setPosition(lua_State *L)
{
	double x = luaL_checknumber(L, 1);
	double y = luaL_checknumber(L, 2);
	instance()->setPosition(x, y);
	return 0;
}

int w_isDown(lua_State *L)
{
	bool istable = lua_istable(L, 1);
	int num = istable ? (int) luax_objlen(L, 1) : lua_gettop(L);

	std::vector<int> buttons;
	buttons.reserve(num);

	if (istable)
	{
		for (int i = 0; i < num; i++)
		{
			lua_rawgeti(L, 1, i + 1);
			buttons.push_back((int) luaL_checkinteger(L, -1));
			lua_pop(L, 1);
		}
	}
	else
	{
		for (int i = 0; i < num; i++)
			buttons.push_back((int) luaL_checkinteger(L, i + 1));
	}

	luax_pushboolean(L, instance()->isDown(buttons));
	return 1;
}

int w_setVisible(lua_State *L)
{
	bool b = luax_checkboolean(L, 1);
	instance()->setVisible(b);
	return 0;
}

int w_isVisible(lua_State *L)
{
	luax_pushboolean(L, instance()->isVisible());
	return 1;
}

int w_setGrabbed(lua_State *L)
{
	bool b = luax_checkboolean(L, 1);
	instance()->setGrabbed(b);
	return 0;
}

int w_isGrabbed(lua_State *L)
{
	luax_pushboolean(L, instance()->isGrabbed());
	return 1;
}

int w_setRelativeMode(lua_State *L)
{
	bool relative = luax_checkboolean(L, 1);
	luax_pushboolean(L, instance()->setRelativeMode(relative));
	return 1;
}

int w_getRelativeMode(lua_State *L)
{
	luax_pushboolean(L, instance()->getRelativeMode());
	return 1;
}

// List of functions to wrap.
static const luaL_Reg functions[] =
{
	{ "newCursor", w_newCursor },
	{ "getSystemCursor", w_getSystemCursor },
	{ "setCursor", w_setCursor },
	{ "getCursor", w_getCursor },
	{ "isCursorSupported", w_isCursorSupported },
	{ "getX", w_getX },
	{ "getY", w_getY },
	{ "setX", w_setX },
	{ "setY", w_setY },
	{ "setPosition", w_setPosition },
	{ "isDown", w_isDown },
	{ "setVisible", w_setVisible },
	{ "isVisible", w_isVisible },
	{ "getPosition", w_getPosition },
	{ "setGrabbed", w_setGrabbed },
	{ "isGrabbed", w_isGrabbed },
	{ "setRelativeMode", w_setRelativeMode },
	{ "getRelativeMode", w_getRelativeMode },
	{ 0, 0 }
};

// Types for this module.
static const lua_CFunction types[] =
{
	luaopen_cursor,
	0,
};

extern "C" int luaopen_love_mouse(lua_State *L)
{
	Mouse *instance = instance();
	if (instance == nullptr)
	{
		luax_catchexcept(L, [&](){ instance = new love::mouse::sdl::Mouse(); });
	}
	else
		instance->retain();

	WrappedModule w;
	w.module = instance;
	w.name = "mouse";
	w.type = &Module::type;
	w.functions = functions;
	w.types = types;

	return luax_register_module(L, w);
}

} // mouse
} // love
