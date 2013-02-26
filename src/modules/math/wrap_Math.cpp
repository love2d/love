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

#include "wrap_Math.h"
#include "ModMath.h"

#include <cmath>
#include <iostream>

namespace
{

union SeedConverter
{
	double   seed_double;
	uint64_t seed_uint;
};

} // anonymous namespace

namespace love
{
namespace math
{

static ModMath *instance = 0;

int w_randomseed(lua_State *L)
{
	SeedConverter s;
	s.seed_double = luaL_checknumber(L, 1);
	instance->randomseed(s.seed_uint);
	return 0;
}

int w_random(lua_State *L)
{
	double r = instance->random();
	int l, u;
	// verbatim from lua 5.1.4 source code: lmathlib.c:185 ff.
	switch (lua_gettop(L))
	{
	case 0:
		lua_pushnumber(L, r);
		break;
	case 1:
		u = luaL_checkint(L, 1);
		luaL_argcheck(L, 1 <= u, 1, "interval is empty");
		lua_pushnumber(L, floor(r * u) + 1);
		break;
	case 2:
		l = luaL_checkint(L, 1);
		u = luaL_checkint(L, 2);
		luaL_argcheck(L, l <= u, 2, "interval is empty");
		lua_pushnumber(L, floor(r * (u - l + 1)) + l);
		break;
	default:
		return luaL_error(L, "wrong number of arguments");
	}
	return 1;
}

int w_randnormal(lua_State *L)
{
	double mean = 0.0, stddev = 1.0;
	if (lua_gettop(L) > 1)
	{
		mean = luaL_checknumber(L, 1);
		stddev = luaL_checknumber(L, 2);
	}
	else
	{
		stddev = luaL_optnumber(L, 1, 1.);
	}

	double r = instance->randnormal(stddev);
	lua_pushnumber(L, r + mean);
	return 1;
}

// List of functions to wrap.
static const luaL_Reg functions[] =
{
	{ "randomseed", w_randomseed },
	{ "random", w_random },
	{ "randnormal", w_randnormal },
	{ 0, 0 }
};

static const lua_CFunction types[] =
{
	0
};

extern "C" int luaopen_love_math(lua_State *L)
{
	if (instance == 0)
		instance = new love::math::ModMath();
	else
		instance->retain();

	if (instance == 0)
		return luaL_error(L, "Could not open module math.");

	WrappedModule w;
	w.module = instance;
	w.name = "math";
	w.flags = MODULE_T;
	w.functions = functions;
	w.types = types;

	int n = luax_register_module(L, w);

	return n;
}

} // math
} // love
