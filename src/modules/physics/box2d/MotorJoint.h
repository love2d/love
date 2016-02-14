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

#ifndef LOVE_PHYSICS_BOX2D_MOTOR_JOINT_H
#define LOVE_PHYSICS_BOX2D_MOTOR_JOINT_H

// Module
#include "Joint.h"

namespace love
{
namespace physics
{
namespace box2d
{

/**
 * A motor joint is used to control the relative motion
 * between two bodies. A typical usage is to control the movement
 * of a dynamic body with respect to the ground.
 */
class MotorJoint : public Joint
{
public:

	MotorJoint(Body *body1, Body* body2);
	MotorJoint(Body *body1, Body* body2, float correctionFactor, bool collideConnected);
	virtual ~MotorJoint();

	/// Set/get the target linear offset, in frame A, in meters.
	void setLinearOffset(float x, float y);
	int getLinearOffset(lua_State *L) const;

	/// Set/get the target angular offset, in radians.
	void setAngularOffset(float angularOffset);
	float getAngularOffset() const;

	/// Set the maximum friction force.
	void setMaxForce(float force);

	/// Get the maximum friction force.
	float getMaxForce() const;

	/// Set the maximum friction torque.
	void setMaxTorque(float torque);

	/// Get the maximum friction torque.
	float getMaxTorque() const;

	/// Set the position correction factor in the range [0,1].
	void setCorrectionFactor(float factor);

	/// Get the position correction factor in the range [0,1].
	float getCorrectionFactor() const;

private:

	// The Box2D MotorJoint object.
	b2MotorJoint *joint;

}; // MotorJoint


} // box2d
} // physics
} // love

#endif // LOVE_PHYSICS_BOX2D_MOTOR_JOINT_H
