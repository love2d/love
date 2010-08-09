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

#include "wrap_Shape.h"
#include <common/StringMap.h>

namespace love
{
namespace physics
{
namespace box2d
{
	Shape * luax_checkshape(lua_State * L, int idx)
	{
		return luax_checktype<Shape>(L, idx, "Shape", PHYSICS_SHAPE_T);
	}

	int w_Shape_getType(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		const char * type = "";
		Shape::getConstant(t->getType(), type);
		lua_pushstring(L, type);
		return 1;
	}

	int w_Shape_setFriction(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		t->setFriction(arg1);
		return 0;
	}

	int w_Shape_setRestitution(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		t->setRestitution(arg1);
		return 0;
	}

	int w_Shape_setDensity(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		t->setDensity(arg1);
		return 0;
	}

	int w_Shape_setSensor(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		bool arg1 = luax_toboolean(L, 2);
		t->setSensor(arg1);
		return 0;
	}

	int w_Shape_getFriction(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		lua_pushnumber(L, t->getFriction());
		return 1;
	}

	int w_Shape_getRestitution(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		lua_pushnumber(L, t->getRestitution());
		return 1;
	}

	int w_Shape_getDensity(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		lua_pushnumber(L, t->getDensity());
		return 1;
	}

	int w_Shape_isSensor(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		luax_pushboolean(L, t->isSensor());
		return 1;
	}

	int w_Shape_getBody(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		Body * body = t->getBody();
		if(body == 0)
			return 0;
		body->retain();
		luax_newtype(L, "Body", PHYSICS_BODY_T, (void*)body);
		return 1;
	}

	int w_Shape_testPoint(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		float x = (float)luaL_checknumber(L, 2);
		float y = (float)luaL_checknumber(L, 3);
		luax_pushboolean(L, t->testPoint(x, y));
		return 1;
	}

	int w_Shape_testSegment(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		lua_remove(L, 1);
		return t->testSegment(L);
	}

	int w_Shape_setFilterData(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		int v[3];
		v[0] = luaL_checkint(L, 2);
		v[1] = luaL_checkint(L, 3);
		v[2] = luaL_checkint(L, 4);
		t->setFilterData(v);
		return 0;
	}

	int w_Shape_getFilterData(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		int v[3];
		t->getFilterData(v);
		lua_pushinteger(L, v[0]);
		lua_pushinteger(L, v[1]);
		lua_pushinteger(L, v[2]);
		return 3;
	}

	int w_Shape_setCategory(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		lua_remove(L, 1);
		return t->setCategory(L);
	}

	int w_Shape_getCategory(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		lua_remove(L, 1);
		return t->getCategory(L);
	}

	int w_Shape_setMask(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		lua_remove(L, 1);
		return t->setMask(L);
	}

	int w_Shape_getMask(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		lua_remove(L, 1);
		return t->getMask(L);
	}

	int w_Shape_setData(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		lua_remove(L, 1);
		return t->setData(L);
	}

	int w_Shape_getData(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		lua_remove(L, 1);
		return t->getData(L);
	}

	int w_Shape_getBoundingBox(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		lua_remove(L, 1);
		return t->getBoundingBox(L);
	}

	int w_Shape_getGroupIndex(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		int i = t->getGroupIndex();
		lua_pushinteger(L, i);
		return 1;
	}

	int w_Shape_setGroupIndex(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		int i = luaL_checkint(L, 2);
		t->setGroupIndex(i);
		return 0;
	}

	int w_Shape_destroy(lua_State * L)
	{
		Proxy * p = (Proxy *)lua_touserdata(L, 1);
		p->own = false;

		Shape * t = (Shape *)p->data;
		t->release();
		return 0;
	}

	static const luaL_Reg functions[] = {
		{ "getType", w_Shape_getType },
		{ "setFriction", w_Shape_setFriction },
		{ "setRestitution", w_Shape_setRestitution },
		{ "setDensity", w_Shape_setDensity },
		{ "setSensor", w_Shape_setSensor },
		{ "getFriction", w_Shape_getFriction },
		{ "getRestitution", w_Shape_getRestitution },
		{ "getDensity", w_Shape_getDensity },
		{ "getBody", w_Shape_getBody },
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

	int luaopen_shape(lua_State * L)
	{
		return luax_register_type(L, "Shape", functions);
	}

} // box2d
} // physics
} // love

