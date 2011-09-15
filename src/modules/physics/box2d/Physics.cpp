/**
* Copyright (c) 2006-2011 LOVE Development Team
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

#include "Physics.h"

// Convex Hull Scan
#include "graham/GrahamScanConvexHull.h"

// LOVE
#include <common/math.h>
#include "wrap_Body.h"

namespace love
{
namespace physics
{
namespace box2d
{

	const char * Physics::getName() const
	{
		return "love.physics.box2d";
	}

	World * Physics::newWorld(float lx, float ly, float ux, float uy, float gx, float gy, bool sleep, int meter)
	{
		b2AABB aabb;
		aabb.lowerBound.Set(lx, ly);
		aabb.upperBound.Set(ux, uy);
		return new World(aabb, b2Vec2(gx, gy), sleep, meter);
	}

	Body * Physics::newBody(World * world, float x, float y, float mass, float i)
	{
		return new Body(world, b2Vec2(x, y), mass, i);
	}

	Body * Physics::newBody(World * world, float x, float y)
	{
		return new Body(world, b2Vec2(x, y), 1, 1);
	}

	Body * Physics::newBody(World * world)
	{
		return new Body(world, b2Vec2(0, 0), 1, 1);
	}

	CircleShape * Physics::newCircleShape(float radius)
	{
		return newCircleShape(0, 0, radius);
	}

	CircleShape * Physics::newCircleShape(float x, float y, float radius)
	{
		b2CircleShape s;
		s.m_p = b2Vec2(x, y);
		s.m_radius = radius;
		return new CircleShape(&s);
	}

	PolygonShape * Physics::newRectangleShape(float w, float h)
	{
		return newRectangleShape(0, 0, w, h, 0);
	}

	PolygonShape * Physics::newRectangleShape(float x, float y, float w, float h)
	{
		return newRectangleShape(x, y, w, h, 0);
	}

	PolygonShape * Physics::newRectangleShape(float x, float y, float w, float h, float angle)
	{
		b2PolygonShape s;
		s.SetAsBox(w/2.0f, h/2.0f, b2Vec2(x, y), angle);
		return new PolygonShape(&s);
	}
	
	EdgeShape * Physics::newEdgeShape(float x1, float y1, float x2, float y2)
	{
		b2EdgeShape s;
		s.Set(b2Vec2(x1, y1), b2Vec2(x2, y2));
		return new EdgeShape(&s);
	}

	int Physics::newPolygonShape(lua_State * L)
	{
		int argc = lua_gettop(L);
		int vcount = (int)argc/2;
		// 3 vertices
		love::luax_assert_argc(L, 2 * 3);

		b2PolygonShape s;

		std::vector<point2d> points(s.m_vertexCount);
		std::vector<point2d> convex_hull;

		for(int i = 0;i<vcount;i++)
		{
			float x = (float)lua_tonumber(L, -2);
			float y = (float)lua_tonumber(L, -1);
			point2d tmp(x, y);
			points.push_back(tmp);
			lua_pop(L, 2);
		}

		// Compute convex hull.
		GrahamScanConvexHull()(points, convex_hull);
		
		int count = convex_hull.size();

		if(count < 3)
			return luaL_error(L, "Polygon degenerated to less than three points.");
		
		b2Vec2 vecs[] = new b2vec2[count];
		
		for (int i = 0; i < count; i++) {
			vecs[i].Set(convex_hull[i].x, convex_hull[i].y);
		}
		
		s.Set(vecs, count);

		PolygonShape * p = new PolygonShape(&s);
		delete[] vecs;

		luax_newtype(L, "PolygonShape", PHYSICS_POLYGON_SHAPE_T, (void*)p);

		return 1;
	}

	DistanceJoint * Physics::newDistanceJoint(Body * body1, Body * body2, float x1, float y1, float x2, float y2)
	{
		return new DistanceJoint(body1, body2, x1, y1, x2, y2);
	}

	MouseJoint * Physics::newMouseJoint(Body * body, float x, float y)
	{
		return new MouseJoint(body, x, y);
	}

	RevoluteJoint * Physics::newRevoluteJoint(Body * body1, Body * body2, float x, float y)
	{
		return new RevoluteJoint(body1, body2, x, y);
	}

	PrismaticJoint * Physics::newPrismaticJoint(Body * body1, Body * body2, float x, float y, float ax, float ay)
	{
		return new PrismaticJoint(body1, body2, x, y, ax, ay);
	}

	PulleyJoint * Physics::newPulleyJoint(Body * body1, Body * body2, b2Vec2 groundAnchor1, b2Vec2 groundAnchor2, b2Vec2 anchor1, b2Vec2 anchor2, float ratio)
	{
		return new PulleyJoint(body1, body2, groundAnchor1, groundAnchor2, anchor1, anchor2, ratio);
	}

	GearJoint * Physics::newGearJoint(Joint * joint1, Joint * joint2, float ratio)
	{
		return new GearJoint(joint1, joint2, ratio);
	}
	
	FrictionJoint * Physics::newFrictionJoint(Body * body1, Body * body2, float x, float y)
	{
		return new FrictionJoint(body1, body2, x, y);
	}
	
	WeldJoint * Physics::newWeldJoint(Body * body1, Body * body2, float x, float y)
	{
		return new WeldJoint(body1, body2, x, y);
	}
	
	WheelJoint * Physics::newWheelJoint(Body * body1, Body * body2, float x, float y, float ax, float ay)
	{
		return new WheelJoint(body1, body2, x, y, ax, ay);
	}
	
	RopeJoint * Physics::newRopeJoint(Body * body1, Body * body2, float x1, float y1, float x2, float y2, float maxLength)
	{
		return new RopeJoint(body1, body2, x1, y1, x2, y2, maxLength);
	}

} // box2d
} // physics
} // love
