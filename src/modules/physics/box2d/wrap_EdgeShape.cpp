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

#include "wrap_EdgeShape.h"

namespace love
{
namespace physics
{
namespace box2d
{

EdgeShape *luax_checkedgeshape(lua_State *L, int idx)
{
	return luax_checktype<EdgeShape>(L, idx, PHYSICS_EDGE_SHAPE_ID);
}

int w_EdgeShape_setNextVertex(lua_State *L)
{
	EdgeShape *t = luax_checkedgeshape(L, 1);
	if (lua_isnoneornil(L, 2))
		t->setNextVertex();
	else
	{
		float x = (float)luaL_checknumber(L, 2);
		float y = (float)luaL_checknumber(L, 3);
		t->setNextVertex(x, y);
	}
	return 0;
}

int w_EdgeShape_setPreviousVertex(lua_State *L)
{
	EdgeShape *t = luax_checkedgeshape(L, 1);
	if (lua_isnoneornil(L, 2))
		t->setPreviousVertex();
	else
	{
		float x = (float)luaL_checknumber(L, 2);
		float y = (float)luaL_checknumber(L, 3);
		t->setPreviousVertex(x, y);
	}
	return 0;
}

int w_EdgeShape_getNextVertex(lua_State *L)
{
	EdgeShape *t = luax_checkedgeshape(L, 1);
	float x, y;
	if (t->getNextVertex(x, y))
	{
		lua_pushnumber(L, x);
		lua_pushnumber(L, y);
		return 2;
	}
	return 0;
}

int w_EdgeShape_getPreviousVertex(lua_State *L)
{
	EdgeShape *t = luax_checkedgeshape(L, 1);
	float x, y;
	if (t->getPreviousVertex(x, y))
	{
		lua_pushnumber(L, x);
		lua_pushnumber(L, y);
		return 2;
	}
	return 0;
}

int w_EdgeShape_getPoints(lua_State *L)
{
	EdgeShape *t = luax_checkedgeshape(L, 1);
	lua_remove(L, 1);
	return t->getPoints(L);
}

static const luaL_Reg w_EdgeShape_functions[] =
{
	{ "setNextVertex", w_EdgeShape_setNextVertex },
	{ "setPreviousVertex", w_EdgeShape_setPreviousVertex },
	{ "getNextVertex", w_EdgeShape_getNextVertex },
	{ "getPreviousVertex", w_EdgeShape_getPreviousVertex },
	{ "getPoints", w_EdgeShape_getPoints },
	{ 0, 0 }
};

extern "C" int luaopen_edgeshape(lua_State *L)
{
	return luax_register_type(L, PHYSICS_EDGE_SHAPE_ID, "EdgeShape", w_Shape_functions, w_EdgeShape_functions, nullptr);
}

} // box2d
} // physics
} // love
