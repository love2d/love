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
#include "common/int.h"

// LOVE
#include "wrap_Touch.h"

#include "sdl/Touch.h"

namespace love
{
namespace touch
{

#define instance() (Module::getInstance<Touch>(Module::M_TOUCH))

int w_getTouchIDs(lua_State *L)
{
	std::vector<int64> ids = instance()->getTouchIDs();

	lua_createtable(L, (int) ids.size(), 0);

	for (size_t i = 0; i < ids.size(); i++)
	{
		// This is a bit hackish and we lose the higher 32 bits of the id on
		// 32-bit systems, but SDL only ever gives id's that at most use as many
		// bits as can fit in a pointer (for now.)
		// We use lightuserdata instead of a lua_Number (double) because doubles
		// can't represent all possible id values on 64-bit systems.
		lua_pushlightuserdata(L, (void *) (intptr_t) ids[i]);
		lua_rawseti(L, -2, (int) i + 1);
	}

	return 1;
}

int w_getPosition(lua_State *L)
{
	if (!lua_islightuserdata(L, 1))
		return luax_typerror(L, 1, "touch id");

	int64 id = (int64) (intptr_t) lua_touserdata(L, 1);

	double x = 0;
	double y = 0;
	luax_catchexcept(L, [&]() { instance()->getPosition(id, x, y); });

	lua_pushnumber(L, x);
	lua_pushnumber(L, y);

	return 2;
}

static const luaL_Reg functions[] =
{
	{ "getTouchIDs", w_getTouchIDs },
	{ "getPosition", w_getPosition },
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
