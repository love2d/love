
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

#include "b2PolygonShape.h"

void b2PolygonDef::SetAsBox(float32 hx, float32 hy)
{
	vertexCount = 4;
	vertices[0].Set(-hx, -hy);
	vertices[1].Set( hx, -hy);
	vertices[2].Set( hx,  hy);
	vertices[3].Set(-hx,  hy);
}

void b2PolygonDef::SetAsBox(float32 hx, float32 hy, const b2Vec2& center, float32 angle)
{
	SetAsBox(hx, hy);
	b2XForm xf;
	xf.position = center;
	xf.R.Set(angle);

	for (int32 i = 0; i < vertexCount; ++i)
	{
		vertices[i] = b2Mul(xf, vertices[i]);
	}
}

static b2Vec2 ComputeCentroid(const b2Vec2* vs, int32 count)
{
	b2Assert(count >= 3);

	b2Vec2 c; c.Set(0.0f, 0.0f);
	float32 area = 0.0f;

	// pRef is the reference point for forming triangles.
	// It's location doesn't change the result (except for rounding error).
	b2Vec2 pRef(0.0f, 0.0f);
#if 0
	// This code would put the reference point inside the polygon.
	for (int32 i = 0; i < count; ++i)
	{
		pRef += vs[i];
	}
	pRef *= 1.0f / count;
#endif

	const float32 inv3 = 1.0f / 3.0f;

	for (int32 i = 0; i < count; ++i)
	{
		// Triangle vertices.
		b2Vec2 p1 = pRef;
		b2Vec2 p2 = vs[i];
		b2Vec2 p3 = i + 1 < count ? vs[i+1] : vs[0];

		b2Vec2 e1 = p2 - p1;
		b2Vec2 e2 = p3 - p1;

		float32 D = b2Cross(e1, e2);

		float32 triangleArea = 0.5f * D;
		area += triangleArea;

		// Area weighted centroid
		c += triangleArea * inv3 * (p1 + p2 + p3);
	}

	// Centroid
	b2Assert(area > B2_FLT_EPSILON);
	c *= 1.0f / area;
	return c;
}

// http://www.geometrictools.com/Documentation/MinimumAreaRectangle.pdf
static void ComputeOBB(b2OBB* obb, const b2Vec2* vs, int32 count)
{
	b2Assert(count <= b2_maxPolygonVertices);
	b2Vec2 p[b2_maxPolygonVertices + 1];
	for (int32 i = 0; i < count; ++i)
	{
		p[i] = vs[i];
	}
	p[count] = p[0];

	float32 minArea = B2_FLT_MAX;
	
	for (int32 i = 1; i <= count; ++i)
	{
		b2Vec2 root = p[i-1];
		b2Vec2 ux = p[i] - root;
		float32 length = ux.Normalize();
		b2Assert(length > B2_FLT_EPSILON);
		b2Vec2 uy(-ux.y, ux.x);
		b2Vec2 lower(B2_FLT_MAX, B2_FLT_MAX);
		b2Vec2 upper(-B2_FLT_MAX, -B2_FLT_MAX);

		for (int32 j = 0; j < count; ++j)
		{
			b2Vec2 d = p[j] - root;
			b2Vec2 r;
			r.x = b2Dot(ux, d);
			r.y = b2Dot(uy, d);
			lower = b2Min(lower, r);
			upper = b2Max(upper, r);
		}

		float32 area = (upper.x - lower.x) * (upper.y - lower.y);
		if (area < 0.95f * minArea)
		{
			minArea = area;
			obb->R.col1 = ux;
			obb->R.col2 = uy;
			b2Vec2 center = 0.5f * (lower + upper);
			obb->center = root + b2Mul(obb->R, center);
			obb->extents = 0.5f * (upper - lower);
		}
	}

	b2Assert(minArea < B2_FLT_MAX);
}

b2PolygonShape::b2PolygonShape(const b2ShapeDef* def)
	 : b2Shape(def)
{
	b2Assert(def->type == e_polygonShape);
	m_type = e_polygonShape;
	const b2PolygonDef* poly = (const b2PolygonDef*)def;

	// Get the vertices transformed into the body frame.
	m_vertexCount = poly->vertexCount;
	b2Assert(3 <= m_vertexCount && m_vertexCount <= b2_maxPolygonVertices);

	// Copy vertices.
	for (int32 i = 0; i < m_vertexCount; ++i)
	{
		m_vertices[i] = poly->vertices[i];
	}

	// Compute normals. Ensure the edges have non-zero length.
	for (int32 i = 0; i < m_vertexCount; ++i)
	{
		int32 i1 = i;
		int32 i2 = i + 1 < m_vertexCount ? i + 1 : 0;
		b2Vec2 edge = m_vertices[i2] - m_vertices[i1];
		b2Assert(edge.LengthSquared() > B2_FLT_EPSILON * B2_FLT_EPSILON);
		m_normals[i] = b2Cross(edge, 1.0f);
		m_normals[i].Normalize();
	}

#ifdef _DEBUG
	// Ensure the polygon is convex.
	for (int32 i = 0; i < m_vertexCount; ++i)
	{
		for (int32 j = 0; j < m_vertexCount; ++j)
		{
			// Don't check vertices on the current edge.
			if (j == i || j == (i + 1) % m_vertexCount)
			{
				continue;
			}
			
			// Your polygon is non-convex (it has an indentation).
			// Or your polygon is too skinny.
			float32 s = b2Dot(m_normals[i], m_vertices[j] - m_vertices[i]);
			b2Assert(s < -b2_linearSlop);
		}
	}

	// Ensure the polygon is counter-clockwise.
	for (int32 i = 1; i < m_vertexCount; ++i)
	{
		float32 cross = b2Cross(m_normals[i-1], m_normals[i]);

		// Keep asinf happy.
		cross = b2Clamp(cross, -1.0f, 1.0f);

		// You have consecutive edges that are almost parallel on your polygon.
		float32 angle = asinf(cross);
		b2Assert(angle > b2_angularSlop);
	}
#endif

	// Compute the polygon centroid.
	m_centroid = ComputeCentroid(poly->vertices, poly->vertexCount);

	// Compute the oriented bounding box.
	ComputeOBB(&m_obb, m_vertices, m_vertexCount);

	// Create core polygon shape by shifting edges inward.
	// Also compute the min/max radius for CCD.
	for (int32 i = 0; i < m_vertexCount; ++i)
	{
		int32 i1 = i - 1 >= 0 ? i - 1 : m_vertexCount - 1;
		int32 i2 = i;

		b2Vec2 n1 = m_normals[i1];
		b2Vec2 n2 = m_normals[i2];
		b2Vec2 v = m_vertices[i] - m_centroid;;

		b2Vec2 d;
		d.x = b2Dot(n1, v) - b2_toiSlop;
		d.y = b2Dot(n2, v) - b2_toiSlop;

		// Shifting the edge inward by b2_toiSlop should
		// not cause the plane to pass the centroid.

		// Your shape has a radius/extent less than b2_toiSlop.
		b2Assert(d.x >= 0.0f);
		b2Assert(d.y >= 0.0f);
		b2Mat22 A;
		A.col1.x = n1.x; A.col2.x = n1.y;
		A.col1.y = n2.x; A.col2.y = n2.y;
		m_coreVertices[i] = A.Solve(d) + m_centroid;
	}
}

void b2PolygonShape::UpdateSweepRadius(const b2Vec2& center)
{
	// Update the sweep radius (maximum radius) as measured from
	// a local center point.
	m_sweepRadius = 0.0f;
	for (int32 i = 0; i < m_vertexCount; ++i)
	{
		b2Vec2 d = m_coreVertices[i] - center;
		m_sweepRadius = b2Max(m_sweepRadius, d.Length());
	}
}

bool b2PolygonShape::TestPoint(const b2XForm& xf, const b2Vec2& p) const
{
	b2Vec2 pLocal = b2MulT(xf.R, p - xf.position);

	for (int32 i = 0; i < m_vertexCount; ++i)
	{
		float32 dot = b2Dot(m_normals[i], pLocal - m_vertices[i]);
		if (dot > 0.0f)
		{
			return false;
		}
	}

	return true;
}

bool b2PolygonShape::TestSegment(
	const b2XForm& xf,
	float32* lambda,
	b2Vec2* normal,
	const b2Segment& segment,
	float32 maxLambda) const
{
	float32 lower = 0.0f, upper = maxLambda;

	b2Vec2 p1 = b2MulT(xf.R, segment.p1 - xf.position);
	b2Vec2 p2 = b2MulT(xf.R, segment.p2 - xf.position);
	b2Vec2 d = p2 - p1;
	int32 index = -1;

	for (int32 i = 0; i < m_vertexCount; ++i)
	{
		// p = p1 + a * d
		// dot(normal, p - v) = 0
		// dot(normal, p1 - v) + a * dot(normal, d) = 0
		float32 numerator = b2Dot(m_normals[i], m_vertices[i] - p1);
		float32 denominator = b2Dot(m_normals[i], d);

		// Note: we want this predicate without division:
		// lower < numerator / denominator, where denominator < 0
		// Since denominator < 0, we have to flip the inequality:
		// lower < numerator / denominator <==> denominator * lower > numerator.

		if (denominator < 0.0f && numerator < lower * denominator)
		{
			// Increase lower.
			// The segment enters this half-space.
			lower = numerator / denominator;
			index = i;
		}
		else if (denominator > 0.0f && numerator < upper * denominator)
		{
			// Decrease upper.
			// The segment exits this half-space.
			upper = numerator / denominator;
		}

		if (upper < lower)
		{
			return false;
		}
	}

	b2Assert(0.0f <= lower && lower <= maxLambda);

	if (index >= 0)
	{
		*lambda = lower;
		*normal = b2Mul(xf.R, m_normals[index]);
		return true;
	}

	return false;
}

void b2PolygonShape::ComputeAABB(b2AABB* aabb, const b2XForm& xf) const
{
	b2Mat22 R = b2Mul(xf.R, m_obb.R);
	b2Mat22 absR = b2Abs(R);
	b2Vec2 h = b2Mul(absR, m_obb.extents);
	b2Vec2 position = xf.position + b2Mul(xf.R, m_obb.center);
	aabb->lowerBound = position - h;
	aabb->upperBound = position + h;
}

void b2PolygonShape::ComputeSweptAABB(b2AABB* aabb,
					  const b2XForm& transform1,
					  const b2XForm& transform2) const
{
	b2AABB aabb1, aabb2;
	ComputeAABB(&aabb1, transform1);
	ComputeAABB(&aabb2, transform2);
	aabb->lowerBound = b2Min(aabb1.lowerBound, aabb2.lowerBound);
	aabb->upperBound = b2Max(aabb1.upperBound, aabb2.upperBound);
}

void b2PolygonShape::ComputeMass(b2MassData* massData) const
{
	// Polygon mass, centroid, and inertia.
	// Let rho be the polygon density in mass per unit area.
	// Then:
	// mass = rho * int(dA)
	// centroid.x = (1/mass) * rho * int(x * dA)
	// centroid.y = (1/mass) * rho * int(y * dA)
	// I = rho * int((x*x + y*y) * dA)
	//
	// We can compute these integrals by summing all the integrals
	// for each triangle of the polygon. To evaluate the integral
	// for a single triangle, we make a change of variables to
	// the (u,v) coordinates of the triangle:
	// x = x0 + e1x * u + e2x * v
	// y = y0 + e1y * u + e2y * v
	// where 0 <= u && 0 <= v && u + v <= 1.
	//
	// We integrate u from [0,1-v] and then v from [0,1].
	// We also need to use the Jacobian of the transformation:
	// D = cross(e1, e2)
	//
	// Simplification: triangle centroid = (1/3) * (p1 + p2 + p3)
	//
	// The rest of the derivation is handled by computer algebra.

	b2Assert(m_vertexCount >= 3);

	b2Vec2 center; center.Set(0.0f, 0.0f);
	float32 area = 0.0f;
	float32 I = 0.0f;

	// pRef is the reference point for forming triangles.
	// It's location doesn't change the result (except for rounding error).
	b2Vec2 pRef(0.0f, 0.0f);
#if 0
	// This code would put the reference point inside the polygon.
	for (int32 i = 0; i < m_vertexCount; ++i)
	{
		pRef += m_vertices[i];
	}
	pRef *= 1.0f / count;
#endif

	const float32 k_inv3 = 1.0f / 3.0f;

	for (int32 i = 0; i < m_vertexCount; ++i)
	{
		// Triangle vertices.
		b2Vec2 p1 = pRef;
		b2Vec2 p2 = m_vertices[i];
		b2Vec2 p3 = i + 1 < m_vertexCount ? m_vertices[i+1] : m_vertices[0];

		b2Vec2 e1 = p2 - p1;
		b2Vec2 e2 = p3 - p1;

		float32 D = b2Cross(e1, e2);

		float32 triangleArea = 0.5f * D;
		area += triangleArea;

		// Area weighted centroid
		center += triangleArea * k_inv3 * (p1 + p2 + p3);

		float32 px = p1.x, py = p1.y;
		float32 ex1 = e1.x, ey1 = e1.y;
		float32 ex2 = e2.x, ey2 = e2.y;

		float32 intx2 = k_inv3 * (0.25f * (ex1*ex1 + ex2*ex1 + ex2*ex2) + (px*ex1 + px*ex2)) + 0.5f*px*px;
		float32 inty2 = k_inv3 * (0.25f * (ey1*ey1 + ey2*ey1 + ey2*ey2) + (py*ey1 + py*ey2)) + 0.5f*py*py;

		I += D * (intx2 + inty2);
	}

	// Total mass
	massData->mass = m_density * area;

	// Center of mass
	b2Assert(area > B2_FLT_EPSILON);
	center *= 1.0f / area;
	massData->center = center;

	// Inertia tensor relative to the local origin.
	massData->I = m_density * I;
}

b2Vec2 b2PolygonShape::Centroid(const b2XForm& xf) const
{
	return b2Mul(xf, m_centroid);
}

b2Vec2 b2PolygonShape::Support(const b2XForm& xf, const b2Vec2& d) const
{
	b2Vec2 dLocal = b2MulT(xf.R, d);

	int32 bestIndex = 0;
	float32 bestValue = b2Dot(m_coreVertices[0], dLocal);
	for (int32 i = 1; i < m_vertexCount; ++i)
	{
		float32 value = b2Dot(m_coreVertices[i], dLocal);
		if (value > bestValue)
		{
			bestIndex = i;
			bestValue = value;
		}
	}

	return b2Mul(xf, m_coreVertices[bestIndex]);
}
