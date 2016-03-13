/**
 * Copyright (c) 2006-2016 LOVE Development Team
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

namespace love
{
namespace event
{

#define instance() (Module::getInstance<Event>(Module::M_EVENT))

static int w_poll_i(lua_State *L)
{
	Message *m = nullptr;

	if (instance()->poll(m))
	{
		int args = m->toLua(L);
		m->release();
		return args;
	}

	// No pending events.
	return 0;
}

int w_poll(lua_State *L)
{
	lua_pushcclosure(L, &w_poll_i, 0);
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
	if (m)
	{
		int args = m->toLua(L);
		m->release();
		return args;
	}

	return 0;
}

int w_push(lua_State *L)
{
	StrongRef<Message> m(Message::fromLua(L, 1), Acquire::NORETAIN);

	luax_pushboolean(L, m.get() != nullptr);

	if (m.get() == nullptr)
		return 1;

	instance()->push(m);
	return 1;
}

int w_clear(lua_State *)
{
	instance()->clear();
	return 0;
}

int w_quit(lua_State *L)
{
	std::vector<Variant> args = {Variant::fromLua(L, 1)};

	StrongRef<Message> m(new Message("quit", args), Acquire::NORETAIN);
	instance()->push(m);

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

	return ret;
}

} // event
} // love
