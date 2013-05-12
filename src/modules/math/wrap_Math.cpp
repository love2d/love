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
#include "wrap_RandomGenerator.h"
#include "wrap_BezierCurve.h"
#include "MathModule.h"
#include "BezierCurve.h"

#include <cmath>
#include <iostream>

namespace love
{
namespace math
{

int w_randomseed(lua_State *L)
{
	uint64 seed = luax_checkrandomseed(L, 1);
	Math::instance.randomseed(seed);
	return 0;
}

int w_random(lua_State *L)
{
	return luax_getrandom(L, 1, Math::instance.random());
}

int w_randomnormal(lua_State *L)
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

	double r = Math::instance.randomnormal(stddev);
	lua_pushnumber(L, r + mean);
	return 1;
}

int w_newRandomGenerator(lua_State *L)
{
	RandomGenerator *t = Math::instance.newRandomGenerator();

	if (lua_gettop(L) > 0)
	{
		uint64 seed = luax_checkrandomseed(L, 1);
		t->randomseed(seed);
	}

	luax_newtype(L, "RandomGenerator", MATH_RANDOM_GENERATOR_T, (void *) t);
	return 1;
}

int w_newBezierCurve(lua_State *L)
{
	std::vector<Vector> points;
	if (lua_istable(L, 1))
	{
		size_t top = lua_objlen(L, 1);
		points.reserve(top / 2);
		for (size_t i = 1; i <= top; i += 2)
		{
			lua_rawgeti(L, 1, i);
			lua_rawgeti(L, 1, i+1);

			Vector v;
			v.x = luaL_checknumber(L, -2);
			v.y = luaL_checknumber(L, -1);
			points.push_back(v);

			lua_pop(L, 2);
		}
	}
	else
	{
		size_t top = lua_gettop(L);
		points.reserve(top / 2);
		for (size_t i = 1; i <= top; i += 2)
		{
			Vector v;
			v.x = luaL_checknumber(L, i);
			v.y = luaL_checknumber(L, i+1);
			points.push_back(v);
		}
	}

	BezierCurve *curve = Math::instance.newBezierCurve(points);
	luax_newtype(L, "BezierCurve", MATH_BEZIER_CURVE_T, (void *)curve);
	return 1;
}

int w_triangulate(lua_State *L)
{
	std::vector<vertex> vertices;
	if (lua_istable(L, 1))
	{
		size_t top = lua_objlen(L, 1);
		vertices.reserve(top / 2);
		for (size_t i = 1; i <= top; i += 2)
		{
			lua_rawgeti(L, 1, i);
			lua_rawgeti(L, 1, i+1);

			vertex v;
			v.x = luaL_checknumber(L, -2);
			v.y = luaL_checknumber(L, -1);
			vertices.push_back(v);

			lua_pop(L, 2);
		}
	}
	else
	{
		size_t top = lua_gettop(L);
		vertices.reserve(top / 2);
		for (size_t i = 1; i <= top; i += 2)
		{
			vertex v;
			v.x = luaL_checknumber(L, i);
			v.y = luaL_checknumber(L, i+1);
			vertices.push_back(v);
		}
	}

	if (vertices.size() < 3)
		return luaL_error(L, "Need at least 3 vertices to triangulate");

	std::vector<Triangle> triangles;
	try
	{
		if (vertices.size() == 3)
			triangles.push_back(Triangle(vertices[0], vertices[1], vertices[2]));
		else
			triangles = Math::instance.triangulate(vertices);
	}
	catch (love::Exception &e)
	{
		return luaL_error(L, e.what());
	}

	lua_createtable(L, triangles.size(), 0);
	for (size_t i = 0; i < triangles.size(); ++i)
	{
		Triangle &tri = triangles[i];

		lua_createtable(L, 6, 0);
		lua_pushnumber(L, tri.a.x);
		lua_rawseti(L, -2, 1);
		lua_pushnumber(L, tri.a.y);
		lua_rawseti(L, -2, 2);
		lua_pushnumber(L, tri.b.x);
		lua_rawseti(L, -2, 3);
		lua_pushnumber(L, tri.b.y);
		lua_rawseti(L, -2, 4);
		lua_pushnumber(L, tri.c.x);
		lua_rawseti(L, -2, 5);
		lua_pushnumber(L, tri.c.y);
		lua_rawseti(L, -2, 6);

		lua_rawseti(L, -2, i+1);
	}

	return 1;
}

int w_isConvex(lua_State *L)
{
	std::vector<vertex> vertices;
	if (lua_istable(L, 1))
	{
		size_t top = lua_objlen(L, 1);
		vertices.reserve(top / 2);
		for (size_t i = 1; i <= top; i += 2)
		{
			lua_rawgeti(L, 1, i);
			lua_rawgeti(L, 1, i+1);

			vertex v;
			v.x = luaL_checknumber(L, -2);
			v.y = luaL_checknumber(L, -1);
			vertices.push_back(v);

			lua_pop(L, 2);
		}
	}
	else
	{
		size_t top = lua_gettop(L);
		vertices.reserve(top / 2);
		for (size_t i = 1; i <= top; i += 2)
		{
			vertex v;
			v.x = luaL_checknumber(L, i);
			v.y = luaL_checknumber(L, i+1);
			vertices.push_back(v);
		}
	}

	lua_pushboolean(L, Math::instance.isConvex(vertices));
	return 1;
}

int w_noise(lua_State *L)
{
	float w, x, y, z;
	float val;

	switch (lua_gettop(L))
	{
	case 1:
		x = luaL_checknumber(L, 1);
		val = Math::instance.noise(x);
		break;
	case 2:
		x = luaL_checknumber(L, 1);
		y = luaL_checknumber(L, 2);
		val = Math::instance.noise(x, y);
		break;
	case 3:
		x = luaL_checknumber(L, 1);
		y = luaL_checknumber(L, 2);
		z = luaL_checknumber(L, 3);
		val = Math::instance.noise(x, y, z);
		break;
	case 4:
	default:
		x = luaL_checknumber(L, 1);
		y = luaL_checknumber(L, 2);
		z = luaL_checknumber(L, 3);
		w = luaL_checknumber(L, 4);
		val = Math::instance.noise(x, y, z, w);
		break;
	}

	lua_pushnumber(L, (lua_Number) val);
	return 1;
}

// List of functions to wrap.
static const luaL_Reg functions[] =
{
	{ "randomseed", w_randomseed },
	{ "random", w_random },
	{ "randomnormal", w_randomnormal },
	{ "newRandomGenerator", w_newRandomGenerator },
	{ "newBezierCurve", w_newBezierCurve },
	{ "triangulate", w_triangulate },
	{ "isConvex", w_isConvex },
	{ "noise", w_noise },
	{ 0, 0 }
};

static const lua_CFunction types[] =
{
	luaopen_randomgenerator,
	luaopen_beziercurve,
	0
};

extern "C" int luaopen_love_math(lua_State *L)
{
	Math::instance.retain();
	WrappedModule w;
	w.module = &Math::instance;
	w.name = "math";
	w.flags = MODULE_T;
	w.functions = functions;
	w.types = types;

	int n = luax_register_module(L, w);

	return n;
}

} // math
} // love
