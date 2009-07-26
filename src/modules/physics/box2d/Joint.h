/**
* Copyright (c) 2006-2009 LOVE Development Team
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

#ifndef LOVE_PHYSICS_BOX2D_JOINT_H
#define LOVE_PHYSICS_BOX2D_JOINT_H

// LOVE
#include <common/runtime.h>
#include <physics/Joint.h>

// Box2D
#include "Include/Box2D.h"

namespace love
{
namespace physics
{
namespace box2d
{
	// Forward declarations.
	class Body;
	class World;

	/**
	* A Joint acts as positioning constraints on Bodies. 
	* A Joint can be used to prevent Bodies from going to 
	* far apart, or coming too close together.
	**/
	class Joint : public love::physics::Joint
	{
		friend class GearJoint;
		
	private:
		
		// A Joint must be destroyed *before* the bodies it acts upon, 
		// and the world they reside in. We therefore need refs
		// parents and associations to prevent wrong destruction order.
		Body * body1, * body2;
		

		// The Box2D joint object.
		b2Joint * joint;

	protected:
		World * world;

	public:

		/**
		* This constructor will connect one end of the joint to body1, 
		* and the other one to the default ground body.
		* 
		* This constructor is mainly used by MouseJoint.
		**/
		Joint(Body * body1);

		/**
		* Create a joint between body1 and body2.
		**/
		Joint(Body * body1, Body * body2);

		virtual ~Joint();

		/**
		* Gets the type of joint.
		**/
		int getType() const;

		/**
		* Gets the anchor positions of the Joint in world
		* coordinates. This is useful for debugdrawing the joint.
		**/
		int getAnchors(lua_State * L);

		/**
		* Gets the reaction force on body2 at the joint anchor.
		**/
		int getReactionForce(lua_State * L);

		/**
		* Gets the reaction torque on body2.
		**/
		float getReactionTorque();

		/**
		* Sets whether connected bodies should collide
		* or not. Default is false.
		**/
		void setCollideConnected(bool collide);

		/**
		* Gets whether connected bodies should collide
		* or not.
		**/
		bool getCollideConnected() const;

	protected:

		/**
		* Joints require pointers to a Box2D joint objects at
		* different polymorphic levels, which is why these function 
		* were created.
		**/

		/**
		* Creates a Joint, and ensures that the parent class
		* gets a copy of the pointer.
		**/
		b2Joint * createJoint(b2JointDef * def);

		/**
		* Destroys the joint. This function was created just to 
		* get some cinsistency.
		**/
		void destroyJoint(b2Joint * joint);
	};

} // box2d
} // physics
} // love

#endif // LOVE_PHYSICS_BOX2D_JOINT_H
