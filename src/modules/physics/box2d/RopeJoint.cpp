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

#include "RopeJoint.h"

// Module
#include "Body.h"
#include "World.h"

namespace love
{
namespace physics
{
namespace box2d
{
	RopeJoint::RopeJoint(Body * body1, Body * body2, float x1, float y1, float x2, float y2, float maxLength)
		: Joint(body1, body2), joint(NULL)
	{
		b2RopeJointDef def;
		def.bodyA = body1->body;
		def.bodyB = body2->body;
		def.localAnchorA = world->scaleDown(b2Vec2(x1,y1));
		def.localAnchorB = world->scaleDown(b2Vec2(x2,y2));
		def.maxLength = world->scaleDown(maxLength);
		joint = (b2RopeJoint*)createJoint(&def);
	}

	RopeJoint::~RopeJoint()
	{
		destroyJoint(joint);
		joint = 0;
	}

	float RopeJoint::getMaxLength() const
	{
		return world->scaleUp(joint->GetMaxLength());
	}


} // box2d
} // physics
} // love
