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

PulleyJoint::PulleyJoint(Body *bodyA, Body *bodyB, b2Vec2 groundAnchorA, b2Vec2 groundAnchorB, b2Vec2 anchorA, b2Vec2 anchorB, float ratio, bool collideConnected)
	: Joint(bodyA, bodyB)
	, joint(NULL)
{
	b2PulleyJointDef def;
	def.Initialize(bodyA->body, bodyB->body, Physics::scaleDown(groundAnchorA), Physics::scaleDown(groundAnchorB), \
				   Physics::scaleDown(anchorA), Physics::scaleDown(anchorB), ratio);
	def.collideConnected = collideConnected;

	joint = (b2PulleyJoint *)createJoint(&def);
}

PulleyJoint::~PulleyJoint()
{
}

int PulleyJoint::getGroundAnchors(lua_State *L)
{
	lua_pushnumber(L, Physics::scaleUp(joint->GetGroundAnchorA().x));
	lua_pushnumber(L, Physics::scaleUp(joint->GetGroundAnchorA().y));
	lua_pushnumber(L, Physics::scaleUp(joint->GetGroundAnchorB().x));
	lua_pushnumber(L, Physics::scaleUp(joint->GetGroundAnchorB().y));
	return 4;
}

float PulleyJoint::getLengthA() const
{
	return Physics::scaleUp(joint->GetLengthA());
}

float PulleyJoint::getLengthB() const
{
	return Physics::scaleUp(joint->GetLengthB());
}

float PulleyJoint::getRatio() const
{
	return joint->GetRatio();
}

} // box2d
} // physics
} // love
