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
public:

	static love::Type type;

	/**
	 * Create a new CircleShape from the a Box2D CircleShape definition.
	 * @param c The CircleShape definition.
	 **/
	CircleShape(b2CircleShape *c, bool own = true);

	virtual ~CircleShape();

	/**
	 * Gets the radius for the circle.
	 **/
	float getRadius() const;

	/**
	 * Sets the radius of the circle.
	 **/
	void setRadius(float r);

	/**
	 * Gets the position of the circle.
	 **/
	void getPoint(float &x_o, float &y_o) const;

	/**
	 * Sets the position for the circle.
	 **/
	void setPoint(float x, float y);

}; // CircleShape

} // box2d
} // physics
} // love

#endif // LOVE_PHYSICS_BOX2D_CIRCLE_SHAPE_H
