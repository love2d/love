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

#include "WheelJoint.h"

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

WheelJoint::WheelJoint(Body *body1, Body *body2, float xA, float yA, float xB, float yB, float ax, float ay, bool collideConnected)
	: Joint(body1, body2)
	, joint(NULL)
{
	b2WheelJointDef def;

	def.Initialize(body1->body, body2->body, Physics::scaleDown(b2Vec2(xA,yA)), b2Vec2(ax,ay));
	def.localAnchorB = body2->body->GetLocalPoint(Physics::scaleDown(b2Vec2(xB, yB)));
	def.collideConnected = collideConnected;
	joint = (b2WheelJoint *)createJoint(&def);
}

WheelJoint::~WheelJoint()
{
}

float WheelJoint::getJointTranslation() const
{
	return Physics::scaleUp(joint->GetJointTranslation());
}

float WheelJoint::getJointSpeed() const
{
	return Physics::scaleUp(joint->GetJointSpeed());
}

void WheelJoint::setMotorEnabled(bool enable)
{
	return joint->EnableMotor(enable);
}

bool WheelJoint::isMotorEnabled() const
{
	return joint->IsMotorEnabled();
}

void WheelJoint::setMotorSpeed(float speed)
{
	joint->SetMotorSpeed(speed);
}

float WheelJoint::getMotorSpeed() const
{
	return joint->GetMotorSpeed();
}

void WheelJoint::setMaxMotorTorque(float torque)
{
	joint->SetMaxMotorTorque(Physics::scaleDown(Physics::scaleDown(torque)));
}

float WheelJoint::getMaxMotorTorque() const
{
	return Physics::scaleUp(Physics::scaleUp(joint->GetMaxMotorTorque()));
}

float WheelJoint::getMotorTorque(float inv_dt) const
{
	return Physics::scaleUp(Physics::scaleUp(joint->GetMotorTorque(inv_dt)));
}

void WheelJoint::setSpringFrequency(float hz)
{
	joint->SetSpringFrequencyHz(hz);
}

float WheelJoint::getSpringFrequency() const
{
	return joint->GetSpringFrequencyHz();
}

void WheelJoint::setSpringDampingRatio(float ratio)
{
	joint->SetSpringDampingRatio(ratio);
}

float WheelJoint::getSpringDampingRatio() const
{
	return joint->GetSpringDampingRatio();
}


} // box2d
} // physics
} // love
