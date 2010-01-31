/**
* Copyright (c) 2006-2010 LOVE Development Team
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
	static Keyboard * instance;

	int w_isDown(lua_State * L)
	{
		Keyboard::Key k;

		if(Keyboard::getConstant(luaL_checkstring(L, 1), k))
		{
			luax_pushboolean(L, instance->isDown(k));
		}
		else
		{
			luax_pushboolean(L, false);
		}

		return 1;
	}

	int w_setKeyRepeat(lua_State * L)
	{
		if(lua_gettop(L) == 0)
		{
			// Disables key repeat.
			instance->setKeyRepeat(0, 0);
			return 0;
		}

		instance->setKeyRepeat(luaL_optint(L, 1, Keyboard::DEFAULT), luaL_optint(L, 2, Keyboard::DEFAULT));
		return 0;
	}

	int w_getKeyRepeat(lua_State * L)
	{
		lua_pushnumber(L, instance->getKeyRepeatDelay());
		lua_pushnumber(L, instance->getKeyRepeatInterval());
		return 2;
	}

	// List of functions to wrap.
	static const luaL_Reg functions[] = {
		{ "isDown", w_isDown },
		{ "setKeyRepeat", w_setKeyRepeat },
		{ "getKeyRepeat", w_getKeyRepeat },
		{ 0, 0 }
	};

	int luaopen_love_keyboard(lua_State * L)
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

		WrappedModule w;
		w.module = instance;
		w.name = "keyboard";
		w.flags = MODULE_T;
		w.functions = functions;
		w.types = 0;

		return luax_register_module(L, w);
	}

} // sdl
} // keyboard
} // love
