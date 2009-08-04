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

#include "PrismaticJoint.h"

// Module
#include "Body.h"
#include "World.h"

namespace love
{
namespace physics
{
namespace box2d
{
	PrismaticJoint::PrismaticJoint(Body * body1, Body * body2, float x, float y, float ax, float ay)
		: Joint(body1, body2)
	{
		b2PrismaticJointDef def;
		
		def.Initialize(body1->body, body2->body, world->scaleDown(b2Vec2(x,y)), b2Vec2(ax,ay));
		def.lowerTranslation = 0.0f;
		def.upperTranslation = 100.0f;
		def.enableLimit = true;
		joint = (b2PrismaticJoint*)createJoint(&def);
	}

	PrismaticJoint::~PrismaticJoint()
	{
		destroyJoint(joint);
		joint = 0;
	}

	float PrismaticJoint::getJointTranslation() const
	{
		return world->scaleDown(joint->GetJointTranslation());
	}

	float PrismaticJoint::getJointSpeed() const
	{
		return world->scaleDown(joint->GetJointSpeed());
	}

	void PrismaticJoint::setMotorEnabled(bool motor)
	{
		return joint->EnableMotor(true);
	}

	bool PrismaticJoint::isMotorEnabled() const
	{
		return joint->IsMotorEnabled();
	}

	void PrismaticJoint::setMaxMotorForce(float force)
	{
		joint->SetMaxMotorForce(force);
	}

	float PrismaticJoint::getMaxMotorForce() const
	{
		return joint->m_maxMotorForce;
	}

	void PrismaticJoint::setMotorSpeed(float speed)
	{
		joint->SetMotorSpeed(speed);
	}

	float PrismaticJoint::getMotorSpeed() const
	{
		return joint->GetMotorSpeed();
	}

	float PrismaticJoint::getMotorForce() const
	{
		return joint->GetMotorForce();
	}

	void PrismaticJoint::setLimitsEnabled(bool limit)
	{
		joint->EnableLimit(limit);
	}

	bool PrismaticJoint::isLimitsEnabled() const
	{
		return joint->IsLimitEnabled();
	}

	void PrismaticJoint::setUpperLimit(float limit)
	{
		joint->SetLimits(joint->GetLowerLimit(), world->scaleDown(limit));
	}

	void PrismaticJoint::setLowerLimit(float limit)
	{
		joint->SetLimits(world->scaleDown(limit), joint->GetUpperLimit());
	}

	void PrismaticJoint::setLimits(float lower, float upper)
	{
		joint->SetLimits(world->scaleDown(lower), world->scaleDown(upper));
	}

	float PrismaticJoint::getLowerLimit() const
	{
		return world->scaleUp(joint->GetLowerLimit());
	}

	float PrismaticJoint::getUpperLimit() const
	{
		return world->scaleUp(joint->GetUpperLimit());
	}

	int PrismaticJoint::getLimits(lua_State * L)
	{
		lua_pushnumber(L, world->scaleUp(joint->GetLowerLimit()));
		lua_pushnumber(L, world->scaleUp(joint->GetUpperLimit()));
		return 2;
	}


} // box2d
} // physics
} // love
