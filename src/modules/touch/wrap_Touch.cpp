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

#include "common/config.h"

// LOVE
#include "wrap_Touch.h"

#include "sdl/Touch.h"

namespace love
{
namespace touch
{

#define instance() (Module::getInstance<Touch>(Module::M_TOUCH))

int w_getTouchCount(lua_State *L)
{
	lua_pushinteger(L, instance()->getTouchCount());
	return 1;
}

int w_getTouch(lua_State *L)
{
	int index = luaL_checkint(L, 1) - 1;

	Touch::TouchInfo info;
	luax_catchexcept(L, [&](){ info = instance()->getTouch(index); });

	// Lets hope the ID can be accurately represented in a Lua number...
	lua_pushnumber(L, (lua_Number) info.id);
	lua_pushnumber(L, info.x);
	lua_pushnumber(L, info.y);

	return 3;
}

static const luaL_Reg functions[] =
{
	{ "getTouchCount", w_getTouchCount },
	{ "getTouch", w_getTouch },
	{ 0, 0 }
};

extern "C" int luaopen_love_touch(lua_State *L)
{
	Touch *instance = instance();
	if (instance == nullptr)
	{
		luax_catchexcept(L, [&](){ instance = new love::touch::sdl::Touch(); });
	}
	else
		instance->retain();

	WrappedModule w;
	w.module = instance;
	w.name = "touch";
	w.flags = MODULE_T;
	w.functions = functions;
	w.types = nullptr;

	return luax_register_module(L, w);
}

} // touch
} // love
