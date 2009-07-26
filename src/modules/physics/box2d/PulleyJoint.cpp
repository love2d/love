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

#include "PulleyJoint.h"

// Module
#include "Body.h"
#include "World.h"

namespace love
{
namespace physics
{
namespace box2d
{
	PulleyJoint::PulleyJoint(Body * body1, Body * body2, b2Vec2 groundAnchor1, b2Vec2 groundAnchor2, b2Vec2 anchor1, b2Vec2 anchor2, float ratio)
		: Joint(body1, body2)
	{
		b2PulleyJointDef def;
		def.Initialize(body1->body, body2->body, world->scaleDown(groundAnchor1), world->scaleDown(groundAnchor2), \
			 world->scaleDown(anchor1), world->scaleDown(anchor2), ratio);
		
		joint = (b2PulleyJoint*)createJoint(&def);
	}

	PulleyJoint::~PulleyJoint()
	{
		destroyJoint(joint);
		joint = 0;
	}

	int PulleyJoint::getGroundAnchors(lua_State * L)
	{
		lua_pushnumber(L, world->scaleUp(joint->GetGroundAnchor1().x));
		lua_pushnumber(L, world->scaleUp(joint->GetGroundAnchor1().y));
		lua_pushnumber(L, world->scaleUp(joint->GetGroundAnchor2().x));
		lua_pushnumber(L, world->scaleUp(joint->GetGroundAnchor2().y));
		return 4;
	}
	
	void PulleyJoint::setMaxLengths(float maxlength1, float maxlength2)
	{	
		// Apply Box2D's maximum lengths too. They know better.
		
		if (maxlength1 > 0)
		{
			joint->m_maxLength1 = b2Min(world->scaleDown(maxlength1), joint->m_constant - joint->m_ratio * b2_minPulleyLength);
		}
		
		if (maxlength2 > 0)
		{
			joint->m_maxLength2 = b2Min(world->scaleDown(maxlength2), (joint->m_constant - b2_minPulleyLength) / joint->m_ratio);
		}
	}
	
	int PulleyJoint::getMaxLengths(lua_State * L)
	{
		lua_pushnumber(L, world->scaleUp(joint->m_maxLength1));
		lua_pushnumber(L, world->scaleUp(joint->m_maxLength2));
		return 2;
	}
	
	void PulleyJoint::setConstant(float constant)
	{
		joint->m_constant = world->scaleDown(constant);
		
		// Update the max lengths, as does Box2D
		joint->m_maxLength1 = joint->m_constant - joint->m_ratio * b2_minPulleyLength;
		joint->m_maxLength2 = (joint->m_constant - b2_minPulleyLength) / joint->m_ratio;
	}
	
	float PulleyJoint::getConstant() const
	{
		return world->scaleUp(joint->m_constant);
	}
	
	float PulleyJoint::getLength1() const
	{
		return world->scaleUp(joint->GetLength1());
	}
		
	float PulleyJoint::getLength2() const
	{
		return world->scaleUp(joint->GetLength2());
	}
	
	void PulleyJoint::setRatio(float ratio)
	{
		joint->m_ratio = ratio;
	}
	
	float PulleyJoint::getRatio() const
	{
		return joint->m_ratio;
	}

} // box2d
} // physics
} // love
