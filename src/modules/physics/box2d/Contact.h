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

#ifndef LOVE_PHYSICS_BOX2D_CONTACT_H
#define LOVE_PHYSICS_BOX2D_CONTACT_H

// LOVE
#include <common/Object.h>
#include <common/runtime.h>
#include "World.h"

// Box2D
#include "Include/Box2D.h"

namespace love
{
namespace physics
{
namespace box2d
{
	class World;

	/**
	* A Contact represents a collision point between
	* two shapes.
	**/
	class Contact : public Object
	{
		// Friends.
		friend class World;
		friend class World::ContactCallback;

	private:

		// The Box2D contact point.
		b2ContactPoint point;

		// The parent world. Needed for scaling.
		World * world;

	public:

		/**
		* Creates a new Contact by copying a Box2D contact
		* point. It does not store the pointer, but copy the
		* data pointed to.
		* @param point Pointer to the Box2D contact.
		**/
		Contact(World * world, const b2ContactPoint * point);

		virtual ~Contact();

		/**
		* Gets the position of the Contact.
		* @return The position along the x-axis.
		* @return The position along the y-axis.
		**/
		int getPosition(lua_State * L);

		/**
		* Gets the linear impact velocity.
		* @return The velocity along the x-axis.
		* @return The velocity along the y-axis.
		**/
		int getVelocity(lua_State * L);

		/**
		* Gets the collision normal.
		* @return The x-component of the normal.
		* @return The y-component of the normal.
		**/
		int getNormal(lua_State * L);

		/**
		* How far apart the shapes are. If they are intersecting
		* this value is negative.
		**/
		float getSeparation() const;

		/**
		* The mixed friction between the two shapes at
		* the point of impact.
		**/
		float getFriction() const;

		/**
		* The mixed restitution of the two shapes
		* at the point of impact.
		**/
		float getRestitution() const;

	};

} // box2d
} // physics
} // love

#endif // LOVE_PHYSICS_BOX2D_CONTACT_H
