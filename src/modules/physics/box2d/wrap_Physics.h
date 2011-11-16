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

#ifndef LOVE_PHYSICS_BOX2D_WRAP_PHYSICS_H
#define LOVE_PHYSICS_BOX2D_WRAP_PHYSICS_H

// LOVE
#include <common/config.h>
#include "Physics.h"
#include "wrap_World.h"
#include "wrap_Contact.h"
#include "wrap_Body.h"
#include "wrap_Fixture.h"
#include "wrap_Shape.h"
#include "wrap_CircleShape.h"
#include "wrap_PolygonShape.h"
#include "wrap_EdgeShape.h"
#include "wrap_ChainShape.h"
#include "wrap_Joint.h"
#include "wrap_MouseJoint.h"
#include "wrap_DistanceJoint.h"
#include "wrap_PrismaticJoint.h"
#include "wrap_RevoluteJoint.h"
#include "wrap_PulleyJoint.h"
#include "wrap_GearJoint.h"
#include "wrap_FrictionJoint.h"
#include "wrap_WeldJoint.h"
#include "wrap_WheelJoint.h"
#include "wrap_RopeJoint.h"

#define ASSERT_GUARD(A) try { A } catch (love::Exception & e) { return luaL_error(L, "%s", e.what()); }

namespace love
{
namespace physics
{
namespace box2d
{
	int w_newWorld(lua_State * L);
	int w_newBody(lua_State * L);
	int w_newFixture(lua_State * L);
	int w_newCircleShape(lua_State * L);
	int w_newRectangleShape(lua_State * L);
	int w_newPolygonShape(lua_State * L);
    int w_newEdgeShape(lua_State * L);
	int w_newChainShape(lua_State * L);
	int w_newDistanceJoint(lua_State * L);
	int w_newMouseJoint(lua_State * L);
	int w_newRevoluteJoint(lua_State * L);
	int w_newPrismaticJoint(lua_State * L);
	int w_newPulleyJoint(lua_State * L);
	int w_newGearJoint(lua_State * L);
	int w_newFrictionJoint(lua_State * L);
	int w_newWeldJoint(lua_State * L);
	int w_newWheelJoint(lua_State * L);
	int w_newRopeJoint(lua_State * L);
	int w_getDistance(lua_State * L);
	int w_setMeter(lua_State * L);
	int w_getMeter(lua_State * L);
	extern "C" LOVE_EXPORT int luaopen_love_physics(lua_State * L);

} // box2d
} // physics
} // love

#endif // LOVE_PHYSICS_BOX2D_WRAP_PHYSICS_H
