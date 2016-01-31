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

#include "wrap_FrictionJoint.h"
#include "wrap_Physics.h"

namespace love
{
namespace physics
{
namespace box2d
{

FrictionJoint *luax_checkfrictionjoint(lua_State *L, int idx)
{
	FrictionJoint *j = luax_checktype<FrictionJoint>(L, idx, PHYSICS_FRICTION_JOINT_ID);
	if (!j->isValid())
		luaL_error(L, "Attempt to use destroyed joint.");
	return j;
}

int w_FrictionJoint_setMaxForce(lua_State *L)
{
	FrictionJoint *t = luax_checkfrictionjoint(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	luax_catchexcept(L, [&](){ t->setMaxForce(arg1); });
	return 0;
}

int w_FrictionJoint_getMaxForce(lua_State *L)
{
	FrictionJoint *t = luax_checkfrictionjoint(L, 1);
	lua_pushnumber(L, t->getMaxForce());
	return 1;
}

int w_FrictionJoint_setMaxTorque(lua_State *L)
{
	FrictionJoint *t = luax_checkfrictionjoint(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	luax_catchexcept(L, [&](){ t->setMaxTorque(arg1); });
	return 0;
}

int w_FrictionJoint_getMaxTorque(lua_State *L)
{
	FrictionJoint *t = luax_checkfrictionjoint(L, 1);
	lua_pushnumber(L, t->getMaxTorque());
	return 1;
}

static const luaL_Reg w_FrictionJoint_functions[] =
{
	{ "setMaxForce", w_FrictionJoint_setMaxForce },
	{ "getMaxForce", w_FrictionJoint_getMaxForce },
	{ "setMaxTorque", w_FrictionJoint_setMaxTorque },
	{ "getMaxTorque", w_FrictionJoint_getMaxTorque },
	{ 0, 0 }
};

extern "C" int luaopen_frictionjoint(lua_State *L)
{
	return luax_register_type(L, PHYSICS_FRICTION_JOINT_ID, "FrictionJoint", w_Joint_functions, w_FrictionJoint_functions, nullptr);
}

} // box2d
} // physics
} // love
