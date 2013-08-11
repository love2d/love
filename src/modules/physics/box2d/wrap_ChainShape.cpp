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

#include "wrap_ChainShape.h"
#include "wrap_Physics.h"
#include "Physics.h"

namespace love
{
namespace physics
{
namespace box2d
{

ChainShape *luax_checkchainshape(lua_State *L, int idx)
{
	return luax_checktype<ChainShape>(L, idx, "ChainShape", PHYSICS_CHAIN_SHAPE_T);
}

int w_ChainShape_setNextVertex(lua_State *L)
{
	ChainShape *c = luax_checkchainshape(L, 1);
	float x = (float)luaL_checknumber(L, 2);
	float y = (float)luaL_checknumber(L, 3);
	ASSERT_GUARD(c->setNextVertex(x, y);)
	return 0;
}

int w_ChainShape_setPrevVertex(lua_State *L)
{
	ChainShape *c = luax_checkchainshape(L, 1);
	float x = (float)luaL_checknumber(L, 2);
	float y = (float)luaL_checknumber(L, 3);
	ASSERT_GUARD(c->setPrevVertex(x, y);)
	return 0;
}

int w_ChainShape_getChildCount(lua_State *L)
{
	ChainShape *c = luax_checkchainshape(L, 1);
	lua_pushinteger(L, c->getChildCount());
	return 1;
}

int w_ChainShape_getChildEdge(lua_State *L)
{
	ChainShape *c = luax_checkchainshape(L, 1);
	int index = luaL_checkint(L, 2) - 1; // Convert from 1-based index
	EdgeShape *e = 0;
	ASSERT_GUARD(e = c->getChildEdge(index);)
	luax_newtype(L, "EdgeShape", PHYSICS_EDGE_SHAPE_T, e);
	return 1;
}

int w_ChainShape_getVertexCount(lua_State *L)
{
	ChainShape *c = luax_checkchainshape(L, 1);
	int count = c->getVertexCount();
	lua_pushinteger(L, count);
	return 1;
}

int w_ChainShape_getPoint(lua_State *L)
{
	ChainShape *c = luax_checkchainshape(L, 1);
	int index = luaL_checkint(L, 2) - 1; // Convert from 1-based index
	b2Vec2 v;
	ASSERT_GUARD(v = c->getPoint(index);)
	lua_pushnumber(L, v.x);
	lua_pushnumber(L, v.y);
	return 2;
}

int w_ChainShape_getPoints(lua_State *L)
{
	ChainShape *c = luax_checkchainshape(L, 1);
	const b2Vec2 *verts = c->getPoints();
	int count = c->getVertexCount();
	if (!lua_checkstack(L, count*2))
		return luaL_error(L, "Too many return values");
	for (int i = 0; i < count; i++)
	{
		b2Vec2 v = Physics::scaleUp(verts[i]);
		lua_pushnumber(L, v.x);
		lua_pushnumber(L, v.y);
	}
	return count*2;
}

static const luaL_Reg functions[] =
{
	{ "setNextVertex", w_ChainShape_setNextVertex },
	{ "setPrevVertex", w_ChainShape_setPrevVertex },
	{ "getChildCount", w_ChainShape_getChildCount },
	{ "getChildEdge", w_ChainShape_getChildEdge },
	{ "getVertexCount", w_ChainShape_getVertexCount },
	{ "getPoint", w_ChainShape_getPoint },
	{ "getPoints", w_ChainShape_getPoints },
	// From Shape.
	{ "getType", w_Shape_getType },
	{ "getRadius", w_Shape_getRadius },
	{ "getChildCount", w_Shape_getChildCount },
	{ "testPoint", w_Shape_testPoint },
	{ "rayCast", w_Shape_rayCast },
	{ "computeAABB", w_Shape_computeAABB },
	{ "computeMass", w_Shape_computeMass },
	{ 0, 0 }
};

extern "C" int luaopen_chainshape(lua_State *L)
{
	return luax_register_type(L, "ChainShape", functions);
}

} // box2d
} // physics
} // love
