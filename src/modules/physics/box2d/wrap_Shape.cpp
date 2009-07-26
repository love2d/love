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

#include "wrap_Shape.h"

namespace love
{
namespace physics
{
namespace box2d
{
	Shape * luax_checkshape(lua_State * L, int idx)
	{
		return luax_checktype<Shape>(L, idx, "Shape", LOVE_PHYSICS_SHAPE_BITS);
	}

	int _wrap_Shape_getType(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		lua_pushinteger(L, t->getType());
		return 1;
	}

	int _wrap_Shape_setFriction(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		t->setFriction(arg1);
		return 0;
	}

	int _wrap_Shape_setRestitution(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		t->setRestitution(arg1);
		return 0;
	}

	int _wrap_Shape_setDensity(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		t->setDensity(arg1);
		return 0;
	}

	int _wrap_Shape_setSensor(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		bool arg1 = luax_toboolean(L, 2);
		t->setSensor(arg1);
		return 0;
	}

	int _wrap_Shape_getFriction(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		lua_pushnumber(L, t->getFriction());
		return 1;
	}

	int _wrap_Shape_getRestituion(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		lua_pushnumber(L, t->getRestituion());
		return 1;
	}

	int _wrap_Shape_getDensity(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		lua_pushnumber(L, t->getDensity());
		return 1;
	}

	int _wrap_Shape_isSensor(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		luax_pushboolean(L, t->isSensor());
		return 1;
	}

	int _wrap_Shape_getBody(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		Body * body = t->getBody();
		if(body == 0)
			return 0;
		luax_newtype(L, "Body", LOVE_PHYSICS_BODY_BITS, (void*)body);
		return 1;
	}

	int _wrap_Shape_testPoint(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		float x = (float)luaL_checknumber(L, 2);
		float y = (float)luaL_checknumber(L, 3);
		luax_pushboolean(L, t->testPoint(x, y));
		return 1;
	}

	int _wrap_Shape_testSegment(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		return t->testSegment(L);
	}

	int _wrap_Shape_setCategory(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		lua_remove(L, 1);
		return t->setCategory(L);
	}

	int _wrap_Shape_getCategory(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		lua_remove(L, 1);
		return t->getCategory(L);
	}

	int _wrap_Shape_setMask(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		lua_remove(L, 1);
		return t->setMask(L);
	}

	int _wrap_Shape_getMask(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		lua_remove(L, 1);
		return t->getMask(L);
	}

	int _wrap_Shape_setData(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		lua_remove(L, 1);
		return t->setData(L);
	}

	int _wrap_Shape_getData(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		lua_remove(L, 1);
		return t->getData(L);
	}

	int _wrap_Shape_getBoundingBox(lua_State * L)
	{
		Shape * t = luax_checkshape(L, 1);
		lua_remove(L, 1);
		return t->getBoundingBox(L);
	}

	static const luaL_Reg wrap_Shape_functions[] = {
		{ "getType", _wrap_Shape_getType },
		{ "setFriction", _wrap_Shape_setFriction },
		{ "setRestitution", _wrap_Shape_setRestitution },
		{ "setDensity", _wrap_Shape_setDensity },
		{ "setSensor", _wrap_Shape_setSensor },
		{ "getFriction", _wrap_Shape_getFriction },
		{ "getRestituion", _wrap_Shape_getRestituion },
		{ "getDensity", _wrap_Shape_getDensity },
		{ "isSensor", _wrap_Shape_isSensor },
		{ "testPoint", _wrap_Shape_testPoint },
		{ "testSegment", _wrap_Shape_testSegment },
		{ "setCategory", _wrap_Shape_setCategory },
		{ "getCategory", _wrap_Shape_getCategory },
		{ "setMask", _wrap_Shape_setMask },
		{ "getMask", _wrap_Shape_getMask },
		{ "setData", _wrap_Shape_setData },
		{ "getData", _wrap_Shape_getData },
		{ "getBoundingBox", _wrap_Shape_getBoundingBox },
		{ 0, 0 }
	};

	int wrap_Shape_open(lua_State * L)
	{
		luax_register_type(L, "Shape", wrap_Shape_functions);
		return 0;
	}

} // box2d
} // physics
} // love

