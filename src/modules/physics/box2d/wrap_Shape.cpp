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

#include "wrap_Shape.h"
#include "common/StringMap.h"

namespace love
{
namespace physics
{
namespace box2d
{

Shape *luax_checkshape(lua_State *L, int idx)
{
	return luax_checktype<Shape>(L, idx, PHYSICS_SHAPE_ID);
}

int w_Shape_getType(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	const char *type = "";
	Shape::getConstant(t->getType(), type);
	lua_pushstring(L, type);
	return 1;
}

int w_Shape_getRadius(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	float radius = t->getRadius();
	lua_pushnumber(L, radius);
	return 1;
}

int w_Shape_getChildCount(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	int childCount = t->getChildCount();
	lua_pushinteger(L, childCount);
	return 1;
}

int w_Shape_testPoint(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	float x = (float)luaL_checknumber(L, 2);
	float y = (float)luaL_checknumber(L, 3);
	float r = (float)luaL_checknumber(L, 4);
	float px = (float)luaL_checknumber(L, 5);
	float py = (float)luaL_checknumber(L, 6);
	bool result = t->testPoint(x, y, r, px, py);
	lua_pushboolean(L, result);
	return 1;
}

int w_Shape_rayCast(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	lua_remove(L, 1);
	int ret = 0;
	luax_catchexcept(L, [&](){ ret = t->rayCast(L); });
	return ret;
}

int w_Shape_computeAABB(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	lua_remove(L, 1);
	return t->computeAABB(L);
}

int w_Shape_computeMass(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	lua_remove(L, 1);
	return t->computeMass(L);
}

const luaL_Reg w_Shape_functions[] =
{
	{ "getType", w_Shape_getType },
	{ "getRadius", w_Shape_getRadius },
	{ "getChildCount", w_Shape_getChildCount },
	{ "testPoint", w_Shape_testPoint },
	{ "rayCast", w_Shape_rayCast },
	{ "computeAABB", w_Shape_computeAABB },
	{ "computeMass", w_Shape_computeMass },
	{ 0, 0 }
};

extern "C" int luaopen_shape(lua_State *L)
{
	return luax_register_type(L, PHYSICS_SHAPE_ID, "Shape", w_Shape_functions, nullptr);
}

} // box2d
} // physics
} // love

