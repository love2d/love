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

#include "Event.h"

namespace love
{
namespace event
{
namespace sdl
{
	const char * Event::getName() const
	{
		return "love.event.sdl";
	}

	void Event::pump()
	{
		SDL_PumpEvents();
	}

	int Event::poll(lua_State * L)
	{
		lua_pushcclosure(L, &poll_i, 0);
		return 1;
	}

	int Event::wait(lua_State * L)
	{
		static SDL_Event e;
		SDL_WaitEvent(&e);
		return pushEvent(L, e); 
	}

	void Event::quit()
	{
		SDL_Event e;
		e.type = Event::EVENT_QUIT;
		SDL_PushEvent(&e);
	}

	int Event::push(lua_State * L)
	{
		SDL_Event e;
		getEvent(L, e);
		SDL_PushEvent(&e);
		return 0;
	}

	int Event::poll_i(lua_State * L)
	{
		SDL_EnableUNICODE(1);

		// The union used to get SDL events. 
		static SDL_Event e;

		// Get ONE event.
		while(SDL_PollEvent(&e))
		{
			int args = Event::pushEvent(L, e);
			if(args > 0)
				return args;
		}

		// No pending events.
		return 0;
	}

	int Event::pushEvent(lua_State * L, SDL_Event & e)
	{
		switch(e.type)
		{
		case SDL_KEYDOWN:
			lua_pushinteger(L, e.type);
			lua_pushinteger(L, e.key.keysym.sym);
			lua_pushinteger(L, e.key.keysym.unicode);
			return 3;
		case SDL_KEYUP:
			lua_pushinteger(L, e.type);
			lua_pushinteger(L, e.key.keysym.sym);
			return 2;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			lua_pushinteger(L, e.type);
			lua_pushinteger(L, e.button.x);
			lua_pushinteger(L, e.button.y);
			lua_pushinteger(L, e.button.button);
			return 4;
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			lua_pushinteger(L, e.type);
			lua_pushinteger(L, e.jbutton.which);
			lua_pushinteger(L, e.jbutton.button);
			return 3;
		case SDL_QUIT:
			lua_pushinteger(L, e.type);
			return 1;
		default:
			break;
		}

		return 0;
	}

	int Event::getEvent(lua_State * L, SDL_Event & e)
	{
		int type = luaL_checkint(L, 1);

		switch(type)
		{
		case EVENT_KEYDOWN:
			e.type = type;
			e.key.keysym.sym = (SDLKey)luaL_checkint(L, 2);
			e.key.keysym.unicode = luaL_checkint(L, 3);
			return 3;
		case EVENT_KEYUP:
			e.type = type;
			e.key.keysym.sym = (SDLKey)luaL_checkint(L, 2);
			return 2;
		case EVENT_MOUSEBUTTONDOWN:
		case EVENT_MOUSEBUTTONUP:
			e.type = type;
			e.button.x = luaL_checkint(L, 2);
			e.button.y = luaL_checkint(L, 3);
			e.button.button = luaL_checkint(L, 4);
			return 4;
		case EVENT_JOYBUTTONDOWN:
		case EVENT_JOYBUTTONUP:
			e.type = type;
			e.jbutton.which = luaL_checkint(L, 2);
			e.jbutton.button = luaL_checkint(L, 3);
			return 3;
		case EVENT_QUIT:
			e.type = type;
			return 1;
		default:
			e.type = EVENT_NOEVENT;
			break;
		}

		return 0;
	}

} // sdl
} // event
} // love
