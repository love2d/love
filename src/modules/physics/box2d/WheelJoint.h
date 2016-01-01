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

#ifndef LOVE_PHYSICS_BOX2D_WHEEL_JOINT_H
#define LOVE_PHYSICS_BOX2D_WHEEL_JOINT_H

// Module
#include "Joint.h"

namespace love
{
namespace physics
{
namespace box2d
{

/**
 * WheelJoints provide two degrees of freedom: translation
 * along a defined axis and rotation in the plane. Designed
 * for vehicle suspensions.
 **/
class WheelJoint : public Joint
{
public:

	/**
	 * Creates a new WheelJoint connecting body1 and body2.
	 **/
	WheelJoint(Body *body1, Body *body2, float xA, float yA, float xB, float yB, float ax, float ay, bool collideConnected);

	virtual ~WheelJoint();

	/**
	 * Get the current joint translation, usually in meters.
	 **/
	float getJointTranslation() const;

	/**
	 * Get the current joint translation speed, usually in meters per second.
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
	 * Set the motor speed, usually in meters per second.
	 **/
	void setMotorSpeed(float speed);

	/**
	 * Get the motor speed, usually in meters per second.
	 **/
	float getMotorSpeed() const;

	/**
	 * Set the maximum motor torque, usually in N.
	 **/
	void setMaxMotorTorque(float torque);

	/**
	 * Get the maximum motor torque, usually in N.
	 **/
	float getMaxMotorTorque() const;

	/**
	 * Get the current motor torque, usually in N.
	 * @param inv_dt The inverse time step.
	 **/
	float getMotorTorque(float inv_dt) const;

	/**
	 * Set the spring frequency, in hertz. Setting the frequency to 0
	 * disables the spring.
	 **/
	void setSpringFrequency(float hz);

	/**
	 * Get the spring frequency, in hertz.
	 **/
	float getSpringFrequency() const;

	/**
	 * Set the spring damping ratio.
	 **/
	void setSpringDampingRatio(float ratio);

	/**
	 * Get the spring damping ratio.
	 **/
	float getSpringDampingRatio() const;

private:

	// The Box2D wheel joint object.
	b2WheelJoint *joint;
};

} // box2d
} // physics
} // love

#endif // LOVE_PHYSICS_BOX2D_WHEEL_JOINT_H
