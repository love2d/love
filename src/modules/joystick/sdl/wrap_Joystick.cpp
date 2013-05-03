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

#include "wrap_Joystick.h"

namespace love
{
namespace joystick
{
namespace sdl
{

static Joystick *instance = 0;

int w_reload(lua_State *L)
{
	try
	{
		instance->reload();
	}
	catch(love::Exception &e)
	{
		return luaL_error(L, "%s", e.what());
	}
	return 0;
}

int w_getJoystickCount(lua_State *L)
{
	lua_pushinteger(L, instance->getJoystickCount());
	return 1;
}

int w_getName(lua_State *L)
{
	int index = luaL_checkint(L, 1)-1;
	lua_pushstring(L, instance->getName(index));
	return 1;
}

int w_getAxisCount(lua_State *L)
{
	int index = luaL_checkint(L, 1)-1;
	lua_pushinteger(L, instance->getAxisCount(index));
	return 1;
}

int w_getBallCount(lua_State *L)
{
	int index = luaL_checkint(L, 1)-1;
	lua_pushinteger(L, instance->getBallCount(index));
	return 1;
}

int w_getButtonCount(lua_State *L)
{
	int index = luaL_checkint(L, 1)-1;
	lua_pushinteger(L, instance->getButtonCount(index));
	return 1;
}

int w_getHatCount(lua_State *L)
{
	int index = luaL_checkint(L, 1)-1;
	lua_pushinteger(L, instance->getHatCount(index));
	return 1;
}

int w_getAxis(lua_State *L)
{
	int index = luaL_checkint(L, 1)-1;
	int axis = luaL_checkint(L, 2)-1;
	lua_pushnumber(L, instance->getAxis(index, axis));
	return 1;
}

int w_getAxes(lua_State *L)
{
	return instance->getAxes(L);
}

int w_getBall(lua_State *L)
{
	return instance->getBall(L);
}

int w_isDown(lua_State *L)
{
	int index = luaL_checkint(L, 1)-1;
	unsigned int num = lua_gettop(L);
	int *buttonlist = new int[num];
	unsigned int counter = 0;

	for (unsigned int i = 1; i < num; i++)
	{
		buttonlist[counter++] = (int) luaL_checknumber(L, i+1)-1;
	}
	buttonlist[counter] = -1;

	luax_pushboolean(L, instance->isDown(index, buttonlist));
	delete[] buttonlist;
	return 1;
}

int w_getHat(lua_State *L)
{
	int index = luaL_checkint(L, 1)-1;
	int hat = luaL_checkint(L, 2)-1;

	Joystick::Hat h = instance->getHat(index, hat);

	const char *direction = "";
	Joystick::getConstant(h, direction);
	lua_pushstring(L, direction);

	return 1;
}

// List of functions to wrap.
static const luaL_Reg functions[] =
{
	{ "reload", w_reload },
	{ "getJoystickCount", w_getJoystickCount },
	{ "getName", w_getName },
	{ "getAxisCount", w_getAxisCount },
	{ "getBallCount", w_getBallCount },
	{ "getButtonCount", w_getButtonCount },
	{ "getHatCount", w_getHatCount },
	{ "getAxis", w_getAxis },

	{ "getAxes", w_getAxes },
	{ "getBall", w_getBall },

	{ "isDown", w_isDown },
	{ "getHat", w_getHat },
	{ 0, 0 }
};

extern "C" int luaopen_love_joystick(lua_State *L)
{
	if (instance == 0)
	{
		try
		{
			instance = new Joystick();
		}
		catch(Exception &e)
		{
			return luaL_error(L, e.what());
		}
	}
	else
		instance->retain();


	WrappedModule w;
	w.module = instance;
	w.name = "joystick";
	w.flags = MODULE_T;
	w.functions = functions;
	w.types = 0;

	return luax_register_module(L, w);
}

} // sdl
} // joystick
} // love
