/**
 * Copyright (c) 2006-2024 LOVE Development Team
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
	return luax_checktype<Shape>(L, idx);
}

void luax_pushshape(lua_State *L, Shape *shape)
{
	if (shape != nullptr)
	{
		switch (shape->getType())
		{
		case Shape::SHAPE_CIRCLE:
			luax_pushtype(L, (CircleShape *) shape);
			break;
		case Shape::SHAPE_POLYGON:
			luax_pushtype(L, (PolygonShape *) shape);
			break;
		case Shape::SHAPE_EDGE:
			luax_pushtype(L, (EdgeShape *) shape);
			break;
		case Shape::SHAPE_CHAIN:
			luax_pushtype(L, (ChainShape *) shape);
			break;
		default:
			luax_pushtype(L, shape);
			break;
		}
	}
	else
	{
		lua_pushnil(L);
	}
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
	float radius = 0;
	luax_catchexcept(L, [&]() { radius = t->getRadius(); });
	lua_pushnumber(L, radius);
	return 1;
}

int w_Shape_getChildCount(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	int childCount = 0;
	luax_catchexcept(L, [&]() { childCount = t->getChildCount(); });
	lua_pushinteger(L, childCount);
	return 1;
}

int w_Shape_setFriction(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	luax_catchexcept(L, [&]() { t->setFriction(arg1); });
	return 0;
}

int w_Shape_setRestitution(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	luax_catchexcept(L, [&]() { t->setRestitution(arg1); });
	return 0;
}

int w_Shape_setDensity(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	luax_catchexcept(L, [&](){ t->setDensity(arg1); });
	return 0;
}

int w_Shape_setSensor(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	bool arg1 = luax_checkboolean(L, 2);
	luax_catchexcept(L, [&]() { t->setSensor(arg1); });
	return 0;
}

int w_Shape_getFriction(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	float friction = 0;
	luax_catchexcept(L, [&]() { friction = t->getFriction(); });
	lua_pushnumber(L, friction);
	return 1;
}

int w_Shape_getRestitution(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	float r = 0;
	luax_catchexcept(L, [&]() { r = t->getRestitution(); });
	lua_pushnumber(L, r);
	return 1;
}

int w_Shape_getDensity(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	float d = 0;
	luax_catchexcept(L, [&]() { d = t->getDensity(); });
	lua_pushnumber(L, d);
	return 1;
}

int w_Shape_isSensor(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	bool sensor = false;
	luax_catchexcept(L, [&]() { sensor = t->isSensor(); });
	luax_pushboolean(L, sensor);
	return 1;
}

int w_Shape_getBody(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	Body *body = t->getBody();
	if (body == nullptr)
		return 0;
	luax_pushtype(L, body);
	return 1;
}

int w_Shape_getShape(lua_State *L)
{
	luax_markdeprecated(L, 1, "Fixture:getShape", API_METHOD, DEPRECATED_NO_REPLACEMENT, nullptr);
	Shape *t = luax_checkshape(L, 1);
	luax_pushshape(L, t);
	return 1;
}

int w_Shape_testPoint(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	float x = (float)luaL_checknumber(L, 2);
	float y = (float)luaL_checknumber(L, 3);
	bool result = false;
	if (!lua_isnoneornil(L, 4))
	{
		float r = (float)luaL_checknumber(L, 4);
		float px = (float)luaL_checknumber(L, 5);
		float py = (float)luaL_checknumber(L, 6);
		luax_catchexcept(L, [&]() { result = t->testPoint(x, y, r, px, py); });
	}
	else
	{
		luax_catchexcept(L, [&]() { result = t->testPoint(x, y); });
	}
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
	int ret = 0;
	luax_catchexcept(L, [&]() { ret = t->computeAABB(L); });
	return ret;
}

int w_Shape_computeMass(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	lua_remove(L, 1);
	int ret = 0;
	luax_catchexcept(L, [&]() { ret = t->computeMass(L); });
	return ret;
}

int w_Shape_setFilterData(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	int v[3];
	v[0] = (int) luaL_checkinteger(L, 2);
	v[1] = (int) luaL_checkinteger(L, 3);
	v[2] = (int) luaL_checkinteger(L, 4);
	luax_catchexcept(L, [&]() { t->setFilterData(v); });
	return 0;
}

int w_Shape_getFilterData(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	int v[3];
	luax_catchexcept(L, [&]() { t->getFilterData(v); });
	lua_pushinteger(L, v[0]);
	lua_pushinteger(L, v[1]);
	lua_pushinteger(L, v[2]);
	return 3;
}

int w_Shape_setCategory(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	lua_remove(L, 1);
	int ret = 0;
	luax_catchexcept(L, [&]() { ret = t->setCategory(L); });
	return ret;
}

int w_Shape_getCategory(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	lua_remove(L, 1);
	int ret = 0;
	luax_catchexcept(L, [&]() { ret = t->getCategory(L); });
	return ret;
}

int w_Shape_setMask(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	lua_remove(L, 1);
	int ret = 0;
	luax_catchexcept(L, [&]() { ret = t->setMask(L); });
	return ret;
}

int w_Shape_getMask(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	lua_remove(L, 1);
	int ret = 0;
	luax_catchexcept(L, [&]() { ret = t->getMask(L); });
	return ret;
}

int w_Shape_setUserData(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	lua_remove(L, 1);
	int ret = 0;
	luax_catchexcept(L, [&]() { ret = t->setUserData(L); });
	return ret;
}

int w_Shape_getUserData(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	lua_remove(L, 1);
	int ret = 0;
	luax_catchexcept(L, [&]() { ret = t->getUserData(L); });
	return ret;
}

int w_Shape_getBoundingBox(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	lua_remove(L, 1);
	int ret = 0;
	luax_catchexcept(L, [&]() { ret = t->getBoundingBox(L); });
	return ret;
}

int w_Shape_getMassData(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	lua_remove(L, 1);
	int ret = 0;
	luax_catchexcept(L, [&]() { ret = t->getMassData(L); });
	return ret;
}

int w_Shape_getGroupIndex(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	int i = 0;
	luax_catchexcept(L, [&]() { i = t->getGroupIndex(); });
	lua_pushinteger(L, i);
	return 1;
}

int w_Shape_setGroupIndex(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	int i = (int) luaL_checkinteger(L, 2);
	luax_catchexcept(L, [&]() { t->setGroupIndex(i); });
	return 0;
}

int w_Shape_destroy(lua_State *L)
{
	Shape *t = luax_checkshape(L, 1);
	luax_catchexcept(L, [&](){ t->destroy(); });
	return 0;
}

int w_Shape_isDestroyed(lua_State *L)
{
	Shape *f = luax_checktype<Shape>(L, 1);
	luax_pushboolean(L, !f->isValid());
	return 1;
}

const luaL_Reg w_Shape_functions[] =
{
	{ "getType", w_Shape_getType },
	{ "getRadius", w_Shape_getRadius },
	{ "getChildCount", w_Shape_getChildCount },
	{ "setFriction", w_Shape_setFriction },
	{ "setRestitution", w_Shape_setRestitution },
	{ "setDensity", w_Shape_setDensity },
	{ "setSensor", w_Shape_setSensor },
	{ "getFriction", w_Shape_getFriction },
	{ "getRestitution", w_Shape_getRestitution },
	{ "getDensity", w_Shape_getDensity },
	{ "getBody", w_Shape_getBody },
	{ "getShape", w_Shape_getShape },
	{ "isSensor", w_Shape_isSensor },
	{ "testPoint", w_Shape_testPoint },
	{ "rayCast", w_Shape_rayCast },
	{ "computeAABB", w_Shape_computeAABB },
	{ "computeMass", w_Shape_computeMass },
	{ "setFilterData", w_Shape_setFilterData },
	{ "getFilterData", w_Shape_getFilterData },
	{ "setCategory", w_Shape_setCategory },
	{ "getCategory", w_Shape_getCategory },
	{ "setMask", w_Shape_setMask },
	{ "getMask", w_Shape_getMask },
	{ "setUserData", w_Shape_setUserData },
	{ "getUserData", w_Shape_getUserData },
	{ "getBoundingBox", w_Shape_getBoundingBox },
	{ "getMassData", w_Shape_getMassData },
	{ "getGroupIndex", w_Shape_getGroupIndex },
	{ "setGroupIndex", w_Shape_setGroupIndex },
	{ "destroy", w_Shape_destroy },
	{ "isDestroyed", w_Shape_isDestroyed },
	{ 0, 0 }
};

extern "C" int luaopen_shape(lua_State *L)
{
	return luax_register_type(L, &Shape::type, w_Shape_functions, nullptr);
}

} // box2d
} // physics
} // love

