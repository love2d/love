/**
* Copyright (c) 2006-2009 LOVE Development Team
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

#ifndef LOVE_PHYSICS_BOX2D_WRAP_SHAPE_H
#define LOVE_PHYSICS_BOX2D_WRAP_SHAPE_H

// LOVE
#include <common/runtime.h>
#include "Shape.h"

namespace love
{
namespace physics
{
namespace box2d
{
	Shape * luax_checkshape(lua_State * L, int idx);
	int w_Shape_getType(lua_State * L);
	int w_Shape_setFriction(lua_State * L);
	int w_Shape_setRestitution(lua_State * L);
	int w_Shape_setDensity(lua_State * L);
	int w_Shape_setSensor(lua_State * L);
	int w_Shape_getFriction(lua_State * L);
	int w_Shape_getRestitution(lua_State * L);
	int w_Shape_getDensity(lua_State * L);
	int w_Shape_isSensor(lua_State * L);
	int w_Shape_getBody(lua_State * L);
	int w_Shape_testPoint(lua_State * L);
	int w_Shape_testSegment(lua_State * L);
	int w_Shape_setFilterData(lua_State * L);
	int w_Shape_getFilterData(lua_State * L);
	int w_Shape_setCategory(lua_State * L);
	int w_Shape_getCategory(lua_State * L);
	int w_Shape_setMask(lua_State * L);
	int w_Shape_getMask(lua_State * L);
	int w_Shape_setData(lua_State * L);
	int w_Shape_getData(lua_State * L);
	int w_Shape_getBoundingBox(lua_State * L);
	int w_Shape_getGroupIndex();
	int w_Shape_setGroupIndex();
	int w_Shape_destroy(lua_State * L);
	int luaopen_shape(lua_State * L);

} // box2d
} // physics
} // love

#endif // LOVE_PHYSICS_BOX2D_WRAP_SHAPE_H
