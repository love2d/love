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
	return luax_checktype<ChainShape>(L, idx, PHYSICS_CHAIN_SHAPE_ID);
}

int w_ChainShape_setNextVertex(lua_State *L)
{
	ChainShape *c = luax_checkchainshape(L, 1);
	float x = (float)luaL_checknumber(L, 2);
	float y = (float)luaL_checknumber(L, 3);
	luax_catchexcept(L, [&](){ c->setNextVertex(x, y); });
	return 0;
}

int w_ChainShape_setPreviousVertex(lua_State *L)
{
	ChainShape *c = luax_checkchainshape(L, 1);
	float x = (float)luaL_checknumber(L, 2);
	float y = (float)luaL_checknumber(L, 3);
	luax_catchexcept(L, [&](){ c->setPreviousVertex(x, y); });
	return 0;
}

int w_ChainShape_getChildEdge(lua_State *L)
{
	ChainShape *c = luax_checkchainshape(L, 1);
	int index = (int) luaL_checknumber(L, 2) - 1; // Convert from 1-based index
	EdgeShape *e = 0;
	luax_catchexcept(L, [&](){ e = c->getChildEdge(index); });
	luax_pushtype(L, PHYSICS_EDGE_SHAPE_ID, e);
	e->release();
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
	int index = (int) luaL_checknumber(L, 2) - 1; // Convert from 1-based index
	b2Vec2 v;
	luax_catchexcept(L, [&](){ v = c->getPoint(index); });
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

static const luaL_Reg w_ChainShape_functions[] =
{
	{ "setNextVertex", w_ChainShape_setNextVertex },
	{ "setPreviousVertex", w_ChainShape_setPreviousVertex },
	{ "getChildEdge", w_ChainShape_getChildEdge },
	{ "getVertexCount", w_ChainShape_getVertexCount },
	{ "getPoint", w_ChainShape_getPoint },
	{ "getPoints", w_ChainShape_getPoints },
	{ 0, 0 }
};

extern "C" int luaopen_chainshape(lua_State *L)
{
	return luax_register_type(L, PHYSICS_CHAIN_SHAPE_ID, "ChainShape", w_Shape_functions, w_ChainShape_functions, nullptr);
}

} // box2d
} // physics
} // love
