/**
* Copyright (c) 2006-2009 LOVE Development Team
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

#ifndef LOVE_PHYSICS_BOX2D_BODY_H
#define LOVE_PHYSICS_BOX2D_BODY_H

// LOVE
#include <common/runtime.h>
#include <common/Object.h>


// Box2D
#include "Include/Box2D.h"

namespace love
{
namespace physics
{
namespace box2d
{
	// Forward declarations.
	class World;

	/**
	* A Body is an entity which has position and orientation
	* in world space. A Body does have collision geometry 
	* by itself, but depend on an arbitrary number of child Shape objects 
	* which together constitute the final geometry for the Body.
	**/
	class Body : public Object
	{
		// Friends.
		friend class Joint;
		friend class DistanceJoint;
		friend class MouseJoint;
		friend class CircleShape;
		friend class PolygonShape;
		friend class Shape;

	private:

		// We need a shared_ptr to the parent World, 
		// because World can not be destroyed as long as
		// bodies exists in it.
		//
		// This ensures that a World only can be destroyed
		// once all bodies have been destroyed too.
		World * world;
		
	public:

		// The Box2D body. (Should not be public?)
		b2Body * body;

		/**
		* Create a Body at position p.
		**/ 
		Body(World * world, b2Vec2 p, float m, float i);

		~Body();

		/**
		* Gets the current x-position of the Body.
		**/
		float getX();

		/**
		* Gets the current y-position of the Body.
		**/
		float getY();

		/**
		* Gets the current angle (deg) of the Body.
		**/
		float getAngle();

		/**
		* Gets the current position of the Body.
		* @returns The current x-position.
		* @returns The current y-position.
		**/
		int getPosition(lua_State * L);

		/**
		* Gets the velocity in the current center of mass.
		* @returns The x-component of the velocity.
		* @returns The y-component of the velocity.
		**/
		int getVelocity(lua_State * L);

		/**
		* The current center of mass for the Body in world
		* coordinates.
		* @returns The x-component of the point.
		* @returns The y-component of the point.
		**/
		int getWorldCenter(lua_State * L);

		/**
		* The current center of mass for the Body in local
		* coordinates.
		* @returns The x-component of the point.
		* @returns The y-component of the point.
		**/
		int getLocalCenter(lua_State * L);

		/**
		* Get the current Body spin. (Angular velocity).
		**/
		float getSpin() const;

		/**
		* Gets the Body's mass.
		**/
		float getMass() const;

		/**
		* Gets the Body's intertia.
		**/
		float getInertia() const;

		/**
		* Gets the Body's angular damping.
		**/
		float getAngularDamping() const;

		/**
		* Gets the Body's linear damping.
		**/
		float getDamping() const;

		/**
		* Apply an impulse (jx, jy) with offset (0, 0).
		**/
		void applyImpulse(float jx, float jy);

		/**
		* Apply an impulse (jx, jy) with offset (rx, ry).
		**/
		void applyImpulse(float jx, float jy, float rx, float ry);

		/**
		* Apply torque (t).
		**/
		void applyTorque(float t);

		/**
		* Apply force (fx, fy) with offset (0, 0).
		**/
		void applyForce(float fx, float fy);

		/**
		* Apply force (fx, fy) with offset (rx, ry).
		**/
		void applyForce(float fx, float fy, float rx, float ry);
	
		/**
		* Sets the x-position of the Body.
		**/
		void setX(float x);

		/**
		* Sets the Y-position of the Body.
		**/
		void setY(float y);

		/**
		* Sets the current velocity of the Body.
		**/
		void setVelocity(float x, float y);

		/**
		* Sets the angle of the Body.
		**/
		void setAngle(float d);

		/**
		* Sets the current spin of the Body. 
		**/
		void setSpin(float r);

		/**
		* Sets the current position of the Body.
		**/ 
		void setPosition(float x, float y);

		/**
		* Sets the mass from the currently attatched shapes.
		**/ 
		void setMassFromShapes();

		/**
		* Sets mass properties.
		* @param x The x-coordinate for the local center of mass.
		* @param y The y-coordinate for the local center of mass.
		* @param m The mass.
		* @param i The inertia.
		**/
		void setMass(float x, float y, float m, float i);

		/**
		* Sets the Body's angular damping.
		**/
		void setAngularDamping(float d);

		/**
		* Sets the Body's linear damping.
		**/
		void setDamping(float d);

		/**
		* Transforms a point (x, y) from local coordinates
		* to world coordinates.
		* @param x The x-coordinate of the local point.
		* @param y The y-coordinate of the local point.
		* @returns The x-coordinate of the point in world coordinates.
		* @returns The y-coordinate of the point in world coordinates.
		**/
		int getWorldPoint(lua_State * L);

		/**
		* Transforms a vector (x, y) from local coordinates
		* to world coordinates.
		* @param x The x-coordinate of the local vector.
		* @param y The y-coordinate of the local vector.
		* @returns The x-coordinate of the vector in world coordinates.
		* @returns The y-coordinate of the vector in world coordinates.
		**/
		int getWorldVector(lua_State * L);

		/**
		* Transforms a point (x, y) from world coordinates
		* to local coordinates.
		* @param x The x-coordinate of the world point.
		* @param y The y-coordinate of the world point.
		* @returns The x-coordinate of the point in local coordinates.
		* @returns The y-coordinate of the point in local coordinates.
		**/
		int getLocalPoint(lua_State * L);

		/**
		* Transforms a vector (x, y) from world coordinates
		* to local coordinates.
		* @param x The x-coordinate of the world vector.
		* @param y The y-coordinate of the world vector.
		* @returns The x-coordinate of the vector in local coordinates.
		* @returns The y-coordinate of the vector in local coordinates.
		**/
		int getLocalVector(lua_State * L);

		/**
		* Gets the velocity on the Body for the given world point.
		* @param x The x-coordinate of the world point.
		* @param y The y-coordinate of the world point.
		* @returns The x-component of the velocity vector.
		* @returns The y-component of the velocity vector.
		**/
		int getVelocityWorldPoint(lua_State * L);

		/**
		* Gets the velocity on the Body for the given local point.
		* @param x The x-coordinate of the local point.
		* @param y The y-coordinate of the local point.
		* @returns The x-component of the velocity vector.
		* @returns The y-component of the velocity vector.
		**/
		int getVelocityLocalPoint(lua_State * L);

		/**
		* Returns true if the Body is a bullet, false otherwise.
		**/
		bool isBullet() const;

		/**
		* Set whether this Body should be treated as a bullet.
		* Bullets require more processing power than normal shapes.
		**/
		void setBullet(bool bullet);

		/**
		* Checks whether a Body is static or not, i.e. terrain
		* or not.
		**/
		bool isStatic() const;

		/**
		* The opposite of isStatic.
		**/
		bool isDynamic() const;

		/**
		* Checks whether a Body is frozen or not.
		* A Body will freeze if hits the world bounding box.
		**/
		bool isFrozen() const;

		/**
		* Checks whether a Body is sleeping or nor. A Body
		* will fall to sleep if nothing happens to it for while.
		**/
		bool isSleeping() const;

		/**
		* Controls whether this Body should be allowed to sleep.
		**/ 
		void setAllowSleep(bool allow);

		/**
		* Controls the Body's sleep.
		* @param sleep True to put to sleep, false to wake up.
		**/
		void setSleep(bool sleep);

	private:

		/**
		* Gets a 2d vector from the arguments on the stack.
		**/
		b2Vec2 getVector(lua_State * L);

		/**
		* Pushed the x- and y-components of a vector on 
		* the stack.
		**/
		int pushVector(lua_State * L, const b2Vec2 & v);
	};

} // box2d
} // physics
} // love

#endif // LOVE_PHYSICS_BOX2D_BODY_H
