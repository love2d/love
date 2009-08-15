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

#include "wrap_Signal.h"

// LOVE
#include <common/runtime.h>

#include "Signal.h"

namespace love
{
namespace signal
{
namespace posix
{
	static Signal * instance = 0;
	
	int w_hook(lua_State *L)
	{
		luaL_argcheck(L, lua_isnumber(L, 1), 1, "Expected number");
		lua_pushboolean(L, instance->hook(lua_tointeger(L, 1)));
		return 1;
	}
	
	int w_setCallback(lua_State *L)
	{
		luaL_argcheck(L, lua_isfunction(L, 1), 1, "Expected function");
		instance->setCallback(L);
		return 0;
	}
	
	int w_raise(lua_State *L)
	{
		luaL_argcheck(L, lua_isnumber(L, 1), 1, "Expected number");
		lua_pushboolean(L, instance->raise(lua_tointeger(L, 1)));
		return 1;
	}

	// List of functions to wrap.
	static const luaL_Reg functions[] = {
		{ "hook", w_hook },
		{ "setCallback", w_setCallback }, 
		{ "raise", w_raise },
		{ 0, 0 }
	};
	
	static const LuaConstant constants[] = {
		{ "signal_abrt", SIGABRT },
		{ "signal_fpe", SIGFPE },
		{ "signal_ill", SIGILL },
		{ "signal_int", SIGINT },
		{ "signal_segv", SIGSEGV },
		{ "signal_term", SIGTERM },
		{ 0, 0 }
	};

	int luaopen_love_signal(lua_State * L)
	{
		if(instance == 0)
		{
			try 
			{
				instance = new Signal();
			} 
			catch(Exception & e)
			{
				return luaL_error(L, e.what());
			}
		}

		luax_register_gc(L, instance);

		return luax_register_module(L, functions, 0, constants, "signal");
	}

} // posix
} // signal
} // love
