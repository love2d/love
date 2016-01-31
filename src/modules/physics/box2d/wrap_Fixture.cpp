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

#include "wrap_Fixture.h"
#include "common/StringMap.h"

namespace love
{
namespace physics
{
namespace box2d
{

Fixture *luax_checkfixture(lua_State *L, int idx)
{
	Fixture *f = luax_checktype<Fixture>(L, idx, PHYSICS_FIXTURE_ID);
	if (!f->isValid())
		luaL_error(L, "Attempt to use destroyed fixture.");
	return f;
}

int w_Fixture_getType(lua_State *L)
{
	Fixture *t = luax_checkfixture(L, 1);
	const char *type = "";
	Shape::getConstant(t->getType(), type);
	lua_pushstring(L, type);
	return 1;
}

int w_Fixture_setFriction(lua_State *L)
{
	Fixture *t = luax_checkfixture(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	t->setFriction(arg1);
	return 0;
}

int w_Fixture_setRestitution(lua_State *L)
{
	Fixture *t = luax_checkfixture(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	t->setRestitution(arg1);
	return 0;
}

int w_Fixture_setDensity(lua_State *L)
{
	Fixture *t = luax_checkfixture(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	luax_catchexcept(L, [&](){ t->setDensity(arg1); });
	return 0;
}

int w_Fixture_setSensor(lua_State *L)
{
	Fixture *t = luax_checkfixture(L, 1);
	bool arg1 = luax_toboolean(L, 2);
	t->setSensor(arg1);
	return 0;
}

int w_Fixture_getFriction(lua_State *L)
{
	Fixture *t = luax_checkfixture(L, 1);
	lua_pushnumber(L, t->getFriction());
	return 1;
}

int w_Fixture_getRestitution(lua_State *L)
{
	Fixture *t = luax_checkfixture(L, 1);
	lua_pushnumber(L, t->getRestitution());
	return 1;
}

int w_Fixture_getDensity(lua_State *L)
{
	Fixture *t = luax_checkfixture(L, 1);
	lua_pushnumber(L, t->getDensity());
	return 1;
}

int w_Fixture_isSensor(lua_State *L)
{
	Fixture *t = luax_checkfixture(L, 1);
	luax_pushboolean(L, t->isSensor());
	return 1;
}

int w_Fixture_getBody(lua_State *L)
{
	Fixture *t = luax_checkfixture(L, 1);
	Body *body = t->getBody();
	if (body == 0)
		return 0;
	luax_pushtype(L, PHYSICS_BODY_ID, body);
	return 1;
}

int w_Fixture_getShape(lua_State *L)
{
	Fixture *t = luax_checkfixture(L, 1);
	Shape *shape = t->getShape();
	if (shape == 0)
		return 0;
	switch (shape->getType())
	{
	case Shape::SHAPE_EDGE:
		luax_pushtype(L, PHYSICS_EDGE_SHAPE_ID, shape);
		break;
	case Shape::SHAPE_CHAIN:
		luax_pushtype(L, PHYSICS_CHAIN_SHAPE_ID, shape);
		break;
	case Shape::SHAPE_CIRCLE:
		luax_pushtype(L, PHYSICS_CIRCLE_SHAPE_ID, shape);
		break;
	case Shape::SHAPE_POLYGON:
		luax_pushtype(L, PHYSICS_POLYGON_SHAPE_ID, shape);
		break;
	default:
		luax_pushtype(L, PHYSICS_SHAPE_ID, shape);
		break;
	}
	shape->release();
	return 1;
}

int w_Fixture_testPoint(lua_State *L)
{
	Fixture *t = luax_checkfixture(L, 1);
	float x = (float)luaL_checknumber(L, 2);
	float y = (float)luaL_checknumber(L, 3);
	luax_pushboolean(L, t->testPoint(x, y));
	return 1;
}

int w_Fixture_rayCast(lua_State *L)
{
	Fixture *t = luax_checkfixture(L, 1);
	lua_remove(L, 1);
	int ret = 0;
	luax_catchexcept(L, [&](){ ret = t->rayCast(L); });
	return ret;
}

int w_Fixture_setFilterData(lua_State *L)
{
	Fixture *t = luax_checkfixture(L, 1);
	int v[3];
	v[0] = (int) luaL_checknumber(L, 2);
	v[1] = (int) luaL_checknumber(L, 3);
	v[2] = (int) luaL_checknumber(L, 4);
	t->setFilterData(v);
	return 0;
}

int w_Fixture_getFilterData(lua_State *L)
{
	Fixture *t = luax_checkfixture(L, 1);
	int v[3];
	t->getFilterData(v);
	lua_pushinteger(L, v[0]);
	lua_pushinteger(L, v[1]);
	lua_pushinteger(L, v[2]);
	return 3;
}

int w_Fixture_setCategory(lua_State *L)
{
	Fixture *t = luax_checkfixture(L, 1);
	lua_remove(L, 1);
	return t->setCategory(L);
}

int w_Fixture_getCategory(lua_State *L)
{
	Fixture *t = luax_checkfixture(L, 1);
	lua_remove(L, 1);
	return t->getCategory(L);
}

int w_Fixture_setMask(lua_State *L)
{
	Fixture *t = luax_checkfixture(L, 1);
	lua_remove(L, 1);
	return t->setMask(L);
}

int w_Fixture_getMask(lua_State *L)
{
	Fixture *t = luax_checkfixture(L, 1);
	lua_remove(L, 1);
	return t->getMask(L);
}

int w_Fixture_setUserData(lua_State *L)
{
	Fixture *t = luax_checkfixture(L, 1);
	lua_remove(L, 1);
	return t->setUserData(L);
}

int w_Fixture_getUserData(lua_State *L)
{
	Fixture *t = luax_checkfixture(L, 1);
	lua_remove(L, 1);
	return t->getUserData(L);
}

int w_Fixture_getBoundingBox(lua_State *L)
{
	Fixture *t = luax_checkfixture(L, 1);
	lua_remove(L, 1);
	return t->getBoundingBox(L);
}

int w_Fixture_getMassData(lua_State *L)
{
	Fixture *t = luax_checkfixture(L, 1);
	lua_remove(L, 1);
	return t->getMassData(L);
}

int w_Fixture_getGroupIndex(lua_State *L)
{
	Fixture *t = luax_checkfixture(L, 1);
	int i = t->getGroupIndex();
	lua_pushinteger(L, i);
	return 1;
}

int w_Fixture_setGroupIndex(lua_State *L)
{
	Fixture *t = luax_checkfixture(L, 1);
	int i = (int) luaL_checknumber(L, 2);
	t->setGroupIndex(i);
	return 0;
}

int w_Fixture_destroy(lua_State *L)
{
	Fixture *t = luax_checkfixture(L, 1);
	luax_catchexcept(L, [&](){ t->destroy(); });
	return 0;
}

int w_Fixture_isDestroyed(lua_State *L)
{
	Fixture *f = luax_checktype<Fixture>(L, 1, PHYSICS_FIXTURE_ID);
	luax_pushboolean(L, !f->isValid());
	return 1;
}

static const luaL_Reg w_Fixture_functions[] =
{
	{ "getType", w_Fixture_getType },
	{ "setFriction", w_Fixture_setFriction },
	{ "setRestitution", w_Fixture_setRestitution },
	{ "setDensity", w_Fixture_setDensity },
	{ "setSensor", w_Fixture_setSensor },
	{ "getFriction", w_Fixture_getFriction },
	{ "getRestitution", w_Fixture_getRestitution },
	{ "getDensity", w_Fixture_getDensity },
	{ "getBody", w_Fixture_getBody },
	{ "getShape", w_Fixture_getShape },
	{ "isSensor", w_Fixture_isSensor },
	{ "testPoint", w_Fixture_testPoint },
	{ "rayCast", w_Fixture_rayCast },
	{ "setFilterData", w_Fixture_setFilterData },
	{ "getFilterData", w_Fixture_getFilterData },
	{ "setCategory", w_Fixture_setCategory },
	{ "getCategory", w_Fixture_getCategory },
	{ "setMask", w_Fixture_setMask },
	{ "getMask", w_Fixture_getMask },
	{ "setUserData", w_Fixture_setUserData },
	{ "getUserData", w_Fixture_getUserData },
	{ "getBoundingBox", w_Fixture_getBoundingBox },
	{ "getMassData", w_Fixture_getMassData },
	{ "getGroupIndex", w_Fixture_getGroupIndex },
	{ "setGroupIndex", w_Fixture_setGroupIndex },
	{ "destroy", w_Fixture_destroy },
	{ "isDestroyed", w_Fixture_isDestroyed },
	{ 0, 0 }
};

extern "C" int luaopen_fixture(lua_State *L)
{
	return luax_register_type(L, PHYSICS_FIXTURE_ID, "Fixture", w_Fixture_functions, nullptr);
}

} // box2d
} // physics
} // love

