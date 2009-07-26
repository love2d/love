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

#ifndef B2_POLYGON_SHAPE_H
#define B2_POLYGON_SHAPE_H

#include "b2Shape.h"

/// Convex polygon. The vertices must be in CCW order for a right-handed
/// coordinate system with the z-axis coming out of the screen.
struct b2PolygonDef : public b2ShapeDef
{
	b2PolygonDef()
	{
		type = e_polygonShape;
		vertexCount = 0;
	}

	/// Build vertices to represent an axis-aligned box.
	/// @param hx the half-width.
	/// @param hy the half-height.
	void SetAsBox(float32 hx, float32 hy);

	/// Build vertices to represent an oriented box.
	/// @param hx the half-width.
	/// @param hy the half-height.
	/// @param center the center of the box in local coordinates.
	/// @param angle the rotation of the box in local coordinates.
	void SetAsBox(float32 hx, float32 hy, const b2Vec2& center, float32 angle);

	/// The polygon vertices in local coordinates.
	b2Vec2 vertices[b2_maxPolygonVertices];

	/// The number of polygon vertices.
	int32 vertexCount;
};


/// A convex polygon.
class b2PolygonShape : public b2Shape
{
public:
	/// @see b2Shape::TestPoint
	bool TestPoint(const b2XForm& transform, const b2Vec2& p) const;

	/// @see b2Shape::TestSegment
	bool TestSegment(	const b2XForm& transform,
		float32* lambda,
		b2Vec2* normal,
		const b2Segment& segment,
		float32 maxLambda) const;

	/// @see b2Shape::ComputeAABB
	void ComputeAABB(b2AABB* aabb, const b2XForm& transform) const;

	/// @see b2Shape::ComputeSweptAABB
	void ComputeSweptAABB(	b2AABB* aabb,
		const b2XForm& transform1,
		const b2XForm& transform2) const;

	/// @see b2Shape::ComputeMass
	void ComputeMass(b2MassData* massData) const;

	/// Get the oriented bounding box relative to the parent body.
	const b2OBB& GetOBB() const;

	/// Get local centroid relative to the parent body.
	const b2Vec2& GetCentroid() const;

	/// Get the vertex count.
	int32 GetVertexCount() const;

	/// Get the vertices in local coordinates.
	const b2Vec2* GetVertices() const;

	/// Get the core vertices in local coordinates. These vertices
	/// represent a smaller polygon that is used for time of impact
	/// computations.
	const b2Vec2* GetCoreVertices() const;

	/// Get the edge normal vectors. There is one for each vertex.
	const b2Vec2* GetNormals() const;

	/// Get the first vertex and apply the supplied transform.
	b2Vec2 GetFirstVertex(const b2XForm& xf) const;

	/// Get the centroid and apply the supplied transform.
	b2Vec2 Centroid(const b2XForm& xf) const;

	/// Get the support point in the given world direction.
	/// Use the supplied transform.
	b2Vec2 Support(const b2XForm& xf, const b2Vec2& d) const;

private:

	friend class b2Shape;

	b2PolygonShape(const b2ShapeDef* def);

	void UpdateSweepRadius(const b2Vec2& center);

	// Local position of the polygon centroid.
	b2Vec2 m_centroid;

	b2OBB m_obb;

	b2Vec2 m_vertices[b2_maxPolygonVertices];
	b2Vec2 m_normals[b2_maxPolygonVertices];
	b2Vec2 m_coreVertices[b2_maxPolygonVertices];
	int32 m_vertexCount;
};

inline b2Vec2 b2PolygonShape::GetFirstVertex(const b2XForm& xf) const
{
	return b2Mul(xf, m_coreVertices[0]);
}

inline const b2OBB& b2PolygonShape::GetOBB() const
{
	return m_obb;
}

inline const b2Vec2& b2PolygonShape::GetCentroid() const
{
	return m_centroid;
}

inline int32 b2PolygonShape::GetVertexCount() const
{
	return m_vertexCount;
}

inline const b2Vec2* b2PolygonShape::GetVertices() const
{
	return m_vertices;
}

inline const b2Vec2* b2PolygonShape::GetCoreVertices() const
{
	return m_coreVertices;
}

inline const b2Vec2* b2PolygonShape::GetNormals() const
{
	return m_normals;
}

#endif
