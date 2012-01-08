/**
* Copyright (c) 2006-2011 LOVE Development Team
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

#ifndef LOVE_PHYSICS_BOX2D_WRAP_WORLD_H
#define LOVE_PHYSICS_BOX2D_WRAP_WORLD_H

// LOVE
#include <common/runtime.h>
#include <common/Exception.h>
#include "World.h"
#include "wrap_Physics.h"

namespace love
{
namespace physics
{
namespace box2d
{
	World * luax_checkworld(lua_State * L, int idx);
	int w_World_update(lua_State * L);
	int w_World_setCallbacks(lua_State * L);
	int w_World_getCallbacks(lua_State * L);
	int w_World_setContactFilter(lua_State * L);
	int w_World_getContactFilter(lua_State * L);
	int w_World_setGravity(lua_State * L);
	int w_World_getGravity(lua_State * L);
	int w_World_setAllowSleeping(lua_State * L);
	int w_World_getAllowSleeping(lua_State * L);
	int w_World_isLocked(lua_State * L);
	int w_World_getBodyCount(lua_State * L);
	int w_World_getJointCount(lua_State * L);
	int w_World_getContactCount(lua_State * L);
	int w_World_getBodyList(lua_State * L);
	int w_World_getJointList(lua_State * L);
	int w_World_getContactList(lua_State * L);
	int w_World_queryBoundingBox(lua_State * L);
	int w_World_rayCast(lua_State * L);
	int w_World_destroy(lua_State * L);
	extern "C" int luaopen_world(lua_State * L);

} // box2d
} // physics
} // love

#endif // LOVE_PHYSICS_BOX2D_WRAP_WORLD_H
