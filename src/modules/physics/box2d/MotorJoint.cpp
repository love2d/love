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

#include "MotorJoint.h"

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

MotorJoint::MotorJoint(Body *body1, Body *body2)
	: Joint(body1, body2)
	, joint(NULL)
{
	b2MotorJointDef def;

	def.Initialize(body1->body, body2->body);
	joint = (b2MotorJoint *) createJoint(&def);
}

MotorJoint::MotorJoint(Body *body1, Body *body2, float correctionFactor, bool collideConnected)
	: Joint(body1, body2)
	, joint(NULL)
{
	b2MotorJointDef def;

	def.Initialize(body1->body, body2->body);
	def.correctionFactor = correctionFactor;
	def.collideConnected = collideConnected;

	joint = (b2MotorJoint *) createJoint(&def);
}

MotorJoint::~MotorJoint()
{
}

void MotorJoint::setLinearOffset(float x, float y)
{
	joint->SetLinearOffset(Physics::scaleDown(b2Vec2(x, y)));
}

int MotorJoint::getLinearOffset(lua_State *L) const
{
	lua_pushnumber(L, Physics::scaleUp(joint->GetLinearOffset().x));
	lua_pushnumber(L, Physics::scaleUp(joint->GetLinearOffset().y));
	return 2;
}

void MotorJoint::setAngularOffset(float angularOffset)
{
	joint->SetAngularOffset(angularOffset);
}

float MotorJoint::getAngularOffset() const
{
	return joint->GetAngularOffset();
}

void MotorJoint::setMaxForce(float force)
{
	joint->SetMaxForce(Physics::scaleDown(force));
}

float MotorJoint::getMaxForce() const
{
	return Physics::scaleUp(joint->GetMaxForce());
}

void MotorJoint::setMaxTorque(float torque)
{
	joint->SetMaxTorque(Physics::scaleDown(Physics::scaleDown(torque)));
}

float MotorJoint::getMaxTorque() const
{
	return Physics::scaleUp(Physics::scaleUp(joint->GetMaxTorque()));
}

void MotorJoint::setCorrectionFactor(float factor)
{
	joint->SetCorrectionFactor(factor);
}

float MotorJoint::getCorrectionFactor() const
{
	return joint->GetCorrectionFactor();
}

} // box2d
} // physics
} // love
