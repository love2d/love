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

#include "wrap_Event.h"

// LOVE
#include <common/runtime.h>

// sdlevent
#include "Event.h"

namespace love
{
namespace event
{
namespace sdl
{
	static Event * instance = 0;
	
	int _wrap_pump(lua_State * L)
	{
		instance->pump();
		return 0;
	}

	int _wrap_poll(lua_State * L)
	{
		return instance->poll(L);
	}

	int _wrap_wait(lua_State * L)
	{
		return instance->wait(L);
	}

	int _wrap_quit(lua_State * L)
	{
		instance->quit();
		return 0;
	}

	int _wrap_push(lua_State * L)
	{
		return instance->push(L);
	}

	// List of functions to wrap.
	static const luaL_Reg wrap_Event_functions[] = {
		{ "pump", _wrap_pump }, 
		{ "poll", _wrap_poll }, 
		{ "wait", _wrap_wait }, 
		{ "quit", _wrap_quit }, 
		{ "push", _wrap_push }, 
		{ 0, 0 }
	};

	int wrap_Event_open(lua_State * L)
	{
		if(instance == 0)
		{
			try 
			{
				instance = new Event();
			} 
			catch(Exception & e)
			{
				return luaL_error(L, e.what());
			}
		}

		luax_register_gc(L, "love.event", instance);

		return luax_register_module(L, wrap_Event_functions, 0, "event");
	}

} // sdl
} // event
} // love
