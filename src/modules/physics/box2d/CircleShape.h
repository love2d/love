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

#ifndef LOVE_PHYSICS_BOX2D_CIRCLE_SHAPE_H
#define LOVE_PHYSICS_BOX2D_CIRCLE_SHAPE_H

// Module
#include "Shape.h"

#include <vector>

namespace love
{
namespace physics
{
namespace box2d
{
	/**
	* A CircleShape represent a Circle which can 
	* be used for collision detection and physics.
	* 
	* The CircleShape is much faster than the PolygonShape, 
	* and should generally be used where possible.
	**/
	class CircleShape : public Shape
	{
	private:

		// The radius of the circle. We need to store this because
		// Box2D has no built-in method for getting the radius.
		float radius;

		// Local offset.
		b2Vec2 localPosition;

	public:
		
		/**
		* Create a new CircleShape from the parent body and a 
		* Box2D CircleShape definition.
		* @param body The parent body.
		* @param def The CircleShape definition.
		**/
		CircleShape(Body * body, b2CircleDef * def);

		virtual ~CircleShape();

		/**
		* Gets the radius for the circle.
		**/
		float getRadius() const;

		// There is no support for setting the radius.

		void getLocalCenter(float & x, float & y) const;
		void getWorldCenter(float & x, float & y) const;
	};

} // box2d
} // physics
} // love

#endif // LOVE_PHYSICS_BOX2D_CIRCLE_SHAPE_H
