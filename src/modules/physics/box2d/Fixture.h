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

#ifndef LOVE_PHYSICS_BOX2D_FIXTURE_H
#define LOVE_PHYSICS_BOX2D_FIXTURE_H

// LOVE
#include "physics/Shape.h"
#include "physics/box2d/Body.h"
#include "physics/box2d/Shape.h"
#include "common/Object.h"
#include "common/Reference.h"

// Box2D
#include <Box2D/Box2D.h>

namespace love
{
namespace physics
{
namespace box2d
{

/**
 * This struct is stored in a void pointer
 * in the Box2D Fixture class. For now, all we
 * need is a Lua reference to arbitrary data,
 * but we might need more later.
 **/
struct fixtureudata
{
	// Reference to arbitrary data.
	Reference *ref = nullptr;
};

/**
 * A Fixture is used to attach a shape to a body for collision detection.
 * A Fixture inherits its transform from its parent. Fixtures hold
 * additional non-geometric data such as friction, collision filters,
 * etc.
 **/
class Fixture : public Object
{
public:
	friend class Physics;

	/**
	 * Creates a Fixture.
	 **/
	Fixture(Body *body, Shape *shape, float density);

	/**
	 * Creates a Fixture.
	 **/
	Fixture(b2Fixture *f);

	virtual ~Fixture();

	/**
	 * Gets the type of the Fixture's Shape. Useful for
	 * debug drawing.
	 **/
	Shape::Type getType() const;

	/**
	 * Gets the Shape attached to this Fixture.
	 **/
	Shape *getShape() const;

	/**
	 * Returns true if the fixture is active in a Box2D world.
	 **/
	bool isValid() const;

	/**
	 * Checks whether this Fixture acts as a sensor.
	 * @return True if sensor, false otherwise.
	 **/
	bool isSensor() const;

	/**
	 * Set whether this Fixture should be a sensor or not.
	 * @param sensor True if sensor, false if not.
	 **/
	void setSensor(bool sensor);

	/**
	 * Gets the Body this Fixture is attached to.
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
	 * arbitrary Lua data in the Box2D Fixture object.
	 **/
	int setUserData(lua_State *L);

	/**
	 * Gets the data set with setData. If no
	 * data is set, nil is returned.
	 **/
	int getUserData(lua_State *L);

	/**
	 * Sets the friction of the Fixture.
	 * @param friction The new friction.
	 **/
	void setFriction(float friction);

	/**
	 * Sets the restitution for the Fixture.
	 * @param restitution The restitution.
	 **/
	void setRestitution(float restitution);

	/**
	 * Sets the density of the Fixture.
	 * @param density The density of the Fixture.
	 **/
	void setDensity(float density);

	/**
	 * Gets the friction of the Fixture.
	 * @returns The friction.
	 **/
	float getFriction() const;

	/**
	 * Gets the restitution of the Fixture.
	 * @return The restitution of the Fixture.
	 **/
	float getRestitution() const;

	/**
	 * Gets the density of the Fixture.
	 * @return The density.
	 **/
	float getDensity() const;

	/**
	 * Checks if a point is inside the Fixture.
	 * @param x The x-component of the point.
	 * @param y The y-component of the point.
	 **/
	bool testPoint(float x, float y) const;

	/**
	 * Cast a ray against this Fixture.
	 **/
	int rayCast(lua_State *L) const;

	void setGroupIndex(int index);
	int getGroupIndex() const;

	int setCategory(lua_State *L);
	int setMask(lua_State *L);
	int getCategory(lua_State *L);
	int getMask(lua_State *L);
	uint16 getBits(lua_State *L);
	int pushBits(lua_State *L, uint16 bits);

	/**
	 * Gets the bounding box for this Fixture.
	 * The function returns eight values which can be
	 * passed directly to love.graphics.polygon.
	 **/
	int getBoundingBox(lua_State *L) const;

	/**
	 * Gets the mass data for this Fixture.
	 * This operation may be expensive.
	 **/
	int getMassData(lua_State *L) const;

	/**
	 * Destroys this fixture.
	 **/
	void destroy(bool implicit = false);

protected:

	Body *body;
	fixtureudata *data;
	b2Fixture *fixture;
};

} // box2d
} // physics
} // love

#endif // LOVE_PHYSICS_BOX2D_FIXTURE_H
