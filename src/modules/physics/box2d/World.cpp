/**
* Copyright (c) 2006-2012 LOVE Development Team
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

#include "Fixture.h"
#include "Shape.h"
#include "Contact.h"
#include "Physics.h"
#include <common/Memoizer.h>
#include <common/Reference.h>

namespace love
{
namespace physics
{
namespace box2d
{

	World::ContactCallback::ContactCallback()
		: ref(0)
	{
	}

	World::ContactCallback::~ContactCallback()
	{
		if (ref != 0)
			delete ref;
	}

	void World::ContactCallback::process(b2Contact* contact, const b2ContactImpulse* impulse)
	{
		// Process contacts.
		if (ref != 0)
		{
			lua_State * L = ref->getL();
			ref->push();

			// Push first fixture.
			{
				Fixture * a = (Fixture *)Memoizer::find(contact->GetFixtureA());
				if (a != 0)
				{
					a->retain();
					luax_newtype(L, "Fixture", PHYSICS_FIXTURE_T, (void*)a);
				}
				else
					throw love::Exception("A fixture has escaped Memoizer!");
			}

			// Push second userdata.
			{
				Fixture * b = (Fixture *)Memoizer::find(contact->GetFixtureB());
				if (b != 0)
				{
					b->retain();
					luax_newtype(L, "Fixture", PHYSICS_FIXTURE_T, (void*)b);
				}
				else
					throw love::Exception("A fixture has escaped Memoizer!");
			}

			Contact * c = new Contact(contact);

			luax_newtype(L, "Contact", (PHYSICS_CONTACT_T), (void*)c);

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
		: ref(0)
	{
	}

	World::ContactFilter::~ContactFilter()
	{
		if (ref != 0)
			delete ref;
	}

	bool World::ContactFilter::process(Fixture * a, Fixture * b)
	{
		if (ref != 0)
		{
			lua_State * L = ref->getL();
			ref->push();
			luax_newtype(L, "Fixture", PHYSICS_FIXTURE_T, (void*)a);
			luax_newtype(L, "Fixture", PHYSICS_FIXTURE_T, (void*)b);
			lua_call(L, 2, 1);
			return luax_toboolean(L, -1);
		}
		return true;
	}

	World::QueryCallback::QueryCallback()
		: ref(0)
	{
	}

	World::QueryCallback::~QueryCallback()
	{
		if (ref != 0)
			delete ref;
	}

	bool World::QueryCallback::ReportFixture(b2Fixture * fixture)
	{
		if (ref != 0)
		{
			lua_State * L = ref->getL();
			ref->push();
			Fixture * f = (Fixture *)Memoizer::find(fixture);
			if (!f)
				throw love::Exception("A fixture has escaped Memoizer!");
			f->retain();
			luax_newtype(L, "Fixture", PHYSICS_FIXTURE_T, (void*)f);
			lua_call(L, 1, 1);
			return luax_toboolean(L, -1);
		}
		return true;
	}

	World::RayCastCallback::RayCastCallback()
	: ref(0)
	{
	}

	World::RayCastCallback::~RayCastCallback()
	{
		if (ref != 0)
			delete ref;
	}

	float32 World::RayCastCallback::ReportFixture(b2Fixture * fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction)
	{
		if (ref != 0)
		{
			lua_State * L = ref->getL();
			ref->push();
			Fixture * f = (Fixture *)Memoizer::find(fixture);
			if (!f)
				throw love::Exception("A fixture has escaped Memoizer!");
			f->retain();
			luax_newtype(L, "Fixture", PHYSICS_FIXTURE_T, (void*)f);
			b2Vec2 scaledPoint = Physics::scaleUp(point);
			lua_pushnumber(L, scaledPoint.x);
			lua_pushnumber(L, scaledPoint.y);
			b2Vec2 scaledNormal = Physics::scaleUp(normal);
			lua_pushnumber(L, scaledNormal.x);
			lua_pushnumber(L, scaledNormal.y);
			lua_pushnumber(L, fraction);
			lua_call(L, 6, 1);
			return (float32)luaL_checknumber(L, -1);
		}
		return 0;
	}

	void World::SayGoodbye(b2Fixture* fixture)
	{
		Fixture * f = (Fixture *)Memoizer::find(fixture);
		// Hint implicit destruction with true.
		if (f) f->destroy(true);
	}

	void World::SayGoodbye(b2Joint* joint)
	{
		Joint * j = (Joint *)Memoizer::find(joint);
		// Hint implicit destruction with true.
		if (j) j->destroyJoint(true);
	}

	World::World()
		: world(NULL), destructWorld(false)
	{
		world = new b2World(b2Vec2(0,0));
		this->retain(); // The Box2D world holds a reference to this World.
		world->SetAllowSleeping(true);
		world->SetContactListener(this);
		world->SetDestructionListener(this);
		b2BodyDef def;
		groundBody = world->CreateBody(&def);
		Memoizer::add(world, this);
	}

	World::World(b2Vec2 gravity, bool sleep)
		: world(NULL), destructWorld(false)
	{
		world = new b2World(Physics::scaleDown(gravity));
		// The Box2D world holds a reference to this World.
		this->retain();
		world->SetAllowSleeping(sleep);
		world->SetContactListener(this);
		world->SetDestructionListener(this);
		b2BodyDef def;
		groundBody = world->CreateBody(&def);
		Memoizer::add(world, this);
	}

	World::~World()
	{
	}

	void World::update(float dt)
	{
		world->Step(dt, 8, 6);

		// Destroy all objects marked during the time step.
		if (destructWorld)
		{
			destroy();
			return;
		}

		for (std::vector<Body*>::iterator i = destructBodies.begin(); i < destructBodies.end(); i++)
		{
			Body * b = *i;
			if (b->body != 0) b->destroy();
			// Release for reference in vector.
			b->release();
		}
		for (std::vector<Fixture*>::iterator i = destructFixtures.begin(); i < destructFixtures.end(); i++)
		{
			Fixture * f = *i;
			if (f->isValid()) f->destroy();
			// Release for reference in vector.
			f->release();
		}
		for (std::vector<Joint*>::iterator i = destructJoints.begin(); i < destructJoints.end(); i++)
		{
			Joint * j = *i;
			if (j->isValid()) j->destroyJoint();
			// Release for reference in vector.
			j->release();
		}
		destructBodies.clear();
		destructFixtures.clear();
		destructJoints.clear();
	}

	void World::BeginContact(b2Contact* contact)
	{
		begin.process(contact);
	}

	void World::EndContact(b2Contact* contact)
	{
		end.process(contact);
	}

	void World::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
	{
		B2_NOT_USED(oldManifold); // not sure what to do with this
		presolve.process(contact);
	}

	void World::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
	{
		postsolve.process(contact, impulse);
	}

	bool World::ShouldCollide(b2Fixture * fixtureA, b2Fixture * fixtureB)
	{
		// Fixtures should be memoized, if we created them
		Fixture * a = (Fixture *)Memoizer::find(fixtureA);
		if (!a)
			throw love::Exception("A fixture has escaped Memoizer!");
		a->retain();
		Fixture * b = (Fixture *)Memoizer::find(fixtureB);
		if (!b)
			throw love::Exception("A fixture has escaped Memoizer!");
		b->retain();
		return filter.process(a, b);
	}

	bool World::isValid() const
	{
		return world != 0;
	}

	int World::setCallbacks(lua_State * L)
	{
		int n = lua_gettop(L);
		luax_assert_argc(L, 1, 4);

		switch(n)
		{
		case 4:
			if (postsolve.ref)
				delete postsolve.ref;
			postsolve.ref = luax_refif(L, LUA_TFUNCTION);
		case 3:
			if (presolve.ref)
				delete presolve.ref;
			presolve.ref = luax_refif(L, LUA_TFUNCTION);
		case 2:
			if (end.ref)
				delete end.ref;
			end.ref = luax_refif(L, LUA_TFUNCTION);
		case 1:
			if (begin.ref)
				delete begin.ref;
			begin.ref = luax_refif(L, LUA_TFUNCTION);
		}

		return 0;
	}

	int World::getCallbacks(lua_State * L)
	{
		begin.ref ? begin.ref->push() : lua_pushnil(L);
		end.ref ? end.ref->push() : lua_pushnil(L);
		presolve.ref ? presolve.ref->push() : lua_pushnil(L);
		postsolve.ref ? postsolve.ref->push() : lua_pushnil(L);
		return 4;
	}

	int World::setContactFilter(lua_State * L)
	{
		luax_assert_argc(L, 1);
		if (filter.ref)
			delete filter.ref;
		filter.ref = luax_refif(L, LUA_TFUNCTION);
		return 0;
	}

	int World::getContactFilter(lua_State * L)
	{
		filter.ref ? filter.ref->push() : lua_pushnil(L);
		return 1;
	}

	void World::setGravity(float x, float y)
	{
		world->SetGravity(Physics::scaleDown(b2Vec2(x, y)));
	}

	int World::getGravity(lua_State * L)
	{
		b2Vec2 v = Physics::scaleUp(world->GetGravity());
		lua_pushnumber(L, v.x);
		lua_pushnumber(L, v.y);
		return 2;
	}

	void World::setAllowSleeping(bool allow)
	{
		world->SetAllowSleeping(allow);
	}

	bool World::getAllowSleeping() const
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

	int World::getBodyList(lua_State * L) const
	{
		lua_newtable(L);
		b2Body * b = world->GetBodyList();
		int i = 1;
		do {
			if (!b)
				break;
			if (b == groundBody)
				continue;
			Body * body = (Body *)Memoizer::find(b);
			if (!body)
				throw love::Exception("A body has escaped Memoizer!");
			body->retain();
			luax_newtype(L, "Body", PHYSICS_BODY_T, (void*)body);
			lua_rawseti(L, -2, i);
			i++;
		} while ((b = b->GetNext()));
		return 1;
	}

	int World::getJointList(lua_State * L) const
	{
		lua_newtable(L);
		b2Joint * j = world->GetJointList();
		int i = 1;
		do {
			if (!j) break;
			Joint * joint = (Joint *)Memoizer::find(j);
			if (!joint) throw love::Exception("A joint has escaped Memoizer!");
			joint->retain();
			luax_newtype(L, "Joint", PHYSICS_JOINT_T, (void*)joint);
			lua_rawseti(L, -2, i);
			i++;
		} while ((j = j->GetNext()));
		return 1;
	}

	int World::getContactList(lua_State * L) const
	{
		lua_newtable(L);
		b2Contact * c = world->GetContactList();
		int i = 1;
		do {
			if (!c) break;
			Contact * contact = (Contact *)Memoizer::find(c);
			if (!contact) throw love::Exception("A contact has escaped Memoizer!");
			contact->retain();
			luax_newtype(L, "Contact", PHYSICS_CONTACT_T, (void*)contact);
			lua_rawseti(L, -2, i);
			i++;
		} while ((c = c->GetNext()));
		return 1;
	}

	b2Body * World::getGroundBody() const
	{
		return groundBody;
	}

	int World::queryBoundingBox(lua_State * L)
	{
		luax_assert_argc(L, 5);
		b2AABB box;
		float lx = (float)luaL_checknumber(L, 1);
		float ly = (float)luaL_checknumber(L, 2);
		float ux = (float)luaL_checknumber(L, 3);
		float uy = (float)luaL_checknumber(L, 4);
		box.lowerBound = Physics::scaleDown(b2Vec2(lx, ly));
		box.upperBound = Physics::scaleDown(b2Vec2(ux, uy));
		if (query.ref) delete query.ref;
		query.ref = luax_refif(L, LUA_TFUNCTION);
		world->QueryAABB(&query, box);
		return 0;
	}

	int World::rayCast(lua_State * L)
	{
		luax_assert_argc(L, 5);
		float x1 = (float)luaL_checknumber(L, 1);
		float y1 = (float)luaL_checknumber(L, 2);
		float x2 = (float)luaL_checknumber(L, 3);
		float y2 = (float)luaL_checknumber(L, 4);
		b2Vec2 v1 = Physics::scaleDown(b2Vec2(x1, y1));
		b2Vec2 v2 = Physics::scaleDown(b2Vec2(x2, y2));
		if (raycast.ref)
			delete raycast.ref;
		raycast.ref = luax_refif(L, LUA_TFUNCTION);
		world->RayCast(&raycast, v1, v2);
		return 0;
	}

	void World::destroy()
	{
		if (world->IsLocked())
		{
			destructWorld = true;
			return;
		}

		// Cleaning up the world.
		b2Body * b = world->GetBodyList();
		while (b)
		{
			b2Body * t = b;
			b = b->GetNext();
			if (t == groundBody)
				continue;
			Body * body = (Body *)Memoizer::find(t);
			if (!body)
				throw love::Exception("A body has escaped Memoizer!");
			body->destroy();
		}

		world->DestroyBody(groundBody);
		Memoizer::remove(world);
		delete world;
		world = 0;

		// Box2D world destroyed. Release its reference.
		this->release();
	}

} // box2d
} // physics
} // love
