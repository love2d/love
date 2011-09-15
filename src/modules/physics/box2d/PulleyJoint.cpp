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

#include "PulleyJoint.h"

// Module
#include "Body.h"
#include "World.h"
#include "Physics.h"

namespace love
{
namespace physics
{
namespace box2d
{
	PulleyJoint::PulleyJoint(Body * body1, Body * body2, b2Vec2 groundAnchor1, b2Vec2 groundAnchor2, b2Vec2 anchor1, b2Vec2 anchor2, float ratio, bool collideConnected)
		: Joint(body1, body2), joint(NULL)
	{
		b2PulleyJointDef def;
		def.Initialize(body1->body, body2->body, Physics::scaleDown(groundAnchor1), Physics::scaleDown(groundAnchor2), \
			 Physics::scaleDown(anchor1), Physics::scaleDown(anchor2), ratio);
		def.collideConnected = collideConnected;
		
		joint = (b2PulleyJoint*)createJoint(&def);
	}

	PulleyJoint::~PulleyJoint()
	{
		destroyJoint(joint);
		joint = 0;
	}

	int PulleyJoint::getGroundAnchors(lua_State * L)
	{
		lua_pushnumber(L, Physics::scaleUp(joint->GetGroundAnchorA().x));
		lua_pushnumber(L, Physics::scaleUp(joint->GetGroundAnchorA().y));
		lua_pushnumber(L, Physics::scaleUp(joint->GetGroundAnchorB().x));
		lua_pushnumber(L, Physics::scaleUp(joint->GetGroundAnchorB().y));
		return 4;
	}
	
	float PulleyJoint::getLength1() const
	{
		return Physics::scaleUp(joint->GetLength1());
	}
		
	float PulleyJoint::getLength2() const
	{
		return Physics::scaleUp(joint->GetLength2());
	}
	
	float PulleyJoint::getRatio() const
	{
		return joint->GetRatio();
	}

} // box2d
} // physics
} // love
