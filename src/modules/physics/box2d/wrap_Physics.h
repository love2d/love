/**
* Copyright (c) 2006-2009 LOVE Development Team
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
#include "Physics.h"
#include "wrap_World.h"
#include "wrap_Contact.h"
#include "wrap_Body.h"
#include "wrap_Shape.h"
#include "wrap_CircleShape.h"
#include "wrap_PolygonShape.h"
#include "wrap_Joint.h"
#include "wrap_MouseJoint.h"
#include "wrap_DistanceJoint.h"
#include "wrap_PrismaticJoint.h"
#include "wrap_RevoluteJoint.h"
#include "wrap_PulleyJoint.h"
#include "wrap_GearJoint.h"

namespace love
{
namespace physics
{
namespace box2d
{
	int _wrap_newWorld(lua_State * L);
	int _wrap_newBody(lua_State * L);
	int _wrap_newCircleShape(lua_State * L);
	int _wrap_newRectangleShape(lua_State * L);
	int _wrap_newPolygonShape(lua_State * L);;
	int _wrap_newDistanceJoint(lua_State * L);
	int _wrap_newMouseJoint(lua_State * L);
	int _wrap_newRevoluteJoint(lua_State * L);
	int _wrap_newPrismaticJoint(lua_State * L);
	int _wrap_newPulleyJoint(lua_State * L);
	int _wrap_newGearJoint(lua_State * L);
	int wrap_Physics_open(lua_State * L);

} // box2d
} // physics
} // love

#endif // LOVE_PHYSICS_BOX2D_WRAP_PHYSICS_H
