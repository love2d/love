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

#include "GearJoint.h"

// Module
#include "Body.h"
#include "World.h"
#include "common/Memoizer.h"

namespace love
{
namespace physics
{
namespace box2d
{

GearJoint::GearJoint(Joint *joint1, Joint *joint2, float ratio, bool collideConnected)
	: Joint(joint1->body2, joint2->body2)
	, joint(NULL)
{
	b2GearJointDef def;
	def.joint1 = joint1->joint;
	def.joint2 = joint2->joint;
	def.bodyA = joint1->body2->body;
	def.bodyB = joint2->body2->body;
	def.ratio = ratio;
	def.collideConnected = collideConnected;

	joint = (b2GearJoint *)createJoint(&def);
}

GearJoint::~GearJoint()
{
}

void GearJoint::setRatio(float ratio)
{
	joint->SetRatio(ratio);
}

float GearJoint::getRatio() const
{
	return joint->GetRatio();
}

Joint *GearJoint::getJointA() const
{
	b2Joint *b2joint = joint->GetJoint1();
	if (b2joint == nullptr)
		return nullptr;

	Joint *j = (Joint *) Memoizer::find(b2joint);
	if (j == nullptr)
		throw love::Exception("A joint has escaped Memoizer!");

	return j;
}

Joint *GearJoint::getJointB() const
{
	b2Joint *b2joint = joint->GetJoint2();
	if (b2joint == nullptr)
		return nullptr;

	Joint *j = (Joint *) Memoizer::find(b2joint);
	if (j == nullptr)
		throw love::Exception("A joint has escaped Memoizer!");

	return j;
}

} // box2d
} // physics
} // love
