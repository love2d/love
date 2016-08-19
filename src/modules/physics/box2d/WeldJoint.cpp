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

#include "WeldJoint.h"

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

WeldJoint::WeldJoint(Body *body1, Body *body2, float xA, float yA, float xB, float yB, bool collideConnected)
	: Joint(body1, body2)
	, joint(NULL)
{
	b2WeldJointDef def;
	init(def, body1, body2, xA, yA, xB, yB, collideConnected);
	joint = (b2WeldJoint *)createJoint(&def);
}

WeldJoint::WeldJoint(Body *body1, Body *body2, float xA, float yA, float xB, float yB, bool collideConnected, float referenceAngle)
	: Joint(body1, body2)
	, joint(NULL)
{
	b2WeldJointDef def;
	init(def, body1, body2, xA, yA, xB, yB, collideConnected);
	def.referenceAngle = referenceAngle;
	joint = (b2WeldJoint *)createJoint(&def);
}

WeldJoint::~WeldJoint()
{
}

void WeldJoint::init(b2WeldJointDef &def, Body *body1, Body *body2, float xA, float yA, float xB, float yB, bool collideConnected)
{
	def.Initialize(body1->body, body2->body, Physics::scaleDown(b2Vec2(xA,yA)));
	def.localAnchorB = body2->body->GetLocalPoint(Physics::scaleDown(b2Vec2(xB, yB)));
	def.collideConnected = collideConnected;
}

void WeldJoint::setFrequency(float hz)
{
	joint->SetFrequency(hz);
}

float WeldJoint::getFrequency() const
{
	return joint->GetFrequency();
}

void WeldJoint::setDampingRatio(float d)
{
	joint->SetDampingRatio(d);
}

float WeldJoint::getDampingRatio() const
{
	return joint->GetDampingRatio();
}

float WeldJoint::getReferenceAngle() const
{
	return joint->GetReferenceAngle();
}

} // box2d
} // physics
} // love
