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

#include "PolygonShape.h"

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

PolygonShape::PolygonShape(b2PolygonShape *p, bool own)
	: Shape(p, own)
{
}

PolygonShape::~PolygonShape()
{
}

int PolygonShape::getPoints(lua_State *L)
{
	love::luax_assert_argc(L, 0);
	b2PolygonShape *p = (b2PolygonShape *)shape;
	int count = p->GetVertexCount();
	for (int i = 0; i<count; i++)
	{
		b2Vec2 v = Physics::scaleUp(p->GetVertex(i));
		lua_pushnumber(L, v.x);
		lua_pushnumber(L, v.y);
	}
	return count*2;
}

bool PolygonShape::validate() const
{
	b2PolygonShape *p = (b2PolygonShape *)shape;
	return p->Validate();
}

} // box2d
} // physics
} // love
