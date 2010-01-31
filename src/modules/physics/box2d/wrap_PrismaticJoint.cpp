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

#include "wrap_PrismaticJoint.h"

namespace love
{
namespace physics
{
namespace box2d
{
	PrismaticJoint * luax_checkprismaticjoint(lua_State * L, int idx)
	{
		return luax_checktype<PrismaticJoint>(L, idx, "PrismaticJoint", PHYSICS_PRISMATIC_JOINT_T);
	}

	int w_PrismaticJoint_getJointTranslation(lua_State * L)
	{
		PrismaticJoint * t = luax_checkprismaticjoint(L, 1);
		lua_pushnumber(L, t->getJointTranslation());
		return 1;
	}

	int w_PrismaticJoint_getJointSpeed(lua_State * L)
	{
		PrismaticJoint * t = luax_checkprismaticjoint(L, 1);
		lua_pushnumber(L, t->getJointSpeed());
		return 1;
	}

	int w_PrismaticJoint_setMotorEnabled(lua_State * L)
	{
		PrismaticJoint * t = luax_checkprismaticjoint(L, 1);
		bool arg1 = luax_toboolean(L, 2);
		t->setMotorEnabled(arg1);
		return 0;
	}

	int w_PrismaticJoint_isMotorEnabled(lua_State * L)
	{
		PrismaticJoint * t = luax_checkprismaticjoint(L, 1);
		luax_pushboolean(L, t->isMotorEnabled());
		return 1;
	}

	int w_PrismaticJoint_setMaxMotorForce(lua_State * L)
	{
		PrismaticJoint * t = luax_checkprismaticjoint(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		t->setMaxMotorForce(arg1);
		return 0;
	}

	int w_PrismaticJoint_getMaxMotorForce(lua_State * L)
	{
		PrismaticJoint * t = luax_checkprismaticjoint(L, 1);
		lua_pushnumber(L, t->getMaxMotorForce());
		return 1;
	}

	int w_PrismaticJoint_setMotorSpeed(lua_State * L)
	{
		PrismaticJoint * t = luax_checkprismaticjoint(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		t->setMotorSpeed(arg1);
		return 0;
	}

	int w_PrismaticJoint_getMotorSpeed(lua_State * L)
	{
		PrismaticJoint * t = luax_checkprismaticjoint(L, 1);
		lua_pushnumber(L, t->getMotorSpeed());
		return 1;
	}

	int w_PrismaticJoint_getMotorForce(lua_State * L)
	{
		PrismaticJoint * t = luax_checkprismaticjoint(L, 1);
		lua_pushnumber(L, t->getMotorForce());
		return 1;
	}

	int w_PrismaticJoint_setLimitsEnabled(lua_State * L)
	{
		PrismaticJoint * t = luax_checkprismaticjoint(L, 1);
		bool arg1 = luax_toboolean(L, 2);
		t->setLimitsEnabled(arg1);
		return 0;
	}

	int w_PrismaticJoint_isLimitsEnabled(lua_State * L)
	{
		PrismaticJoint * t = luax_checkprismaticjoint(L, 1);
		luax_pushboolean(L, t->isLimitsEnabled());
		return 1;
	}

	int w_PrismaticJoint_setUpperLimit(lua_State * L)
	{
		PrismaticJoint * t = luax_checkprismaticjoint(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		t->setUpperLimit(arg1);
		return 0;
	}

	int w_PrismaticJoint_setLowerLimit(lua_State * L)
	{
		PrismaticJoint * t = luax_checkprismaticjoint(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		t->setLowerLimit(arg1);
		return 0;
	}

	int w_PrismaticJoint_setLimits(lua_State * L)
	{
		PrismaticJoint * t = luax_checkprismaticjoint(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		float arg2 = (float)luaL_checknumber(L, 3);
		t->setLimits(arg1, arg2);
		return 0;
	}

	int w_PrismaticJoint_getLowerLimit(lua_State * L)
	{
		PrismaticJoint * t = luax_checkprismaticjoint(L, 1);
		lua_pushnumber(L, t->getLowerLimit());
		return 1;
	}

	int w_PrismaticJoint_getUpperLimit(lua_State * L)
	{
		PrismaticJoint * t = luax_checkprismaticjoint(L, 1);
		lua_pushnumber(L, t->getUpperLimit());
		return 1;
	}

	int w_PrismaticJoint_getLimits(lua_State * L)
	{
		PrismaticJoint * t = luax_checkprismaticjoint(L, 1);
		lua_remove(L, 1);
		return t->getLimits(L);
	}

	static const luaL_Reg functions[] = {
		{ "getJointTranslation", w_PrismaticJoint_getJointTranslation },
		{ "getJointSpeed", w_PrismaticJoint_getJointSpeed },
		{ "setMotorEnabled", w_PrismaticJoint_setMotorEnabled },
		{ "isMotorEnabled", w_PrismaticJoint_isMotorEnabled },
		{ "setMaxMotorForce", w_PrismaticJoint_setMaxMotorForce },
		{ "getMaxMotorForce", w_PrismaticJoint_getMaxMotorForce },
		{ "setMotorSpeed", w_PrismaticJoint_setMotorSpeed },
		{ "getMotorSpeed", w_PrismaticJoint_getMotorSpeed },
		{ "getMotorForce", w_PrismaticJoint_getMotorForce },
		{ "setLimitsEnabled", w_PrismaticJoint_setLimitsEnabled },
		{ "isLimitsEnabled", w_PrismaticJoint_isLimitsEnabled },
		{ "setUpperLimit", w_PrismaticJoint_setUpperLimit },
		{ "setLowerLimit", w_PrismaticJoint_setLowerLimit },
		{ "setLimits", w_PrismaticJoint_setLimits },
		{ "getLowerLimit", w_PrismaticJoint_getLowerLimit },
		{ "getUpperLimit", w_PrismaticJoint_getUpperLimit },
		{ "getLimits", w_PrismaticJoint_getLimits },
		// From Joint.
		{ "getType", w_Joint_getType },
		{ "getAnchors", w_Joint_getAnchors },
		{ "getReactionForce", w_Joint_getReactionForce },
		{ "getReactionTorque", w_Joint_getReactionTorque },
		{ "setCollideConnected", w_Joint_setCollideConnected },
		{ "getCollideConnected", w_Joint_getCollideConnected },
		{ "destroy", w_Joint_destroy },
		{ 0, 0 }
	};

	int luaopen_prismaticjoint(lua_State * L)
	{
		return luax_register_type(L, "PrismaticJoint", functions);
	}

} // box2d
} // physics
} // love
