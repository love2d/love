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

#include "wrap_RevoluteJoint.h"

namespace love
{
namespace physics
{
namespace box2d
{
	RevoluteJoint * luax_checkrevolutejoint(lua_State * L, int idx)
	{
		return luax_checktype<RevoluteJoint>(L, idx, "RevoluteJoint", LOVE_PHYSICS_REVOLUTE_JOINT_BITS);
	}

	int _wrap_RevoluteJoint_getJointAngle(lua_State * L)
	{
		RevoluteJoint * t = luax_checkrevolutejoint(L, 1);
		lua_pushnumber(L, t->getJointAngle());
		return 1;
	}

	int _wrap_RevoluteJoint_getJointSpeed(lua_State * L)
	{
		RevoluteJoint * t = luax_checkrevolutejoint(L, 1);
		lua_pushnumber(L, t->getJointSpeed());
		return 1;
	}

	int _wrap_RevoluteJoint_setMotorEnabled(lua_State * L)
	{
		RevoluteJoint * t = luax_checkrevolutejoint(L, 1);
		bool arg1 = luax_toboolean(L, 2);
		t->setMotorEnabled(arg1);
		return 0;
	}

	int _wrap_RevoluteJoint_isMotorEnabled(lua_State * L)
	{
		RevoluteJoint * t = luax_checkrevolutejoint(L, 1);
		luax_pushboolean(L, t->isMotorEnabled());
		return 1;
	}

	int _wrap_RevoluteJoint_setMaxMotorTorque(lua_State * L)
	{
		RevoluteJoint * t = luax_checkrevolutejoint(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		t->setMaxMotorTorque(arg1);
		return 0;
	}

	int _wrap_RevoluteJoint_getMaxMotorTorque(lua_State * L)
	{
		RevoluteJoint * t = luax_checkrevolutejoint(L, 1);
		lua_pushnumber(L, t->getMaxMotorTorque());
		return 1;
	}

	int _wrap_RevoluteJoint_setMotorSpeed(lua_State * L)
	{
		RevoluteJoint * t = luax_checkrevolutejoint(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		t->setMotorSpeed(arg1);
		return 0;
	}

	int _wrap_RevoluteJoint_getMotorSpeed(lua_State * L)
	{
		RevoluteJoint * t = luax_checkrevolutejoint(L, 1);
		lua_pushnumber(L, t->getMotorSpeed());
		return 1;
	}

	int _wrap_RevoluteJoint_getMotorTorque(lua_State * L)
	{
		RevoluteJoint * t = luax_checkrevolutejoint(L, 1);
		lua_pushnumber(L, t->getMotorTorque());
		return 1;
	}

	int _wrap_RevoluteJoint_setLimitsEnabled(lua_State * L)
	{
		RevoluteJoint * t = luax_checkrevolutejoint(L, 1);
		bool arg1 = luax_toboolean(L, 2);
		t->setLimitsEnabled(arg1);
		return 0;
	}

	int _wrap_RevoluteJoint_isLimitsEnabled(lua_State * L)
	{
		RevoluteJoint * t = luax_checkrevolutejoint(L, 1);
		luax_pushboolean(L, t->isLimitsEnabled());
		return 1;
	}

	int _wrap_RevoluteJoint_setUpperLimit(lua_State * L)
	{
		RevoluteJoint * t = luax_checkrevolutejoint(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		t->setUpperLimit(arg1);
		return 0;
	}

	int _wrap_RevoluteJoint_setLowerLimit(lua_State * L)
	{
		RevoluteJoint * t = luax_checkrevolutejoint(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		t->setLowerLimit(arg1);
		return 0;
	}

	int _wrap_RevoluteJoint_setLimits(lua_State * L)
	{
		RevoluteJoint * t = luax_checkrevolutejoint(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		float arg2 = (float)luaL_checknumber(L, 3);
		t->setLimits(arg1, arg2);
		return 0;
	}

	int _wrap_RevoluteJoint_getLowerLimit(lua_State * L)
	{
		RevoluteJoint * t = luax_checkrevolutejoint(L, 1);
		lua_pushnumber(L, t->getLowerLimit());
		return 1;
	}

	int _wrap_RevoluteJoint_getUpperLimit(lua_State * L)
	{
		RevoluteJoint * t = luax_checkrevolutejoint(L, 1);
		lua_pushnumber(L, t->getUpperLimit());
		return 1;
	}

	int _wrap_RevoluteJoint_getLimits(lua_State * L)
	{
		RevoluteJoint * t = luax_checkrevolutejoint(L, 1);
		lua_remove(L, 1);
		return t->getLimits(L);
	}

	static const luaL_Reg wrap_RevoluteJoint_functions[] = {
		{ "getJointAngle", _wrap_RevoluteJoint_getJointAngle },
		{ "getJointSpeed", _wrap_RevoluteJoint_getJointSpeed },
		{ "setMotorEnabled", _wrap_RevoluteJoint_setMotorEnabled },
		{ "isMotorEnabled", _wrap_RevoluteJoint_isMotorEnabled },
		{ "setMaxMotorTorque", _wrap_RevoluteJoint_setMaxMotorTorque },
		{ "getMaxMotorTorque", _wrap_RevoluteJoint_getMaxMotorTorque },
		{ "setMotorSpeed", _wrap_RevoluteJoint_setMotorSpeed },
		{ "getMotorSpeed", _wrap_RevoluteJoint_getMotorSpeed },
		{ "getMotorTorque", _wrap_RevoluteJoint_getMotorTorque },
		{ "setLimitsEnabled", _wrap_RevoluteJoint_setLimitsEnabled },
		{ "isLimitsEnabled", _wrap_RevoluteJoint_isLimitsEnabled },
		{ "setUpperLimit", _wrap_RevoluteJoint_setUpperLimit },
		{ "setLowerLimit", _wrap_RevoluteJoint_setLowerLimit },
		{ "setLimits", _wrap_RevoluteJoint_setLimits },
		{ "getLowerLimit", _wrap_RevoluteJoint_getLowerLimit },
		{ "getUpperLimit", _wrap_RevoluteJoint_getUpperLimit },
		{ "getLimits", _wrap_RevoluteJoint_getLimits },
		// From Joint.
		{ "getType", _wrap_Joint_getType },
		{ "getAnchors", _wrap_Joint_getAnchors },
		{ "getReactionForce", _wrap_Joint_getReactionForce },
		{ "getReactionTorque", _wrap_Joint_getReactionTorque },
		{ "setCollideConnected", _wrap_Joint_setCollideConnected },
		{ "getCollideConnected", _wrap_Joint_getCollideConnected },
		{ 0, 0 }
	};

	int wrap_RevoluteJoint_open(lua_State * L)
	{
		luax_register_type(L, "RevoluteJoint", wrap_RevoluteJoint_functions);
		return 0;
	}

} // box2d
} // physics
} // love
