/**
 * Copyright (c) 2006-2023 LOVE Development Team
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

#include "common/config.h"

// LOVE
#include "wrap_Timer.h"

namespace love
{
namespace timer
{

#define instance() (Module::getInstance<Timer>(Module::M_TIMER))

int w_step(lua_State *L)
{
	lua_pushnumber(L, instance()->step());
	return 1;
}

int w_getDelta(lua_State *L)
{
	lua_pushnumber(L, instance()->getDelta());
	return 1;
}

int w_getFPS(lua_State *L)
{
	lua_pushinteger(L, instance()->getFPS());
	return 1;
}

int w_getAverageDelta(lua_State *L)
{
	lua_pushnumber(L, instance()->getAverageDelta());
	return 1;
}

int w_sleep(lua_State *L)
{
	instance()->sleep(luaL_checknumber(L, 1));
	return 0;
}

int w_getTime(lua_State *L)
{
	lua_pushnumber(L, instance()->getTime());
	return 1;
}

// List of functions to wrap.
static const luaL_Reg functions[] =
{
	{ "step", w_step },
	{ "getDelta", w_getDelta },
	{ "getFPS", w_getFPS },
	{ "getAverageDelta", w_getAverageDelta },
	{ "sleep", w_sleep },
	{ "getTime", w_getTime },
	{ 0, 0 }
};


extern "C" int luaopen_love_timer(lua_State *L)
{
	Timer *instance = instance();
	if (instance == nullptr)
	{
		luax_catchexcept(L, [&](){ instance = new love::timer::Timer(); });
	}
	else
		instance->retain();

	WrappedModule w;
	w.module = instance;
	w.name = "timer";
	w.type = &Module::type;
	w.functions = functions;
	w.types = 0;

	return luax_register_module(L, w);
}

} // timer
} // love
