/**
 * Copyright (c) 2006-2015 LOVE Development Team
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

#include "sdl/Event.h"

// Put the Lua code directly into a raw string literal.
static const char event_lua[] =
#include "wrap_Event.lua"
;

namespace love
{
namespace event
{

#define instance() (Module::getInstance<Event>(Module::M_EVENT))

static const char *listenersKey = "_listeners";

static int processMessage(lua_State *L, Message *m)
{
	if (m == nullptr)
		return 0;

	int args = m->toLua(L);

	// Push the love.event[listenersKey][eventname] table onto the stack.
	lua_getfield(L, lua_upvalueindex(1), listenersKey);
	lua_getfield(L, -1, m->getName().c_str());

	if (lua_istable(L, -1))
	{
		int len = (int) luax_objlen(L, -1);

		// Each array entry in the table is a listener function we should call.
		for (int i = 1; i <= len; i++)
		{
			lua_rawgeti(L, -1, i);

			// Push all message args (including the event name), in order.
			for (int j = 1; j <= args; j++)
				lua_pushvalue(L, -(args + 3));

			lua_call(L, args, 0);
		}
	}

	lua_pop(L, 2);

	m->release();

	// Leave the message's args on the stack.
	return args;
}

static int w_poll_i(lua_State *L)
{
	Message *m = nullptr;

	if (instance()->poll(m))
		return processMessage(L, m);

	// No pending events.
	return 0;
}

int w_poll(lua_State *L)
{
	// w_poll_i needs access to the love.event table via an upvalue.
	lua_pushvalue(L, lua_upvalueindex(1));
	lua_pushcclosure(L, w_poll_i, 1);
	return 1;
}

int w_pump(lua_State *)
{
	instance()->pump();
	return 0;
}

int w_wait(lua_State *L)
{
	Message *m = instance()->wait();
	return processMessage(L, m);
}

int w_push(lua_State *L)
{
	Message *m = Message::fromLua(L, 1);

	luax_pushboolean(L, m != nullptr);

	if (m == nullptr)
		return 1;

	instance()->push(m);
	m->release();

	return 1;
}

int w_clear(lua_State *)
{
	instance()->clear();
	return 0;
}

int w_quit(lua_State *L)
{
	std::vector<StrongRef<Variant>> args;

	Variant *v = Variant::fromLua(L, 1);
	if (v)
	{
		args.push_back(v);
		v->release();
	}

	Message *m = new Message("quit", args);
	instance()->push(m);
	m->release();

	luax_pushboolean(L, true);
	return 1;
}

// List of functions to wrap.
static const luaL_Reg functions[] =
{
	// addListener and removeListener are defined in wrap_Event.lua
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
	Event *instance = instance();
	if (instance == nullptr)
	{
		luax_catchexcept(L, [&](){ instance = new love::event::sdl::Event(); });
	}
	else
		instance->retain();

	WrappedModule w;
	w.module = instance;
	w.name = "event";
	w.type = MODULE_ID;
	w.functions = functions;
	w.types = nullptr;

	int ret = luax_register_module(L, w);

	// love.event[listenersKey] = {}
	lua_newtable(L);
	lua_setfield(L, -2, listenersKey);

	// Execute wrap_Event.lua, sending the event and listeners tables as args.
	luaL_loadbuffer(L, event_lua, sizeof(event_lua), "wrap_Event.lua");
	lua_pushvalue(L, -2);
	lua_getfield(L, -1, listenersKey);
	lua_call(L, 2, 0);

	return ret;
}

} // event
} // love
