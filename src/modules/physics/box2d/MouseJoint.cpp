/**
* Copyright (c) 2006-2010 LOVE Development Team
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

#include "MouseJoint.h"

// Module
#include "Body.h"
#include "World.h"

namespace love
{
namespace physics
{
namespace box2d
{
	MouseJoint::MouseJoint(Body * body1, float x, float y)
		: Joint(body1)
	{
		b2MouseJointDef def;
		
		def.body1 = body1->world->world->GetGroundBody();
		def.body2 = body1->body;
		def.maxForce = 1000.0f * body1->body->GetMass();
		def.target = body1->world->scaleDown(b2Vec2(x,y));
		joint = (b2MouseJoint*)createJoint(&def);
	}

	MouseJoint::~MouseJoint()
	{
		destroyJoint(joint);
		joint = 0;
	}

	void MouseJoint::setTarget(float x, float y)
	{
		joint->SetTarget(world->scaleDown(b2Vec2(x, y)));
	}

	int MouseJoint::getTarget(lua_State * L)
	{
		lua_pushnumber(L, world->scaleUp(joint->m_target.x));
		lua_pushnumber(L, world->scaleUp(joint->m_target.y));
		return 2;
	}

	void MouseJoint::setMaxForce(float force)
	{
		joint->m_maxForce = force;
	}

	float MouseJoint::getMaxForce() const
	{
		return joint->m_maxForce;
	}

} // box2d
} // physics
} // love
