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

#ifndef B2_SHAPE_H
#define B2_SHAPE_H

#include "../../Common/b2Math.h"
#include "../b2Collision.h"

class b2BlockAllocator;
class b2Body;
class b2BroadPhase;

/// This holds the mass data computed for a shape.
struct b2MassData
{
	/// The mass of the shape, usually in kilograms.
	float32 mass;

	/// The position of the shape's centroid relative to the shape's origin.
	b2Vec2 center;

	/// The rotational inertia of the shape.
	float32 I;
};

/// This holds contact filtering data.
struct b2FilterData
{
	/// The collision category bits. Normally you would just set one bit.
	uint16 categoryBits;

	/// The collision mask bits. This states the categories that this
	/// shape would accept for collision.
	uint16 maskBits;

	/// Collision groups allow a certain group of objects to never collide (negative)
	/// or always collide (positive). Zero means no collision group. Non-zero group
	/// filtering always wins against the mask bits.
	int16 groupIndex;
};

/// The various collision shape types supported by Box2D.
enum b2ShapeType
{
	e_unknownShape = -1,
	e_circleShape,
	e_polygonShape,
	e_shapeTypeCount,
};

/// A shape definition is used to construct a shape. This class defines an
/// abstract shape definition. You can reuse shape definitions safely.
struct b2ShapeDef
{
	/// The constructor sets the default shape definition values.
	b2ShapeDef()
	{
		type = e_unknownShape;
		userData = NULL;
		friction = 0.2f;
		restitution = 0.0f;
		density = 0.0f;
		filter.categoryBits = 0x0001;
		filter.maskBits = 0xFFFF;
		filter.groupIndex = 0;
		isSensor = false;
	}

	virtual ~b2ShapeDef() {}

	/// Holds the shape type for down-casting.
	b2ShapeType type;

	/// Use this to store application specify shape data.
	void* userData;

	/// The shape's friction coefficient, usually in the range [0,1].
	float32 friction;

	/// The shape's restitution (elasticity) usually in the range [0,1].
	float32 restitution;

	/// The shape's density, usually in kg/m^2.
	float32 density;

	/// A sensor shape collects contact information but never generates a collision
	/// response.
	bool isSensor;

	/// Contact filtering data.
	b2FilterData filter;
};

/// A shape is used for collision detection. Shapes are created in b2World.
/// You can use shape for collision detection before they are attached to the world.
/// @warning you cannot reuse shapes.
class b2Shape
{
public:
	/// Get the type of this shape. You can use this to down cast to the concrete shape.
	/// @return the shape type.
	b2ShapeType GetType() const;

	/// Is this shape a sensor (non-solid)?
	/// @return the true if the shape is a sensor.
	bool IsSensor() const;

	/// Set the contact filtering data. You must call b2World::Refilter to correct
	/// existing contacts/non-contacts.
	void SetFilterData(const b2FilterData& filter);

	/// Get the contact filtering data.
	const b2FilterData& GetFilterData() const;

	/// Get the parent body of this shape. This is NULL if the shape is not attached.
	/// @return the parent body.
	b2Body* GetBody();

	/// Get the next shape in the parent body's shape list.
	/// @return the next shape.
	b2Shape* GetNext();

	/// Get the user data that was assigned in the shape definition. Use this to
	/// store your application specific data.
	void* GetUserData();

	/// Set the user data. Use this to store your application specific data.
	void SetUserData(void* data);

	/// Test a point for containment in this shape. This only works for convex shapes.
	/// @param xf the shape world transform.
	/// @param p a point in world coordinates.
	virtual bool TestPoint(const b2XForm& xf, const b2Vec2& p) const = 0;

	/// Perform a ray cast against this shape.
	/// @param xf the shape world transform.
	/// @param lambda returns the hit fraction. You can use this to compute the contact point
	/// p = (1 - lambda) * segment.p1 + lambda * segment.p2.
	/// @param normal returns the normal at the contact point. If there is no intersection, the normal
	/// is not set.
	/// @param segment defines the begin and end point of the ray cast.
	/// @param maxLambda a number typically in the range [0,1].
	/// @return true if there was an intersection.
	virtual bool TestSegment(	const b2XForm& xf,
								float32* lambda,
								b2Vec2* normal,
								const b2Segment& segment,
								float32 maxLambda) const = 0;

	/// Given a transform, compute the associated axis aligned bounding box for this shape.
	/// @param aabb returns the axis aligned box.
	/// @param xf the world transform of the shape.
	virtual void ComputeAABB(b2AABB* aabb, const b2XForm& xf) const = 0;

	/// Given two transforms, compute the associated swept axis aligned bounding box for this shape.
	/// @param aabb returns the axis aligned box.
	/// @param xf1 the starting shape world transform.
	/// @param xf2 the ending shape world transform.
	virtual void ComputeSweptAABB(	b2AABB* aabb,
									const b2XForm& xf1,
									const b2XForm& xf2) const = 0;

	/// Compute the mass properties of this shape using its dimensions and density.
	/// The inertia tensor is computed about the local origin, not the centroid.
	/// @param massData returns the mass data for this shape.
	virtual void ComputeMass(b2MassData* massData) const = 0;

	/// Get the maximum radius about the parent body's center of mass.
	float32 GetSweepRadius() const;

	/// Get the coefficient of friction.
	float32 GetFriction() const;

	/// Get the coefficient of restitution.
	float32 GetRestitution() const;

	float32 m_density;
	float32 m_friction;
	float32 m_restitution;
	bool m_isSensor;

protected:

	friend class b2Body;
	friend class b2World;

	static b2Shape* Create(const b2ShapeDef* def, b2BlockAllocator* allocator);
	static void Destroy(b2Shape* shape, b2BlockAllocator* allocator);

	b2Shape(const b2ShapeDef* def);
	virtual ~b2Shape();

	void CreateProxy(b2BroadPhase* broadPhase, const b2XForm& xf);
	void DestroyProxy(b2BroadPhase* broadPhase);
	bool Synchronize(b2BroadPhase* broadPhase, const b2XForm& xf1, const b2XForm& xf2);
	void RefilterProxy(b2BroadPhase* broadPhase, const b2XForm& xf);

	virtual void UpdateSweepRadius(const b2Vec2& center) = 0;

	b2ShapeType m_type;
	b2Shape* m_next;
	b2Body* m_body;

	// Sweep radius relative to the parent body's center of mass.
	float32 m_sweepRadius;

	uint16 m_proxyId;
	b2FilterData m_filter;

	

	void* m_userData;
};

inline b2ShapeType b2Shape::GetType() const
{
	return m_type;
}

inline bool b2Shape::IsSensor() const
{
	return m_isSensor;
}

inline void b2Shape::SetFilterData(const b2FilterData& filter)
{
	m_filter = filter;
}

inline const b2FilterData& b2Shape::GetFilterData() const
{
	return m_filter;
}

inline void* b2Shape::GetUserData()
{
	return m_userData;
}

inline void b2Shape::SetUserData(void* data)
{
	m_userData = data;
}

inline b2Body* b2Shape::GetBody()
{
	return m_body;
}

inline b2Shape* b2Shape::GetNext()
{
	return m_next;
}

inline float32 b2Shape::GetSweepRadius() const
{
	return m_sweepRadius;
}

inline float32 b2Shape::GetFriction() const
{
	return m_friction;
}

inline float32 b2Shape::GetRestitution() const
{
	return m_restitution;
}

#endif
