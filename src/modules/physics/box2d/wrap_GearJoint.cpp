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

#include "wrap_GearJoint.h"
#include "wrap_Physics.h"

namespace love
{
namespace physics
{
namespace box2d
{

GearJoint *luax_checkgearjoint(lua_State *L, int idx)
{
	GearJoint *j = luax_checktype<GearJoint>(L, idx, "GearJoint", PHYSICS_GEAR_JOINT_T);
	if (!j->isValid())
		luaL_error(L, "Attempt to use destroyed joint.");
	return j;
}

int w_GearJoint_setRatio(lua_State *L)
{
	GearJoint *t = luax_checkgearjoint(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	luax_catchexcept(L, [&](){ t->setRatio(arg1); });
	return 0;
}

int w_GearJoint_getRatio(lua_State *L)
{
	GearJoint *t = luax_checkgearjoint(L, 1);
	lua_pushnumber(L, t->getRatio());
	return 1;
}

int w_GearJoint_getJoints(lua_State *L)
{
	GearJoint *t = luax_checkgearjoint(L, 1);
	Joint *j1 = nullptr;
	Joint *j2 = nullptr;

	luax_catchexcept(L, [&]() {
		j1 = t->getJointA();
		j2 = t->getJointB();
	});

	auto pushjoint = [](lua_State *L, Joint *j) -> void
	{
		if (j == nullptr)
			return lua_pushnil(L);

		switch (j->getType())
		{
		case Joint::JOINT_DISTANCE:
			return luax_pushtype(L, "DistanceJoint", PHYSICS_DISTANCE_JOINT_T, j);
		case Joint::JOINT_REVOLUTE:
			return luax_pushtype(L, "RevoluteJoint", PHYSICS_REVOLUTE_JOINT_T, j);
		case Joint::JOINT_PRISMATIC:
			return luax_pushtype(L, "PrismaticJoint", PHYSICS_PRISMATIC_JOINT_T, j);
		case Joint::JOINT_MOUSE:
			return luax_pushtype(L, "MouseJoint", PHYSICS_MOUSE_JOINT_T, j);
		case Joint::JOINT_PULLEY:
			return luax_pushtype(L, "PulleyJoint", PHYSICS_PULLEY_JOINT_T, j);
		case Joint::JOINT_GEAR:
			return luax_pushtype(L, "GearJoint", PHYSICS_GEAR_JOINT_T, j);
		case Joint::JOINT_FRICTION:
			return luax_pushtype(L, "FrictionJoint", PHYSICS_FRICTION_JOINT_T, j);
		case Joint::JOINT_WELD:
			return luax_pushtype(L, "WeldJoint", PHYSICS_WELD_JOINT_T, j);
		case Joint::JOINT_WHEEL:
			return luax_pushtype(L, "WheelJoint", PHYSICS_WHEEL_JOINT_T, j);
		case Joint::JOINT_ROPE:
			return luax_pushtype(L, "RopeJoint", PHYSICS_ROPE_JOINT_T, j);
		case Joint::JOINT_MOTOR:
			return luax_pushtype(L, "MotorJoint", PHYSICS_MOTOR_JOINT_T, j);
		default:
			return lua_pushnil(L);
		}
	};

	pushjoint(L, j1);
	pushjoint(L, j2);
	return 2;
}

static const luaL_Reg functions[] =
{
	{ "setRatio", w_GearJoint_setRatio },
	{ "getRatio", w_GearJoint_getRatio },
	{ "getJoints", w_GearJoint_getJoints },
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
	{ 0, 0 }
};

extern "C" int luaopen_gearjoint(lua_State *L)
{
	return luax_register_type(L, "GearJoint", functions);
}

} // box2d
} // physics
} // love
