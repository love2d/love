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

#include "wrap_PolygonShape.h"

namespace love
{
namespace physics
{
namespace box2d
{
	PolygonShape * luax_checkpolygonshape(lua_State * L, int idx)
	{
		return luax_checktype<PolygonShape>(L, idx, "PolygonShape", PHYSICS_POLYGON_SHAPE_T);
	}

	int w_PolygonShape_getPoints(lua_State * L)
	{
		PolygonShape * t = luax_checkpolygonshape(L, 1);
		lua_remove(L, 1);
		return t->getPoints(L);
	}

	static const luaL_Reg functions[] = {
		{ "getPoints", w_PolygonShape_getPoints },
		// From Shape.
		{ "getType", w_Shape_getType },
		{ "setFriction", w_Shape_setFriction },
		{ "setRestitution", w_Shape_setRestitution },
		{ "setDensity", w_Shape_setDensity },
		{ "setSensor", w_Shape_setSensor },
		{ "getFriction", w_Shape_getFriction },
		{ "getRestitution", w_Shape_getRestitution },
		{ "getDensity", w_Shape_getDensity },
		{ "isSensor", w_Shape_isSensor },
		{ "testPoint", w_Shape_testPoint },
		{ "testSegment", w_Shape_testSegment },
		{ "setFilterData", w_Shape_setFilterData },
		{ "getFilterData", w_Shape_getFilterData },
		{ "setCategory", w_Shape_setCategory },
		{ "getCategory", w_Shape_getCategory },
		{ "setMask", w_Shape_setMask },
		{ "getMask", w_Shape_getMask },
		{ "setData", w_Shape_setData },
		{ "getData", w_Shape_getData },
		{ "getBoundingBox", w_Shape_getBoundingBox },
		{ "destroy", w_Shape_destroy },
		{ 0, 0 }
	};

	int luaopen_polygonshape(lua_State * L)
	{
		return luax_register_type(L, "PolygonShape", functions);
	}

} // box2d
} // physics
} // love
