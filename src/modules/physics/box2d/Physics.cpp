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

#include "Physics.h"

// LOVE
#include "common/math.h"
#include "wrap_Body.h"

namespace love
{
namespace physics
{
namespace box2d
{

int Physics::meter = Physics::DEFAULT_METER;

const char *Physics::getName() const
{
	return "love.physics.box2d";
}

World *Physics::newWorld(float gx, float gy, bool sleep)
{
	return new World(b2Vec2(gx, gy), sleep);
}

Body *Physics::newBody(World *world, float x, float y, Body::Type type)
{
	return new Body(world, b2Vec2(x, y), type);
}

Body *Physics::newBody(World *world, Body::Type type)
{
	return new Body(world, b2Vec2(0, 0), type);
}

CircleShape *Physics::newCircleShape(float radius)
{
	return newCircleShape(0, 0, radius);
}

CircleShape *Physics::newCircleShape(float x, float y, float radius)
{
	b2CircleShape *s = new b2CircleShape();
	s->m_p = Physics::scaleDown(b2Vec2(x, y));
	s->m_radius = Physics::scaleDown(radius);
	return new CircleShape(s);
}

PolygonShape *Physics::newRectangleShape(float w, float h)
{
	return newRectangleShape(0, 0, w, h, 0);
}

PolygonShape *Physics::newRectangleShape(float x, float y, float w, float h)
{
	return newRectangleShape(x, y, w, h, 0);
}

PolygonShape *Physics::newRectangleShape(float x, float y, float w, float h, float angle)
{
	b2PolygonShape *s = new b2PolygonShape();
	s->SetAsBox(Physics::scaleDown(w/2.0f), Physics::scaleDown(h/2.0f), Physics::scaleDown(b2Vec2(x, y)), angle);
	return new PolygonShape(s);
}

EdgeShape *Physics::newEdgeShape(float x1, float y1, float x2, float y2)
{
	b2EdgeShape *s = new b2EdgeShape();
	s->Set(Physics::scaleDown(b2Vec2(x1, y1)), Physics::scaleDown(b2Vec2(x2, y2)));
	return new EdgeShape(s);
}

int Physics::newPolygonShape(lua_State *L)
{
	int argc = lua_gettop(L);

	bool istable = lua_istable(L, 1);

	if (istable)
		argc = (int) luax_objlen(L, 1);

	if (argc % 2 != 0)
		return luaL_error(L, "Number of vertex components must be a multiple of two.");

	// 3 to 8 (b2_maxPolygonVertices) vertices
	int vcount = argc / 2;
	if (vcount < 3)
		return luaL_error(L, "Expected a minimum of 3 vertices, got %d.", vcount);
	else if (vcount > b2_maxPolygonVertices)
		return luaL_error(L, "Expected a maximum of %d vertices, got %d.", b2_maxPolygonVertices, vcount);

	b2Vec2 vecs[b2_maxPolygonVertices];

	if (istable)
	{
		for (int i = 0; i < vcount; i++)
		{
			lua_rawgeti(L, 1, 1 + i * 2);
			lua_rawgeti(L, 1, 2 + i * 2);
			float x = (float)luaL_checknumber(L, -2);
			float y = (float)luaL_checknumber(L, -1);
			vecs[i] = Physics::scaleDown(b2Vec2(x, y));
			lua_pop(L, 2);
		}
	}
	else
	{
		for (int i = 0; i < vcount; i++)
		{
			float x = (float)luaL_checknumber(L, 1 + i * 2);
			float y = (float)luaL_checknumber(L, 2 + i * 2);
			vecs[i] = Physics::scaleDown(b2Vec2(x, y));
		}
	}

	b2PolygonShape *s = new b2PolygonShape();

	try
	{
		s->Set(vecs, vcount);
	}
	catch (love::Exception &)
	{
		delete s;
		throw;
	}

	PolygonShape *p = new PolygonShape(s);
	luax_pushtype(L, PHYSICS_POLYGON_SHAPE_ID, p);
	p->release();
	return 1;
}

int Physics::newChainShape(lua_State *L)
{
	int argc = lua_gettop(L)-1; // first argument is looping

	bool istable = lua_istable(L, 2);

	if (istable)
		argc = (int) luax_objlen(L, 2);

	if (argc % 2 != 0)
		return luaL_error(L, "Number of vertex components must be a multiple of two.");

	int vcount = argc/2;
	bool loop = luax_toboolean(L, 1);
	b2Vec2 *vecs = new b2Vec2[vcount];

	if (istable)
	{
		for (int i = 0; i < vcount; i++)
		{
			lua_rawgeti(L, 2, 1 + i * 2);
			lua_rawgeti(L, 2, 2 + i * 2);
			float x = (float)lua_tonumber(L, -2);
			float y = (float)lua_tonumber(L, -1);
			vecs[i] = Physics::scaleDown(b2Vec2(x, y));
			lua_pop(L, 2);
		}
	}
	else
	{
		for (int i = 0; i < vcount; i++)
		{
			float x = (float)luaL_checknumber(L, 2 + i * 2);
			float y = (float)luaL_checknumber(L, 3 + i * 2);
			vecs[i] = Physics::scaleDown(b2Vec2(x, y));
		}
	}

	b2ChainShape *s = new b2ChainShape();

	try
	{
		if (loop)
			s->CreateLoop(vecs, vcount);
		else
			s->CreateChain(vecs, vcount);
	}
	catch (love::Exception &)
	{
		delete[] vecs;
		delete s;
		throw;
	}

	delete[] vecs;

	ChainShape *c = new ChainShape(s);
	luax_pushtype(L, PHYSICS_CHAIN_SHAPE_ID, c);
	c->release();
	return 1;
}

DistanceJoint *Physics::newDistanceJoint(Body *body1, Body *body2, float x1, float y1, float x2, float y2, bool collideConnected)
{
	return new DistanceJoint(body1, body2, x1, y1, x2, y2, collideConnected);
}

MouseJoint *Physics::newMouseJoint(Body *body, float x, float y)
{
	return new MouseJoint(body, x, y);
}

RevoluteJoint *Physics::newRevoluteJoint(Body *body1, Body *body2, float xA, float yA, float xB, float yB, bool collideConnected)
{
	return new RevoluteJoint(body1, body2, xA, yA, xB, yB, collideConnected);
}

RevoluteJoint *Physics::newRevoluteJoint(Body *body1, Body *body2, float xA, float yA, float xB, float yB, bool collideConnected, float referenceAngle)
{
	return new RevoluteJoint(body1, body2, xA, yA, xB, yB, collideConnected, referenceAngle);
}

PrismaticJoint *Physics::newPrismaticJoint(Body *body1, Body *body2, float xA, float yA, float xB, float yB, float ax, float ay, bool collideConnected)
{
	return new PrismaticJoint(body1, body2, xA, yA, xB, yB, ax, ay, collideConnected);
}

PrismaticJoint *Physics::newPrismaticJoint(Body *body1, Body *body2, float xA, float yA, float xB, float yB, float ax, float ay, bool collideConnected, float referenceAngle)
{
	return new PrismaticJoint(body1, body2, xA, yA, xB, yB, ax, ay, collideConnected, referenceAngle);
}

PulleyJoint *Physics::newPulleyJoint(Body *body1, Body *body2, b2Vec2 groundAnchor1, b2Vec2 groundAnchor2, b2Vec2 anchor1, b2Vec2 anchor2, float ratio, bool collideConnected)
{
	return new PulleyJoint(body1, body2, groundAnchor1, groundAnchor2, anchor1, anchor2, ratio, collideConnected);
}

GearJoint *Physics::newGearJoint(Joint *joint1, Joint *joint2, float ratio, bool collideConnected)
{
	return new GearJoint(joint1, joint2, ratio, collideConnected);
}

FrictionJoint *Physics::newFrictionJoint(Body *body1, Body *body2, float xA, float yA, float xB, float yB, bool collideConnected)
{
	return new FrictionJoint(body1, body2, xA, yA, xB, yB, collideConnected);
}

WeldJoint *Physics::newWeldJoint(Body *body1, Body *body2, float xA, float yA, float xB, float yB, bool collideConnected)
{
	return new WeldJoint(body1, body2, xA, yA, xB, yB, collideConnected);
}

WeldJoint *Physics::newWeldJoint(Body *body1, Body *body2, float xA, float yA, float xB, float yB, bool collideConnected, float referenceAngle)
{
	return new WeldJoint(body1, body2, xA, yA, xB, yB, collideConnected, referenceAngle);
}

WheelJoint *Physics::newWheelJoint(Body *body1, Body *body2, float xA, float yA, float xB, float yB, float ax, float ay, bool collideConnected)
{
	return new WheelJoint(body1, body2, xA, yA, xB, yB, ax, ay, collideConnected);
}

RopeJoint *Physics::newRopeJoint(Body *body1, Body *body2, float x1, float y1, float x2, float y2, float maxLength, bool collideConnected)
{
	return new RopeJoint(body1, body2, x1, y1, x2, y2, maxLength, collideConnected);
}

MotorJoint *Physics::newMotorJoint(Body *body1, Body *body2)
{
	return new MotorJoint(body1, body2);
}

MotorJoint *Physics::newMotorJoint(Body *body1, Body *body2, float correctionFactor, bool collideConnected)
{
	return new MotorJoint(body1, body2, correctionFactor, collideConnected);
}


Fixture *Physics::newFixture(Body *body, Shape *shape, float density)
{
	return new Fixture(body, shape, density);
}

int Physics::getDistance(lua_State *L)
{
	Fixture *fixtureA = luax_checktype<Fixture>(L, 1, PHYSICS_FIXTURE_ID);
	Fixture *fixtureB = luax_checktype<Fixture>(L, 2, PHYSICS_FIXTURE_ID);
	b2DistanceProxy pA, pB;
	b2DistanceInput i;
	b2DistanceOutput o;
	b2SimplexCache c;
	c.count = 0;

	luax_catchexcept(L, [&]() {
		pA.Set(fixtureA->fixture->GetShape(), 0);
		pB.Set(fixtureB->fixture->GetShape(), 0);
		i.proxyA = pA;
		i.proxyB = pB;
		i.transformA = fixtureA->fixture->GetBody()->GetTransform();
		i.transformB = fixtureB->fixture->GetBody()->GetTransform();
		i.useRadii = true;
		b2Distance(&o, &c, &i);
	});

	lua_pushnumber(L, Physics::scaleUp(o.distance));
	lua_pushnumber(L, Physics::scaleUp(o.pointA.x));
	lua_pushnumber(L, Physics::scaleUp(o.pointA.y));
	lua_pushnumber(L, Physics::scaleUp(o.pointB.x));
	lua_pushnumber(L, Physics::scaleUp(o.pointB.y));
	return 5;
}

void Physics::setMeter(int scale)
{
	if (scale < 1) throw love::Exception("Physics error: invalid meter");
	Physics::meter = scale;
}

int Physics::getMeter()
{
	return meter;
}

void Physics::scaleDown(float &x, float &y)
{
	x /= (float)meter;
	y /= (float)meter;
}

void Physics::scaleUp(float &x, float &y)
{
	x *= (float)meter;
	y *= (float)meter;
}

float Physics::scaleDown(float f)
{
	return f/(float)meter;
}

float Physics::scaleUp(float f)
{
	return f*(float)meter;
}

b2Vec2 Physics::scaleDown(const b2Vec2 &v)
{
	b2Vec2 t = v;
	scaleDown(t.x, t.y);
	return t;
}

b2Vec2 Physics::scaleUp(const b2Vec2 &v)
{
	b2Vec2 t = v;
	scaleUp(t.x, t.y);
	return t;
}

b2AABB Physics::scaleDown(const b2AABB &aabb)
{
	b2AABB t;
	t.lowerBound = scaleDown(aabb.lowerBound);
	t.upperBound = scaleDown(aabb.upperBound);
	return t;
}

b2AABB Physics::scaleUp(const b2AABB &aabb)
{
	b2AABB t;
	t.lowerBound = scaleUp(aabb.lowerBound);
	t.upperBound = scaleUp(aabb.upperBound);
	return t;
}

} // box2d
} // physics
} // love
