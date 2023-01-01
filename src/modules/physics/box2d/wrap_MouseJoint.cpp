/**
 * Copyright (c) 2006-2023 LOVE Development Team
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

#include "wrap_MouseJoint.h"

namespace love
{
namespace physics
{
namespace box2d
{

MouseJoint *luax_checkmousejoint(lua_State *L, int idx)
{
	MouseJoint *j = luax_checktype<MouseJoint>(L, idx);
	if (!j->isValid())
		luaL_error(L, "Attempt to use destroyed joint.");
	return j;
}

int w_MouseJoint_setTarget(lua_State *L)
{
	MouseJoint *t = luax_checkmousejoint(L, 1);
	float x = (float)luaL_checknumber(L, 2);
	float y = (float)luaL_checknumber(L, 3);
	t->setTarget(x, y);
	return 0;
}

int w_MouseJoint_getTarget(lua_State *L)
{
	MouseJoint *t = luax_checkmousejoint(L, 1);
	lua_remove(L, 1);
	return t->getTarget(L);
}

int w_MouseJoint_setMaxForce(lua_State *L)
{
	MouseJoint *t = luax_checkmousejoint(L, 1);
	float f = (float)luaL_checknumber(L, 2);
	t->setMaxForce(f);
	return 0;
}

int w_MouseJoint_getMaxForce(lua_State *L)
{
	MouseJoint *t = luax_checkmousejoint(L, 1);
	lua_pushnumber(L, t->getMaxForce());
	return 1;
}

int w_MouseJoint_setFrequency(lua_State *L)
{
	MouseJoint *t = luax_checkmousejoint(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	luax_catchexcept(L, [&]() { t->setFrequency(arg1); });
	return 0;
}

int w_MouseJoint_getFrequency(lua_State *L)
{
	MouseJoint *t = luax_checkmousejoint(L, 1);
	lua_pushnumber(L, t->getFrequency());
	return 1;
}

int w_MouseJoint_setDampingRatio(lua_State *L)
{
	MouseJoint *t = luax_checkmousejoint(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	t->setDampingRatio(arg1);
	return 0;
}

int w_MouseJoint_getDampingRatio(lua_State *L)
{
	MouseJoint *t = luax_checkmousejoint(L, 1);
	lua_pushnumber(L, t->getDampingRatio());
	return 1;
}

static const luaL_Reg w_MouseJoint_functions[] =
{
	{ "setTarget", w_MouseJoint_setTarget },
	{ "getTarget", w_MouseJoint_getTarget },
	{ "setMaxForce", w_MouseJoint_setMaxForce },
	{ "getMaxForce", w_MouseJoint_getMaxForce },
	{ "setFrequency", w_MouseJoint_setFrequency },
	{ "getFrequency", w_MouseJoint_getFrequency },
	{ "setDampingRatio", w_MouseJoint_setDampingRatio },
	{ "getDampingRatio", w_MouseJoint_getDampingRatio },
	{ 0, 0 }
};

extern "C" int luaopen_mousejoint(lua_State *L)
{
	return luax_register_type(L, &MouseJoint::type, w_Joint_functions, w_MouseJoint_functions, nullptr);
}

} // box2d
} // physics
} // love
