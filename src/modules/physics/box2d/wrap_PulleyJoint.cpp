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
		return luax_checktype<PulleyJoint>(L, idx, "PulleyJoint", LOVE_PHYSICS_PULLEY_JOINT_BITS);
	}

	int _wrap_PulleyJoint_getGroundAnchors(lua_State * L)
	{
		PulleyJoint * t = luax_checkpulleyjoint(L, 1);
		lua_remove(L, 1);
		return t->getGroundAnchors(L);
	}
	
	int _wrap_PulleyJoint_setMaxLengths(lua_State * L)
	{
		PulleyJoint * t = luax_checkpulleyjoint(L, 1);
		float arg1 = (float)luaL_optnumber(L, 2, 0.0);
		float arg2 = (float)luaL_optnumber(L, 3, 0.0);
		t->setMaxLengths(arg1, arg2);
		return 0;
	}
	
	int _wrap_PulleyJoint_getMaxLengths(lua_State * L)
	{
		PulleyJoint * t = luax_checkpulleyjoint(L, 1);
		lua_remove(L, 1);
		return t->getMaxLengths(L);
	}
	
	int _wrap_PulleyJoint_setConstant(lua_State * L)
	{
		PulleyJoint * t = luax_checkpulleyjoint(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		t->setConstant(arg1);
		return 0;
	}
	
	int _wrap_PulleyJoint_getConstant(lua_State * L)
	{
		PulleyJoint * t = luax_checkpulleyjoint(L, 1);
		lua_pushnumber(L, t->getConstant());
		return 1;
	}

	int _wrap_PulleyJoint_getLength1(lua_State * L)
	{
		PulleyJoint * t = luax_checkpulleyjoint(L, 1);
		lua_pushnumber(L, t->getLength1());
		return 1;
	}
	
	int _wrap_PulleyJoint_getLength2(lua_State * L)
	{
		PulleyJoint * t = luax_checkpulleyjoint(L, 1);
		lua_pushnumber(L, t->getLength2());
		return 1;
	}
	
	int _wrap_PulleyJoint_setRatio(lua_State * L)
	{
		PulleyJoint * t = luax_checkpulleyjoint(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		t->setRatio(arg1);
		return 0;
	}
	
	int _wrap_PulleyJoint_getRatio(lua_State * L)
	{
		PulleyJoint * t = luax_checkpulleyjoint(L, 1);
		lua_pushnumber(L, t->getRatio());
		return 1;
	}
	
	static const luaL_Reg wrap_PulleyJoint_functions[] = {
		{ "getGroundAnchors", _wrap_PulleyJoint_getGroundAnchors },
		{ "setMaxLengths", _wrap_PulleyJoint_setMaxLengths },
		{ "getMaxLengths", _wrap_PulleyJoint_getMaxLengths },
		{ "setConstant", _wrap_PulleyJoint_setConstant },
		{ "getConstant", _wrap_PulleyJoint_getConstant },
		{ "getLength1", _wrap_PulleyJoint_getLength1 },
		{ "getLength2", _wrap_PulleyJoint_getLength2 },
		{ "setRatio", _wrap_PulleyJoint_setRatio },
		{ "getRatio", _wrap_PulleyJoint_getRatio },
		// From Joint.
		{ "getType", _wrap_Joint_getType },
		{ "getAnchors", _wrap_Joint_getAnchors },
		{ "getReactionForce", _wrap_Joint_getReactionForce },
		{ "getReactionTorque", _wrap_Joint_getReactionTorque },
		{ "setCollideConnected", _wrap_Joint_setCollideConnected },
		{ "getCollideConnected", _wrap_Joint_getCollideConnected },
		{ 0, 0 }
	};

	int wrap_PulleyJoint_open(lua_State * L)
	{
		luax_register_type(L, "PulleyJoint", wrap_PulleyJoint_functions);
		return 0; 
	}

} // box2d
} // physics
} // love
