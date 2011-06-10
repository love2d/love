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

#include "Shape.h"
#include "Contact.h"
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

	void World::ContactCallback::add(World * world, const b2ContactPoint* point)
	{
		/**
		* We must copy contacts, since we're not allowed to process
		* them inside this function. Removing bodies in this function
		* pretty much guarantees segfault. ^^
		**/

		if(ref != 0)
			contacts.push_back(new Contact(world, point));
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
					shapeudata * d = (shapeudata *)(contacts[i]->point.shape1->GetUserData());
					if(d->ref != 0)
						d->ref->push();
					else
						lua_pushnil(L);
				}

				// Push first userdata.
				{
					shapeudata * d = (shapeudata *)(contacts[i]->point.shape2->GetUserData());
					if(d->ref != 0)
						d->ref->push();
					else
						lua_pushnil(L);
				}

				luax_newtype(L, "Contact", (PHYSICS_CONTACT_T), (void*)contacts[i], false);
				lua_call(L, 3, 0);
			}

			// Clear contacts.
			for(int i = 0;i<(int)contacts.size();i++)
				delete contacts[i];
			contacts.clear();
		}

	}

	World::World(b2AABB aabb)
		: world(NULL), meter(DEFAULT_METER)
	{
		world = new b2World(scaleDown(aabb), b2Vec2(0,0), true);
		world->SetContactListener(this);
	}

	World::World(b2AABB aabb, b2Vec2 gravity, bool sleep, int meter)
		: world(NULL), meter(meter)
	{
		world = new b2World(scaleDown(aabb), scaleDown(gravity), sleep);
		world->SetContactListener(this);
	}

	World::~World()
	{
		delete world;
	}

	void World::update(float dt)
	{
		world->Step(dt, 10);


		add.process();
		persist.process();
		remove.process();
		result.process();
		
		// Really destroy all marked bodies.
		for (std::vector<Body*>::iterator i = destructBodies.begin(); i < destructBodies.end(); i++)
		{
			Body * b = *i;
			b->release();
		}
		destructBodies.clear();
	}

	void World::Add(const b2ContactPoint* point)
	{
		add.add(this, point);
	}

	void World::Persist(const b2ContactPoint* point)
	{
		persist.add(this, point);
	}

	void World::Remove(const b2ContactPoint* point)
	{
		remove.add(this, point);
	}

	void World::Result(const b2ContactPoint* point)
	{
		result.add(this, point);
	}

	int World::setCallbacks(lua_State * L)
	{
		int n = lua_gettop(L);
		luax_assert_argc(L, 1, 4);

		switch(n)
		{
		case 4:
			if (result.ref) delete result.ref;
			result.ref = luax_refif(L, LUA_TFUNCTION);
		case 3:
			if (remove.ref) delete remove.ref;
			remove.ref = luax_refif(L, LUA_TFUNCTION);
		case 2:
			if (persist.ref) delete persist.ref;
			persist.ref = luax_refif(L, LUA_TFUNCTION);
		case 1:
			if (add.ref) delete add.ref;
			add.ref = luax_refif(L, LUA_TFUNCTION);
		}

		return 0;
	}

	int World::getCallbacks(lua_State * L)
	{
		add.ref ? add.ref->push() : lua_pushnil(L);
		persist.ref ? persist.ref->push() : lua_pushnil(L);
		remove.ref ? remove.ref->push() : lua_pushnil(L);
		result.ref ? result.ref->push() : lua_pushnil(L);
		return lua_gettop(L);
	}

	void World::setGravity(float x, float y)
	{
		world->SetGravity(scaleDown(b2Vec2(x, y)));
	}

	int World::getGravity(lua_State * L)
	{
		b2Vec2 v = scaleUp(world->m_gravity);
		lua_pushnumber(L, v.x);
		lua_pushnumber(L, v.y);
		return 2;
	}

	void World::setAllowSleep(bool allow)
	{
		world->m_allowSleep = allow;
	}

	bool World::isAllowSleep() const
	{
		return world->m_allowSleep;
	}

	int World::getBodyCount()
	{
		return world->GetBodyCount();
	}

	int World::getJointCount()
	{
		return world->GetJointCount();
	}

	void World::setMeter(int meter)
	{
		this->meter = meter;
	}

	int World::getMeter() const
	{
		return this->meter;
	}

	void World::scaleDown(float & x, float & y)
	{
		x /= (float)meter;
		y /= (float)meter;
	}

	void World::scaleUp(float & x, float & y)
	{
		x *= (float)meter;
		y *= (float)meter;
	}

	float World::scaleDown(float f)
	{
		return f/(float)meter;
	}

	float World::scaleUp(float f)
	{
		return f*(float)meter;
	}

	b2Vec2 World::scaleDown(const b2Vec2 & v)
	{
		b2Vec2 t = v;
		scaleDown(t.x, t.y);
		return t;
	}

	b2Vec2 World::scaleUp(const b2Vec2 & v)
	{
		b2Vec2 t = v;
		scaleUp(t.x, t.y);
		return t;
	}

	b2AABB World::scaleDown(const b2AABB & aabb)
	{
		b2AABB t;
		t.lowerBound = scaleDown(aabb.lowerBound);
		t.upperBound = scaleDown(aabb.upperBound);
		return t;
	}

	b2AABB World::scaleUp(const b2AABB & aabb)
	{
		b2AABB t;
		t.lowerBound = scaleUp(aabb.lowerBound);
		t.upperBound = scaleUp(aabb.upperBound);
		return t;
	}
	
	void World::destroyBody(Body * b)
	{
		destructBodies.push_back(b);
	}

} // box2d
} // physics
} // love
