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

#ifndef LOVE_PHYSICS_BOX2D_PHYSICS_H
#define LOVE_PHYSICS_BOX2D_PHYSICS_H

// LOVE
#include <common/Module.h>
#include "World.h"
#include "Contact.h"
#include "Body.h"
#include "Shape.h"
#include "CircleShape.h"
#include "PolygonShape.h"
#include "EdgeShape.h"
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

namespace love
{
namespace physics
{
namespace box2d
{
	class Physics : public Module
	{
	public:

		// Implements Module.
		const char * getName() const;

		/**
		* Creates a new World.
		* @param lx Lower bound on the x-axis.
		* @param ly Lower bound on the y-axis.
		* @param ux Upper bound on the x-axis.
		* @param uy Upper bound on the y-axis.
		* @param gx Gravity along x-axis.
		* @param gy Gravity along y-axis.
		* @param sleep Whether the World allows sleep.
		* @param meter The scale in px/m.
		**/
		World * newWorld(float lx, float ly, float ux, float uy, float gx, float gy, bool sleep, int meter);

		/**
		* Creates a new Body at the specified position.
		* @param world The world to create the Body in.
		* @param x The position along the x-axis.
		* @param x The position along the y-axis.
		* @param m The mass.
		* @param i The rotational inertia.
		**/
		Body * newBody(World * world, float x, float y, float m, float i);

		/**
		* Creates a new Body at the specified position.
		* @param world The world to create the Body in.
		* @param x The position along the x-axis.
		* @param x The position along the y-axis.
		**/
		Body * newBody(World * world, float x, float y);

		/**
		* Creates a new Body at (0, 0)
		* @param world The world to create the Body in.
		**/
		Body * newBody(World * world);

		/**
		* Creates a new CircleShape at (0, 0).
		* @param radius The radius of the circle.
		**/
		CircleShape * newCircleShape(float radius);

		/**
		* Creates a new CircleShape at (x,y) in local coordinates.
		* @param x The offset along the x-axis.
		* @param y The offset along the y-axis.
		* @param radius The radius of the circle.
		**/
		CircleShape * newCircleShape(float x, float y, float radius);

		/**
		* Shorthand for creating rectangular PolygonShapes. The rectangle
		* will be created at the local origin.
		* @param w The width of the rectangle.
		* @param h The height of the rectangle.
		**/
		PolygonShape * newRectangleShape(float w, float h);

		/**
		* Shorthand for creating rectangular PolygonShapes. The rectangle
		* will be created at (x,y) in local coordinates.
		* @param x The offset along the x-axis.
		* @param y The offset along the y-axis.
		* @param w The width of the rectangle.
		* @param h The height of the rectangle.
		**/
		PolygonShape * newRectangleShape(float x, float y, float w, float h);

		/**
		* Shorthand for creating rectangular PolygonShapes. The rectangle
		* will be created at (x,y) in local coordinates.
		* @param x The offset along the x-axis.
		* @param y The offset along the y-axis.
		* @param w The width of the rectangle.
		* @param h The height of the rectangle.
		* @param angle The angle of the rectangle. (rad)
		**/
		PolygonShape * newRectangleShape(float x, float y, float w, float h, float angle);
        
        /**
		* Creates a new EdgeShape. The edge will be created from
		* (x1,y1) to (x2,y2) in local coordinates.
		* @param x1 The x coordinate of the first point.
		* @param y1 The y coordinate of the first point.
		* @param x2 The x coordinate of the second point.
		* @param y2 The y coordinate of the second point.
		**/
		EdgeShape * newEdgeShape(float x1, float y1, float x2, float y2);

		/**
		* Creates a new PolygonShape.
		* @param ... A variable number of vertices.
		**/
		int newPolygonShape(lua_State * L);

		/**
		* Creates a new DistanceJoint connecting body1 with body2.
		* @param x1 Anchor1 along the x-axis. (World coordinates)
		* @param y1 Anchor1 along the y-axis. (World coordinates)
		* @param x2 Anchor2 along the x-axis. (World coordinates)
		* @param y2 Anchor2 along the y-axis. (World coordinates)
		**/
		DistanceJoint * newDistanceJoint(Body * body1, Body * body2, float x1, float y1, float x2, float y2, bool collideConnected);

		/**
		* Creates a new MouseJoint connecting the body with an arbitrary point.
		* @param x Anchor along the x-axis. (World coordinates)
		* @param y Anchor along the y-axis. (World coordinates)
		**/
		MouseJoint * newMouseJoint(Body * body, float x, float y);

		/**
		* Creates a new RevoluteJoint connecting body1 with body2.
		* @param x Anchor along the x-axis. (World coordinates)
		* @param y Anchor along the y-axis. (World coordinates)
		**/
		RevoluteJoint * newRevoluteJoint(Body * body1, Body * body2, float x, float y, bool collideConnected);

		/**
		* Creates a new PrismaticJoint connecting body1 with body2.
		* @param x World-anchor along the x-axis.
		* @param y World-anchor along the y-axis.
		* @param ax The x-component of the world-axis.
		* @param ay The y-component of the world-axis.
		**/
		PrismaticJoint * newPrismaticJoint(Body * body1, Body * body2, float x, float y, float ax, float ay, bool collideConnected);

		/**
		* Creates a new PulleyJoint connecting body1 with body2.
		* @param groundAnchor1 World ground-anchor for body1.
		* @param groundAnchor2 World ground-anchor for body2.
		* @param anchor1 World anchor on body1.
		* @param anchor2 World anchor on body2.
		* @param ratio The pulley ratio.
		**/
		PulleyJoint * newPulleyJoint(Body * body1, Body * body2, b2Vec2 groundAnchor1, b2Vec2 groundAnchor2, b2Vec2 anchor1, b2Vec2 anchor2, float ratio, bool collideConnected);

		/**
		* Creates a new GearJoint connecting joint1 with joint2.
		* @param joint1 The first joint.
		* @param joint2 The second joint.
		* @param ratio The gear ratio.
		**/
		GearJoint * newGearJoint(Joint * joint1, Joint * joint2, float ratio, bool collideConnected);

		/**
		* Creates a new FrictionJoint connecting body1 with body2.
		* @param x Anchor along the x-axis. (World coordinates)
		* @param y Anchor along the y-axis. (World coordinates)
		**/
		FrictionJoint * newFrictionJoint(Body * body1, Body * body2, float x, float y, bool collideConnected);
		
		/**
		* Creates a new WeldJoint connecting body1 with body2.
		* @param x Anchor along the x-axis. (World coordinates)
		* @param y Anchor along the y-axis. (World coordinates)
		**/
		WeldJoint * newWeldJoint(Body * body1, Body * body2, float x, float y, bool collideConnected);
		
		/**
		* Creates a new WheelJoint connecting body1 with body2.
		* @param x Anchor along the x-axis. (World coordinates)
		* @param y Anchor along the y-axis. (World coordinates)
		* @param ax The x-component of the world-axis.
		* @param ay The y-component of the world-axis.
		**/
		WheelJoint * newWheelJoint(Body * body1, Body * body2, float x, float y, float ax, float ay, bool collideConnected);
		
		/**
		* Creates a new RopeJoint connecting body1 with body2.
		* @param x1 Anchor1 along the x-axis. (Local coordinates)
		* @param y1 Anchor1 along the y-axis. (Local coordinates)
		* @param x2 Anchor2 along the x-axis. (Local coordinates)
		* @param y2 Anchor2 along the y-axis. (Local coordinates)
		* @param maxLength The maximum distance for the bodies.
		**/
		RopeJoint * newRopeJoint(Body * body1, Body * body2, float x1, float y1, float x2, float y2, float maxLength, bool collideConnected);


	}; // Physics

} // box2d
} // physics
} // love

#endif // LOVE_PHYSICS_BOX2D_PHYSICS_H
