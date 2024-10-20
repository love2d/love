/**
 * Copyright (c) 2006-2024 LOVE Development Team
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

#include <algorithm>

// Shove the wrap_Event.lua code directly into a raw string literal.
static const char event_lua[] =
#include "wrap_Event.lua"
;

namespace love
{
namespace event
{

#define instance() (Module::getInstance<Event>(Module::M_EVENT))

static int luax_pushmessage(lua_State *L, const Message &m)
{
	luax_pushstring(L, m.name);

	for (const Variant &v : m.args)
		luax_pushvariant(L, v);

	return (int) m.args.size() + 1;
}

static int w_poll_i(lua_State *L)
{
	Message *m = nullptr;

	if (instance()->poll(m) && m != nullptr)
	{
		int args = luax_pushmessage(L, *m);
		m->release();
		return args;
	}

	// No pending events.
	return 0;
}

int w_pump(lua_State *L)
{
	float waitTimeout = (float)luaL_optnumber(L, 1, 0.0f);
	luax_catchexcept(L, [&]() { instance()->pump(waitTimeout); });
	return 0;
}

int w_wait(lua_State *L)
{
	luax_markdeprecated(L, 1, "love.event.wait", API_FUNCTION, DEPRECATED_REPLACED, "waitTimeout parameter in love.event.pump");

	Message *m = nullptr;
	luax_catchexcept(L, [&]() { m = instance()->wait(); });
	if (m != nullptr)
	{
		int args = luax_pushmessage(L, *m);
		m->release();
		return args;
	}

	return 0;
}

int w_push(lua_State *L)
{
	std::string name = luax_checkstring(L, 1);
	std::vector<Variant> vargs;

	int nargs = lua_gettop(L);
	for (int i = 2; i <= nargs; i++)
	{
		if (lua_isnoneornil(L, i))
			break;

		luax_catchexcept(L, [&]() { vargs.push_back(luax_checkvariant(L, i)); });

		if (vargs.back().getType() == Variant::UNKNOWN)
		{
			vargs.clear();
			return luaL_error(L, "Argument %d can't be stored safely\nExpected boolean, number, string or userdata.", i);
		}
	}

	StrongRef<Message> m(new Message(name, vargs), Acquire::NORETAIN);

	instance()->push(m);
	luax_pushboolean(L, true);
	return 1;
}

int w_clear(lua_State *L)
{
	luax_catchexcept(L, [&]() { instance()->clear(); });
	return 0;
}

int w_quit(lua_State *L)
{
	luax_catchexcept(L, [&]() {
		std::vector<Variant> args;
		for (int i = 1; i <= std::max(1, lua_gettop(L)); i++)
			args.push_back(luax_checkvariant(L, i));

		StrongRef<Message> m(new Message("quit", args), Acquire::NORETAIN);
		instance()->push(m);
	});

	luax_pushboolean(L, true);
	return 1;
}

int w_restart(lua_State *L)
{
	luax_catchexcept(L, [&]() {
		std::vector<Variant> args;
		args.emplace_back("restart", strlen("restart"));

		for (int i = 1; i <= lua_gettop(L); i++)
			args.push_back(luax_checkvariant(L, i));

		StrongRef<Message> m(new Message("quit", args), Acquire::NORETAIN);
		instance()->push(m);
	});

	luax_pushboolean(L, true);
	return 1;
}

// List of functions to wrap.
static const luaL_Reg functions[] =
{
	{ "pump", w_pump },
	{ "poll_i", w_poll_i },
	{ "wait", w_wait },
	{ "push", w_push },
	{ "clear", w_clear },
	{ "quit", w_quit },
	{ "restart", w_restart },
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
	w.type = &Module::type;
	w.functions = functions;
	w.types = nullptr;

	int ret = luax_register_module(L, w);

	if (luaL_loadbuffer(L, (const char *)event_lua, sizeof(event_lua), "=[love \"wrap_Event.lua\"]") == 0)
		lua_call(L, 0, 0);
	else
		lua_error(L);

	return ret;
}

} // event
} // love
