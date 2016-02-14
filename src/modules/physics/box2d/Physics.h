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

#ifndef LOVE_PHYSICS_BOX2D_PHYSICS_H
#define LOVE_PHYSICS_BOX2D_PHYSICS_H

// LOVE
#include "common/Module.h"
#include "World.h"
#include "Contact.h"
#include "Body.h"
#include "Fixture.h"
#include "Shape.h"
#include "CircleShape.h"
#include "PolygonShape.h"
#include "EdgeShape.h"
#include "ChainShape.h"
#include "Joint.h"
#include "MouseJoint.h"
#include "DistanceJoint.h"
#include "PrismaticJoint.h"
#include "RevoluteJoint.h"
#include "PulleyJoint.h"
#include "GearJoint.h"
#include "FrictionJoint.h"
#include "WeldJoint.h"
#include "WheelJoint.h"
#include "RopeJoint.h"
#include "MotorJoint.h"

namespace love
{
namespace physics
{
namespace box2d
{

class Physics : public Module
{
public:

	/**
	 * 30 pixels in one meter by default.
	 **/
	static const int DEFAULT_METER = 30;

	// Implements Module.
	const char *getName() const;
	virtual ModuleType getModuleType() const { return M_PHYSICS; }

	/**
	 * Creates a new World.
	 * @param gx Gravity along x-axis.
	 * @param gy Gravity along y-axis.
	 * @param sleep Whether the World allows sleep.
	 **/
	World *newWorld(float gx, float gy, bool sleep);

	/**
	 * Creates a new Body at the specified position.
	 * @param world The world to create the Body in.
	 * @param x The position along the x-axis.
	 * @param y The position along the y-axis.
	 * @param type The type of body to create.
	 **/
	Body *newBody(World *world, float x, float y, Body::Type type);

	/**
	 * Creates a new Body at (0, 0)
	 * @param world The world to create the Body in.
	 * @param type The type of Body to create.
	 **/
	Body *newBody(World *world, Body::Type type);

	/**
	 * Creates a new CircleShape at (0, 0).
	 * @param radius The radius of the circle.
	 **/
	CircleShape *newCircleShape(float radius);

	/**
	 * Creates a new CircleShape at (x,y) in local coordinates.
	 * @param x The offset along the x-axis.
	 * @param y The offset along the y-axis.
	 * @param radius The radius of the circle.
	 **/
	CircleShape *newCircleShape(float x, float y, float radius);

	/**
	 * Shorthand for creating rectangular PolygonShapes. The rectangle
	 * will be created at the local origin.
	 * @param w The width of the rectangle.
	 * @param h The height of the rectangle.
	 **/
	PolygonShape *newRectangleShape(float w, float h);

	/**
	 * Shorthand for creating rectangular PolygonShapes. The rectangle
	 * will be created at (x,y) in local coordinates.
	 * @param x The offset along the x-axis.
	 * @param y The offset along the y-axis.
	 * @param w The width of the rectangle.
	 * @param h The height of the rectangle.
	 **/
	PolygonShape *newRectangleShape(float x, float y, float w, float h);

	/**
	 * Shorthand for creating rectangular PolygonShapes. The rectangle
	 * will be created at (x,y) in local coordinates.
	 * @param x The offset along the x-axis.
	 * @param y The offset along the y-axis.
	 * @param w The width of the rectangle.
	 * @param h The height of the rectangle.
	 * @param angle The angle of the rectangle. (rad)
	 **/
	PolygonShape *newRectangleShape(float x, float y, float w, float h, float angle);

	/**
	 * Creates a new EdgeShape. The edge will be created from
	 * (x1,y1) to (x2,y2) in local coordinates.
	 * @param x1 The x coordinate of the first point.
	 * @param y1 The y coordinate of the first point.
	 * @param x2 The x coordinate of the second point.
	 * @param y2 The y coordinate of the second point.
	 **/
	EdgeShape *newEdgeShape(float x1, float y1, float x2, float y2);

	/**
	 * Creates a new PolygonShape from a variable number of vertices.
	 **/
	int newPolygonShape(lua_State *L);

	/**
	 * Creates a new ChainShape from a variable number of vertices.
	 **/
	int newChainShape(lua_State *L);

	/**
	 * Creates a new DistanceJoint connecting body1 with body2.
	 * @param x1 Anchor1 along the x-axis. (World coordinates)
	 * @param y1 Anchor1 along the y-axis. (World coordinates)
	 * @param x2 Anchor2 along the x-axis. (World coordinates)
	 * @param y2 Anchor2 along the y-axis. (World coordinates)
	 * @param collideConnected Whether the connected bodies should collide with each other. Defaults to false.
	 **/
	DistanceJoint *newDistanceJoint(Body *body1, Body *body2, float x1, float y1, float x2, float y2, bool collideConnected);

	/**
	 * Creates a new MouseJoint connecting the body with an arbitrary point.
	 * @param x Anchor along the x-axis. (World coordinates)
	 * @param y Anchor along the y-axis. (World coordinates)
	 **/
	MouseJoint *newMouseJoint(Body *body, float x, float y);

	/**
	 * Creates a new RevoluteJoint connecting body1 with body2.
	 * @param x Anchor along the x-axis. (World coordinates)
	 * @param y Anchor along the y-axis. (World coordinates)
	 * @param collideConnected Whether the connected bodies should collide with each other. Defaults to false.
	 **/
	RevoluteJoint *newRevoluteJoint(Body *body1, Body *body2, float x, float y, bool collideConnected);

	/**
	 * Creates a new PrismaticJoint connecting body1 with body2.
	 * @param xA World-anchor for body1 along the x-axis.
	 * @param yA World-anchor for body1 along the y-axis.
	 * @param xB World-anchor for body2 along the x-axis.
	 * @param yB World-anchor for body2 along the y-axis.
	 * @param ax The x-component of the world-axis.
	 * @param ay The y-component of the world-axis.
	 * @param collideConnected Whether the connected bodies should collide with each other. Defaults to false.
	 **/
	PrismaticJoint *newPrismaticJoint(Body *body1, Body *body2, float xA, float yA, float xB, float yB, float ax, float ay, bool collideConnected);

	/**
	 * Creates a new PulleyJoint connecting body1 with body2.
	 * @param groundAnchor1 World ground-anchor for body1.
	 * @param groundAnchor2 World ground-anchor for body2.
	 * @param anchor1 World anchor on body1.
	 * @param anchor2 World anchor on body2.
	 * @param ratio The pulley ratio.
	 * @param collideConnected Whether the connected bodies should collide with each other. Defaults to true.
	 **/
	PulleyJoint *newPulleyJoint(Body *body1, Body *body2, b2Vec2 groundAnchor1, b2Vec2 groundAnchor2, b2Vec2 anchor1, b2Vec2 anchor2, float ratio, bool collideConnected);

	/**
	 * Creates a new GearJoint connecting joint1 with joint2.
	 * @param joint1 The first joint.
	 * @param joint2 The second joint.
	 * @param ratio The gear ratio.
	 * @param collideConnected Whether the connected bodies should collide with each other. Defaults to false.
	 **/
	GearJoint *newGearJoint(Joint *joint1, Joint *joint2, float ratio, bool collideConnected);

	/**
	 * Creates a new FrictionJoint connecting body1 with body2.
	 * @param xA Anchor for body 1 along the x-axis. (World coordinates)
	 * @param yA Anchor for body 1 along the y-axis. (World coordinates)
	 * @param xB Anchor for body 2 along the x-axis. (World coordinates)
	 * @param yB Anchor for body 2 along the y-axis. (World coordinates)
	 * @param collideConnected Whether the connected bodies should collide with each other. Defaults to false.
	 **/
	FrictionJoint *newFrictionJoint(Body *body1, Body *body2, float xA, float yA, float xB, float yB, bool collideConnected);

	/**
	 * Creates a new WeldJoint connecting body1 with body2.
	 * @param xA Anchor for body 1 along the x-axis. (World coordinates)
	 * @param yA Anchor for body 1 along the y-axis. (World coordinates)
	 * @param xB Anchor for body 2 along the x-axis. (World coordinates)
	 * @param yB Anchor for body 2 along the y-axis. (World coordinates)
	 * @param collideConnected Whether the connected bodies should collide with each other. Defaults to false.
	 **/
	WeldJoint *newWeldJoint(Body *body1, Body *body2, float xA, float yA, float xB, float yB, bool collideConnected);

	/**
	 * Creates a new WheelJoint connecting body1 with body2.
	 * @param xA Anchor for body 1 along the x-axis. (World coordinates)
	 * @param yA Anchor for body 1 along the y-axis. (World coordinates)
	 * @param xB Anchor for body 2 along the x-axis. (World coordinates)
	 * @param yB Anchor for body 2 along the y-axis. (World coordinates)
	 * @param ax The x-component of the world-axis.
	 * @param ay The y-component of the world-axis.
	 * @param collideConnected Whether the connected bodies should collide with each other. Defaults to false.
	 **/
	WheelJoint *newWheelJoint(Body *body1, Body *body2, float xA, float yA, float xB, float yB, float ax, float ay, bool collideConnected);

	/**
	 * Creates a new RopeJoint connecting body1 with body2.
	 * @param x1 Anchor1 along the x-axis. (Local coordinates)
	 * @param y1 Anchor1 along the y-axis. (Local coordinates)
	 * @param x2 Anchor2 along the x-axis. (Local coordinates)
	 * @param y2 Anchor2 along the y-axis. (Local coordinates)
	 * @param maxLength The maximum distance for the bodies.
	 * @param collideConnected Whether the connected bodies should collide with each other. Defaults to false.
	 **/
	RopeJoint *newRopeJoint(Body *body1, Body *body2, float x1, float y1, float x2, float y2, float maxLength, bool collideConnected);

	/**
	 * Creates a new MotorJoint controlling the relative motion between body1
	 * and body2.
	 **/
	MotorJoint *newMotorJoint(Body *body1, Body *body2);
	MotorJoint *newMotorJoint(Body *body1, Body *body2, float correctionFactor, bool collideConnected);

	/**
	 * Creates a new Fixture attaching shape to body.
	 * @param body The body to attach the Fixture to.
	 * @param shape The shape to attach to the Fixture,
	 * @param density The density of the Fixture.
	 **/

	Fixture *newFixture(Body *body, Shape *shape, float density);

	/**
	 * Calculates the distance between two Fixtures.
	 * @param fixtureA The first Fixture.
	 * @param fixtureB The sceond Fixture.
	 * @return The distance between them, and the two points closest
	 *         to each other.
	 **/
	int getDistance(lua_State *L);

	/**
	 * Sets the number of pixels in one meter.
	 * @param scale The number of pixels in one meter. (1m ~= 3.3ft).
	 **/
	static void setMeter(int scale);

	/**
	 * Gets the number of pixels in one meter.
	 * @return The number of pixels in one meter. (1m ~= 3.3ft).
	 **/
	static int getMeter();

	/**
	 * Scales a value down according to the current meter in pixels.
	 * @param f The unscaled input value.
	 **/
	static float scaleDown(float f);

	/**
	 * Scales a value up according to the current meter in pixels.
	 * @param f The unscaled input value.
	 **/
	static float scaleUp(float f);

	/**
	 * Scales a point down according to the current meter
	 * in pixels, for instance x = x0/meter, y = x0/meter.
	 * @param x The x-coordinate of the point to scale.
	 * @param y The y-coordinate of the point to scale.
	 **/
	static void scaleDown(float &x, float &y);

	/**
	 * Scales a point up according to the current meter
	 * in pixels, for instance x = x0/meter, y = x0/meter.
	 * @param x The x-coordinate of the point to scale.
	 * @param y The y-coordinate of the point to scale.
	 **/
	static void scaleUp(float &x, float &y);

	/**
	 * Scales a b2Vec2 down according to the current meter in pixels.
	 * @param v The unscaled input vector.
	 * @return The scaled vector.
	 **/
	static b2Vec2 scaleDown(const b2Vec2 &v);

	/**
	 * Scales a b2Vec up according to the current meter in pixels.
	 * @param v The unscaled input vector.
	 * @return The scaled vector.
	 **/
	static b2Vec2 scaleUp(const b2Vec2 &v);

	/**
	 * Scales a b2AABB down according to the current meter in pixels.
	 * @param aabb The unscaled input AABB.
	 * @return The scaled AABB.
	 **/
	static b2AABB scaleDown(const b2AABB &aabb);

	/**
	 * Scales a b2AABB up according to the current meter in pixels.
	 * @param aabb The unscaled input AABB.
	 * @return The scaled AABB.
	 **/
	static b2AABB scaleUp(const b2AABB &aabb);

private:

	// The length of one meter in pixels.
	static int meter;
}; // Physics

} // box2d
} // physics
} // love

#endif // LOVE_PHYSICS_BOX2D_PHYSICS_H
