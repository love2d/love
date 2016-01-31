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

#include "wrap_Body.h"
#include "wrap_Physics.h"

namespace love
{
namespace physics
{
namespace box2d
{

Body *luax_checkbody(lua_State *L, int idx)
{
	Body *b = luax_checktype<Body>(L, idx, PHYSICS_BODY_ID);
	if (b->body == 0)
		luaL_error(L, "Attempt to use destroyed body.");
	return b;
}

int w_Body_getX(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	lua_pushnumber(L, t->getX());
	return 1;
}

int w_Body_getY(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	lua_pushnumber(L, t->getY());
	return 1;
}

int w_Body_getAngle(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	lua_pushnumber(L, t->getAngle());
	return 1;
}

int w_Body_getPosition(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);

	float x_o, y_o;
	t->getPosition(x_o, y_o);
	lua_pushnumber(L, x_o);
	lua_pushnumber(L, y_o);

	return 2;
}

int w_Body_getLinearVelocity(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);

	float x_o, y_o;
	t->getLinearVelocity(x_o, y_o);
	lua_pushnumber(L, x_o);
	lua_pushnumber(L, y_o);

	return 2;
}

int w_Body_getWorldCenter(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);

	float x_o, y_o;
	t->getWorldCenter(x_o, y_o);
	lua_pushnumber(L, x_o);
	lua_pushnumber(L, y_o);

	return 2;
}

int w_Body_getLocalCenter(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);

	float x_o, y_o;
	t->getLocalCenter(x_o, y_o);
	lua_pushnumber(L, x_o);
	lua_pushnumber(L, y_o);

	return 2;
}

int w_Body_getAngularVelocity(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	lua_pushnumber(L, t->getAngularVelocity());
	return 1;
}

int w_Body_getMass(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	lua_pushnumber(L, t->getMass());
	return 1;
}

int w_Body_getInertia(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	lua_pushnumber(L, t->getInertia());
	return 1;
}

int w_Body_getMassData(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	lua_remove(L, 1);
	return t->getMassData(L);
}

int w_Body_getAngularDamping(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	lua_pushnumber(L, t->getAngularDamping());
	return 1;
}

int w_Body_getLinearDamping(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	lua_pushnumber(L, t->getLinearDamping());
	return 1;
}

int w_Body_getGravityScale(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	lua_pushnumber(L, t->getGravityScale());
	return 1;
}

int w_Body_getType(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	const char *type = "";
	Body::getConstant(t->getType(), type);
	lua_pushstring(L, type);
	return 1;
}

int w_Body_applyLinearImpulse(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	float jx = (float)luaL_checknumber(L, 2);
	float jy = (float)luaL_checknumber(L, 3);

	int nargs = lua_gettop(L);

	if (nargs <= 3 || (nargs == 4 && lua_type(L, 4) == LUA_TBOOLEAN))
	{
		bool awake = luax_optboolean(L, 4, true);
		t->applyLinearImpulse(jx, jy, awake);
	}
	else if (nargs >= 5)
	{
		float rx = (float)luaL_checknumber(L, 4);
		float ry = (float)luaL_checknumber(L, 5);
		bool awake = luax_optboolean(L, 6, true);
		t->applyLinearImpulse(jx, jy, rx, ry, awake);
	}
	else
	{
		return luaL_error(L, "Wrong number of parameters.");
	}

	return 0;
}

int w_Body_applyAngularImpulse(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	float i = (float)luaL_checknumber(L, 2);
	bool awake = luax_optboolean(L, 3, true);
	t->applyAngularImpulse(i, awake);
	return 0;
}

int w_Body_applyTorque(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	float arg = (float)luaL_checknumber(L, 2);
	bool awake = luax_optboolean(L, 3, true);
	t->applyTorque(arg, awake);
	return 0;
}

int w_Body_applyForce(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	float fx = (float)luaL_checknumber(L, 2);
	float fy = (float)luaL_checknumber(L, 3);

	int nargs = lua_gettop(L);

	if (nargs <= 3 || (nargs == 4 && lua_type(L, 4) == LUA_TBOOLEAN))
	{
		bool awake = luax_optboolean(L, 4, true);
		t->applyForce(fx, fy, awake);
	}
	else if (lua_gettop(L) >= 5)
	{
		float rx = (float)luaL_checknumber(L, 4);
		float ry = (float)luaL_checknumber(L, 5);
		bool awake = luax_optboolean(L, 6, true);
		t->applyForce(fx, fy, rx, ry, awake);
	}
	else
	{
		return luaL_error(L, "Wrong number of parameters.");
	}

	return 0;
}

int w_Body_setX(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	luax_catchexcept(L, [&](){ t->setX(arg1); });
	return 0;
}

int w_Body_setY(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	luax_catchexcept(L, [&](){ t->setY(arg1); });
	return 0;
}

int w_Body_setLinearVelocity(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	float arg2 = (float)luaL_checknumber(L, 3);
	t->setLinearVelocity(arg1, arg2);
	return 0;
}

int w_Body_setAngle(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	luax_catchexcept(L, [&](){ t->setAngle(arg1); });
	return 0;
}

int w_Body_setAngularVelocity(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	t->setAngularVelocity(arg1);
	return 0;
}

int w_Body_setPosition(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	float arg2 = (float)luaL_checknumber(L, 3);
	luax_catchexcept(L, [&](){ t->setPosition(arg1, arg2); });
	return 0;
}

int w_Body_resetMassData(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	luax_catchexcept(L, [&](){ t->resetMassData(); });
	return 0;
}

int w_Body_setMassData(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	float x = (float)luaL_checknumber(L, 2);
	float y = (float)luaL_checknumber(L, 3);
	float m = (float)luaL_checknumber(L, 4);
	float i = (float)luaL_checknumber(L, 5);
	luax_catchexcept(L, [&](){ t->setMassData(x, y, m, i); });
	return 0;
}

int w_Body_setMass(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	float m = (float)luaL_checknumber(L, 2);
	luax_catchexcept(L, [&](){ t->setMass(m); });
	return 0;
}

int w_Body_setInertia(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	float i = (float)luaL_checknumber(L, 2);
	luax_catchexcept(L, [&](){ t->setInertia(i); });
	return 0;
}

int w_Body_setAngularDamping(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	t->setAngularDamping(arg1);
	return 0;
}

int w_Body_setLinearDamping(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	t->setLinearDamping(arg1);
	return 0;
}

int w_Body_setGravityScale(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	t->setGravityScale(arg1);
	return 0;
}

int w_Body_setType(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	const char *typeStr = luaL_checkstring(L, 2);
	Body::Type type;
	Body::getConstant(typeStr, type);
	luax_catchexcept(L, [&](){ t->setType(type); });
	return 0;
}

int w_Body_getWorldPoint(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);

	float x = (float)luaL_checknumber(L, 2);
	float y = (float)luaL_checknumber(L, 3);
	float x_o, y_o;
	t->getWorldPoint(x, y, x_o, y_o);
	lua_pushnumber(L, x_o);
	lua_pushnumber(L, y_o);

	return 2;
}

int w_Body_getWorldVector(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);

	float x = (float)luaL_checknumber(L, 2);
	float y = (float)luaL_checknumber(L, 3);
	float x_o, y_o;
	t->getWorldVector(x, y, x_o, y_o);
	lua_pushnumber(L, x_o);
	lua_pushnumber(L, y_o);

	return 2;
}

int w_Body_getWorldPoints(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	lua_remove(L, 1);
	return t->getWorldPoints(L);
}

int w_Body_getLocalPoint(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);

	float x = (float)luaL_checknumber(L, 2);
	float y = (float)luaL_checknumber(L, 3);
	float x_o, y_o;
	t->getLocalPoint(x, y, x_o, y_o);
	lua_pushnumber(L, x_o);
	lua_pushnumber(L, y_o);

	return 2;
}

int w_Body_getLocalVector(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);

	float x = (float)luaL_checknumber(L, 2);
	float y = (float)luaL_checknumber(L, 3);
	float x_o, y_o;
	t->getLocalVector(x, y, x_o, y_o);
	lua_pushnumber(L, x_o);
	lua_pushnumber(L, y_o);

	return 2;
}

int w_Body_getLinearVelocityFromWorldPoint(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);

	float x = (float)luaL_checknumber(L, 2);
	float y = (float)luaL_checknumber(L, 3);
	float x_o, y_o;
	t->getLinearVelocityFromWorldPoint(x, y, x_o, y_o);
	lua_pushnumber(L, x_o);
	lua_pushnumber(L, y_o);

	return 2;
}

int w_Body_getLinearVelocityFromLocalPoint(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);

	float x = (float)luaL_checknumber(L, 2);
	float y = (float)luaL_checknumber(L, 3);
	float x_o, y_o;
	t->getLinearVelocityFromLocalPoint(x, y, x_o, y_o);
	lua_pushnumber(L, x_o);
	lua_pushnumber(L, y_o);

	return 2;
}

int w_Body_isBullet(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	luax_pushboolean(L, t->isBullet());
	return 1;
}

int w_Body_setBullet(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	bool b = luax_toboolean(L, 2);
	t->setBullet(b);
	return 0;
}

int w_Body_isActive(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	luax_pushboolean(L, t->isActive());
	return 1;
}

int w_Body_isAwake(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	luax_pushboolean(L, t->isAwake());
	return 1;
}

int w_Body_setSleepingAllowed(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	bool b = luax_toboolean(L, 2);
	t->setSleepingAllowed(b);
	return 0;
}

int w_Body_isSleepingAllowed(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	lua_pushboolean(L, t->isSleepingAllowed());
	return 1;
}

int w_Body_setActive(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	bool b = luax_toboolean(L, 2);
	luax_catchexcept(L, [&](){ t->setActive(b); });
	return 0;
}

int w_Body_setAwake(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	bool b = luax_toboolean(L, 2);
	t->setAwake(b);
	return 0;
}

int w_Body_setFixedRotation(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	bool b = luax_toboolean(L, 2);
	luax_catchexcept(L, [&](){ t->setFixedRotation(b); });
	return 0;
}

int w_Body_isFixedRotation(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	bool b = t->isFixedRotation();
	luax_pushboolean(L, b);
	return 1;
}

int w_Body_getWorld(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	World *world = t->getWorld();
	luax_pushtype(L, PHYSICS_WORLD_ID, world);
	return 1;
}

int w_Body_getFixtureList(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	lua_remove(L, 1);
	int n = 0;
	luax_catchexcept(L, [&](){ n = t->getFixtureList(L); });
	return n;
}

int w_Body_getJointList(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	lua_remove(L, 1);
	int n = 0;
	luax_catchexcept(L, [&](){ n = t->getJointList(L); });
	return n;
}

int w_Body_getContactList(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	lua_remove(L, 1);
	int n = 0;
	luax_catchexcept(L, [&](){ n = t->getContactList(L); });
	return n;
}

int w_Body_destroy(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	luax_catchexcept(L, [&](){ t->destroy(); });
	return 0;
}

int w_Body_isDestroyed(lua_State *L)
{
	Body *b = luax_checktype<Body>(L, 1, PHYSICS_BODY_ID);
	luax_pushboolean(L, b->body == nullptr);
	return 1;
}

int w_Body_setUserData(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	lua_remove(L, 1);
	return t->setUserData(L);
}

int w_Body_getUserData(lua_State *L)
{
	Body *t = luax_checkbody(L, 1);
	lua_remove(L, 1);
	return t->getUserData(L);
}

static const luaL_Reg w_Body_functions[] =
{
	{ "getX", w_Body_getX },
	{ "getY", w_Body_getY },
	{ "getAngle", w_Body_getAngle },
	{ "getPosition", w_Body_getPosition },
	{ "getLinearVelocity", w_Body_getLinearVelocity },
	{ "getWorldCenter", w_Body_getWorldCenter },
	{ "getLocalCenter", w_Body_getLocalCenter },
	{ "getAngularVelocity", w_Body_getAngularVelocity },
	{ "getMass", w_Body_getMass },
	{ "getInertia", w_Body_getInertia },
	{ "getMassData", w_Body_getMassData },
	{ "getAngularDamping", w_Body_getAngularDamping },
	{ "getLinearDamping", w_Body_getLinearDamping },
	{ "getGravityScale", w_Body_getGravityScale },
	{ "getType", w_Body_getType },
	{ "applyLinearImpulse", w_Body_applyLinearImpulse },
	{ "applyAngularImpulse", w_Body_applyAngularImpulse },
	{ "applyTorque", w_Body_applyTorque },
	{ "applyForce", w_Body_applyForce },
	{ "setX", w_Body_setX },
	{ "setY", w_Body_setY },
	{ "setLinearVelocity", w_Body_setLinearVelocity },
	{ "setAngle", w_Body_setAngle },
	{ "setAngularVelocity", w_Body_setAngularVelocity },
	{ "setPosition", w_Body_setPosition },
	{ "resetMassData", w_Body_resetMassData },
	{ "setMassData", w_Body_setMassData },
	{ "setMass", w_Body_setMass },
	{ "setInertia", w_Body_setInertia },
	{ "setAngularDamping", w_Body_setAngularDamping },
	{ "setLinearDamping", w_Body_setLinearDamping },
	{ "setGravityScale", w_Body_setGravityScale },
	{ "setType", w_Body_setType },
	{ "getWorldPoint", w_Body_getWorldPoint },
	{ "getWorldVector", w_Body_getWorldVector },
	{ "getWorldPoints", w_Body_getWorldPoints },
	{ "getLocalPoint", w_Body_getLocalPoint },
	{ "getLocalVector", w_Body_getLocalVector },
	{ "getLinearVelocityFromWorldPoint", w_Body_getLinearVelocityFromWorldPoint },
	{ "getLinearVelocityFromLocalPoint", w_Body_getLinearVelocityFromLocalPoint },
	{ "isBullet", w_Body_isBullet },
	{ "setBullet", w_Body_setBullet },
	{ "isActive", w_Body_isActive },
	{ "isAwake", w_Body_isAwake },
	{ "setSleepingAllowed", w_Body_setSleepingAllowed },
	{ "isSleepingAllowed", w_Body_isSleepingAllowed },
	{ "setActive", w_Body_setActive },
	{ "setAwake", w_Body_setAwake },
	{ "setFixedRotation", w_Body_setFixedRotation },
	{ "isFixedRotation", w_Body_isFixedRotation },
	{ "getWorld", w_Body_getWorld },
	{ "getFixtureList", w_Body_getFixtureList },
	{ "getJointList", w_Body_getJointList },
	{ "getContactList", w_Body_getContactList },
	{ "destroy", w_Body_destroy },
	{ "isDestroyed", w_Body_isDestroyed },
	{ "setUserData", w_Body_setUserData },
	{ "getUserData", w_Body_getUserData },
	{ 0, 0 }
};

extern "C" int luaopen_body(lua_State *L)
{
	return luax_register_type(L, PHYSICS_BODY_ID, "Body", w_Body_functions, nullptr);
}

} // box2d
} // physics
} // love
