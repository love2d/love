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

#ifndef B2_COLLISION_H
#define B2_COLLISION_H

#include "../Common/b2Math.h"
#include <climits>

/// @file
/// Structures and functions used for computing contact points, distance
/// queries, and TOI queries.

class b2Shape;
class b2CircleShape;
class b2PolygonShape;

const uint8 b2_nullFeature = UCHAR_MAX;

/// Contact ids to facilitate warm starting.
union b2ContactID
{
	/// The features that intersect to form the contact point
	struct Features
	{
		uint8 referenceEdge;	///< The edge that defines the outward contact normal.
		uint8 incidentEdge;		///< The edge most anti-parallel to the reference edge.
		uint8 incidentVertex;	///< The vertex (0 or 1) on the incident edge that was clipped.
		uint8 flip;				///< A value of 1 indicates that the reference edge is on shape2.
	} features;
	uint32 key;					///< Used to quickly compare contact ids.
};

/// A manifold point is a contact point belonging to a contact
/// manifold. It holds details related to the geometry and dynamics
/// of the contact points.
/// The point is stored in local coordinates because CCD
/// requires sub-stepping in which the separation is stale.
struct b2ManifoldPoint
{
	b2Vec2 localPoint1;		///< local position of the contact point in body1
	b2Vec2 localPoint2;		///< local position of the contact point in body2
	float32 separation;		///< the separation of the shapes along the normal vector
	float32 normalImpulse;	///< the non-penetration impulse
	float32 tangentImpulse;	///< the friction impulse
	b2ContactID id;			///< uniquely identifies a contact point between two shapes
};

/// A manifold for two touching convex shapes.
struct b2Manifold
{
	b2ManifoldPoint points[b2_maxManifoldPoints];	///< the points of contact
	b2Vec2 normal;	///< the shared unit normal vector
	int32 pointCount;	///< the number of manifold points
};

/// A line segment.
struct b2Segment
{
	/// Ray cast against this segment with another segment.
	bool TestSegment(float32* lambda, b2Vec2* normal, const b2Segment& segment, float32 maxLambda) const;

	b2Vec2 p1;	///< the starting point
	b2Vec2 p2;	///< the ending point
};

/// An axis aligned bounding box.
struct b2AABB
{
	/// Verify that the bounds are sorted.
	bool IsValid() const;

	b2Vec2 lowerBound;	///< the lower vertex
	b2Vec2 upperBound;	///< the upper vertex
};

/// An oriented bounding box.
struct b2OBB
{
	b2Mat22 R;			///< the rotation matrix
	b2Vec2 center;		///< the local centroid
	b2Vec2 extents;		///< the half-widths
};

/// Compute the collision manifold between two circles.
void b2CollideCircles(b2Manifold* manifold,
					  const b2CircleShape* circle1, const b2XForm& xf1,
					  const b2CircleShape* circle2, const b2XForm& xf2);

/// Compute the collision manifold between a polygon and a circle.
void b2CollidePolygonAndCircle(b2Manifold* manifold,
							   const b2PolygonShape* polygon, const b2XForm& xf1,
							   const b2CircleShape* circle, const b2XForm& xf2);

/// Compute the collision manifold between two circles.
void b2CollidePolygons(b2Manifold* manifold,
					   const b2PolygonShape* polygon1, const b2XForm& xf1,
					   const b2PolygonShape* polygon2, const b2XForm& xf2);

/// Compute the distance between two shapes and the closest points.
/// @return the distance between the shapes or zero if they are overlapped/touching.
float32 b2Distance(b2Vec2* x1, b2Vec2* x2,
				   const b2Shape* shape1, const b2XForm& xf1,
				   const b2Shape* shape2, const b2XForm& xf2);

/// Compute the time when two shapes begin to touch or touch at a closer distance.
/// @warning the sweeps must have the same time interval.
/// @return the fraction between [0,1] in which the shapes first touch.
/// fraction=0 means the shapes begin touching/overlapped, and fraction=1 means the shapes don't touch.
float32 b2TimeOfImpact(const b2Shape* shape1, const b2Sweep& sweep1,
					   const b2Shape* shape2, const b2Sweep& sweep2);


// ---------------- Inline Functions ------------------------------------------

inline bool b2AABB::IsValid() const
{
	b2Vec2 d = upperBound - lowerBound;
	bool valid = d.x >= 0.0f && d.y >= 0.0f;
	valid = valid && lowerBound.IsValid() && upperBound.IsValid();
	return valid;
}

inline bool b2TestOverlap(const b2AABB& a, const b2AABB& b)
{
	b2Vec2 d1, d2;
	d1 = b.lowerBound - a.upperBound;
	d2 = a.lowerBound - b.upperBound;

	if (d1.x > 0.0f || d1.y > 0.0f)
		return false;

	if (d2.x > 0.0f || d2.y > 0.0f)
		return false;

	return true;
}

#endif
