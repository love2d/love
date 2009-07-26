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

#include "wrap_GearJoint.h"

namespace love
{
namespace physics
{
namespace box2d
{
	GearJoint * luax_checkgearjoint(lua_State * L, int idx)
	{
		return luax_checktype<GearJoint>(L, idx, "GearJoint", LOVE_PHYSICS_GEAR_JOINT_BITS);
	}

	int _wrap_GearJoint_setRatio(lua_State * L)
	{
		GearJoint * t = luax_checkgearjoint(L, 1);
		float arg1 = (float)luaL_checknumber(L, 2);
		t->setRatio(arg1);
		return 0;
	}
	
	int _wrap_GearJoint_getRatio(lua_State * L)
	{
		GearJoint * t = luax_checkgearjoint(L, 1);
		lua_pushnumber(L, t->getRatio());
		return 1;
	}
	
	static const luaL_Reg wrap_GearJoint_functions[] = {
		{ "setRatio", _wrap_GearJoint_setRatio },
		{ "getRatio", _wrap_GearJoint_getRatio },
		// From Joint.
		{ "getType", _wrap_Joint_getType },
		{ "getAnchors", _wrap_Joint_getAnchors },
		{ "getReactionForce", _wrap_Joint_getReactionForce },
		{ "getReactionTorque", _wrap_Joint_getReactionTorque },
		{ "setCollideConnected", _wrap_Joint_setCollideConnected },
		{ "getCollideConnected", _wrap_Joint_getCollideConnected },
		{ 0, 0 }
	};

	int wrap_GearJoint_open(lua_State * L)
	{
		luax_register_type(L, "GearJoint", wrap_GearJoint_functions);
		return 0; 
	}

} // box2d
} // physics
} // love
