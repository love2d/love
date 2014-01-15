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

// LOVE
#include "wrap_Joint.h"
#include "common/StringMap.h"

namespace love
{
namespace physics
{
namespace box2d
{

Joint *luax_checkjoint(lua_State *L, int idx)
{
	Joint *t = luax_checktype<Joint>(L, idx, "Joint", PHYSICS_JOINT_T);
	if (!t->isValid())
		luaL_error(L, "Attempt to use destroyed joint.");
	return t;
}

int w_Joint_getType(lua_State *L)
{
	Joint *t = luax_checkjoint(L, 1);
	const char *type = "";
	Joint::getConstant(t->getType(), type);
	lua_pushstring(L, type);
	return 1;
}

int w_Joint_getAnchors(lua_State *L)
{
	Joint *t = luax_checkjoint(L, 1);
	lua_remove(L, 1);
	return t->getAnchors(L);
}

int w_Joint_getReactionForce(lua_State *L)
{
	Joint *t = luax_checkjoint(L, 1);
	lua_remove(L, 1);
	return t->getReactionForce(L);
}

int w_Joint_getReactionTorque(lua_State *L)
{
	Joint *t = luax_checkjoint(L, 1);
	float inv_dt = (float)luaL_checknumber(L, 2);
	lua_pushnumber(L, t->getReactionTorque(inv_dt));
	return 1;
}

int w_Joint_getCollideConnected(lua_State *L)
{
	Joint *t = luax_checkjoint(L, 1);
	luax_pushboolean(L, t->getCollideConnected());
	return 1;
}

int w_Joint_destroy(lua_State *L)
{
	Joint *t = luax_checkjoint(L, 1);
	EXCEPT_GUARD(t->destroyJoint();)
	return 0;
}

static const luaL_Reg functions[] =
{
	{ "getType", w_Joint_getType },
	{ "getAnchors", w_Joint_getAnchors },
	{ "getReactionForce", w_Joint_getReactionForce },
	{ "getReactionTorque", w_Joint_getReactionTorque },
	{ "getCollideConnected", w_Joint_getCollideConnected },
	{ "destroy", w_Joint_destroy },
	{ 0, 0 }
};

extern "C" int luaopen_joint(lua_State *L)
{
	return luax_register_type(L, "Joint", functions);
}

} // box2d
} // physics
} // love
