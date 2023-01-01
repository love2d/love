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

#include "RevoluteJoint.h"

#include "common/math.h"

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

love::Type RevoluteJoint::type("RevoluteJoint", &Joint::type);

RevoluteJoint::RevoluteJoint(Body *body1, Body *body2, float xA, float yA, float xB, float yB, bool collideConnected)
	: Joint(body1, body2)
	, joint(NULL)
{
	b2RevoluteJointDef def;
	init(def, body1, body2, xA, yA, xB, yB, collideConnected);
	joint = (b2RevoluteJoint *)createJoint(&def);
}

RevoluteJoint::RevoluteJoint(Body *body1, Body *body2, float xA, float yA, float xB, float yB, bool collideConnected, float referenceAngle)
	: Joint(body1, body2)
	, joint(NULL)
{
	b2RevoluteJointDef def;
	init(def, body1, body2, xA, yA, xB, yB, collideConnected);
	def.referenceAngle = referenceAngle;
	joint = (b2RevoluteJoint *)createJoint(&def);
}

RevoluteJoint::~RevoluteJoint()
{
}

void RevoluteJoint::init(b2RevoluteJointDef &def, Body *body1, Body *body2, float xA, float yA, float xB, float yB, bool collideConnected)
{
	def.Initialize(body1->body, body2->body, Physics::scaleDown(b2Vec2(xA,yA)));
	def.localAnchorB = body2->body->GetLocalPoint(Physics::scaleDown(b2Vec2(xB, yB)));
	def.collideConnected = collideConnected;
}

float RevoluteJoint::getJointAngle() const
{
	return joint->GetJointAngle();
}

float RevoluteJoint::getJointSpeed() const
{
	return joint->GetJointSpeed();
}

void RevoluteJoint::setMotorEnabled(bool enable)
{
	return joint->EnableMotor(enable);
}

bool RevoluteJoint::isMotorEnabled() const
{
	return joint->IsMotorEnabled();
}

void RevoluteJoint::setMaxMotorTorque(float torque)
{
	joint->SetMaxMotorTorque(Physics::scaleDown(Physics::scaleDown(torque)));
}

void RevoluteJoint::setMotorSpeed(float speed)
{
	joint->SetMotorSpeed(speed);
}

float RevoluteJoint::getMotorSpeed() const
{
	return joint->GetMotorSpeed();
}

float RevoluteJoint::getMotorTorque(float inv_dt) const
{
	return Physics::scaleUp(Physics::scaleUp(joint->GetMotorTorque(inv_dt)));
}

float RevoluteJoint::getMaxMotorTorque() const
{
	return Physics::scaleUp(Physics::scaleUp(joint->GetMaxMotorTorque()));
}

void RevoluteJoint::setLimitsEnabled(bool enable)
{
	joint->EnableLimit(enable);
}

bool RevoluteJoint::areLimitsEnabled() const
{
	return joint->IsLimitEnabled();
}

void RevoluteJoint::setUpperLimit(float limit)
{
	joint->SetLimits(joint->GetLowerLimit(), limit);
}

void RevoluteJoint::setLowerLimit(float limit)
{
	joint->SetLimits(limit, joint->GetUpperLimit());
}

void RevoluteJoint::setLimits(float lower, float upper)
{
	joint->SetLimits(lower, upper);
}

float RevoluteJoint::getLowerLimit() const
{
	return joint->GetLowerLimit();
}

float RevoluteJoint::getUpperLimit() const
{
	return joint->GetUpperLimit();
}

int RevoluteJoint::getLimits(lua_State *L)
{
	lua_pushnumber(L, joint->GetLowerLimit());
	lua_pushnumber(L, joint->GetUpperLimit());
	return 2;
}

float RevoluteJoint::getReferenceAngle() const
{
	return joint->GetReferenceAngle();
}

} // box2d
} // physics
} // love
