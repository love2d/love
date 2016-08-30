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

#include "wrap_PrismaticJoint.h"
#include "wrap_Physics.h"

namespace love
{
namespace physics
{
namespace box2d
{

PrismaticJoint *luax_checkprismaticjoint(lua_State *L, int idx)
{
	PrismaticJoint *j = luax_checktype<PrismaticJoint>(L, idx, PHYSICS_PRISMATIC_JOINT_ID);
	if (!j->isValid())
		luaL_error(L, "Attempt to use destroyed joint.");
	return j;
}

int w_PrismaticJoint_getJointTranslation(lua_State *L)
{
	PrismaticJoint *t = luax_checkprismaticjoint(L, 1);
	lua_pushnumber(L, t->getJointTranslation());
	return 1;
}

int w_PrismaticJoint_getJointSpeed(lua_State *L)
{
	PrismaticJoint *t = luax_checkprismaticjoint(L, 1);
	lua_pushnumber(L, t->getJointSpeed());
	return 1;
}

int w_PrismaticJoint_setMotorEnabled(lua_State *L)
{
	PrismaticJoint *t = luax_checkprismaticjoint(L, 1);
	bool arg1 = luax_toboolean(L, 2);
	t->setMotorEnabled(arg1);
	return 0;
}

int w_PrismaticJoint_isMotorEnabled(lua_State *L)
{
	PrismaticJoint *t = luax_checkprismaticjoint(L, 1);
	luax_pushboolean(L, t->isMotorEnabled());
	return 1;
}

int w_PrismaticJoint_setMaxMotorForce(lua_State *L)
{
	PrismaticJoint *t = luax_checkprismaticjoint(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	t->setMaxMotorForce(arg1);
	return 0;
}

int w_PrismaticJoint_setMotorSpeed(lua_State *L)
{
	PrismaticJoint *t = luax_checkprismaticjoint(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	t->setMotorSpeed(arg1);
	return 0;
}

int w_PrismaticJoint_getMotorSpeed(lua_State *L)
{
	PrismaticJoint *t = luax_checkprismaticjoint(L, 1);
	lua_pushnumber(L, t->getMotorSpeed());
	return 1;
}

int w_PrismaticJoint_getMotorForce(lua_State *L)
{
	PrismaticJoint *t = luax_checkprismaticjoint(L, 1);
	float inv_dt = (float)luaL_checknumber(L, 2);
	lua_pushnumber(L, t->getMotorForce(inv_dt));
	return 1;
}

int w_PrismaticJoint_getMaxMotorForce(lua_State *L)
{
	PrismaticJoint *t = luax_checkprismaticjoint(L, 1);
	lua_pushnumber(L, t->getMaxMotorForce());
	return 1;
}

int w_PrismaticJoint_setLimitsEnabled(lua_State *L)
{
	PrismaticJoint *t = luax_checkprismaticjoint(L, 1);
	bool arg1 = luax_toboolean(L, 2);
	t->setLimitsEnabled(arg1);
	return 0;
}

int w_PrismaticJoint_hasLimitsEnabled(lua_State *L)
{
	PrismaticJoint *t = luax_checkprismaticjoint(L, 1);
	luax_pushboolean(L, t->hasLimitsEnabled());
	return 1;
}

int w_PrismaticJoint_setUpperLimit(lua_State *L)
{
	PrismaticJoint *t = luax_checkprismaticjoint(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	luax_catchexcept(L, [&](){ t->setUpperLimit(arg1); });
	return 0;
}

int w_PrismaticJoint_setLowerLimit(lua_State *L)
{
	PrismaticJoint *t = luax_checkprismaticjoint(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	luax_catchexcept(L, [&](){ t->setLowerLimit(arg1); });
	return 0;
}

int w_PrismaticJoint_setLimits(lua_State *L)
{
	PrismaticJoint *t = luax_checkprismaticjoint(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	float arg2 = (float)luaL_checknumber(L, 3);
	luax_catchexcept(L, [&](){ t->setLimits(arg1, arg2); });
	return 0;
}

int w_PrismaticJoint_getLowerLimit(lua_State *L)
{
	PrismaticJoint *t = luax_checkprismaticjoint(L, 1);
	lua_pushnumber(L, t->getLowerLimit());
	return 1;
}

int w_PrismaticJoint_getUpperLimit(lua_State *L)
{
	PrismaticJoint *t = luax_checkprismaticjoint(L, 1);
	lua_pushnumber(L, t->getUpperLimit());
	return 1;
}

int w_PrismaticJoint_getLimits(lua_State *L)
{
	PrismaticJoint *t = luax_checkprismaticjoint(L, 1);
	lua_remove(L, 1);
	return t->getLimits(L);
}

int w_PrismaticJoint_getAxis(lua_State *L)
{
	PrismaticJoint *t = luax_checkprismaticjoint(L, 1);
	lua_remove(L, 1);
	return t->getAxis(L);
}

int w_PrismaticJoint_getReferenceAngle(lua_State *L)
{
	PrismaticJoint *t = luax_checkprismaticjoint(L, 1);
	lua_pushnumber(L, t->getReferenceAngle());
	return 1;
}

static const luaL_Reg w_PrismaticJoint_functions[] =
{
	{ "getJointTranslation", w_PrismaticJoint_getJointTranslation },
	{ "getJointSpeed", w_PrismaticJoint_getJointSpeed },
	{ "setMotorEnabled", w_PrismaticJoint_setMotorEnabled },
	{ "isMotorEnabled", w_PrismaticJoint_isMotorEnabled },
	{ "setMaxMotorForce", w_PrismaticJoint_setMaxMotorForce },
	{ "setMotorSpeed", w_PrismaticJoint_setMotorSpeed },
	{ "getMotorSpeed", w_PrismaticJoint_getMotorSpeed },
	{ "getMotorForce", w_PrismaticJoint_getMotorForce },
	{ "getMaxMotorForce", w_PrismaticJoint_getMaxMotorForce },
	{ "setLimitsEnabled", w_PrismaticJoint_setLimitsEnabled },
	{ "hasLimitsEnabled", w_PrismaticJoint_hasLimitsEnabled },
	{ "setUpperLimit", w_PrismaticJoint_setUpperLimit },
	{ "setLowerLimit", w_PrismaticJoint_setLowerLimit },
	{ "setLimits", w_PrismaticJoint_setLimits },
	{ "getLowerLimit", w_PrismaticJoint_getLowerLimit },
	{ "getUpperLimit", w_PrismaticJoint_getUpperLimit },
	{ "getLimits", w_PrismaticJoint_getLimits },
	{ "getAxis", w_PrismaticJoint_getAxis },
	{ "getReferenceAngle", w_PrismaticJoint_getReferenceAngle },
	{ 0, 0 }
};

extern "C" int luaopen_prismaticjoint(lua_State *L)
{
	return luax_register_type(L, PHYSICS_PRISMATIC_JOINT_ID, "PrismaticJoint", w_Joint_functions, w_PrismaticJoint_functions, nullptr);
}

} // box2d
} // physics
} // love
