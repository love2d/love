/**
* Copyright (c) 2006-2012 LOVE Development Team
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

#ifndef LOVE_PHYSICS_BOX2D_WRAP_FIXTURE_H
#define LOVE_PHYSICS_BOX2D_WRAP_FIXTURE_H

// LOVE
#include <common/runtime.h>
#include "Fixture.h"
#include "wrap_Physics.h"

namespace love
{
namespace physics
{
namespace box2d
{
	Fixture * luax_checkfixture(lua_State * L, int idx);
	int w_Fixture_getType(lua_State * L);
	int w_Fixture_setFriction(lua_State * L);
	int w_Fixture_setRestitution(lua_State * L);
	int w_Fixture_setDensity(lua_State * L);
	int w_Fixture_setSensor(lua_State * L);
	int w_Fixture_getFriction(lua_State * L);
	int w_Fixture_getRestitution(lua_State * L);
	int w_Fixture_getDensity(lua_State * L);
	int w_Fixture_isSensor(lua_State * L);
	int w_Fixture_getBody(lua_State * L);
	int w_Fixture_getShape(lua_State * L);
	int w_Fixture_testPoint(lua_State * L);
	int w_Fixture_rayCast(lua_State * L);
	int w_Fixture_setFilterData(lua_State * L);
	int w_Fixture_getFilterData(lua_State * L);
	int w_Fixture_setCategory(lua_State * L);
	int w_Fixture_getCategory(lua_State * L);
	int w_Fixture_setMask(lua_State * L);
	int w_Fixture_getMask(lua_State * L);
	int w_Fixture_setUserData(lua_State * L);
	int w_Fixture_getUserData(lua_State * L);
	int w_Fixture_getBoundingBox(lua_State * L);
	int w_Fixture_getMassData(lua_State * L);
	int w_Fixture_getGroupIndex(lua_State * L);
	int w_Fixture_setGroupIndex(lua_State * L);
	int w_Fixture_destroy(lua_State * L);
	extern "C" int luaopen_fixture(lua_State * L);

} // box2d
} // physics
} // love

#endif // LOVE_PHYSICS_BOX2D_WRAP_FIXTURE_H
