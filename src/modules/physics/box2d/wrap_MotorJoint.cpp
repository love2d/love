/**
 * Copyright (c) 2006-2014 LOVE Development Team
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

#include "wrap_MotorJoint.h"

namespace love
{
namespace physics
{
namespace box2d
{

MotorJoint *luax_checkmotorjoint(lua_State *L, int idx)
{
	MotorJoint *j = luax_checktype<MotorJoint>(L, idx, "MotorJoint", PHYSICS_MOTOR_JOINT_T);
	if (!j->isValid())
		luaL_error(L, "Attempt to use destroyed joint.");
	return j;
}

int w_MotorJoint_setLinearOffset(lua_State *L)
{
	MotorJoint *t = luax_checkmotorjoint(L, 1);
	float x = (float) luaL_checknumber(L, 2);
	float y = (float) luaL_checknumber(L, 3);
	t->setLinearOffset(x, y);
	return 0;
}

int w_MotorJoint_getLinearOffset(lua_State *L)
{
	MotorJoint *t = luax_checkmotorjoint(L, 1);
	return t->getLinearOffset(L);
}

int w_MotorJoint_setAngularOffset(lua_State *L)
{
	MotorJoint *t = luax_checkmotorjoint(L, 1);
	float arg1 = (float) luaL_checknumber(L, 2);
	t->setAngularOffset(arg1);
	return 0;
}

int w_MotorJoint_getAngularOffset(lua_State *L)
{
	MotorJoint *t = luax_checkmotorjoint(L, 1);
	lua_pushnumber(L, t->getAngularOffset());
	return 1;
}

int w_MotorJoint_setMaxForce(lua_State *L)
{
	MotorJoint *t = luax_checkmotorjoint(L, 1);
	float arg1 = (float) luaL_checknumber(L, 2);
	EXCEPT_GUARD(t->setMaxForce(arg1);)
	return 0;
}

int w_MotorJoint_getMaxForce(lua_State *L)
{
	MotorJoint *t = luax_checkmotorjoint(L, 1);
	lua_pushnumber(L, t->getMaxForce());
	return 1;
}

int w_MotorJoint_setMaxTorque(lua_State *L)
{
	MotorJoint *t = luax_checkmotorjoint(L, 1);
	float arg1 = (float) luaL_checknumber(L, 2);
	EXCEPT_GUARD(t->setMaxTorque(arg1);)
	return 0;
}

int w_MotorJoint_getMaxTorque(lua_State *L)
{
	MotorJoint *t = luax_checkmotorjoint(L, 1);
	lua_pushnumber(L, t->getMaxTorque());
	return 1;
}

int w_MotorJoint_setCorrectionFactor(lua_State *L)
{
	MotorJoint *t = luax_checkmotorjoint(L, 1);
	float arg1 = (float) luaL_checknumber(L, 2);
	EXCEPT_GUARD(t->setCorrectionFactor(arg1);)
	return 0;
}

int w_MotorJoint_getCorrectionFactor(lua_State *L)
{
	MotorJoint *t = luax_checkmotorjoint(L, 1);
	lua_pushnumber(L, t->getCorrectionFactor());
	return 1;
}

static const luaL_Reg functions[] =
{
	{ "setLinearOffset", w_MotorJoint_setLinearOffset },
	{ "getLinearOffset", w_MotorJoint_getLinearOffset },
	{ "setAngularOffset", w_MotorJoint_setAngularOffset },
	{ "getAngularOffset", w_MotorJoint_getAngularOffset },
	{ "setMaxForce", w_MotorJoint_setMaxForce },
	{ "getMaxForce", w_MotorJoint_getMaxForce },
	{ "setMaxTorque", w_MotorJoint_setMaxTorque },
	{ "getMaxTorque", w_MotorJoint_getMaxTorque },
	{ "setCorrectionFactor", w_MotorJoint_setCorrectionFactor },
	{ "getCorrectionFactor", w_MotorJoint_getCorrectionFactor },
	// From Joint.
	{ "getType", w_Joint_getType },
	{ "getAnchors", w_Joint_getAnchors },
	{ "getReactionForce", w_Joint_getReactionForce },
	{ "getReactionTorque", w_Joint_getReactionTorque },
	{ "getCollideConnected", w_Joint_getCollideConnected },
	{ "destroy", w_Joint_destroy },
	{ 0, 0 }
};

extern "C" int luaopen_motorjoint(lua_State *L)
{
	return luax_register_type(L, "MotorJoint", functions);
}


} // box2d
} // phyics
} // love
