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

#ifndef B2_CIRCLE_SHAPE_H
#define B2_CIRCLE_SHAPE_H

#include "b2Shape.h"

/// This structure is used to build circle shapes.
struct b2CircleDef : public b2ShapeDef
{
	b2CircleDef()
	{
		type = e_circleShape;
		localPosition.SetZero();
		radius = 1.0f;
	}

	b2Vec2 localPosition;
	float32 radius;
};

/// A circle shape.
class b2CircleShape : public b2Shape
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

	/// Get the local position of this circle in its parent body.
	const b2Vec2& GetLocalPosition() const;

	/// Get the radius of this circle.
	float32 GetRadius() const;

private:

	friend class b2Shape;

	b2CircleShape(const b2ShapeDef* def);

	void UpdateSweepRadius(const b2Vec2& center);

	// Local position in parent body
	b2Vec2 m_localPosition;
	float32 m_radius;
};

inline const b2Vec2& b2CircleShape::GetLocalPosition() const
{
	return m_localPosition;
}

inline float32 b2CircleShape::GetRadius() const
{
	return m_radius;
}

#endif
