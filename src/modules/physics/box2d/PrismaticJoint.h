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

#ifndef LOVE_PHYSICS_BOX2D_PRISMATIC_JOINT_H
#define LOVE_PHYSICS_BOX2D_PRISMATIC_JOINT_H

// Module
#include "Joint.h"

namespace love
{
namespace physics
{
namespace box2d
{
	/**
	* PrismaticJoints allow shapes to move in relation to eachother
	* along a defined axis.
	**/
	class PrismaticJoint : public Joint
	{
	private:

		// The Box2D prismatic joint object.
		b2PrismaticJoint * joint;

	public:

		/**
		* Creates a new PrismaticJoint connecting body1 and body2.
		**/
		PrismaticJoint(Body * body1, Body * body2, float x, float y, float ax, float ay);

		virtual ~PrismaticJoint();

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
		void setMotorEnabled(bool motor);

		/**
		* Checks whether the motor is enabled.
		**/
		bool isMotorEnabled() const;

		/**
		* Set the maximum motor force, usually in N.
		**/
		void setMaxMotorForce(float force);

		/**
		* Get the current motor force, usually in N.
		**/
		float getMaxMotorForce() const;

		/**
		* Set the motor speed, usually in meters per second.
		**/
		void setMotorSpeed(float speed);

		/**
		* Get the motor speed, usually in meters per second.
		**/
		float getMotorSpeed() const;

		/**
		* Get the current motor force, usually in N.
		**/
		float getMotorForce() const;

		/**
		* Enable/disable the joint limit.
		**/
		void setLimitsEnabled(bool limit);

		/**
		* Checks whether limits are enabled.
		**/
		bool isLimitsEnabled() const;

		/**
		* Sets the upper limit, usually in meters.
		**/
		void setUpperLimit(float limit);

		/**
		* Sets the lower limit, usually in meters.
		**/
		void setLowerLimit(float limit);

		/**
		* Sets the limits, usually in meters.
		**/
		void setLimits(float lower, float upper);

		/**
		* Gets the lower limit, usually in meters.
		**/
		float getLowerLimit() const;

		/**
		* Gets the upper limit, usually in meters.
		**/
		float getUpperLimit() const;
		
		/**
		* Gets the limits, usually in meters.
		* @returns The upper limit.
		* @returns The lower limit.
		**/
		int getLimits(lua_State * L);
		
	};

} // box2d
} // physics
} // love

#endif // LOVE_PHYSICS_BOX2D_PRISMATIC_JOINT_H
