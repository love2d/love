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

#include "World.h"

#include "Shape.h"
#include "Contact.h"
#include "Physics.h"
#include "common/Reference.h"

// Needed for World::getJoints. It should be moved to wrapper code...
#include "wrap_Joint.h"
#include "wrap_Shape.h"

namespace love
{
namespace physics
{
namespace box2d
{

love::Type World::type("World", &Object::type);

World::ContactCallback::ContactCallback(World *world)
	: ref(nullptr)
	, L(nullptr)
	, world(world)
{
}

World::ContactCallback::~ContactCallback()
{
	if (ref != nullptr)
		delete ref;
}

void World::ContactCallback::process(b2Contact *contact, const b2ContactImpulse *impulse)
{
	// Process contacts.
	if (ref != nullptr && L != nullptr)
	{
		ref->push(L);

		// Push first shape.
		{
			Shape *a = (Shape *)(contact->GetFixtureA()->GetUserData().pointer);
			if (a != nullptr)
				luax_pushshape(L, a);
			else
				throw love::Exception("A Shape has escaped Memoizer!");
		}

		// Push second shape.
		{
			Shape *b = (Shape *)(contact->GetFixtureB()->GetUserData().pointer);
			if (b != nullptr)
				luax_pushshape(L, b);
			else
				throw love::Exception("A Shape has escaped Memoizer!");
		}

		Contact *cobj = (Contact *)world->findObject(contact);
		if (!cobj)
			cobj = new Contact(world, contact);
		else
			cobj->retain();

		luax_pushtype(L, cobj);
		cobj->release();

		int args = 3;
		if (impulse)
		{
			for (int c = 0; c < impulse->count; c++)
			{
				lua_pushnumber(L, Physics::scaleUp(impulse->normalImpulses[c]));
				lua_pushnumber(L, Physics::scaleUp(impulse->tangentImpulses[c]));
				args += 2;
			}
		}
		lua_call(L, args, 0);
	}

}

World::ContactFilter::ContactFilter()
	: ref(nullptr)
	, L(nullptr)
{
}

World::ContactFilter::~ContactFilter()
{
	if (ref != nullptr)
		delete ref;
}

bool World::ContactFilter::process(Shape *a, Shape *b)
{
	if (ref != nullptr && L != nullptr)
	{
		ref->push(L);
		luax_pushshape(L, a);
		luax_pushshape(L, b);
		lua_call(L, 2, 1);
		return luax_toboolean(L, -1);
	}

	return true;
}

World::QueryCallback::QueryCallback(lua_State *L, int idx)
	: L(L)
	, funcidx(idx)
{
	luaL_checktype(L, funcidx, LUA_TFUNCTION);
	userargs = lua_gettop(L) - funcidx;
}

World::QueryCallback::~QueryCallback()
{
}

bool World::QueryCallback::ReportFixture(b2Fixture *fixture)
{
	if (L != nullptr)
	{
		lua_pushvalue(L, funcidx);
		Shape *f = (Shape *)(fixture->GetUserData().pointer);
		if (!f)
			throw love::Exception("A Shape has escaped Memoizer!");
		luax_pushshape(L, f);
		for (int i = 1; i <= userargs; i++)
			lua_pushvalue(L, funcidx + i);
		lua_call(L, 1 + userargs, 1);
		bool cont = luax_toboolean(L, -1);
		lua_pop(L, 1);
		return cont;
	}

	return true;
}

World::CollectCallback::CollectCallback(uint16 categoryMask, lua_State *L)
	: categoryMask(categoryMask)
	, L(L)
{
	lua_newtable(L);
}

World::CollectCallback::~CollectCallback()
{
}

bool World::CollectCallback::ReportFixture(b2Fixture *f)
{
	if (categoryMask != 0xFFFF && (categoryMask & f->GetFilterData().categoryBits) == 0)
		return true;

	Shape *shape = (Shape *)(f->GetUserData().pointer);
	if (!shape)
		throw love::Exception("A Shape has escaped Memoizer!");
	luax_pushshape(L, shape);
	lua_rawseti(L, -2, i);
	i++;
	return true;
}

World::RayCastCallback::RayCastCallback(lua_State *L, int idx)
	: L(L)
	, funcidx(idx)
{
	luaL_checktype(L, funcidx, LUA_TFUNCTION);
	userargs = lua_gettop(L) - funcidx;
}

World::RayCastCallback::~RayCastCallback()
{
}

float World::RayCastCallback::ReportFixture(b2Fixture *fixture, const b2Vec2 &point, const b2Vec2 &normal, float fraction)
{
	if (L != nullptr)
	{
		lua_pushvalue(L, funcidx);
		Shape *f = (Shape *)(fixture->GetUserData().pointer);
		if (!f)
			throw love::Exception("A Shape has escaped Memoizer!");
		luax_pushshape(L, f);
		b2Vec2 scaledPoint = Physics::scaleUp(point);
		lua_pushnumber(L, scaledPoint.x);
		lua_pushnumber(L, scaledPoint.y);
		lua_pushnumber(L, normal.x);
		lua_pushnumber(L, normal.y);
		lua_pushnumber(L, fraction);
		for (int i = 1; i <= userargs; i++)
			lua_pushvalue(L, funcidx + i);
		lua_call(L, 6 + userargs, 1);
		if (!lua_isnumber(L, -1))
			luaL_error(L, "Raycast callback didn't return a number!");
		float fraction = (float) lua_tonumber(L, -1);
		lua_pop(L, 1);
		return fraction;
	}

	return 0;
}

World::RayCastOneCallback::RayCastOneCallback(uint16 categoryMask, bool any)
	: hitFixture(nullptr)
	, hitPoint()
	, hitNormal()
	, hitFraction(1.0f)
	, categoryMask(categoryMask)
	, any(any)
{
}

float World::RayCastOneCallback::ReportFixture(b2Fixture *fixture, const b2Vec2 &point, const b2Vec2 &normal, float fraction)
{
	if (categoryMask != 0xFFFF && (categoryMask & fixture->GetFilterData().categoryBits) == 0)
		return -1;

	hitFixture = fixture;
	hitPoint = point;
	hitNormal = normal;
	hitFraction = fraction;

	// Returning the fraction makes sure it doesn't process anything farther away in subsequent iterations.
	return any ? 0 : fraction;
}

void World::SayGoodbye(b2Fixture *fixture)
{
	Shape *s = (Shape *)(fixture->GetUserData().pointer);
	// Hint implicit destruction with true.
	if (s) s->destroy(true);
}

void World::SayGoodbye(b2Joint *joint)
{
	Joint *j = (Joint *)(joint->GetUserData().pointer);
	// Hint implicit destruction with true.
	if (j) j->destroyJoint(true);
}

World::World()
	: world(nullptr)
	, destructWorld(false)
	, begin(this)
	, end(this)
	, presolve(this)
	, postsolve(this)
{
	world = new b2World(b2Vec2(0,0));
	world->SetAllowSleeping(true);
	world->SetContactListener(this);
	world->SetContactFilter(this);
	world->SetDestructionListener(this);
	b2BodyDef def;
	groundBody = world->CreateBody(&def);
	registerObject(world, this);
}

World::World(b2Vec2 gravity, bool sleep)
	: world(nullptr)
	, destructWorld(false)
	, begin(this)
	, end(this)
	, presolve(this)
	, postsolve(this)
{
	world = new b2World(Physics::scaleDown(gravity));
	world->SetAllowSleeping(sleep);
	world->SetContactListener(this);
	world->SetContactFilter(this);
	world->SetDestructionListener(this);
	b2BodyDef def;
	groundBody = world->CreateBody(&def);
	registerObject(world, this);
}

World::~World()
{
	destroy();
}

void World::update(float dt)
{
	update(dt, 8, 3); // Box2D 2.3's recommended defaults.
}

void World::update(float dt, int velocityIterations, int positionIterations)
{
	world->Step(dt, velocityIterations, positionIterations);

	// Destroy all objects marked during the time step.
	for (Body *b : destructBodies)
	{
		if (b->body != nullptr) b->destroy();
		// Release for reference in vector.
		b->release();
	}
	for (Shape *s : destructShapes)
	{
		if (s->isValid()) s->destroy();
		// Release for reference in vector.
		s->release();
	}
	for (Joint *j : destructJoints)
	{
		if (j->isValid()) j->destroyJoint();
		// Release for reference in vector.
		j->release();
	}
	destructBodies.clear();
	destructShapes.clear();
	destructJoints.clear();

	if (destructWorld)
		destroy();
}

void World::BeginContact(b2Contact *contact)
{
	begin.process(contact);
}

void World::EndContact(b2Contact *contact)
{
	end.process(contact);

	// Letting the Contact know that the b2Contact will be destroyed any second.
	Contact *c = (Contact *)findObject(contact);
	if (c != nullptr)
		c->invalidate();
}

void World::PreSolve(b2Contact *contact, const b2Manifold *oldManifold)
{
	B2_NOT_USED(oldManifold); // not sure what to do with this
	presolve.process(contact);
}

void World::PostSolve(b2Contact *contact, const b2ContactImpulse *impulse)
{
	postsolve.process(contact, impulse);
}

bool World::ShouldCollide(b2Fixture *fixtureA, b2Fixture *fixtureB)
{
	const b2Filter &filterA = fixtureA->GetFilterData();
	const b2Filter &filterB = fixtureB->GetFilterData();

	// From b2_world_callbacks.cpp
	// 0 is the default group index. If they're customized to be the same group,
	// allow collisions if it's positive and disallow if it's negative.
	if (filterA.groupIndex != 0 && filterA.groupIndex == filterB.groupIndex)
		return filterA.groupIndex > 0;

	if ((filterA.maskBits & filterB.categoryBits) == 0 || (filterA.categoryBits & filterB.maskBits) == 0)
		return false;

	// Shapes should be memoized, if we created them
	Shape *a = (Shape *)(fixtureA->GetUserData().pointer);
	Shape *b = (Shape *)(fixtureB->GetUserData().pointer);
	if (!a || !b)
		throw love::Exception("A Shape has escaped Memoizer!");

	return filter.process(a, b);
}

bool World::isValid() const
{
	return world != nullptr;
}

int World::setCallbacks(lua_State *L)
{
	int nargs = lua_gettop(L);

	for (int i = 1; i <= 4; i++)
	{
		if (!lua_isnoneornil(L, i))
			luaL_checktype(L, i, LUA_TFUNCTION);
	}

	delete begin.ref;
	begin.ref = nullptr;

	delete end.ref;
	end.ref = nullptr;

	delete presolve.ref;
	presolve.ref = nullptr;

	delete postsolve.ref;
	postsolve.ref = nullptr;

	if (nargs >= 1)
	{
		lua_pushvalue(L, 1);
		begin.ref = luax_refif(L, LUA_TFUNCTION);
		begin.L = L;
	}

	if (nargs >= 2)
	{
		lua_pushvalue(L, 2);
		end.ref = luax_refif(L, LUA_TFUNCTION);
		end.L = L;
	}

	if (nargs >= 3)
	{
		lua_pushvalue(L, 3);
		presolve.ref = luax_refif(L, LUA_TFUNCTION);
		presolve.L = L;
	}

	if (nargs >= 4)
	{
		lua_pushvalue(L, 4);
		postsolve.ref = luax_refif(L, LUA_TFUNCTION);
		postsolve.L = L;
	}

	return 0;
}

int World::getCallbacks(lua_State *L)
{
	begin.ref ? begin.ref->push(L) : lua_pushnil(L);
	end.ref ? end.ref->push(L) : lua_pushnil(L);
	presolve.ref ? presolve.ref->push(L) : lua_pushnil(L);
	postsolve.ref ? postsolve.ref->push(L) : lua_pushnil(L);
	return 4;
}

void World::setCallbacksL(lua_State *L)
{
	begin.L = end.L = presolve.L = postsolve.L = filter.L = L;
}

int World::setContactFilter(lua_State *L)
{
	if (!lua_isnoneornil(L, 1))
		luaL_checktype(L, 1, LUA_TFUNCTION);

	if (filter.ref)
		delete filter.ref;
	filter.ref = luax_refif(L, LUA_TFUNCTION);
	filter.L = L;
	return 0;
}

int World::getContactFilter(lua_State *L)
{
	filter.ref ? filter.ref->push(L) : lua_pushnil(L);
	return 1;
}

void World::setGravity(float x, float y)
{
	world->SetGravity(Physics::scaleDown(b2Vec2(x, y)));
}

int World::getGravity(lua_State *L)
{
	b2Vec2 v = Physics::scaleUp(world->GetGravity());
	lua_pushnumber(L, v.x);
	lua_pushnumber(L, v.y);
	return 2;
}

void World::translateOrigin(float x, float y)
{
	world->ShiftOrigin(Physics::scaleDown(b2Vec2(x, y)));
}

void World::setSleepingAllowed(bool allow)
{
	world->SetAllowSleeping(allow);
}

bool World::isSleepingAllowed() const
{
	return world->GetAllowSleeping();
}

bool World::isLocked() const
{
	return world->IsLocked();
}

int World::getBodyCount() const
{
	return world->GetBodyCount()-1; // ignore the ground body
}

int World::getJointCount() const
{
	return world->GetJointCount();
}

int World::getContactCount() const
{
	return world->GetContactCount();
}

int World::getBodies(lua_State *L) const
{
	lua_newtable(L);
	b2Body *b = world->GetBodyList();
	int i = 1;
	do
	{
		if (!b)
			break;
		if (b == groundBody)
			continue;
		Body *body = (Body *)(b->GetUserData().pointer);
		if (!body)
			throw love::Exception("A body has escaped Memoizer!");
		luax_pushtype(L, body);
		lua_rawseti(L, -2, i);
		i++;
	}
	while ((b = b->GetNext()));
	return 1;
}

int World::getJoints(lua_State *L) const
{
	lua_newtable(L);
	b2Joint *j = world->GetJointList();
	int i = 1;
	do
	{
		if (!j) break;
		Joint *joint = (Joint *)(j->GetUserData().pointer);
		if (!joint) throw love::Exception("A joint has escaped Memoizer!");
		luax_pushjoint(L, joint);
		lua_rawseti(L, -2, i);
		i++;
	}
	while ((j = j->GetNext()));
	return 1;
}

int World::getContacts(lua_State *L)
{
	lua_newtable(L);
	b2Contact *c = world->GetContactList();
	int i = 1;
	do
	{
		if (!c) break;
		Contact *contact = (Contact *)findObject(c);
		if (!contact)
			contact = new Contact(this, c);
		else
			contact->retain();
		luax_pushtype(L, contact);
		contact->release();
		lua_rawseti(L, -2, i);
		i++;
	}
	while ((c = c->GetNext()));
	return 1;
}

b2Body *World::getGroundBody() const
{
	return groundBody;
}

int World::queryShapesInArea(lua_State *L)
{
	b2AABB box;
	float lx = (float)luaL_checknumber(L, 1);
	float ly = (float)luaL_checknumber(L, 2);
	float ux = (float)luaL_checknumber(L, 3);
	float uy = (float)luaL_checknumber(L, 4);
	box.lowerBound = Physics::scaleDown(b2Vec2(lx, ly));
	box.upperBound = Physics::scaleDown(b2Vec2(ux, uy));
	luaL_checktype(L, 5, LUA_TFUNCTION);
	QueryCallback query(L, 5);
	world->QueryAABB(&query, box);
	return 0;
}

int World::getShapesInArea(lua_State *L)
{
	float lx = (float)luaL_checknumber(L, 1);
	float ly = (float)luaL_checknumber(L, 2);
	float ux = (float)luaL_checknumber(L, 3);
	float uy = (float)luaL_checknumber(L, 4);
	uint16 categoryMaskBits = (uint16)luaL_optinteger(L, 5, 0xFFFF);
	b2AABB box;
	box.lowerBound = Physics::scaleDown(b2Vec2(lx, ly));
	box.upperBound = Physics::scaleDown(b2Vec2(ux, uy));
	CollectCallback query(categoryMaskBits, L);
	world->QueryAABB(&query, box);
	return 1;
}

int World::rayCast(lua_State *L)
{
	float x1 = (float)luaL_checknumber(L, 1);
	float y1 = (float)luaL_checknumber(L, 2);
	float x2 = (float)luaL_checknumber(L, 3);
	float y2 = (float)luaL_checknumber(L, 4);
	b2Vec2 v1 = Physics::scaleDown(b2Vec2(x1, y1));
	b2Vec2 v2 = Physics::scaleDown(b2Vec2(x2, y2));
	luaL_checktype(L, 5, LUA_TFUNCTION);
	RayCastCallback raycast(L, 5);
	world->RayCast(&raycast, v1, v2);
	return 0;
}

int World::rayCastAny(lua_State *L)
{
	float x1 = (float)luaL_checknumber(L, 1);
	float y1 = (float)luaL_checknumber(L, 2);
	float x2 = (float)luaL_checknumber(L, 3);
	float y2 = (float)luaL_checknumber(L, 4);
	uint16 categoryMaskBits = (uint16)luaL_optinteger(L, 5, 0xFFFF);
	b2Vec2 v1 = Physics::scaleDown(b2Vec2(x1, y1));
	b2Vec2 v2 = Physics::scaleDown(b2Vec2(x2, y2));
	RayCastOneCallback raycast(categoryMaskBits, true);
	world->RayCast(&raycast, v1, v2);
	if (raycast.hitFixture)
	{
		Shape *f = (Shape *)(raycast.hitFixture->GetUserData().pointer);
		if (f == nullptr)
			return luaL_error(L, "A Shape has escaped Memoizer!");
		luax_pushshape(L, f);

		b2Vec2 hitPoint = Physics::scaleUp(raycast.hitPoint);
		lua_pushnumber(L, hitPoint.x);
		lua_pushnumber(L, hitPoint.y);
		lua_pushnumber(L, raycast.hitNormal.x);
		lua_pushnumber(L, raycast.hitNormal.y);
		lua_pushnumber(L, raycast.hitFraction);
		return 6;
	}
	return 0;
}

int World::rayCastClosest(lua_State *L)
{
	float x1 = (float)luaL_checknumber(L, 1);
	float y1 = (float)luaL_checknumber(L, 2);
	float x2 = (float)luaL_checknumber(L, 3);
	float y2 = (float)luaL_checknumber(L, 4);
	uint16 categoryMaskBits = (uint16)luaL_optinteger(L, 5, 0xFFFF);
	b2Vec2 v1 = Physics::scaleDown(b2Vec2(x1, y1));
	b2Vec2 v2 = Physics::scaleDown(b2Vec2(x2, y2));
	RayCastOneCallback raycast(categoryMaskBits, false);
	world->RayCast(&raycast, v1, v2);
	if (raycast.hitFixture)
	{
		Shape *f = (Shape *)(raycast.hitFixture->GetUserData().pointer);
		if (f == nullptr)
			return luaL_error(L, "A Shape has escaped Memoizer!");
		luax_pushshape(L, f);

		b2Vec2 hitPoint = Physics::scaleUp(raycast.hitPoint);
		lua_pushnumber(L, hitPoint.x);
		lua_pushnumber(L, hitPoint.y);
		lua_pushnumber(L, raycast.hitNormal.x);
		lua_pushnumber(L, raycast.hitNormal.y);
		lua_pushnumber(L, raycast.hitFraction);
		return 6;
	}
	return 0;
}

void World::destroy()
{
	if (world == nullptr)
		return;

	if (world->IsLocked())
	{
		destructWorld = true;
		return;
	}

	// Remove userdata reference to avoid it sticking around after GC
	if (begin.ref)     begin.ref->unref();
	if (end.ref)       end.ref->unref();
	if (presolve.ref)  presolve.ref->unref();
	if (postsolve.ref) postsolve.ref->unref();
	if (filter.ref)    filter.ref->unref();

	//disable callbacks
	begin.ref = end.ref = presolve.ref = postsolve.ref = filter.ref = nullptr;

	// Cleaning up the world.
	b2Body *b = world->GetBodyList();
	while (b)
	{
		b2Body *t = b;
		b = b->GetNext();
		if (t == groundBody)
			continue;
		Body *body = (Body *)(t->GetUserData().pointer);
		if (!body)
			throw love::Exception("A body has escaped Memoizer!");
		body->destroy();
	}

	world->DestroyBody(groundBody);
	unregisterObject(world);

	delete world;
	world = nullptr;
}

void World::registerObject(void *b2object, love::Object *object)
{
	box2dObjectMap[b2object] = object;
}

void World::unregisterObject(void *b2object)
{
	box2dObjectMap.erase(b2object);
}

love::Object *World::findObject(void *b2object) const
{
	auto it = box2dObjectMap.find(b2object);
	if (it != box2dObjectMap.end())
		return it->second;
	else
		return nullptr;
}

} // box2d
} // physics
} // love
