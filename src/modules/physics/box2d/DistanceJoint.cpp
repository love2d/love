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

#include "DistanceJoint.h"

// Module
#include "Body.h"
#include "World.h"

namespace love
{
namespace physics
{
namespace box2d
{
	DistanceJoint::DistanceJoint(Body * body1, Body * body2, float x1, float y1, float x2, float y2)
		: Joint(body1, body2), joint(NULL)
	{
		b2DistanceJointDef def;
		def.Initialize(body1->body, body2->body, world->scaleDown(b2Vec2(x1,y1)), world->scaleDown(b2Vec2(x2,y2)));
		joint = (b2DistanceJoint*)createJoint(&def);
	}

	DistanceJoint::~DistanceJoint()
	{
		destroyJoint(joint);
		joint = 0;
	}

	void DistanceJoint::setLength(float length)
	{
		joint->m_length = world->scaleDown(length);
	}

	float DistanceJoint::getLength() const
	{
		return world->scaleUp(joint->m_length);
	}

	void DistanceJoint::setFrequency(float hz)
	{
		joint->m_frequencyHz = hz;
	}

	float DistanceJoint::getFrequency() const
	{
		return joint->m_frequencyHz;
	}

	void DistanceJoint::setDampingRatio(float d)
	{
		joint->m_dampingRatio = d;
	}

	float DistanceJoint::getDampingRatio() const
	{
		return joint->m_dampingRatio;
	}


} // box2d
} // physics
} // love
