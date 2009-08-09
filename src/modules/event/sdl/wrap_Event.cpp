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
	
	int w_pump(lua_State * L)
	{
		instance->pump();
		return 0;
	}

	int w_poll(lua_State * L)
	{
		return instance->poll(L);
	}

	int w_wait(lua_State * L)
	{
		return instance->wait(L);
	}

	int w_quit(lua_State * L)
	{
		instance->quit();
		return 0;
	}

	int w_push(lua_State * L)
	{
		return instance->push(L);
	}

	int luaopen_love_event(lua_State * L)
	{
		// List of functions to wrap.
		static const luaL_Reg functions[] = {
			{ "pump", w_pump }, 
			{ "poll", w_poll }, 
			{ "wait", w_wait }, 
			{ "quit", w_quit }, 
			{ "push", w_push }, 
			{ 0, 0 }
		};

		// List of constants.
		static const LuaConstant constants[] = {
			{ "event_keypressed", Event::EVENT_KEYDOWN },
			{ "event_keyreleased", Event::EVENT_KEYUP },
			{ "event_mousepressed", Event::EVENT_MOUSEBUTTONDOWN },
			{ "event_mousereleased", Event::EVENT_MOUSEBUTTONUP },
			{ "event_joystickpressed", Event::EVENT_JOYBUTTONDOWN },
			{ "event_joystickreleased", Event::EVENT_JOYBUTTONUP },
			{ "event_quit", Event::EVENT_QUIT },
			{ 0, 0 }
		};
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

		luax_register_gc(L, instance);

		return luax_register_module(L, functions, 0, constants, "event");
	}

} // sdl
} // event
} // love
