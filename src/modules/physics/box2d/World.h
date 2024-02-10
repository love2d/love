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

#ifndef LOVE_PHYSICS_BOX2D_WORLD_H
#define LOVE_PHYSICS_BOX2D_WORLD_H

// LOVE
#include "common/Object.h"
#include "common/runtime.h"
#include "common/Reference.h"

// STD
#include <vector>
#include <unordered_map>

// Box2D
#include <box2d/Box2D.h>

namespace love
{
namespace physics
{
namespace box2d
{

class Contact;
class Body;
class Shape;
class Joint;

/**
 * The World is the "God" container class,
 * which contains all Bodies and Joints. Shapes
 * are contained in their associated Body.
 *
 * Bodies in different worlds can obviously not
 * collide.
 *
 * The world also controls global parameters, like
 * gravity.
 **/
class World : public Object, public b2ContactListener, public b2ContactFilter, public b2DestructionListener
{
public:

	// Friends.
	friend class Joint;
	friend class DistanceJoint;
	friend class MouseJoint;
	friend class Body;
	friend class Shape;

	static love::Type type;

	class ContactCallback
	{
	public:
		Reference *ref;
		lua_State *L;
		World *world;
		ContactCallback(World *world);
		~ContactCallback();
		void process(b2Contact *contact, const b2ContactImpulse *impulse = NULL);
	};

	class ContactFilter
	{
	public:
		Reference *ref;
		lua_State *L;
		ContactFilter();
		~ContactFilter();
		bool process(Shape *a, Shape *b);
	};

	class QueryCallback : public b2QueryCallback
	{
	public:
		QueryCallback(World *world, lua_State *L, int idx);
		virtual ~QueryCallback();
		bool ReportFixture(b2Fixture *fixture) override;
	private:
		World *world;
		lua_State *L;
		int funcidx;
		int userargs;
	};

	class CollectCallback : public b2QueryCallback
	{
	public:
		CollectCallback(World *world, uint16 categoryMask, lua_State *L);
		virtual ~CollectCallback();
		bool ReportFixture(b2Fixture *fixture) override;
	private:
		World *world;
		uint16 categoryMask;
		lua_State *L;
		int i = 1;
	};

	class RayCastCallback : public b2RayCastCallback
	{
	public:
		RayCastCallback(World *world, lua_State *L, int idx);
		virtual ~RayCastCallback();
		float ReportFixture(b2Fixture *fixture, const b2Vec2 &point, const b2Vec2 &normal, float fraction) override;
	private:
		World *world;
		lua_State *L;
		int funcidx;
		int userargs;
	};

	class RayCastOneCallback : public b2RayCastCallback
	{
	public:
		RayCastOneCallback(uint16 categoryMask, bool any);
		virtual ~RayCastOneCallback() {};
		float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override;

		b2Fixture *hitFixture;
		b2Vec2 hitPoint;
		b2Vec2 hitNormal;
		float hitFraction;

	private:
		uint16 categoryMask;
		bool any;
	};

	/**
	 * Creates a new world.
	 **/
	World();

	/**
	 * Creates a new world with the given gravity
	 * and whether or not the bodies should sleep when appropriate.
	 * @param gravity The gravity of the World.
	 * @param sleep True if the bodies should be able to sleep,
	 * false otherwise.
	 **/
	World(b2Vec2 gravity, bool sleep);

	virtual ~World();

	/**
	 * Updates everything in the world one timestep.
	 * This is called update() and not step() to conform
	 * with all other objects in LOVE.
	 * @param dt The timestep.
	 **/
	void update(float dt);
	void update(float dt, int velocityIterations, int positionIterations);

	// From b2ContactListener
	void BeginContact(b2Contact *contact);
	void EndContact(b2Contact *contact);
	void PreSolve(b2Contact *contact, const b2Manifold *oldManifold);
	void PostSolve(b2Contact *contact, const b2ContactImpulse *impulse);

	// From b2ContactFilter
	bool ShouldCollide(b2Fixture *fixtureA, b2Fixture *fixtureB);

	// From b2DestructionListener
	void SayGoodbye(b2Fixture *fixture);
	void SayGoodbye(b2Joint *joint);

	/**
	 * Returns true if the Box2D world is alive.
	 **/
	bool isValid() const;

	/**
	 * Receives up to four Lua functions as arguments. Each function is
	 * collision callback for the four events (in order): begin, end,
	 * presolve and postsolve. The value "nil" is accepted if one or
	 * more events are uninteresting.
	 **/
	int setCallbacks(lua_State *L);

	/**
	 * Returns the functions previously set by setCallbacks.
	 **/
	int getCallbacks(lua_State *L);

	/**
	 * Updates the Lua thread/coroutine used when callbacks are executed in
	 * the update method. This should be called in the same Lua function which
	 * calls update().
	 **/
	void setCallbacksL(lua_State *L);

	/**
	 * Sets the ContactFilter callback.
	 **/
	int setContactFilter(lua_State *L);

	/**
	 * Gets the ContactFilter callback.
	 **/
	int getContactFilter(lua_State *L);

	/**
	 * Sets the current gravity of the World.
	 * @param x Gravity in the x-direction.
	 * @param y Gravity in the y-direction.
	 **/
	void setGravity(float x, float y);

	/**
	 * Gets the current gravity.
	 * @returns Gravity in the x-direction.
	 * @returns Gravity in the y-direction.
	 **/
	int getGravity(lua_State *L);

	/**
	 * Translate the world origin.
	 * @param x The new world origin's x-coordinate relative to the old origin.
	 * @param y The new world origin's y-coordinate relative to the old origin.
	 **/
	void translateOrigin(float x, float y);

	/**
	 * Sets whether this World allows sleep.
	 * @param allow True to allow, false to disallow.
	 **/
	void setSleepingAllowed(bool allow);

	/**
	 * Returns whether this World allows sleep.
	 * @return True if allowed, false if disallowed.
	 **/
	bool isSleepingAllowed() const;

	/**
	 * Returns whether this World is currently locked.
	 * If it's locked, it's in the middle of a timestep.
	 * @return Whether the World is locked.
	 **/
	bool isLocked() const;

	/**
	 * Get the current body count.
	 * @return The number of bodies.
	 **/
	int getBodyCount() const;

	/**
	 * Get the current joint count.
	 * @return The number of joints.
	 **/
	int getJointCount() const;

	/**
	 * Get the current contact count.
	 * @return The number of contacts.
	 **/
	int getContactCount() const;

	/**
	 * Get an array of all the Bodies in the World.
	 * @return An array of Bodies.
	 **/
	int getBodies(lua_State *L) const;

	/**
	 * Get an array of all the Joints in the World.
	 * @return An array of Joints.
	 **/
	int getJoints(lua_State *L) const;

	/**
	 * Get an array of all the Contacts in the World.
	 * @return An array of Contacts.
	 **/
	int getContacts(lua_State *L);

	/**
	 * Gets the ground body.
	 * @return The ground body.
	 **/
	b2Body *getGroundBody() const;

	/**
	 * Calls a callback on all Shapes that overlap a given bounding box.
	 **/
	int queryShapesInArea(lua_State *L);

	/**
	 * Gets all Shapes that overlap a given bounding box.
	 **/
	int getShapesInArea(lua_State *L);

	/**
	 * Raycasts the World for all Fixtures in the path of the ray.
	 **/
	int rayCast(lua_State *L);

	int rayCastAny(lua_State *L);
	int rayCastClosest(lua_State *L);

	/**
	 * Destroy this world.
	 **/
	void destroy();

	void registerObject(void *b2object, love::Object *object);
	void unregisterObject(void *b2object);
	love::Object *findObject(void *b2object) const;

private:

	// Pointer to the Box2D world.
	b2World *world;

	// Ground body
	b2Body *groundBody;

	// The list of to be destructed bodies.
	std::vector<Body *> destructBodies;
	std::vector<Shape *> destructShapes;
	std::vector<Joint *> destructJoints;
	bool destructWorld;

	// Contact callbacks.
	ContactCallback begin, end, presolve, postsolve;
	ContactFilter filter;

	std::unordered_map<void *, love::Object *> box2dObjectMap;

}; // World

} // box2d
} // physics
} // love

#endif // LOVE_PHYSICS_BOX2D_WORLD_H
