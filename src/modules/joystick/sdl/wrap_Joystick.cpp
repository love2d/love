/**
* Copyright (c) 2006-2009 LOVE Development Team
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
	static Joystick * instance = 0;

	int _wrap_getNumJoysticks(lua_State * L)
	{
		lua_pushinteger(L, instance->getNumJoysticks());
		return 1;
	}

	int _wrap_getName(lua_State * L)
	{
		int index = luaL_checkint(L, 1);
		lua_pushstring(L, instance->getName(index));
		return 1;
	}
	
	int _wrap_open(lua_State * L)
	{
		int index = luaL_checkint(L, 1);
		luax_pushboolean(L, instance->open(index));
		return 1;
	}

	int _wrap_isOpen(lua_State * L)
	{
		int index = luaL_checkint(L, 1);
		luax_pushboolean(L, instance->isOpen(index));
		return 1;
	}

	int _wrap_getNumAxes(lua_State * L)
	{
		int index = luaL_checkint(L, 1);
		lua_pushinteger(L, instance->getNumAxes(index));
		return 1;
	}

	int _wrap_getNumBalls(lua_State * L)
	{
		int index = luaL_checkint(L, 1);
		lua_pushinteger(L, instance->getNumBalls(index));
		return 1;
	}

	int _wrap_getNumButtons(lua_State * L)
	{
		int index = luaL_checkint(L, 1);
		lua_pushinteger(L, instance->getNumButtons(index));
		return 1;
	}

	int _wrap_getNumHats(lua_State * L)
	{
		int index = luaL_checkint(L, 1);
		lua_pushinteger(L, instance->getNumHats(index));
		return 1;
	}

	int _wrap_getAxis(lua_State * L)
	{
		int index = luaL_checkint(L, 1);
		int axis = luaL_checkint(L, 2);
		lua_pushnumber(L, instance->getAxis(index, axis));
		return 1;
	}

	int _wrap_getAxes(lua_State * L)
	{
		return instance->getAxes(L);
	}

	int _wrap_getBall(lua_State * L)
	{
		return instance->getBall(L);
	}

	int _wrap_isDown(lua_State * L)
	{
		int index = luaL_checkint(L, 1);
		int button = luaL_checkint(L, 2);
		luax_pushboolean(L, instance->isDown(index, button));
		return 1;
	}

	int _wrap_getHat(lua_State * L)
	{
		int index = luaL_checkint(L, 1);
		int hat = luaL_checkint(L, 2);
		lua_pushinteger(L, instance->getHat(index, hat));
		return 1;
	}

	int _wrap_close(lua_State * L)
	{
		int index = luaL_checkint(L, 1);
		instance->close(index);
		return 0;
	}
	
	// List of functions to wrap.
	static const luaL_Reg wrap_Joystick_functions[] = {
		{ "getNumJoysticks", _wrap_getNumJoysticks },
		{ "getName", _wrap_getName },
		{ "open", _wrap_open },
		{ "isOpen", _wrap_isOpen },
		{ "getNumAxes", _wrap_getNumAxes },
		{ "getNumBalls", _wrap_getNumBalls },
		{ "getNumButtons", _wrap_getNumButtons },
		{ "getNumHats", _wrap_getNumHats },
		{ "getAxis", _wrap_getAxis },

		{ "getAxes", _wrap_getAxes },
		{ "getBall", _wrap_getBall },

		{ "isDown", _wrap_isDown },
		{ "getHat", _wrap_getHat },
		{ "close", _wrap_close },
		{ 0, 0 }
	};

	int wrap_Joystick_open(lua_State * L)
	{

		if(instance == 0)
		{
			try 
			{
				instance = new Joystick();
			} 
			catch(Exception & e)
			{
				return luaL_error(L, e.what());
			}
		}

		luax_register_gc(L, "love.joystick", instance);

		return luax_register_module(L, wrap_Joystick_functions, 0);
	}

} // sdl
} // joystick
} // love
