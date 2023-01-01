/**
 * Copyright (c) 2006-2023 LOVE Development Team
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

// Box2D
#include <Box2D/Box2D.h>

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

	friend class Fixture;

	/**
	 * Creates a Shape.
	 **/
	Shape();
	Shape(b2Shape *shape, bool own = true);

	virtual ~Shape();

	/**
	 * Gets the type of Shape. Useful for
	 * debug drawing.
	 **/
	Type getType() const;
	float getRadius() const;
	int getChildCount() const;
	bool testPoint(float x, float y, float r, float px, float py) const;
	int rayCast(lua_State *L) const;
	int computeAABB(lua_State *L) const;
	int computeMass(lua_State *L) const;

protected:

	// The Box2D shape.
	b2Shape *shape;
	bool own;
};

} // box2d
} // physics
} // love

#endif // LOVE_PHYSICS_BOX2D_SHAPE_H
