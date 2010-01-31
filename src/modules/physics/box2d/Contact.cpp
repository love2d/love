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

#include "Contact.h"
#include "World.h"

namespace love
{
namespace physics
{
namespace box2d
{
	Contact::Contact(World * world, const b2ContactPoint * point)
		: point(*point), world(world)
	{
		world->retain();
	}

	Contact::~Contact()
	{
		world->release();
	}

	int Contact::getPosition(lua_State * L)
	{
		love::luax_assert_argc(L, 1, 1);
		lua_pushnumber(L, world->scaleUp(point.position.x));
		lua_pushnumber(L, world->scaleUp(point.position.y));
		return 2;
	}

	int Contact::getVelocity(lua_State * L)
	{
		love::luax_assert_argc(L, 1, 1);
		lua_pushnumber(L, world->scaleUp(point.velocity.x));
		lua_pushnumber(L, world->scaleUp(point.velocity.y));
		return 2;
	}

	int Contact::getNormal(lua_State * L)
	{
		love::luax_assert_argc(L, 1, 1);
		lua_pushnumber(L, world->scaleUp(point.normal.x));
		lua_pushnumber(L, world->scaleUp(point.normal.y));
		return 2;
	}

	float Contact::getSeparation() const
	{
		return world->scaleUp(point.separation);
	}

	float Contact::getFriction() const
	{
		return point.friction;
	}

	float Contact::getRestitution() const
	{
		return point.restitution;
	}

} // box2d
} // physics
} // love
