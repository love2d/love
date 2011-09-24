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

				// Push first userdata.
				{
					fixtureudata * d = (fixtureudata *)(contacts[i]->contact->GetFixtureA()->GetUserData());
					if(d->ref != 0)
						d->ref->push();
					else
						lua_pushnil(L);
				}

				// Push second userdata.
				{
					fixtureudata * d = (fixtureudata *)(contacts[i]->contact->GetFixtureB()->GetUserData());
					if(d->ref != 0)
						d->ref->push();
					else
						lua_pushnil(L);
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

	World::World()
		: world(NULL)
	{
		world = new b2World(b2Vec2(0,0));
		world->SetAllowSleeping(true);
		world->SetContactListener(this);
		b2BodyDef def;
		groundBody = world->CreateBody(&def);
	}

	World::World(b2Vec2 gravity, bool sleep)
		: world(NULL)
	{
		world = new b2World(Physics::scaleDown(gravity));
		world->SetAllowSleeping(sleep);
		world->SetContactListener(this);
		b2BodyDef def;
		groundBody = world->CreateBody(&def);
	}

	World::~World()
	{
		world->DestroyBody(groundBody);
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
	
	b2Body * World::getGroundBody()
	{
		return groundBody;
	}

} // box2d
} // physics
} // love
