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

#include "b2CircleShape.h"

b2CircleShape::b2CircleShape(const b2ShapeDef* def)
: b2Shape(def)
{
	b2Assert(def->type == e_circleShape);
	const b2CircleDef* circleDef = (const b2CircleDef*)def;

	m_type = e_circleShape;
	m_localPosition = circleDef->localPosition;
	m_radius = circleDef->radius;
}

void b2CircleShape::UpdateSweepRadius(const b2Vec2& center)
{
	// Update the sweep radius (maximum radius) as measured from
	// a local center point.
	b2Vec2 d = m_localPosition - center;
	m_sweepRadius = d.Length() + m_radius - b2_toiSlop;
}

bool b2CircleShape::TestPoint(const b2XForm& transform, const b2Vec2& p) const
{
	b2Vec2 center = transform.position + b2Mul(transform.R, m_localPosition);
	b2Vec2 d = p - center;
	return b2Dot(d, d) <= m_radius * m_radius;
}

// Collision Detection in Interactive 3D Environments by Gino van den Bergen
// From Section 3.1.2
// x = s + a * r
// norm(x) = radius
bool b2CircleShape::TestSegment(const b2XForm& transform,
								float32* lambda,
								b2Vec2* normal,
								const b2Segment& segment,
								float32 maxLambda) const
{
	b2Vec2 position = transform.position + b2Mul(transform.R, m_localPosition);
	b2Vec2 s = segment.p1 - position;
	float32 b = b2Dot(s, s) - m_radius * m_radius;

	// Does the segment start inside the circle?
	if (b < 0.0f)
	{
		return false;
	}

	// Solve quadratic equation.
	b2Vec2 r = segment.p2 - segment.p1;
	float32 c =  b2Dot(s, r);
	float32 rr = b2Dot(r, r);
	float32 sigma = c * c - rr * b;

	// Check for negative discriminant and short segment.
	if (sigma < 0.0f || rr < B2_FLT_EPSILON)
	{
		return false;
	}

	// Find the point of intersection of the line with the circle.
	float32 a = -(c + b2Sqrt(sigma));

	// Is the intersection point on the segment?
	if (0.0f <= a && a <= maxLambda * rr)
	{
		a /= rr;
		*lambda = a;
		*normal = s + a * r;
		normal->Normalize();
		return true;
	}

	return false;
}

void b2CircleShape::ComputeAABB(b2AABB* aabb, const b2XForm& transform) const
{
	b2Vec2 p = transform.position + b2Mul(transform.R, m_localPosition);
	aabb->lowerBound.Set(p.x - m_radius, p.y - m_radius);
	aabb->upperBound.Set(p.x + m_radius, p.y + m_radius);
}

void b2CircleShape::ComputeSweptAABB(b2AABB* aabb, const b2XForm& transform1, const b2XForm& transform2) const
{
	b2Vec2 p1 = transform1.position + b2Mul(transform1.R, m_localPosition);
	b2Vec2 p2 = transform2.position + b2Mul(transform2.R, m_localPosition);
	b2Vec2 lower = b2Min(p1, p2);
	b2Vec2 upper = b2Max(p1, p2);

	aabb->lowerBound.Set(lower.x - m_radius, lower.y - m_radius);
	aabb->upperBound.Set(upper.x + m_radius, upper.y + m_radius);
}

void b2CircleShape::ComputeMass(b2MassData* massData) const
{
	massData->mass = m_density * b2_pi * m_radius * m_radius;
	massData->center = m_localPosition;

	// inertia about the local origin
	massData->I = massData->mass * (0.5f * m_radius * m_radius + b2Dot(m_localPosition, m_localPosition));
}
