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

#include "wrap_World.h"

namespace love
{
namespace physics
{
namespace box2d
{

World *luax_checkworld(lua_State *L, int idx)
{
	World *w = luax_checktype<World>(L, idx);
	if (!w->isValid())
		luaL_error(L, "Attempt to use destroyed world.");
	return w;
}

int w_World_update(lua_State *L)
{
	World *t = luax_checkworld(L, 1);
	float dt = (float)luaL_checknumber(L, 2);

	// Make sure the world callbacks are using the calling Lua thread.
	t->setCallbacksL(L);

	if (lua_isnoneornil(L, 3))
		luax_catchexcept(L, [&](){ t->update(dt); });
	else
	{
		int velocityiterations = (int) luaL_checkinteger(L, 3);
		int positioniterations = (int) luaL_checkinteger(L, 4);
		luax_catchexcept(L, [&](){ t->update(dt, velocityiterations, positioniterations); });
	}

	return 0;
}

int w_World_setCallbacks(lua_State *L)
{
	World *t = luax_checkworld(L, 1);
	lua_remove(L, 1);
	return t->setCallbacks(L);
}

int w_World_getCallbacks(lua_State *L)
{
	World *t = luax_checkworld(L, 1);
	lua_remove(L, 1);
	return t->getCallbacks(L);
}

int w_World_setContactFilter(lua_State *L)
{
	World *t = luax_checkworld(L, 1);
	lua_remove(L, 1);
	return t->setContactFilter(L);
}

int w_World_getContactFilter(lua_State *L)
{
	World *t = luax_checkworld(L, 1);
	lua_remove(L, 1);
	return t->getContactFilter(L);
}

int w_World_setGravity(lua_State *L)
{
	World *t = luax_checkworld(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	float arg2 = (float)luaL_checknumber(L, 3);
	t->setGravity(arg1, arg2);
	return 0;
}

int w_World_getGravity(lua_State *L)
{
	World *t = luax_checkworld(L, 1);
	lua_remove(L, 1);
	return t->getGravity(L);
}

int w_World_translateOrigin(lua_State *L)
{
	World *t = luax_checkworld(L, 1);
	float arg1 = (float)luaL_checknumber(L, 2);
	float arg2 = (float)luaL_checknumber(L, 3);
	luax_catchexcept(L, [&](){ t->translateOrigin(arg1, arg2); });
	return 0;
}

int w_World_setSleepingAllowed(lua_State *L)
{
	World *t = luax_checkworld(L, 1);
	bool b = luax_checkboolean(L, 2);
	t->setSleepingAllowed(b);
	return 0;
}

int w_World_isSleepingAllowed(lua_State *L)
{
	World *t = luax_checkworld(L, 1);
	luax_pushboolean(L, t->isSleepingAllowed());
	return 1;
}

int w_World_isLocked(lua_State *L)
{
	World *t = luax_checkworld(L, 1);
	luax_pushboolean(L, t->isLocked());
	return 1;
}

int w_World_getBodyCount(lua_State *L)
{
	World *t = luax_checkworld(L, 1);
	lua_pushinteger(L, t->getBodyCount());
	return 1;
}

int w_World_getJointCount(lua_State *L)
{
	World *t = luax_checkworld(L, 1);
	lua_pushinteger(L, t->getJointCount());
	return 1;
}

int w_World_getContactCount(lua_State *L)
{
	World *t = luax_checkworld(L, 1);
	lua_pushinteger(L, t->getContactCount());
	return 1;
}

int w_World_getBodies(lua_State *L)
{
	World *t = luax_checkworld(L, 1);
	lua_remove(L, 1);
	int ret = 0;
	luax_catchexcept(L, [&](){ ret = t->getBodies(L); });
	return ret;
}

int w_World_getJoints(lua_State *L)
{
	World *t = luax_checkworld(L, 1);
	lua_remove(L, 1);
	int ret = 0;
	luax_catchexcept(L, [&](){ ret = t->getJoints(L); });
	return ret;
}

int w_World_getContacts(lua_State *L)
{
	World *t = luax_checkworld(L, 1);
	lua_remove(L, 1);
	int ret = 0;
	luax_catchexcept(L, [&](){ ret = t->getContacts(L); });
	return ret;
}

int w_World_queryBoundingBox(lua_State *L)
{
	World *t = luax_checkworld(L, 1);
	lua_remove(L, 1);
	return t->queryBoundingBox(L);
}

int w_World_rayCast(lua_State *L)
{
	World *t = luax_checkworld(L, 1);
	lua_remove(L, 1);
	int ret = 0;
	luax_catchexcept(L, [&](){ ret = t->rayCast(L); });
	return ret;
}

int w_World_destroy(lua_State *L)
{
	World *t = luax_checkworld(L, 1);
	luax_catchexcept(L, [&](){ t->destroy(); });
	return 0;
}

int w_World_isDestroyed(lua_State *L)
{
	World *w = luax_checktype<World>(L, 1);
	luax_pushboolean(L, !w->isValid());
	return 1;
}

int w_World_getBodyList(lua_State *L)
{
	luax_markdeprecated(L, "World:getBodyList", API_METHOD, DEPRECATED_RENAMED, "World:getBodies");
	return w_World_getBodies(L);
}

int w_World_getJointList(lua_State *L)
{
	luax_markdeprecated(L, "World:getJointList", API_METHOD, DEPRECATED_RENAMED, "World:getJoints");
	return w_World_getJoints(L);
}

int w_World_getContactList(lua_State *L)
{
	luax_markdeprecated(L, "World:getContactList", API_METHOD, DEPRECATED_RENAMED, "World:getContacts");
	return w_World_getContacts(L);
}

static const luaL_Reg w_World_functions[] =
{
	{ "update", w_World_update },
	{ "setCallbacks", w_World_setCallbacks },
	{ "getCallbacks", w_World_getCallbacks },
	{ "setContactFilter", w_World_setContactFilter },
	{ "getContactFilter", w_World_getContactFilter },
	{ "setGravity", w_World_setGravity },
	{ "getGravity", w_World_getGravity },
	{ "translateOrigin", w_World_translateOrigin },
	{ "setSleepingAllowed", w_World_setSleepingAllowed },
	{ "isSleepingAllowed", w_World_isSleepingAllowed },
	{ "isLocked", w_World_isLocked },
	{ "getBodyCount", w_World_getBodyCount },
	{ "getJointCount", w_World_getJointCount },
	{ "getContactCount", w_World_getContactCount },
	{ "getBodies", w_World_getBodies },
	{ "getJoints", w_World_getJoints },
	{ "getContacts", w_World_getContacts },
	{ "queryBoundingBox", w_World_queryBoundingBox },
	{ "rayCast", w_World_rayCast },
	{ "destroy", w_World_destroy },
	{ "isDestroyed", w_World_isDestroyed },

	// Deprecated
	{ "getBodyList", w_World_getBodyList },
	{ "getJointList", w_World_getJointList },
	{ "getContactList", w_World_getContactList },

	{ 0, 0 }
};

extern "C" int luaopen_world(lua_State *L)
{
	return luax_register_type(L, &World::type, w_World_functions, nullptr);
}

} // box2d
} // physics
} // love
