/**
 * Copyright (c) 2006-2015 LOVE Development Team
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

#include "wrap_RopeJoint.h"

namespace love
{
namespace physics
{
namespace box2d
{

RopeJoint *luax_checkropejoint(lua_State *L, int idx)
{
	RopeJoint *j = luax_checktype<RopeJoint>(L, idx, PHYSICS_ROPE_JOINT_ID);
	if (!j->isValid())
		luaL_error(L, "Attempt to use destroyed joint.");
	return j;
}

int w_RopeJoint_getMaxLength(lua_State *L)
{
	RopeJoint *t = luax_checkropejoint(L, 1);
	lua_pushnumber(L, t->getMaxLength());
	return 1;
}

static const luaL_Reg functions[] =
{
	{ "getMaxLength", w_RopeJoint_getMaxLength },
	// From Joint.
	{ "getType", w_Joint_getType },
	{ "getBodies", w_Joint_getBodies },
	{ "getAnchors", w_Joint_getAnchors },
	{ "getReactionForce", w_Joint_getReactionForce },
	{ "getReactionTorque", w_Joint_getReactionTorque },
	{ "getCollideConnected", w_Joint_getCollideConnected },
	{ "setUserData", w_Joint_setUserData },
	{ "getUserData", w_Joint_getUserData },
	{ "destroy", w_Joint_destroy },
	{ "isDestroyed", w_Joint_isDestroyed },
	{ 0, 0 }
};

extern "C" int luaopen_ropejoint(lua_State *L)
{
	return luax_register_type(L, PHYSICS_ROPE_JOINT_ID, functions);
}

} // box2d
} // physics
} // love
