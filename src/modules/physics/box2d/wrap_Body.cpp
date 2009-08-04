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

#include "wrap_Body.h"

namespace love
{
namespace physics
{
namespace box2d
{
	Body * luax_checkbody(lua_State * L, int idx)
	{
		return luax_checktype<Body>(L, idx, "Body", LOVE_PHYSICS_BODY_BITS);
	}

	int _wrap_Body_getX(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		lua_pushnumber(L, t->getX());
		return 1;
	}

	int _wrap_Body_getY(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		lua_pushnumber(L, t->getY());
		return 1;
	}

	int _wrap_Body_getAngle(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		lua_pushnumber(L, t->getAngle());
		return 1;
	}

	int _wrap_Body_getPosition(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		return t->getPosition(L);
	}

	int _wrap_Body_getLinearVelocity(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		return t->getLinearVelocity(L);
	}

	int _wrap_Body_getWorldCenter(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		return t->getWorldCenter(L);
	}

	int _wrap_Body_getLocalCenter(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		return t->getLocalCenter(L);
	}

	int _wrap_Body_getAngularVelocity(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		lua_pushnumber(L, t->getAngularVelocity());
		return 1;
	}

	int _wrap_Body_getMass(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		lua_pushnumber(L, t->getMass());
		return 1;
	}

	int _wrap_Body_getInertia(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		lua_pushnumber(L, t->getInertia());
		return 1;
	}

	int _wrap_Body_getAngularDamping(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		lua_pushnumber(L, t->getAngularDamping());
		return 1;
	}

	int _wrap_Body_getLinearDamping(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		lua_pushnumber(L, t->getLinearDamping());
		return 1;
	}

	int _wrap_Body_applyImpulse(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		float jx = (float)luaL_checknumber(L, 2);
		float jy = (float)luaL_checknumber(L, 3);
		float rx = (float)luaL_optnumber(L, 4, 0);
		float ry = (float)luaL_optnumber(L, 5, 0);
		t->applyImpulse(jx, jy, rx, ry);
		return 0;
	}

	int _wrap_Body_applyTorque(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		float arg = (float)luaL_checknumber(L, 2);
		t->applyTorque(arg);
		return 0;
	}

	int _wrap_Body_applyForce(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		float fx = (float)luaL_checknumber(L, 2);
		float fy = (float)luaL_checknumber(L, 3);
		float rx = (float)luaL_optnumber(L, 4, 0);
		float ry = (float)luaL_optnumber(L, 5, 0);
		t->applyForce(fx, fy, rx, ry);
		return 0;
	}

	int _wrap_Body_setX(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		t->setX(arg1);
		return 0;
	}

	int _wrap_Body_setY(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		t->setY(arg1);
		return 0;
	}

	int _wrap_Body_setLinearVelocity(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		float arg2 = (float)luaL_checknumber(L, 3);
		t->setLinearVelocity(arg1, arg2);
		return 0;
	}

	int _wrap_Body_setAngle(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		t->setAngle(arg1);
		return 0;
	}

	int _wrap_Body_setAngularVelocity(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		t->setAngularVelocity(arg1);
		return 0;
	}

	int _wrap_Body_setPosition(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		float arg2 = (float)luaL_checknumber(L, 3);
		t->setPosition(arg1, arg2);
		return 0;
	}

	int _wrap_Body_setMassFromShapes(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		t->setMassFromShapes();
		return 0;
	}

	int _wrap_Body_setMass(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		float x = (float)luaL_checknumber(L, 2);
		float y = (float)luaL_checknumber(L, 3);
		float m = (float)luaL_checknumber(L, 4);
		float i = (float)luaL_checknumber(L, 5);
		t->setMass(x, y, m, i);
		return 0;
	}

	int _wrap_Body_setAngularDamping(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		t->setAngularDamping(arg1);
		return 0;
	}

	int _wrap_Body_setLinearDamping(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		t->setLinearDamping(arg1);
		return 0;
	}

	int _wrap_Body_getWorldPoint(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		return t->getWorldPoint(L);
	}

	int _wrap_Body_getWorldVector(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		return t->getWorldVector(L);
	}

	int _wrap_Body_getLocalPoint(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		return t->getLocalPoint(L);
	}

	int _wrap_Body_getLocalVector(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		return t->getLocalVector(L);
	}

	int _wrap_Body_getLinearVelocityFromWorldPoint(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		return t->getLinearVelocityFromWorldPoint(L);
	}

	int _wrap_Body_getLinearVelocityFromLocalPoint(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		return t->getLinearVelocityFromLocalPoint(L);
	}

	int _wrap_Body_isBullet(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		luax_pushboolean(L, t->isBullet());
		return 1;
	}

	int _wrap_Body_setBullet(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		bool b = luax_toboolean(L, 2);
		t->setBullet(b);
		return 0;
	}

	int _wrap_Body_isStatic(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		luax_pushboolean(L, t->isStatic());
		return 1;
	}

	int _wrap_Body_isDynamic(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		luax_pushboolean(L, t->isDynamic());
		return 1;
	}

	int _wrap_Body_isFrozen(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		luax_pushboolean(L, t->isFrozen());
		return 1;
	}

	int _wrap_Body_isSleeping(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		luax_pushboolean(L, t->isSleeping());
		return 1;
	}

	int _wrap_Body_setAllowSleeping(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		bool b = luax_toboolean(L, 2);
		t->setAllowSleeping(b);
		return 0;
	}

	int _wrap_Body_putToSleep(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		t->putToSleep();
		return 0;
	}

	int _wrap_Body_wakeUp(lua_State * L)
	{
		Body * t = luax_checkbody(L, 1);
		t->wakeUp();
		return 0;
	}

	static const luaL_Reg wrap_Body_functions[] = {
		{ "getX", _wrap_Body_getX },
		{ "getY", _wrap_Body_getY },
		{ "getAngle", _wrap_Body_getAngle },
		{ "getPosition", _wrap_Body_getPosition },
		{ "getLinearVelocity", _wrap_Body_getLinearVelocity },
		{ "getWorldCenter", _wrap_Body_getWorldCenter },
		{ "getLocalCenter", _wrap_Body_getLocalCenter },
		{ "getAngularVelocity", _wrap_Body_getAngularVelocity },
		{ "getMass", _wrap_Body_getMass },
		{ "getInertia", _wrap_Body_getInertia },
		{ "getAngularDamping", _wrap_Body_getAngularDamping },
		{ "getLinearDamping", _wrap_Body_getLinearDamping },
		{ "applyImpulse", _wrap_Body_applyImpulse },
		{ "applyTorque", _wrap_Body_applyTorque },
		{ "applyForce", _wrap_Body_applyForce },
		{ "setX", _wrap_Body_setX },
		{ "setY", _wrap_Body_setY },
		{ "setLinearVelocity", _wrap_Body_setLinearVelocity },
		{ "setAngle", _wrap_Body_setAngle },
		{ "setAngularVelocity", _wrap_Body_setAngularVelocity },
		{ "setPosition", _wrap_Body_setPosition },
		{ "setMassFromShapes", _wrap_Body_setMassFromShapes },
		{ "setMass", _wrap_Body_setMass },
		{ "setAngularDamping", _wrap_Body_setAngularDamping },
		{ "setLinearDamping", _wrap_Body_setLinearDamping },
		{ "getWorldPoint", _wrap_Body_getWorldPoint },
		{ "getWorldVector", _wrap_Body_getWorldVector },
		{ "getLocalPoint", _wrap_Body_getLocalPoint },
		{ "getLocalVector", _wrap_Body_getLocalVector },
		{ "getLinearVelocityFromWorldPoint", _wrap_Body_getLinearVelocityFromWorldPoint },
		{ "getLinearVelocityFromLocalPoint", _wrap_Body_getLinearVelocityFromLocalPoint },
		{ "isBullet", _wrap_Body_isBullet },
		{ "setBullet", _wrap_Body_setBullet },
		{ "isStatic", _wrap_Body_isStatic },
		{ "isDynamic", _wrap_Body_isDynamic },
		{ "isFrozen", _wrap_Body_isFrozen },
		{ "isSleeping", _wrap_Body_isSleeping },
		{ "setAllowSleeping", _wrap_Body_setAllowSleeping },
		{ "putToSleep", _wrap_Body_putToSleep },
		{ "wakeUp", _wrap_Body_wakeUp },
		{ 0, 0 }
	};

	int wrap_Body_open(lua_State * L)
	{
		luax_register_type(L, "Body", wrap_Body_functions);
		return 0;
	}

} // box2d
} // physics
} // love
