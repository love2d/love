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

#ifndef LOVE_PHYSICS_BOX2D_GEAR_JOINT_H
#define LOVE_PHYSICS_BOX2D_GEAR_JOINT_H

// Module
#include "Joint.h"

namespace love
{
namespace physics
{
namespace box2d
{

/**
 * A gear joint is used to connect two joints together. Either joint
 * can be a revolute or prismatic joint. You specify a gear ratio
 * to bind the motions together:
 * coordinate1 + ratio * coordinate2 = constant
 *
 * The ratio can be negative or positive. If one joint is a revolute joint
 * and the other joint is a prismatic joint, then the ratio will have units
 * of length or units of 1/length.
 * Warning: The revolute and prismatic joints must be attached to
 * fixed bodies (which must be body1 on those joints).
 **/
class GearJoint : public Joint
{
public:

	static love::Type type;

	/**
	 * Creates a GearJoint connecting joint1 to joint2.
	 **/
	GearJoint(Joint *joint1, Joint *joint2, float ratio, bool collideConnected);

	virtual ~GearJoint();

	/**
	 * Sets the ratio.
	 **/
	void setRatio(float ratio);

	/**
	 * Gets the ratio.
	 **/
	float getRatio() const;

	Joint *getJointA() const;
	Joint *getJointB() const;

private:
	// The Box2D GearJoint object.
	b2GearJoint *joint;
};

} // box2d
} // physics
} // love

#endif // LOVE_PHYSICS_BOX2D_GEAR_JOINT_H
