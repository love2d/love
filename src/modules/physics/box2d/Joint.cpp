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

#include "Joint.h"

// STD
#include <bitset>

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
		if (body1 != 0)
			body1->release();
		if (body2 != 0)
			body2->release();

		joint = 0;
	}

	Joint::Type Joint::getType() const
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
		case e_frictionJoint:
			return JOINT_FRICTION;
		case e_weldJoint:
			return JOINT_WELD;
		case e_wheelJoint:
			return JOINT_WHEEL;
		case e_ropeJoint:
			return JOINT_ROPE;
		default:
			return JOINT_INVALID;
		}
	}

	int Joint::getAnchors(lua_State * L)
	{
		lua_pushnumber(L, Physics::scaleUp(joint->GetAnchorA().x));
		lua_pushnumber(L, Physics::scaleUp(joint->GetAnchorA().y));
		lua_pushnumber(L, Physics::scaleUp(joint->GetAnchorB().x));
		lua_pushnumber(L, Physics::scaleUp(joint->GetAnchorB().y));
		return 4;
	}

	int Joint::getReactionForce(lua_State * L)
	{
		float dt = (float)luaL_checknumber(L, 1);
		b2Vec2 v = Physics::scaleUp(joint->GetReactionForce(dt));
		lua_pushnumber(L, v.x);
		lua_pushnumber(L, v.y);
		return 2;
	}

	float Joint::getReactionTorque(float dt)
	{
		return Physics::scaleUp(Physics::scaleUp(joint->GetReactionTorque(dt)));
	}

	b2Joint * Joint::createJoint(b2JointDef * def)
	{
		joint = world->world->CreateJoint(def);
		return joint;
	}

	void Joint::destroyJoint(b2Joint * joint)
	{
		if (joint != NULL)
			world->world->DestroyJoint(joint);
	}

	bool Joint::isActive() const
	{
		return joint->IsActive();
	}

	bool Joint::getCollideConnected() const
	{
		return joint->GetCollideConnected();
	}

} // box2d
} // physics
} // love
