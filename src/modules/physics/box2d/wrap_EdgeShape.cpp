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

int w_EdgeShape_getPoints(lua_State *L)
{
	EdgeShape *t = luax_checkedgeshape(L, 1);
	lua_remove(L, 1);
	return t->getPoints(L);
}

static const luaL_Reg w_EdgeShape_functions[] =
{
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
