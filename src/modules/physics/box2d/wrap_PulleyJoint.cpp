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

#include "wrap_PulleyJoint.h"

namespace love
{
namespace physics
{
namespace box2d
{
	PulleyJoint * luax_checkpulleyjoint(lua_State * L, int idx)
	{
		return luax_checktype<PulleyJoint>(L, idx, "PulleyJoint", PHYSICS_PULLEY_JOINT_T);
	}

	int w_PulleyJoint_getGroundAnchors(lua_State * L)
	{
		PulleyJoint * t = luax_checkpulleyjoint(L, 1);
		lua_remove(L, 1);
		return t->getGroundAnchors(L);
	}
	
	int w_PulleyJoint_setMaxLengths(lua_State * L)
	{
		PulleyJoint * t = luax_checkpulleyjoint(L, 1);
		float arg1 = (float)luaL_optnumber(L, 2, 0.0);
		float arg2 = (float)luaL_optnumber(L, 3, 0.0);
		t->setMaxLengths(arg1, arg2);
		return 0;
	}
	
	int w_PulleyJoint_getMaxLengths(lua_State * L)
	{
		PulleyJoint * t = luax_checkpulleyjoint(L, 1);
		lua_remove(L, 1);
		return t->getMaxLengths(L);
	}
	
	int w_PulleyJoint_setConstant(lua_State * L)
	{
		PulleyJoint * t = luax_checkpulleyjoint(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		t->setConstant(arg1);
		return 0;
	}
	
	int w_PulleyJoint_getConstant(lua_State * L)
	{
		PulleyJoint * t = luax_checkpulleyjoint(L, 1);
		lua_pushnumber(L, t->getConstant());
		return 1;
	}

	int w_PulleyJoint_getLength1(lua_State * L)
	{
		PulleyJoint * t = luax_checkpulleyjoint(L, 1);
		lua_pushnumber(L, t->getLength1());
		return 1;
	}
	
	int w_PulleyJoint_getLength2(lua_State * L)
	{
		PulleyJoint * t = luax_checkpulleyjoint(L, 1);
		lua_pushnumber(L, t->getLength2());
		return 1;
	}
	
	int w_PulleyJoint_setRatio(lua_State * L)
	{
		PulleyJoint * t = luax_checkpulleyjoint(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		t->setRatio(arg1);
		return 0;
	}
	
	int w_PulleyJoint_getRatio(lua_State * L)
	{
		PulleyJoint * t = luax_checkpulleyjoint(L, 1);
		lua_pushnumber(L, t->getRatio());
		return 1;
	}

	int luaopen_pulleyjoint(lua_State * L)
	{
		static const luaL_Reg functions[] = {
			{ "getGroundAnchors", w_PulleyJoint_getGroundAnchors },
			{ "setMaxLengths", w_PulleyJoint_setMaxLengths },
			{ "getMaxLengths", w_PulleyJoint_getMaxLengths },
			{ "setConstant", w_PulleyJoint_setConstant },
			{ "getConstant", w_PulleyJoint_getConstant },
			{ "getLength1", w_PulleyJoint_getLength1 },
			{ "getLength2", w_PulleyJoint_getLength2 },
			{ "setRatio", w_PulleyJoint_setRatio },
			{ "getRatio", w_PulleyJoint_getRatio },
			// From Joint.
			{ "getType", w_Joint_getType },
			{ "getAnchors", w_Joint_getAnchors },
			{ "getReactionForce", w_Joint_getReactionForce },
			{ "getReactionTorque", w_Joint_getReactionTorque },
			{ "setCollideConnected", w_Joint_setCollideConnected },
			{ "getCollideConnected", w_Joint_getCollideConnected },
			{ 0, 0 }
		};
		return luax_register_type(L, "PulleyJoint", functions);
	}

} // box2d
} // physics
} // love
