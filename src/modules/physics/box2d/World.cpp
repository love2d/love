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
		if(ref != 0)
			delete ref;
	}

	void World::ContactCallback::add(b2Contact* contact)
	{
		/**
		* We must copy contacts, since we're not allowed to process
		* them inside this function. Removing bodies in this function
		* pretty much guarantees segfault. ^^
		**/

		if(ref != 0)
			contacts.push_back(new Contact(contact));
	}
	
	void World::ContactCallback::add(b2Contact* contact, const b2ContactImpulse* impulse)
	{
		if(ref != 0) {
			contacts.push_back(new Contact(contact));
			impulses.push_back(impulse);
		}
	}

	void World::ContactCallback::process()
	{
		// Process contacts.
		if(ref != 0)
		{
			lua_State * L = ref->getL();
			for(int i = 0;i<(int)contacts.size();i++)
			{
				// Push the function.
				ref->push();

				// Push first fixture.
				{
					Fixture * a = (Fixture *)Memoizer::find(contacts[i]->contact->GetFixtureA());
					if(a != 0)
						luax_newtype(L, "Fixture", PHYSICS_FIXTURE_T, (void*)a);
					else
						throw love::Exception("A fixture has escaped Memoizer!");
				}

				// Push second userdata.
				{
					Fixture * b = (Fixture *)Memoizer::find(contacts[i]->contact->GetFixtureB());
					if(b != 0)
						luax_newtype(L, "Fixture", PHYSICS_FIXTURE_T, (void*)b);
					else
						throw love::Exception("A fixture has escaped Memoizer!");
				}

				luax_newtype(L, "Contact", (PHYSICS_CONTACT_T), (void*)contacts[i], false);
				
				int args = 3;
				if ((int)impulses.size() > i) {
					const b2ContactImpulse * impulse = impulses[i];
					for (int c = 0; c < impulse->count; c++) {
						lua_pushnumber(L, Physics::scaleUp(impulse->normalImpulses[c]));
						lua_pushnumber(L, Physics::scaleUp(impulse->tangentImpulses[c]));
						args += 2;
					}
				}
				lua_call(L, args, 0);
			}

			// Clear contacts.
			for(int i = 0;i<(int)contacts.size();i++)
				delete contacts[i];
			contacts.clear();
			// Clear impulses.
			impulses.clear();
		}

	}
	
	World::ContactFilter::ContactFilter()
	: ref(0)
	{
	}
	
	World::ContactFilter::~ContactFilter()
	{
		if(ref != 0)
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
		if(ref != 0)
			delete ref;
	}
	
	bool World::QueryCallback::ReportFixture(b2Fixture * fixture)
	{
		if (ref != 0)
		{
			lua_State * L = ref->getL();
			ref->push();
			Fixture * f = (Fixture *)Memoizer::find(fixture);
			if (!f) throw love::Exception("A fixture has escaped Memoizer!");
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
		if(ref != 0)
			delete ref;
	}
	
	float32 World::RayCastCallback::ReportFixture(b2Fixture * fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction)
	{
		if (ref != 0)
		{
			lua_State * L = ref->getL();
			ref->push();
			Fixture * f = (Fixture *)Memoizer::find(fixture);
			if (!f) throw love::Exception("A fixture has escaped Memoizer!");
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

	World::World()
		: world(NULL)
	{
		world = new b2World(b2Vec2(0,0));
		world->SetAllowSleeping(true);
		world->SetContactListener(this);
		b2BodyDef def;
		groundBody = world->CreateBody(&def);
		Memoizer::add(world, this);
	}

	World::World(b2Vec2 gravity, bool sleep)
		: world(NULL)
	{
		world = new b2World(Physics::scaleDown(gravity));
		world->SetAllowSleeping(sleep);
		world->SetContactListener(this);
		b2BodyDef def;
		groundBody = world->CreateBody(&def);
		Memoizer::add(world, this);
	}

	World::~World()
	{
		world->DestroyBody(groundBody);
		Memoizer::remove(world);
		delete world;
	}

	void World::update(float dt)
	{
		world->Step(dt, 8, 6);

		begin.process();
		end.process();
		presolve.process();
		postsolve.process();
	}

	void World::BeginContact(b2Contact* contact)
	{
		begin.add(contact);
	}

	void World::EndContact(b2Contact* contact)
	{
		end.add(contact);
	}

	void World::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
	{
		B2_NOT_USED(oldManifold); // not sure what to do with this
		presolve.add(contact);
	}

	void World::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
	{
		postsolve.add(contact, impulse);
	}
	
	bool World::ShouldCollide(b2Fixture * fixtureA, b2Fixture * fixtureB)
	{
		// Fixtures should be memoized, if we created them
		Fixture * a = (Fixture *)Memoizer::find(fixtureA);
		if (!a) throw love::Exception("A fixture has escaped Memoizer!");
		Fixture * b = (Fixture *)Memoizer::find(fixtureB);
		if (!b) throw love::Exception("A fixture has escaped Memoizer!");
		return filter.process(a, b);
	}

	int World::setCallbacks(lua_State * L)
	{
		int n = lua_gettop(L);
		luax_assert_argc(L, 1, 4);

		switch(n)
		{
		case 4:
			if (postsolve.ref) delete postsolve.ref;
			postsolve.ref = luax_refif(L, LUA_TFUNCTION);
		case 3:
			if (presolve.ref) delete presolve.ref;
			presolve.ref = luax_refif(L, LUA_TFUNCTION);
		case 2:
			if (end.ref) delete end.ref;
			end.ref = luax_refif(L, LUA_TFUNCTION);
		case 1:
			if (begin.ref) delete begin.ref;
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
		return lua_gettop(L);
	}
	
	int World::setContactFilter(lua_State * L)
	{
		luax_assert_argc(L, 1);
		if (filter.ref) delete filter.ref;
		filter.ref = luax_refif(L, LUA_TFUNCTION);
		return 0;
	}
	
	int World::getContactFilter(lua_State * L)
	{
		filter.ref ? filter.ref->push() : lua_pushnil(L);
		return lua_gettop(L);
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

	int World::getBodyCount()
	{
		return world->GetBodyCount();
	}

	int World::getJointCount()
	{
		return world->GetJointCount();
	}
	
	int World::getContactCount()
	{
		return world->GetContactCount();
	}
	
	int World::getBodyList(lua_State * L)
	{
		lua_newtable(L);
		b2Body * b = world->GetBodyList();
		for (int i = 1; i <= world->GetBodyCount(); i++) {
			if (!b) return 1;
			Body * body = (Body *)Memoizer::find(b);
			if (!body) throw love::Exception("A body has escaped Memoizer!");
			luax_newtype(L, "Body", PHYSICS_BODY_T, (void*)body);
			lua_rawseti(L, -1, i);
			b = b->GetNext();
		}
		return 1;
	}
	
	int World::getJointList(lua_State * L)
	{
		lua_newtable(L);
		b2Joint * j = world->GetJointList();
		for (int i = 1; i <= world->GetJointCount(); i++) {
			if (!j) return 1;
			Joint * joint = (Joint *)Memoizer::find(j);
			if (!joint) throw love::Exception("A joint has escaped Memoizer!");
			luax_newtype(L, "Joint", PHYSICS_JOINT_T, (void*)joint);
			lua_rawseti(L, -1, i);
			j = j->GetNext();
		}
		return 1;
	}
	
	int World::getContactList(lua_State * L)
	{
		lua_newtable(L);
		b2Contact * c = world->GetContactList();
		for (int i = 1; i <= world->GetContactCount(); i++) {
			if (!c) return 1;
			Contact	* contact = (Contact *)Memoizer::find(c);
			if (!contact) throw love::Exception("A contact has escaped Memoizer!");
			luax_newtype(L, "Contact", PHYSICS_CONTACT_T, (void*)contact);
			lua_rawseti(L, -1, i);
			c = c->GetNext();
		}
		return 1;
	}
	
	b2Body * World::getGroundBody()
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
		if (raycast.ref) delete raycast.ref;
		raycast.ref = luax_refif(L, LUA_TFUNCTION);
		world->RayCast(&raycast, v1, v2);
		return 0;
	}

} // box2d
} // physics
} // love
