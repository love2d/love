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

#include "wrap_RandomGenerator.h"

#include <cmath>
#include <algorithm>

// Put the Lua code directly into a raw string literal.
static const char randomgenerator_lua[] =
#include "wrap_RandomGenerator.lua"
;

namespace love
{
namespace math
{

template <typename T>
static T checkrandomseed_part(lua_State *L, int idx)
{
	double num = luaL_checknumber(L, idx);
	double inf = std::numeric_limits<double>::infinity();

	// Disallow conversions from infinity and NaN.
	if (num == inf || num == -inf || num != num)
		luaL_argerror(L, idx, "invalid random seed");

	return (T) num;
}

RandomGenerator::Seed luax_checkrandomseed(lua_State *L, int idx)
{
	RandomGenerator::Seed s;

	if (!lua_isnoneornil(L, idx + 1))
	{
		s.b32.low = checkrandomseed_part<uint32>(L, idx);
		s.b32.high = checkrandomseed_part<uint32>(L, idx + 1);
	}
	else
		s.b64 = checkrandomseed_part<uint64>(L, idx);

	return s;
}

RandomGenerator *luax_checkrandomgenerator(lua_State *L, int idx)
{
	return luax_checktype<RandomGenerator>(L, idx, MATH_RANDOM_GENERATOR_ID);
}

int w_RandomGenerator__random(lua_State *L)
{
	RandomGenerator *rng = luax_checkrandomgenerator(L, 1);
	lua_pushnumber(L, rng->random());
	return 1;
}

int w_RandomGenerator_randomNormal(lua_State *L)
{
	RandomGenerator *rng = luax_checkrandomgenerator(L, 1);

	double stddev = luaL_optnumber(L, 2, 1.0);
	double mean = luaL_optnumber(L, 3, 0.0);
	double r = rng->randomNormal(stddev);

	lua_pushnumber(L, r + mean);
	return 1;
}

int w_RandomGenerator_setSeed(lua_State *L)
{
	RandomGenerator *rng = luax_checkrandomgenerator(L, 1);
	luax_catchexcept(L, [&](){ rng->setSeed(luax_checkrandomseed(L, 2)); });
	return 0;
}

int w_RandomGenerator_getSeed(lua_State *L)
{
	RandomGenerator *rng = luax_checkrandomgenerator(L, 1);
	RandomGenerator::Seed s = rng->getSeed();
	lua_pushnumber(L, (lua_Number) s.b32.low);
	lua_pushnumber(L, (lua_Number) s.b32.high);
	return 2;
}

int w_RandomGenerator_setState(lua_State *L)
{
	RandomGenerator *rng = luax_checkrandomgenerator(L, 1);
	luax_catchexcept(L, [&](){ rng->setState(luax_checkstring(L, 2)); });
	return 0;
}

int w_RandomGenerator_getState(lua_State *L)
{
	RandomGenerator *rng = luax_checkrandomgenerator(L, 1);
	luax_pushstring(L, rng->getState());
	return 1;
}

// C functions in a struct, necessary for the FFI versions of RandomGenerator functions.
struct FFI_RandomGenerator
{
	double (*random)(Proxy *p);
};

static FFI_RandomGenerator ffifuncs =
{
	[](Proxy *p) -> double // random()
	{
		// FIXME: We need better type-checking...
		if (p == nullptr || !typeFlags[p->type][MATH_RANDOM_GENERATOR_ID])
			return 0.0;

		RandomGenerator *rng = (RandomGenerator *) p->object;
		return rng->random();
	}
};

static const luaL_Reg w_RandomGenerator_functions[] =
{
	{ "_random", w_RandomGenerator__random }, // random() is defined in wrap_RandomGenerator.lua.
	{ "randomNormal", w_RandomGenerator_randomNormal },
	{ "setSeed", w_RandomGenerator_setSeed },
	{ "getSeed", w_RandomGenerator_getSeed },
	{ "setState", w_RandomGenerator_setState },
	{ "getState", w_RandomGenerator_getState },
	{ 0, 0 }
};

extern "C" int luaopen_randomgenerator(lua_State *L)
{
	int n = luax_register_type(L, MATH_RANDOM_GENERATOR_ID, "RandomGenerator", w_RandomGenerator_functions, nullptr);

	luax_gettypemetatable(L, MATH_RANDOM_GENERATOR_ID);

	// Load and execute wrap_RandomGenerator.lua, sending the metatable and the
	// ffi functions struct pointer as arguments.
	if (lua_istable(L, -1))
	{
		luaL_loadbuffer(L, randomgenerator_lua, sizeof(randomgenerator_lua), "wrap_RandomGenerator.lua");
		lua_pushvalue(L, -2);
		lua_pushlightuserdata(L, &ffifuncs);
		lua_call(L, 2, 0);
	}

	// Pop the metatable.
	lua_pop(L, 1);

	return n;
}

} // math
} // love
