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
	private:
		// The Box2D DistanceJoint object.
		b2PulleyJoint * joint;
	public:

		/**
		* Creates a PulleyJoint connecting body1 to body2. 
		**/
		PulleyJoint(Body * body1, Body * body2, b2Vec2 groundAnchor1, b2Vec2 groundAnchor2, b2Vec2 anchor1, b2Vec2 anchor2, float ratio);
		
		virtual ~PulleyJoint();
		
		/**
		* Gets the ground anchors position in world
		* coordinates.
		**/
		int getGroundAnchors(lua_State * L);
		
		/**
		* Sets the max rope lengths (a value of zero keeps it
		* unchanged). Beware that Box2D also imposes a maximum,
		* so the smallest of these is actually set.
		**/
		void setMaxLengths(float maxlength1, float maxlength2);
		
		/**
		* Gets the max rope lengths.
		**/
		int getMaxLengths(lua_State * L);
				
		/**
		* Sets the total length of the rope and updates the 
		* MaxLengths values accordingly.
		**/
		void setConstant(float length);
		
		/**
		* Gets the total length of the rope.
		**/
		float getConstant() const;
		
		/**
		* Gets the current length of the segment attached to body1.
		**/
		float getLength1() const;
	
		/**
		* Gets the current length of the segment attached to body2.
		**/
		float getLength2() const;
		
		/**
		* Sets the pulley ratio.
		**/
		void setRatio(float ratio);
		
		/**
		* Gets the pulley ratio.
		**/
		float getRatio() const;
		
	};

} // box2d
} // physics
} // love

#endif // LOVE_PHYSICS_BOX2D_PULLEY_JOINT_H
