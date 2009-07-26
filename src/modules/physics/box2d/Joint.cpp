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

#include "Joint.h"

// STD
#include <bitset>

// Module
#include "Body.h"
#include "World.h"

namespace love
{
namespace physics
{
namespace box2d
{
	Joint::Joint(Body * body1)
		: body1(body1), body2(0), world(body1->world)
	{
		body1->retain();
	}

	Joint::Joint(Body * body1, Body * body2)
		: body1(body1), body2(body2), world(body1->world)
	{
		body1->retain();
		body2->retain();
	}

	Joint::~Joint()
	{
		if(body1 != 0)
			body1->release();
		if(body2 != 0)
			body2->release();

		joint = 0;
	}

	int Joint::getType() const
	{
		switch(joint->GetType())
		{
		case e_revoluteJoint: 
			return JOINT_REVOLUTE;
		case e_prismaticJoint: 
			return JOINT_PRISMATIC;
		case e_distanceJoint: 
			return JOINT_DISTANCE;
		case e_pulleyJoint: 
			return JOINT_PULLEY;
		case e_mouseJoint: 
			return JOINT_MOUSE;
		case e_gearJoint: 
			return JOINT_GEAR;
		default:
			return -1;
		}
	}

	int Joint::getAnchors(lua_State * L)
	{
		lua_pushnumber(L, world->scaleUp(joint->GetAnchor1().x));
		lua_pushnumber(L, world->scaleUp(joint->GetAnchor1().y));
		lua_pushnumber(L, world->scaleUp(joint->GetAnchor2().x));
		lua_pushnumber(L, world->scaleUp(joint->GetAnchor2().y));
		return 4;
	}

	int Joint::getReactionForce(lua_State * L)
	{
		b2Vec2 v = joint->GetReactionForce();
		lua_pushnumber(L, v.x);
		lua_pushnumber(L, v.y);
		return 2;
	}

	float Joint::getReactionTorque()
	{
		return joint->GetReactionTorque();
	}

	void Joint::setCollideConnected(bool collide)
	{
		joint->m_collideConnected = collide;
	}

	bool Joint::getCollideConnected() const
	{
		return joint->m_collideConnected;
	}

	b2Joint * Joint::createJoint(b2JointDef * def)
	{
		joint = world->world->CreateJoint(def);
		return joint;
	}

	void Joint::destroyJoint(b2Joint * joint)
	{
		world->world->DestroyJoint(joint);
	}

} // box2d
} // physics
} // love
