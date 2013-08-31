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

#include "wrap_JoystickModule.h"
#include "wrap_Joystick.h"

namespace love
{
namespace joystick
{
namespace sdl
{

static JoystickModule *instance = 0;

int w_getJoysticks(lua_State *L)
{
	int stickcount = instance->getJoystickCount();
	lua_createtable(L, stickcount, 0);

	for (int i = 0; i < stickcount; i++)
	{
		love::joystick::Joystick *stick = instance->getJoystick(i);
		stick->retain();
		luax_pushtype(L, "Joystick", JOYSTICK_JOYSTICK_T, stick);
		lua_rawseti(L, -2, i + 1);
	}

	return 1;
}

int w_getIndex(lua_State *L)
{
	love::joystick::Joystick *j = luax_checkjoystick(L, 1);
	int index = instance->getIndex(j);
	if (index >= 0)
		lua_pushinteger(L, index + 1);
	else
		lua_pushnil(L);
	return 1;
}

int w_getJoystickCount(lua_State *L)
{
	lua_pushinteger(L, instance->getJoystickCount());
	return 1;
}

int w_setGamepadMapping(lua_State *L)
{
	// Only accept a GUID string. We don't accept a Joystick object because
	// the gamepad mapping applies to all joysticks with the same GUID (e.g. all
	// Xbox 360 controllers on the system), rather than individual objects.
	std::string guid = luax_checkstring(L, 1);

	const char *gpbindstr = luaL_checkstring(L, 2);
	Joystick::GamepadInput gpinput;

	if (love::joystick::Joystick::getConstant(gpbindstr, gpinput.axis))
		gpinput.type = Joystick::INPUT_TYPE_AXIS;
	else if (love::joystick::Joystick::getConstant(gpbindstr, gpinput.button))
		gpinput.type = Joystick::INPUT_TYPE_BUTTON;
	else
		return luaL_error(L, "Invalid gamepad axis/button: %s", gpbindstr);

	const char *jinputtypestr = luaL_checkstring(L, 3);
	Joystick::JoystickInput jinput;

	if (!love::joystick::Joystick::getConstant(jinputtypestr, jinput.type))
		return luaL_error(L, "Invalid joystick input type: %s", jinputtypestr);

	const char *hatstr;
	switch (jinput.type)
	{
	case Joystick::INPUT_TYPE_AXIS:
		jinput.axis = luaL_checkint(L, 4) - 1;
		break;
	case Joystick::INPUT_TYPE_BUTTON:
		jinput.button = luaL_checkint(L, 4) - 1;
		break;
	case Joystick::INPUT_TYPE_HAT:
		// Hats need both a hat index and a hat value.
		jinput.hat.index = luaL_checkint(L, 4) - 1;
		hatstr = luaL_checkstring(L, 5);
		if (!love::joystick::Joystick::getConstant(hatstr, jinput.hat.value))
			return luaL_error(L, "Invalid joystick hat: %s", hatstr);
		break;
	default:
		return luaL_error(L, "Invalid joystick input type: %s", jinputtypestr);
	}

	bool success = false;
	try
	{
		success = instance->setGamepadMapping(guid, gpinput, jinput);
	}
	catch (love::Exception &e)
	{
		return luaL_error(L, "%s", e.what());
	}
	luax_pushboolean(L, success);
	return 1;
}

int w_getGamepadMapping(lua_State *L)
{
	std::string guid;

	// Accept either a GUID string or a Joystick object. This way we can re-use
	// the function for Joystick:getGamepadMapping.
	if (lua_type(L, 1) == LUA_TSTRING)
		guid = luax_checkstring(L, 1);
	else
	{
		love::joystick::Joystick *stick = luax_checkjoystick(L, 1);
		guid = stick->getGUID();
	}

	const char *gpbindstr = luaL_checkstring(L, 2);
	Joystick::GamepadInput gpinput;

	if (love::joystick::Joystick::getConstant(gpbindstr, gpinput.axis))
		gpinput.type = Joystick::INPUT_TYPE_AXIS;
	else if (love::joystick::Joystick::getConstant(gpbindstr, gpinput.button))
		gpinput.type = Joystick::INPUT_TYPE_BUTTON;
	else
		return luaL_error(L, "Invalid gamepad axis/button: %s", gpbindstr);

	Joystick::JoystickInput jinput;
	jinput.type = Joystick::INPUT_TYPE_MAX_ENUM;

	try
	{
		jinput = instance->getGamepadMapping(guid, gpinput);
	}
	catch (love::Exception &e)
	{
		return luaL_error(L, "%s", e.what());
	}

	if (jinput.type == Joystick::INPUT_TYPE_MAX_ENUM)
		return 0;

	const char *inputtypestr;
	if (!love::joystick::Joystick::getConstant(jinput.type, inputtypestr))
		return luaL_error(L, "Unknown joystick input type.");

	lua_pushstring(L, inputtypestr);

	const char *hatstr;
	switch (jinput.type)
	{
	case Joystick::INPUT_TYPE_AXIS:
		lua_pushinteger(L, jinput.axis + 1);
		return 2;
	case Joystick::INPUT_TYPE_BUTTON:
		lua_pushinteger(L, jinput.button + 1);
		return 2;
	case Joystick::INPUT_TYPE_HAT:
		lua_pushinteger(L, jinput.hat.index + 1);
		if (love::joystick::Joystick::getConstant(jinput.hat.value, hatstr))
		{
			lua_pushstring(L, hatstr);
			return 3;
		}
		else
			return luaL_error(L, "Unknown joystick hat.");
	default:
		break; // ?
	}

	return 1;
}

// List of functions to wrap.
static const luaL_Reg functions[] =
{
	{ "getJoysticks", w_getJoysticks },
	{ "getJoystickCount", w_getJoystickCount },
	{ "setGamepadMapping", w_setGamepadMapping },
	{ "getGamepadMapping", w_getGamepadMapping },
	{ 0, 0 }
};

static const lua_CFunction types[] =
{
	luaopen_joystick,
	0,
};

extern "C" int luaopen_love_joystick(lua_State *L)
{
	if (instance == 0)
	{
		try
		{
			instance = new JoystickModule();
		}
		catch (Exception &e)
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
	w.types = types;

	return luax_register_module(L, w);
}

} // sdl
} // joystick
} // love
