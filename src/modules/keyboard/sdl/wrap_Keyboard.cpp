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

#include "wrap_Keyboard.h"

namespace love
{
namespace keyboard
{
namespace sdl
{
	static Keyboard * instance = 0;

	int _wrap_isDown(lua_State * L)
	{
		int b = luaL_checkint(L, 1);
		luax_pushboolean(L, instance->isDown(b));
		return 1;
	}

	int _wrap_setKeyRepeat(lua_State * L)
	{
		if(lua_gettop(L) == 0)
		{
			instance->setKeyRepeat();
			return 0;
		}

		int a = luaL_checkint(L, 1);
		int b = luaL_checkint(L, 2);
		instance->setKeyRepeat(a, b);
		return 0;
	}

	int _wrap_getKeyRepeat(lua_State * L)
	{
		lua_pushnumber(L, instance->getKeyRepeatDelay());
		lua_pushnumber(L, instance->getKeyRepeatInterval());
		return 2;
	}
	
	// List of functions to wrap.
	static const luaL_Reg wrap_Keyboard_functions[] = {
		{ "isDown", _wrap_isDown },
		{ "setKeyRepeat", _wrap_setKeyRepeat },
		{ "getKeyRepeat", _wrap_getKeyRepeat },
		{ 0, 0 }
	};

	int wrap_Keyboard_open(lua_State * L)
	{
		if(instance == 0)
		{
			try 
			{
				instance = new Keyboard();
			} 
			catch(Exception & e)
			{
				return luaL_error(L, e.what());
			}
		}

		luax_register_gc(L, "love.keyboard", instance);

		return luax_register_module(L, wrap_Keyboard_functions, 0, "keyboard");
	}

} // sdl
} // keyboard
} // love
