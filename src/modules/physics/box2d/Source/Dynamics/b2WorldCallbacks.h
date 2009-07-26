/*
* Copyright (c) 2006-2007 Erin Catto http://www.gphysics.com
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef B2_WORLD_CALLBACKS_H
#define B2_WORLD_CALLBACKS_H

#include "../Common/b2Settings.h"

struct b2Vec2;
struct b2XForm;
class b2Shape;
class b2Body;
class b2Joint;
class b2Contact;
struct b2ContactPoint;
struct b2ContactResult;

/// Joints and shapes are destroyed when their associated
/// body is destroyed. Implement this listener so that you
/// may nullify references to these joints and shapes.
class b2DestructionListener
{
public:
	virtual ~b2DestructionListener() {}

	/// Called when any joint is about to be destroyed due
	/// to the destruction of one of its attached bodies.
	virtual void SayGoodbye(b2Joint* joint) = 0;

	/// Called when any shape is about to be destroyed due
	/// to the destruction of its parent body.
	virtual void SayGoodbye(b2Shape* shape) = 0;
};


/// This is called when a body's shape passes outside of the world boundary.
class b2BoundaryListener
{
public:
	virtual ~b2BoundaryListener() {}

	/// This is called for each body that leaves the world boundary.
	/// @warning you can't modify the world inside this callback.
	virtual void Violation(b2Body* body) = 0;
};


/// Implement this class to provide collision filtering. In other words, you can implement
/// this class if you want finer control over contact creation.
class b2ContactFilter
{
public:
	virtual ~b2ContactFilter() {}

	/// Return true if contact calculations should be performed between these two shapes.
	/// @warning for performance reasons this is only called when the AABBs begin to overlap.
	virtual bool ShouldCollide(b2Shape* shape1, b2Shape* shape2);
};

/// The default contact filter.
extern b2ContactFilter b2_defaultFilter;

/// Implement this class to get collision results. You can use these results for
/// things like sounds and game logic. You can also get contact results by
/// traversing the contact lists after the time step. However, you might miss
/// some contacts because continuous physics leads to sub-stepping.
/// Additionally you may receive multiple callbacks for the same contact in a
/// single time step.
/// You should strive to make your callbacks efficient because there may be
/// many callbacks per time step.
/// @warning The contact separation is the last computed value.
/// @warning You cannot create/destroy Box2D entities inside these callbacks.
class b2ContactListener
{
public:
	virtual ~b2ContactListener() {}

	/// Called when a contact point is added. This includes the geometry
	/// and the forces.
	virtual void Add(const b2ContactPoint* point) { }

	/// Called when a contact point persists. This includes the geometry
	/// and the forces.
	virtual void Persist(const b2ContactPoint* point) { }

	/// Called when a contact point is removed. This includes the last
	/// computed geometry and forces.
	virtual void Remove(const b2ContactPoint* point) { }

	/// Called after a contact point is solved.
	virtual void Result(const b2ContactResult* point) { }
};

/// Color for debug drawing. Each value has the range [0,1].
struct b2Color
{
	b2Color() {}
	b2Color(float32 r, float32 g, float32 b) : r(r), g(g), b(b) {}
	float32 r, g, b;
};

/// Implement and register this class with a b2World to provide debug drawing of physics
/// entities in your game.
class b2DebugDraw
{
public:
	b2DebugDraw();

	virtual ~b2DebugDraw() {}

	enum
	{
		e_shapeBit				= 0x0001, ///< draw shapes
		e_jointBit				= 0x0002, ///< draw joint connections
		e_coreShapeBit			= 0x0004, ///< draw core (TOI) shapes
		e_aabbBit				= 0x0008, ///< draw axis aligned bounding boxes
		e_obbBit				= 0x0010, ///< draw oriented bounding boxes
		e_pairBit				= 0x0020, ///< draw broad-phase pairs
		e_centerOfMassBit		= 0x0040, ///< draw center of mass frame
	};

	/// Set the drawing flags.
	void SetFlags(uint32 flags);

	/// Get the drawing flags.
	uint32 GetFlags() const;
	
	/// Append flags to the current flags.
	void AppendFlags(uint32 flags);

	/// Clear flags from the current flags.
	void ClearFlags(uint32 flags);

	/// Draw a closed polygon provided in CCW order.
	virtual void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) = 0;

	/// Draw a solid closed polygon provided in CCW order.
	virtual void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) = 0;

	/// Draw a circle.
	virtual void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color) = 0;
	
	/// Draw a solid circle.
	virtual void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color) = 0;
	
	/// Draw a line segment.
	virtual void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) = 0;

	/// Draw a transform. Choose your own length scale.
	/// @param xf a transform.
	virtual void DrawXForm(const b2XForm& xf) = 0;

protected:
	uint32 m_drawFlags;
};

#endif
