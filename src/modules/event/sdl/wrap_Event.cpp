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

#include "wrap_Event.h"

// LOVE
#include "common/runtime.h"

// sdlevent
#include "Event.h"

namespace love
{
namespace event
{
namespace sdl
{

static Event *instance = 0;

static int poll_i(lua_State *L)
{
	Message *m;

	while (instance->poll(m))
	{
		int args = m->toLua(L);
		m->release();
		return args;
	}

	// No pending events.
	return 0;
}

int w_pump(lua_State *)
{
	instance->pump();
	return 0;
}

int w_poll(lua_State *L)
{
	lua_pushcclosure(L, &poll_i, 0);
	return 1;
}

int w_wait(lua_State *L)
{
	static Message *m;

	if ((m = instance->wait()))
	{
		int args = m->toLua(L);
		m->release();
		return args;
	}

	return 0;
}

int w_push(lua_State *L)
{
	static Message *m;

	bool success = (m = Message::fromLua(L, 1)) != NULL;
	luax_pushboolean(L, success);

	if (!success)
		return 1;

	instance->push(m);
	m->release();

	return 1;
}

int w_clear(lua_State *)
{
	instance->clear();
	return 0;
}

int w_quit(lua_State *L)
{
	Message *m = new Message("quit");
	instance->push(m);
	m->release();
	luax_pushboolean(L, true);
	return 1;
}

// List of functions to wrap.
static const luaL_Reg functions[] =
{
	{ "pump", w_pump },
	{ "poll", w_poll },
	{ "wait", w_wait },
	{ "push", w_push },
	{ "clear", w_clear },
	{ "quit", w_quit },
	{ 0, 0 }
};

extern "C" int luaopen_love_event(lua_State *L)
{
	if (instance == 0)
	{
		try
		{
			instance = new Event();
		}
		catch (love::Exception &e)
		{
			return luaL_error(L, "%s", e.what());
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
