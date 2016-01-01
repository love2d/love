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

#include "wrap_PulleyJoint.h"

namespace love
{
namespace physics
{
namespace box2d
{

PulleyJoint *luax_checkpulleyjoint(lua_State *L, int idx)
{
	PulleyJoint *j = luax_checktype<PulleyJoint>(L, idx, PHYSICS_PULLEY_JOINT_ID);
	if (!j->isValid())
		luaL_error(L, "Attempt to use destroyed joint.");
	return j;
}

int w_PulleyJoint_getGroundAnchors(lua_State *L)
{
	PulleyJoint *t = luax_checkpulleyjoint(L, 1);
	lua_remove(L, 1);
	return t->getGroundAnchors(L);
}

int w_PulleyJoint_getLengthA(lua_State *L)
{
	PulleyJoint *t = luax_checkpulleyjoint(L, 1);
	lua_pushnumber(L, t->getLengthA());
	return 1;
}

int w_PulleyJoint_getLengthB(lua_State *L)
{
	PulleyJoint *t = luax_checkpulleyjoint(L, 1);
	lua_pushnumber(L, t->getLengthB());
	return 1;
}

int w_PulleyJoint_getRatio(lua_State *L)
{
	PulleyJoint *t = luax_checkpulleyjoint(L, 1);
	lua_pushnumber(L, t->getRatio());
	return 1;
}

static const luaL_Reg w_PulleyJoint_functions[] =
{
	{ "getGroundAnchors", w_PulleyJoint_getGroundAnchors },
	{ "getLengthA", w_PulleyJoint_getLengthA },
	{ "getLengthB", w_PulleyJoint_getLengthB },
	{ "getRatio", w_PulleyJoint_getRatio },
	{ 0, 0 }
};

extern "C" int luaopen_pulleyjoint(lua_State *L)
{
	return luax_register_type(L, PHYSICS_PULLEY_JOINT_ID, "PulleyJoint", w_Joint_functions, w_PulleyJoint_functions, nullptr);
}

} // box2d
} // physics
} // love
