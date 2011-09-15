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

#include "WheelJoint.h"

// Module
#include "Body.h"
#include "World.h"

namespace love
{
namespace physics
{
namespace box2d
{
	WheelJoint::WheelJoint(Body * body1, Body * body2, float x, float y, float ax, float ay, bool collideConnected)
		: Joint(body1, body2), joint(NULL)
	{
		b2WheelJointDef def;
		
		def.Initialize(body1->body, body2->body, world->scaleDown(b2Vec2(x,y)), b2Vec2(ax,ay));
		def.collideConnected = collideConnected;
		joint = (b2WheelJoint*)createJoint(&def);
	}

	WheelJoint::~WheelJoint()
	{
		destroyJoint(joint);
		joint = 0;
	}

	float WheelJoint::getJointTranslation() const
	{
		return world->scaleDown(joint->GetJointTranslation());
	}

	float WheelJoint::getJointSpeed() const
	{
		return world->scaleDown(joint->GetJointSpeed());
	}

	void WheelJoint::enableMotor(bool motor)
	{
		return joint->EnableMotor(motor);
	}

	bool WheelJoint::isMotorEnabled() const
	{
		return joint->IsMotorEnabled();
	}
	
	void WheelJoint::setMotorSpeed(float speed)
	{
		joint->SetMotorSpeed(world->scaleDown(speed));
	}

	float WheelJoint::getMotorSpeed() const
	{
		return world->scaleUp(joint->GetMotorSpeed());
	}
	
	void WheelJoint::setMaxMotorTorque(float torque)
	{
		joint->SetMaxMotorTorque(world->scaleDown(torque));
	}
	
	float WheelJoint::getMaxMotorTorque() const
	{
		return world->scaleUp(joint->GetMaxMotorTorque());
	}

	float WheelJoint::getMotorTorque(float inv_dt) const
	{
		return world->scaleUp(joint->getMotorTorque(inv_dt));
	}
	
	void WheelJoint::setSpringFrequencyHz(float hz)
	{
		joint->SetSpringFrequencyHz(world->scaleDown(hz));
	}
	
	float WheelJoint::getSpringFrequencyHz() const
	{
		return world->scaleUp(joint->GetSpringFrequencyHz());
	}
	
	void WheelJoint::setSpringDampingRatio(float ratio)
	{
		joint->SetSpringDampingRatio(world->scaleDown(ratio));
	}
	
	float WheelJoint::getSpringDampingRatio() const
	{
		return world->scaleUp(joint->GetSpringDampingRatio());
	}


} // box2d
} // physics
} // love
