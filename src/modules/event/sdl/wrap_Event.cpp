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

	static bool to_message(lua_State * L, Event::Message & msg)
	{
		const char * str = luaL_checkstring(L, 1);

		if(!Event::getConstant(str, msg.type))
			return false;

		switch(msg.type)
		{
		case Event::TYPE_KEY_PRESSED:
			if(!Event::getConstant(luaL_checkstring(L, 2), msg.keyboard.k))
				return false;
			msg.keyboard.u = (unsigned short)luaL_optint(L, 3, 0);
			return true;
		case Event::TYPE_KEY_RELEASED:
			if(!Event::getConstant(luaL_checkstring(L, 2), msg.keyboard.k))
				return false;
			return true;
		case Event::TYPE_MOUSE_PRESSED:
		case Event::TYPE_MOUSE_RELEASED:
			if(!Event::getConstant(luaL_checkstring(L, 2), msg.mouse.b))
				return false;
			msg.mouse.x = luaL_checkint(L, 3);
			msg.mouse.y = luaL_checkint(L, 4);
			return true;
		case Event::TYPE_JOYSTICK_PRESSED:
		case Event::TYPE_JOYSTICK_RELEASED:
			msg.joystick.index = luaL_checkint(L, 2);
			msg.joystick.button = luaL_checkint(L, 3);
			return true;
		case Event::TYPE_QUIT:
			return true;
		default:
			return false;
		}

		return false;
	}

	static int push_message(lua_State * L, const Event::Message & msg)
	{
		const char * str = 0;

		if(!Event::getConstant(msg.type, str))
			return 0;

		lua_pushstring(L, str);

		switch(msg.type)
		{
		case Event::TYPE_KEY_PRESSED:
			if(!Event::getConstant(msg.keyboard.k, str))
				return 0;
			lua_pushstring(L, str);
			lua_pushinteger(L, msg.keyboard.u);
			return 3;
		case Event::TYPE_KEY_RELEASED:
			if(!Event::getConstant(msg.keyboard.k, str))
				return 0;
			lua_pushstring(L, str);
			return 2;
		case Event::TYPE_MOUSE_PRESSED:
		case Event::TYPE_MOUSE_RELEASED:
			if(!Event::getConstant(msg.mouse.b, str))
				return 0;
			lua_pushinteger(L, msg.mouse.x);
			lua_pushinteger(L, msg.mouse.y);
			lua_pushstring(L, str);
			return 4;
		case Event::TYPE_JOYSTICK_PRESSED:
		case Event::TYPE_JOYSTICK_RELEASED:
			lua_pushinteger(L, msg.joystick.index);
			lua_pushinteger(L, msg.joystick.button);
			return 3;
		case Event::TYPE_QUIT:
			return 1;
		default:
			return 0;
		}

		return 0;
	}

	static int poll_i(lua_State * L)
	{
		static Event::Message m;

		while(instance->poll(m))
		{
			int args = push_message(L, m);
			if(args > 0)
				return args;
		}

		// No pending events.
		return 0;
	}

	int w_pump(lua_State * L)
	{
		instance->pump();
		return 0;
	}

	int w_poll(lua_State * L)
	{
		lua_pushcclosure(L, &poll_i, 0);
		return 1;
	}

	int w_wait(lua_State * L)
	{
		static Event::Message m;

		if(instance->wait(m))
		{
			return push_message(L, m);
		}

		return 0;
	}

	int w_push(lua_State * L)
	{
		static Event::Message m;

		if(!to_message(L, m))
		{
			luax_pushboolean(L, false);
			return 1;
		}

		luax_pushboolean(L, instance->push(m));

		return 1;
	}

	// List of functions to wrap.
	static const luaL_Reg functions[] = {
		{ "pump", w_pump },
		{ "poll", w_poll },
		{ "wait", w_wait },
		{ "push", w_push },
		{ 0, 0 }
	};

	int luaopen_love_event(lua_State * L)
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
		else
			instance->retain();

		WrappedModule w;
		w.module = instance;
		w.name = "event";
		w.flags = MODULE_T;
		w.functions = functions;
		w.types = 0;

		return luax_register_module(L, w);
	}

} // sdl
} // event
} // love
