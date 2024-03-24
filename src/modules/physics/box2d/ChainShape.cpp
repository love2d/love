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

#include "ChainShape.h"

// Module
#include "Body.h"
#include "World.h"
#include "Physics.h"

namespace love
{
namespace physics
{
namespace box2d
{

love::Type ChainShape::type("ChainShape", &Shape::type);

ChainShape::ChainShape(Body *body, const b2ChainShape &c)
	: Shape(body, c)
{
}

ChainShape::~ChainShape()
{
}

void ChainShape::setNextVertex(float x, float y)
{
	throwIfShapeNotValid();
	b2Vec2 v(x, y);
	b2ChainShape *c = (b2ChainShape *)shape;
	c->m_nextVertex = Physics::scaleDown(v);
}

void ChainShape::setPreviousVertex(float x, float y)
{
	throwIfShapeNotValid();
	b2Vec2 v(x, y);
	b2ChainShape *c = (b2ChainShape *)shape;
	c->m_prevVertex = Physics::scaleDown(v);
}

b2Vec2 ChainShape::getNextVertex() const
{
	throwIfShapeNotValid();
	b2ChainShape *c = (b2ChainShape *)shape;
	return Physics::scaleUp(c->m_nextVertex);
}

b2Vec2 ChainShape::getPreviousVertex() const
{
	throwIfShapeNotValid();
	b2ChainShape *c = (b2ChainShape *)shape;
	return Physics::scaleUp(c->m_prevVertex);
}

EdgeShape *ChainShape::getChildEdge(int index) const
{
	throwIfShapeNotValid();

	b2ChainShape *c = (b2ChainShape *)shape;

	b2EdgeShape e;
	c->GetChildEdge(&e, index);

	return new EdgeShape(nullptr, e);
}

int ChainShape::getVertexCount() const
{
	throwIfShapeNotValid();
	b2ChainShape *c = (b2ChainShape *)shape;
	return c->m_count;
}

b2Vec2 ChainShape::getPoint(int index) const
{
	throwIfShapeNotValid();
	b2ChainShape *c = (b2ChainShape *)shape;
	if (index < 0 || index >= c->m_count)
		throw love::Exception("Physics error: index out of bounds");
	const b2Vec2 &v = c->m_vertices[index];
	return Physics::scaleUp(v);
}

const b2Vec2 *ChainShape::getPoints() const
{
	throwIfShapeNotValid();
	b2ChainShape *c = (b2ChainShape *)shape;
	return c->m_vertices;
}

} // box2d
} // physics
} // love
