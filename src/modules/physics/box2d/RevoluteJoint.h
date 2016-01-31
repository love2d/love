/**
 * Copyright (c) 2006-2016 LOVE Development Team
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

#ifndef LOVE_PHYSICS_BOX2D_REVOLUTE_JOINT_H
#define LOVE_PHYSICS_BOX2D_REVOLUTE_JOINT_H

// Module
#include "Joint.h"

namespace love
{
namespace physics
{
namespace box2d
{

/**
 * A RevoluteJoint allows two bodies relative rotation
 * around a single point.
 **/
class RevoluteJoint : public Joint
{
public:

	/**
	 * Creates a new RevoluteJoint connecting body1 and body2.
	 **/
	RevoluteJoint(Body *body1, Body *body2, float x, float y, bool collideConnected);

	virtual ~RevoluteJoint();

	/**
	 * Get the current joint angle in degrees.
	 **/
	float getJointAngle() const;

	/**
	 * Get the current joint angle speed in degrees per second.
	 **/
	float getJointSpeed() const;

	/**
	 * Enable/disable the joint motor.
	 **/
	void setMotorEnabled(bool enable);

	/**
	 * Checks whether the motor is enabled.
	 **/
	bool isMotorEnabled() const;

	/**
	 * Set the maximum motor torque, usually in N-m.
	 **/
	void setMaxMotorTorque(float torque);

	/**
	 * Sets the motor speed in radians per second.
	 **/
	void setMotorSpeed(float speed);

	/**
	 * Gets the motor speed in radians per second.
	 **/
	float getMotorSpeed() const;

	/**
	 * Get the current motor torque, usually in N-m.
	 * @param inv_dt The inverse timestep.
	 **/
	float getMotorTorque(float inv_dt) const;

	/**
	 * Get the maximum motor torque, usually in N-m.
	 **/
	float getMaxMotorTorque() const;

	/**
	 * Enable/disable the joint limit.
	 **/
	void setLimitsEnabled(bool enable);

	/**
	 * Checks whether limits are enabled.
	 **/
	bool hasLimitsEnabled() const;

	/**
	 * Sets the upper limit in degrees.
	 **/
	void setUpperLimit(float limit);

	/**
	 * Sets the lower limit in degrees.
	 **/
	void setLowerLimit(float limit);

	/**
	 * Sets the limits in degrees.
	 **/
	void setLimits(float lower, float upper);

	/**
	 * Gets the lower limit in degrees.
	 **/
	float getLowerLimit() const;

	/**
	 * Gets the upper limit in degrees.
	 **/
	float getUpperLimit() const;

	/**
	 * Gets the limits in degrees.
	 * @returns The lower limit.
	 * @returns The upper limit.
	 **/
	int getLimits(lua_State *L);

private:

	// The Box2D revolute joint object.
	b2RevoluteJoint *joint;
};

} // box2d
} // physics
} // love

#endif // LOVE_PHYSICS_BOX2D_REVOLUTE_JOINT_H
