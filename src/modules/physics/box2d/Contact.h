/**
* Copyright (c) 2006-2012 LOVE Development Team
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
#include <Box2D/Box2D.h>

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

		// The Box2D contact.
		b2Contact* contact;

	public:

		/**
		* Creates a new Contact by copying a Box2D contact
		* point. It does not store the pointer, but copy the
		* data pointed to.
		* @param point Pointer to the Box2D contact.
		**/
		Contact(b2Contact * contact);

		virtual ~Contact();

		/**
		* Gets the position of each point of contact.
		* @return The position along the x-axis.
		* @return The position along the y-axis.
		**/
		int getPositions(lua_State * L);

		/**
		* Gets the collision normal.
		* @return The x-component of the normal.
		* @return The y-component of the normal.
		**/
		int getNormal(lua_State * L);

		/**
		* The mixed friction between the two fixtures at
		* the point of impact.
		**/
		float getFriction() const;

		/**
		* The mixed restitution of the two fixtures
		* at the point of impact.
		**/
		float getRestitution() const;

		/**
		* Check if the contact is enabled.
		**/
		bool isEnabled() const;

		/**
		* Check if the contact is touching.
		**/
		bool isTouching() const;

		// Only call the setters in PreSolve

		/**
		* Override the default friction mixture.
		**/
		void setFriction(float friction);

		/**
		* Override the default restitution mixture.
		**/
		void setRestitution(float restitution);

		/**
		* Enable/disable this contact.
		**/
		void setEnabled(bool enabled);

		/**
		* Reset the friction mixture to the default
		* value.
		**/
		void resetFriction();

		/**
		* Reset the restitution mixture to the default
		* value.
		**/
		void resetRestitution();

	};

} // box2d
} // physics
} // love

#endif // LOVE_PHYSICS_BOX2D_CONTACT_H
