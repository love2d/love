/**
 * Copyright (c) 2006-2024 LOVE Development Team
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
#include "physics/Shape.h"
#include "physics/box2d/Body.h"
#include "common/Reference.h"

// Box2D
#include <box2d/Box2D.h>

namespace love
{
namespace physics
{
namespace box2d
{

/**
 * A Shape is geometry, attached to a Body via a Fixture.
 * A Body has position and orientation, and
 * a Shape's geometry will be affected by the parent
 * body's transformation.
 **/
class Shape : public love::physics::Shape
{
public:

	friend class Physics;

	/**
	 * Creates a Shape.
	 **/
	Shape(Body *body, const b2Shape &shape);

	virtual ~Shape();

	void destroy(bool implicit = false);

	/**
	 * Returns true if the shape is active in a Box2D world.
	 **/
	bool isValid() const { return fixture != nullptr; }
	
	/**
	 * Returns true if the shape has not been destroyed.
	 **/
	bool isShapeValid() const { return shape != nullptr; }

	/**
	 * Checks whether this Shape acts as a sensor.
	 **/
	bool isSensor() const;

	/**
	 * Set whether this Shape should be a sensor or not.
	 **/
	void setSensor(bool sensor);

	/**
	 * Gets the Body this Shape is attached to.
	 **/
	Body *getBody() const;

	/**
	 * Sets the filter data. An integer array is used even though the
	 * first two elements are unsigned shorts. The elements are:
	 * category (16-bits), mask (16-bits) and group (32-bits/int).
	 **/
	void setFilterData(int *v);

	/**
	 * Gets the filter data. An integer array is used even though the
	 * first two elements are unsigned shorts. The elements are:
	 * category (16-bits), mask (16-bits) and group (32-bits/int).
	 **/
	void getFilterData(int *v);

	/**
	 * This function stores an in-C reference to
	 * arbitrary Lua data in the Shape object.
	 **/
	int setUserData(lua_State *L);

	/**
	 * Gets the data set with setUserData. If no
	 * data is set, nil is returned.
	 **/
	int getUserData(lua_State *L);

	/**
	 * Sets the friction of the Shape.
	 **/
	void setFriction(float friction);

	/**
	 * Sets the restitution for the Shape.
	 **/
	void setRestitution(float restitution);

	/**
	 * Sets the density of the Shape.
	 **/
	void setDensity(float density);

	/**
	 * Gets the friction of the Shape.
	 **/
	float getFriction() const;

	/**
	 * Gets the restitution of the Shape.
	 **/
	float getRestitution() const;

	/**
	 * Gets the density of the Shape.
	 **/
	float getDensity() const;

	/**
	 * Checks if a point is inside the Shape.
	 **/
	bool testPoint(float x, float y) const;
	bool testPoint(float x, float y, float r, float px, float py) const;

	/**
	 * Gets the type of Shape. Useful for
	 * debug drawing.
	 **/
	Type getType() const;

	float getRadius() const;
	int getChildCount() const;
	int rayCast(lua_State *L) const;
	int computeAABB(lua_State *L) const;
	int computeMass(lua_State *L) const;

	void setGroupIndex(int index);
	int getGroupIndex() const;

	int setCategory(lua_State *L);
	int setMask(lua_State *L);
	int getCategory(lua_State *L);
	int getMask(lua_State *L);
	uint16 getBits(lua_State *L);
	int pushBits(lua_State *L, uint16 bits);

	/**
	 * Gets the bounding box for this Shape.
	 **/
	int getBoundingBox(lua_State *L) const;

	/**
	 * Gets the mass data for this Shape.
	 * This operation may be expensive.
	 **/
	int getMassData(lua_State *L) const;

	void throwIfFixtureNotValid() const;
	void throwIfShapeNotValid() const;

protected:

	// The Box2D shape.
	b2Shape *shape;
	bool own;

	Shape::Type shapeType;

	Body *body;
	b2Fixture *fixture;

	// Reference to arbitrary data.
	Reference* ref = nullptr;

}; // Shape

} // box2d
} // physics
} // love

#endif // LOVE_PHYSICS_BOX2D_SHAPE_H
