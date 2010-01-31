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

#ifndef LOVE_PHYSICS_BOX2D_SHAPE_H
#define LOVE_PHYSICS_BOX2D_SHAPE_H

// LOVE
#include <physics/Shape.h>
#include <common/Reference.h>

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

	/**
	* This struct is stored in a void pointer
	* in the Box2D Shape class. For now, all we
	* need is a Lua reference to arbitrary data,
	* but we might need more later.
	**/
	struct shapeudata
	{
		// Reference to arbitrary data.
		Reference * ref;
	};

	/**
	* A Shape is geometry attached to a Body.
	* A Body has position and orientation, and
	* a Shape's geometry will be affected by the parent
	* body's transformation.
	**/
	class Shape : public love::physics::Shape
	{
	protected:

		// A pointer to the parent Body. If the Body
		// is destroyed, all child shapes are destroyed as well.
		// This ensures that all child shapes are always destroyed
		// *before* the parent Body.
		Body * body;

		// The Box2D shape.
		b2Shape * shape;

		// Our shape data, to be stored in
		// the Box2D shape.
		shapeudata * data;

	public:

		/**
		* Creates a Shape attatched to the specified
		* Body.
		**/
		Shape(Body * body);

		virtual ~Shape();

		/**
		* Gets the type of Shape. Useful for
		* debug drawing.
		**/
		Type getType() const;

		/**
		* Sets the friction of the Shape.
		* @param friction The new friction.
		**/
		void setFriction(float friction);

		/**
		* Sets the restitution for the Shape.
		* @param restitution The restitution.
		**/
		void setRestitution(float restitution);

		/**
		* Sets the density of the Shape.
		* @param density The density of the Shape.
		**/
		void setDensity(float density);

		/**
		* Set whether this shape should be a sensor or not.
		* @param sensor True if sensor, false if not.
		**/
		void setSensor(bool sensor);

		/**
		* Gets the friction of the Shape.
		* @returns The friction.
		**/
		float getFriction() const;

		/**
		* Gets the restitution of the Shape.
		* @return The restitution of the Shape.
		**/
		float getRestitution() const;

		/**
		* Gets the density of the Shape.
		* @return The density.
		**/
		float getDensity() const;

		/**
		* Checks whether this Shape acts as a sensor.
		* @return True if sensor, false otherwise.
		**/
		bool isSensor() const;

		/**
		* Get the body attatched to this Shape.
		* @return The parent Body.
		**/
		Body * getBody() const;

		/**
		* Checks if a point is inside the Shape.
		* @param x The x-component of the Shape.
		* @param y The y-component of the Shape.
		**/
		bool testPoint(float x, float y) const;

		/**
		* Tests whether a line segment intersects a Shape.
		**/
		int testSegment(lua_State * L);

		/**
		* Sets the filter data. An integer array is used even though the
		* first two elements are unsigned shorts. The elements are:
		* category (16-bits), mask (16-bits) and group (32-bits/int).
		**/
		void setFilterData(int * v);

		/**
		* Gets the filter data. An integer array is used even though the
		* first two elements are unsigned shorts. The elements are:
		* category (16-bits), mask (16-bits) and group (32-bits/int).
		**/
		void getFilterData(int * v);

		void setGroupIndex(int index);
		int getGroupIndex() const;

		int setCategory(lua_State * L);
		int setMask(lua_State * L);
		int getCategory(lua_State * L);
		int getMask(lua_State * L);
		uint16 getBits(lua_State * L);
		int pushBits(lua_State * L, uint16 bits);

		/**
		* This function stores an in-C reference to
		* arbitrary Lua data in the Box2D shape object.
		*
		* The data set here will be passed to the collision
		* handler when collisions occur.
		**/
		int setData(lua_State * L);

		/**
		* Gets the data set with setData. If no
		* data is set, nil is returned.
		**/
		int getData(lua_State * L);

		/**
		* Gets the bounding box for this Shape.
		* The function returns eight values which can be
		* passed directly to love.graphics.polygon.
		**/
		int getBoundingBox(lua_State * L);
	};

} // box2d
} // physics
} // love

#endif // LOVE_PHYSICS_BOX2D_SHAPE_H
