/**
 * Copyright (c) 2006-2023 LOVE Development Team
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

#ifndef LOVE_PHYSICS_BOX2D_PULLEY_JOINT_H
#define LOVE_PHYSICS_BOX2D_PULLEY_JOINT_H

// Module
#include "Joint.h"

namespace love
{
namespace physics
{
namespace box2d
{

/**
 * The PulleyJoint The pulley connects two bodies to ground and
 * to each other. As one body goes up, the other goes down. The
 * total length of the pulley rope is conserved according to the
 * initial configuration: length1 + ratio * length2 <= constant.
 **/
class PulleyJoint : public Joint
{
public:

	static love::Type type;

	/**
	 * Creates a PulleyJoint connecting bodyA to bodyB.
	 **/
	PulleyJoint(Body *bodyA, Body *bodyB, b2Vec2 groundAnchorA, b2Vec2 groundAnchorB, b2Vec2 anchorA, b2Vec2 anchorB, float ratio, bool collideConnected);

	virtual ~PulleyJoint();

	/**
	 * Gets the ground anchors position in world
	 * coordinates.
	 **/
	int getGroundAnchors(lua_State *L);

	/**
	 * Gets the current length of the segment attached to bodyA.
	 **/
	float getLengthA() const;

	/**
	 * Gets the current length of the segment attached to bodyB.
	 **/
	float getLengthB() const;

	/**
	 * Gets the pulley ratio.
	 **/
	float getRatio() const;

private:
	// The Box2D DistanceJoint object.
	b2PulleyJoint *joint;
};

} // box2d
} // physics
} // love

#endif // LOVE_PHYSICS_BOX2D_PULLEY_JOINT_H
