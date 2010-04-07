/**
* Copyright (c) 2006-2010 LOVE Development Team
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

#include "wrap_CircleShape.h"

namespace love
{
namespace physics
{
namespace box2d
{
	CircleShape * luax_checkcircleshape(lua_State * L, int idx)
	{
		return luax_checktype<CircleShape>(L, idx, "CircleShape", PHYSICS_CIRCLE_SHAPE_T);
	}

	int w_CircleShape_getRadius(lua_State * L)
	{
		CircleShape * c = luax_checkcircleshape(L, 1);
		lua_pushnumber(L, c->getRadius());
		return 1;
	}

	int w_CircleShape_getLocalCenter(lua_State * L)
	{
		CircleShape * c = luax_checkcircleshape(L, 1);
		float x, y;
		c->getLocalCenter(x, y);
		lua_pushnumber(L, x);
		lua_pushnumber(L, y);
		return 2;
	}

	int w_CircleShape_getWorldCenter(lua_State * L)
	{
		CircleShape * c = luax_checkcircleshape(L, 1);
		float x, y;
		c->getWorldCenter(x, y);
		lua_pushnumber(L, x);
		lua_pushnumber(L, y);
		return 2;
	}

	static const luaL_Reg functions[] = {
		{ "getRadius", w_CircleShape_getRadius },
		{ "getLocalCenter", w_CircleShape_getLocalCenter },
		{ "getWorldCenter", w_CircleShape_getWorldCenter },
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
		{ "getGroupIndex", w_Shape_getGroupIndex },
		{ "setGroupIndex", w_Shape_setGroupIndex },
		{ "destroy", w_Shape_destroy },
		{ 0, 0 }
	};

	int luaopen_circleshape(lua_State * L)
	{
		return luax_register_type(L, "CircleShape", functions);
	}

} // box2d
} // physics
} // love
