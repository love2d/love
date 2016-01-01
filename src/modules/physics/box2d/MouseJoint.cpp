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

#include "MouseJoint.h"

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

MouseJoint::MouseJoint(Body *body1, float x, float y)
	: Joint(body1)
	, joint(NULL)
{
	b2MouseJointDef def;

	def.bodyA = body1->world->getGroundBody();
	def.bodyB = body1->body;
	def.maxForce = 1000.0f * body1->body->GetMass();
	def.target = Physics::scaleDown(b2Vec2(x,y));
	joint = (b2MouseJoint *)createJoint(&def);
}

MouseJoint::~MouseJoint()
{
}

void MouseJoint::setTarget(float x, float y)
{
	joint->SetTarget(Physics::scaleDown(b2Vec2(x, y)));
}

int MouseJoint::getTarget(lua_State *L)
{
	lua_pushnumber(L, Physics::scaleUp(joint->GetTarget().x));
	lua_pushnumber(L, Physics::scaleUp(joint->GetTarget().y));
	return 2;
}

void MouseJoint::setMaxForce(float force)
{
	joint->SetMaxForce(Physics::scaleDown(force));
}

float MouseJoint::getMaxForce() const
{
	return Physics::scaleUp(joint->GetMaxForce());
}

void MouseJoint::setFrequency(float hz)
{
	joint->SetFrequency(hz);
}

float MouseJoint::getFrequency() const
{
	return joint->GetFrequency();
}

void MouseJoint::setDampingRatio(float d)
{
	joint->SetDampingRatio(d);
}

float MouseJoint::getDampingRatio() const
{
	return joint->GetDampingRatio();
}

} // box2d
} // physics
} // love
