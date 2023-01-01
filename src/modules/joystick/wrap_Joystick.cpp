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
#include "wrap_Joystick.h"
#include "wrap_JoystickModule.h"

#include <vector>

namespace love
{
namespace joystick
{

Joystick *luax_checkjoystick(lua_State *L, int idx)
{
	return luax_checktype<Joystick>(L, idx);
}

int w_Joystick_isConnected(lua_State *L)
{
	Joystick *j = luax_checkjoystick(L, 1);
	luax_pushboolean(L, j->isConnected());
	return 1;
}

int w_Joystick_getName(lua_State *L)
{
	Joystick *j = luax_checkjoystick(L, 1);
	lua_pushstring(L, j->getName());
	return 1;
}

int w_Joystick_getID(lua_State *L)
{
	Joystick *j = luax_checkjoystick(L, 1);

	// IDs are 1-based in Lua.
	lua_pushinteger(L, j->getID() + 1);

	int instanceid = j->getInstanceID();
	if (instanceid >= 0)
		lua_pushinteger(L, instanceid + 1);
	else
		lua_pushnil(L);

	return 2;
}

int w_Joystick_getGUID(lua_State *L)
{
	Joystick *j = luax_checkjoystick(L, 1);
	luax_pushstring(L, j->getGUID());
	return 1;
}

int w_Joystick_getDeviceInfo(lua_State *L)
{
	Joystick *j = luax_checkjoystick(L, 1);

	int vendorID = 0, productID = 0, productVersion = 0;
	j->getDeviceInfo(vendorID, productID, productVersion);

	lua_pushnumber(L, vendorID);
	lua_pushnumber(L, productID);
	lua_pushnumber(L, productVersion);

	return 3;
}

int w_Joystick_getAxisCount(lua_State *L)
{
	Joystick *j = luax_checkjoystick(L, 1);
	lua_pushinteger(L, j->getAxisCount());
	return 1;
}

int w_Joystick_getButtonCount(lua_State *L)
{
	Joystick *j = luax_checkjoystick(L, 1);
	lua_pushinteger(L, j->getButtonCount());
	return 1;
}

int w_Joystick_getHatCount(lua_State *L)
{
	Joystick *j = luax_checkjoystick(L, 1);
	lua_pushinteger(L, j->getHatCount());
	return 1;
}

int w_Joystick_getAxis(lua_State *L)
{
	Joystick *j = luax_checkjoystick(L, 1);
	int axisindex = (int) luaL_checkinteger(L, 2) - 1;
	lua_pushnumber(L, j->getAxis(axisindex));
	return 1;
}

int w_Joystick_getAxes(lua_State *L)
{
	Joystick *j = luax_checkjoystick(L, 1);
	std::vector<float> axes = j->getAxes();

	for (float value : axes)
		lua_pushnumber(L, value);

	return (int) axes.size();
}

int w_Joystick_getHat(lua_State *L)
{
	Joystick *j = luax_checkjoystick(L, 1);
	int hatindex = (int) luaL_checkinteger(L, 2) - 1;

	Joystick::Hat h = j->getHat(hatindex);

	const char *direction = "";
	Joystick::getConstant(h, direction);

	lua_pushstring(L, direction);
	return 1;
}

int w_Joystick_isDown(lua_State *L)
{
	Joystick *j = luax_checkjoystick(L, 1);

	bool istable = lua_istable(L, 2);
	int num = istable ? (int) luax_objlen(L, 2) : (lua_gettop(L) - 1);

	if (num == 0)
		luaL_checkinteger(L, 2);

	std::vector<int> buttons;
	buttons.reserve(num);

	if (istable)
	{
		for (int i = 0; i < num; i++)
		{
			lua_rawgeti(L, 2, i + 1);
			buttons.push_back((int) luaL_checkinteger(L, -1) - 1);
			lua_pop(L, 1);
		}
	}
	else
	{
		for (int i = 0; i < num; i++)
			buttons.push_back((int) luaL_checkinteger(L, i + 2) - 1);
	}

	luax_pushboolean(L, j->isDown(buttons));
	return 1;
}

int w_Joystick_isGamepad(lua_State *L)
{
	Joystick *j = luax_checkjoystick(L, 1);
	luax_pushboolean(L, j->isGamepad());
	return 1;
}

int w_Joystick_getGamepadAxis(lua_State *L)
{
	Joystick *j = luax_checkjoystick(L, 1);

	const char *str = luaL_checkstring(L, 2);
	Joystick::GamepadAxis axis;

	if (!joystick::Joystick::getConstant(str, axis))
		return luax_enumerror(L, "gamepad axis", str);

	lua_pushnumber(L, j->getGamepadAxis(axis));
	return 1;
}

int w_Joystick_isGamepadDown(lua_State *L)
{
	Joystick *j = luax_checkjoystick(L, 1);

	bool istable = lua_istable(L, 2);
	int num = istable ? (int) luax_objlen(L, 2) : (lua_gettop(L) - 1);

	if (num == 0)
		luaL_checkstring(L, 2);

	std::vector<Joystick::GamepadButton> buttons;
	buttons.reserve(num);

	Joystick::GamepadButton button;

	if (istable)
	{
		for (int i = 0; i < num; i++)
		{
			lua_rawgeti(L, 2, i + 1);
			const char *str = luaL_checkstring(L, -1);

			if (!joystick::Joystick::getConstant(str, button))
				return luax_enumerror(L, "gamepad button", str);

			buttons.push_back(button);

			lua_pop(L, 1);
		}
	}
	else
	{
		for (int i = 0; i < num; i++)
		{
			const char *str = luaL_checkstring(L, i + 2);

			if (!joystick::Joystick::getConstant(str, button))
				return luax_enumerror(L, "gamepad button", str);

			buttons.push_back(button);
		}
	}

	luax_pushboolean(L, j->isGamepadDown(buttons));
	return 1;
}

int w_Joystick_getGamepadMapping(lua_State *L)
{
	Joystick *j = luax_checkjoystick(L, 1);

	const char *gpbindstr = luaL_checkstring(L, 2);
	Joystick::GamepadInput gpinput;

	if (Joystick::getConstant(gpbindstr, gpinput.axis))
		gpinput.type = Joystick::INPUT_TYPE_AXIS;
	else if (Joystick::getConstant(gpbindstr, gpinput.button))
		gpinput.type = Joystick::INPUT_TYPE_BUTTON;
	else
		return luax_enumerror(L, "gamepad axis/button", gpbindstr);

	Joystick::JoystickInput jinput;
	jinput.type = Joystick::INPUT_TYPE_MAX_ENUM;

	luax_catchexcept(L, [&](){ jinput = j->getGamepadMapping(gpinput); });

	if (jinput.type == Joystick::INPUT_TYPE_MAX_ENUM)
		return 0;

	const char *inputtypestr;
	if (!Joystick::getConstant(jinput.type, inputtypestr))
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
		if (Joystick::getConstant(jinput.hat.value, hatstr))
		{
			lua_pushstring(L, hatstr);
			return 3;
		}
		else
			return luaL_error(L, "Unknown joystick hat.");
	default:
		return luaL_error(L, "Unknown joystick input type.");
	}
	
	return 1;
}

int w_Joystick_getGamepadMappingString(lua_State *L)
{
	Joystick *j = luax_checkjoystick(L, 1);
	std::string mapping = j->getGamepadMappingString();
	if (mapping.empty())
		lua_pushnil(L);
	else
		luax_pushstring(L, mapping);
	return 1;
}

int w_Joystick_isVibrationSupported(lua_State *L)
{
	Joystick *j = luax_checkjoystick(L, 1);
	luax_pushboolean(L, j->isVibrationSupported());
	return 1;
}

int w_Joystick_setVibration(lua_State *L)
{
	Joystick *j = luax_checkjoystick(L, 1);
	bool success = false;

	if (lua_isnoneornil(L, 2))
	{
		// Disable joystick vibration if no argument is given.
		success = j->setVibration();
	}
	else
	{
		float left = (float) luaL_checknumber(L, 2);
		float right = (float) luaL_optnumber(L, 3, left);
		float duration = (float) luaL_optnumber(L, 4, -1.0); // -1 is infinite.
		success = j->setVibration(left, right, duration);
	}

	luax_pushboolean(L, success);
	return 1;
}

int w_Joystick_getVibration(lua_State *L)
{
	Joystick *j = luax_checkjoystick(L, 1);
	float left, right;
	j->getVibration(left, right);
	lua_pushnumber(L, left);
	lua_pushnumber(L, right);
	return 2;
}

// List of functions to wrap.
static const luaL_Reg w_Joystick_functions[] =
{
	{ "isConnected", w_Joystick_isConnected },
	{ "getName", w_Joystick_getName },
	{ "getID", w_Joystick_getID },
	{ "getGUID", w_Joystick_getGUID },
	{ "getDeviceInfo", w_Joystick_getDeviceInfo },
	{ "getAxisCount", w_Joystick_getAxisCount },
	{ "getButtonCount", w_Joystick_getButtonCount },
	{ "getHatCount", w_Joystick_getHatCount },
	{ "getAxis", w_Joystick_getAxis },
	{ "getAxes", w_Joystick_getAxes },
	{ "getHat", w_Joystick_getHat },
	{ "isDown", w_Joystick_isDown },

	{ "isGamepad", w_Joystick_isGamepad },
	{ "getGamepadAxis", w_Joystick_getGamepadAxis },
	{ "isGamepadDown", w_Joystick_isGamepadDown },
	{ "getGamepadMapping", w_Joystick_getGamepadMapping },
	{ "getGamepadMappingString", w_Joystick_getGamepadMappingString },

	{ "isVibrationSupported", w_Joystick_isVibrationSupported },
	{ "setVibration", w_Joystick_setVibration },
	{ "getVibration", w_Joystick_getVibration },

	// From wrap_JoystickModule.
	{ "getConnectedIndex", w_getIndex },

	{ 0, 0 },
};

extern "C" int luaopen_joystick(lua_State *L)
{
	return luax_register_type(L, &Joystick::type, w_Joystick_functions, nullptr);
}

} // joystick
} // love
