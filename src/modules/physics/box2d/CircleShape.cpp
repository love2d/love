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

#include "CircleShape.h"

// Module
#include "Body.h"
#include "World.h"
#include "Physics.h"

#include "common/Memoizer.h"

namespace love
{
namespace physics
{
namespace box2d
{

CircleShape::CircleShape(b2CircleShape *c, bool own)
	: Shape(c, own)
{
}

CircleShape::~CircleShape()
{
}

float CircleShape::getRadius() const
{
	return Physics::scaleUp(shape->m_radius);
}

void CircleShape::setRadius(float r)
{
	shape->m_radius = Physics::scaleDown(r);
}

void CircleShape::getPoint(float &x_o, float &y_o) const
{
	b2CircleShape *c = (b2CircleShape *) shape;
	x_o = Physics::scaleUp(c->m_p.x);
	y_o = Physics::scaleUp(c->m_p.y);
}

void CircleShape::setPoint(float x, float y)
{
	b2CircleShape *c = (b2CircleShape *) shape;
	c->m_p = Physics::scaleDown(b2Vec2(x, y));
}

} // box2d
} // physics
} // love
