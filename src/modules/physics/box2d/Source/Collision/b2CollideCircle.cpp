/*
* Copyright (c) 2007 Erin Catto http://www.gphysics.com
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

#include "b2Collision.h"
#include "Shapes/b2CircleShape.h"
#include "Shapes/b2PolygonShape.h"

void b2CollideCircles(
	b2Manifold* manifold,
	const b2CircleShape* circle1, const b2XForm& xf1,
	const b2CircleShape* circle2, const b2XForm& xf2)
{
	manifold->pointCount = 0;

	b2Vec2 p1 = b2Mul(xf1, circle1->GetLocalPosition());
	b2Vec2 p2 = b2Mul(xf2, circle2->GetLocalPosition());

	b2Vec2 d = p2 - p1;
	float32 distSqr = b2Dot(d, d);
	float32 r1 = circle1->GetRadius();
	float32 r2 = circle2->GetRadius();
	float32 radiusSum = r1 + r2;
	if (distSqr > radiusSum * radiusSum)
	{
		return;
	}

	float32 separation;
	if (distSqr < B2_FLT_EPSILON)
	{
		separation = -radiusSum;
		manifold->normal.Set(0.0f, 1.0f);
	}
	else
	{
		float32 dist = b2Sqrt(distSqr);
		separation = dist - radiusSum;
		float32 a = 1.0f / dist;
		manifold->normal.x = a * d.x;
		manifold->normal.y = a * d.y;
	}

	manifold->pointCount = 1;
	manifold->points[0].id.key = 0;
	manifold->points[0].separation = separation;

	p1 += r1 * manifold->normal;
	p2 -= r2 * manifold->normal;

	b2Vec2 p = 0.5f * (p1 + p2);

	manifold->points[0].localPoint1 = b2MulT(xf1, p);
	manifold->points[0].localPoint2 = b2MulT(xf2, p);
}

void b2CollidePolygonAndCircle(
	b2Manifold* manifold,
	const b2PolygonShape* polygon, const b2XForm& xf1,
	const b2CircleShape* circle, const b2XForm& xf2)
{
	manifold->pointCount = 0;

	// Compute circle position in the frame of the polygon.
	b2Vec2 c = b2Mul(xf2, circle->GetLocalPosition());
	b2Vec2 cLocal = b2MulT(xf1, c);

	// Find the min separating edge.
	int32 normalIndex = 0;
	float32 separation = -B2_FLT_MAX;
	float32 radius = circle->GetRadius();
	int32 vertexCount = polygon->GetVertexCount();
	const b2Vec2* vertices = polygon->GetVertices();
	const b2Vec2* normals = polygon->GetNormals();

	for (int32 i = 0; i < vertexCount; ++i)
	{
		float32 s = b2Dot(normals[i], cLocal - vertices[i]);

		if (s > radius)
		{
			// Early out.
			return;
		}

		if (s > separation)
		{
			separation = s;
			normalIndex = i;
		}
	}

	// If the center is inside the polygon ...
	if (separation < B2_FLT_EPSILON)
	{
		manifold->pointCount = 1;
		manifold->normal = b2Mul(xf1.R, normals[normalIndex]);
		manifold->points[0].id.features.incidentEdge = (uint8)normalIndex;
		manifold->points[0].id.features.incidentVertex = b2_nullFeature;
		manifold->points[0].id.features.referenceEdge = 0;
		manifold->points[0].id.features.flip = 0;
		b2Vec2 position = c - radius * manifold->normal;
		manifold->points[0].localPoint1 = b2MulT(xf1, position);
		manifold->points[0].localPoint2 = b2MulT(xf2, position);
		manifold->points[0].separation = separation - radius;
		return;
	}

	// Project the circle center onto the edge segment.
	int32 vertIndex1 = normalIndex;
	int32 vertIndex2 = vertIndex1 + 1 < vertexCount ? vertIndex1 + 1 : 0;
	b2Vec2 e = vertices[vertIndex2] - vertices[vertIndex1];

	float32 length = e.Normalize();
	b2Assert(length > B2_FLT_EPSILON);

	// Project the center onto the edge.
	float32 u = b2Dot(cLocal - vertices[vertIndex1], e);
	b2Vec2 p;
	if (u <= 0.0f)
	{
		p = vertices[vertIndex1];
		manifold->points[0].id.features.incidentEdge = b2_nullFeature;
		manifold->points[0].id.features.incidentVertex = (uint8)vertIndex1;
	}
	else if (u >= length)
	{
		p = vertices[vertIndex2];
		manifold->points[0].id.features.incidentEdge = b2_nullFeature;
		manifold->points[0].id.features.incidentVertex = (uint8)vertIndex2;
	}
	else
	{
		p = vertices[vertIndex1] + u * e;
		manifold->points[0].id.features.incidentEdge = (uint8)normalIndex;
		manifold->points[0].id.features.incidentVertex = 0;
	}

	b2Vec2 d = cLocal - p;
	float32 dist = d.Normalize();
	if (dist > radius)
	{
		return;
	}

	manifold->pointCount = 1;
	manifold->normal = b2Mul(xf1.R, d);
	b2Vec2 position = c - radius * manifold->normal;
	manifold->points[0].localPoint1 = b2MulT(xf1, position);
	manifold->points[0].localPoint2 = b2MulT(xf2, position);
	manifold->points[0].separation = dist - radius;
	manifold->points[0].id.features.referenceEdge = 0;
	manifold->points[0].id.features.flip = 0;
}
