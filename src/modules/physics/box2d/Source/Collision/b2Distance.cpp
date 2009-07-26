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

int32 g_GJK_Iterations = 0;

// GJK using Voronoi regions (Christer Ericson) and region selection
// optimizations (Casey Muratori).

// The origin is either in the region of points[1] or in the edge region. The origin is
// not in region of points[0] because that is the old point.
static int32 ProcessTwo(b2Vec2* x1, b2Vec2* x2, b2Vec2* p1s, b2Vec2* p2s, b2Vec2* points)
{
	// If in point[1] region
	b2Vec2 r = -points[1];
	b2Vec2 d = points[0] - points[1];
	float32 length = d.Normalize();
	float32 lambda = b2Dot(r, d);
	if (lambda <= 0.0f || length < B2_FLT_EPSILON)
	{
		// The simplex is reduced to a point.
		*x1 = p1s[1];
		*x2 = p2s[1];
		p1s[0] = p1s[1];
		p2s[0] = p2s[1];
		points[0] = points[1];
		return 1;
	}

	// Else in edge region
	lambda /= length;
	*x1 = p1s[1] + lambda * (p1s[0] - p1s[1]);
	*x2 = p2s[1] + lambda * (p2s[0] - p2s[1]);
	return 2;
}

// Possible regions:
// - points[2]
// - edge points[0]-points[2]
// - edge points[1]-points[2]
// - inside the triangle
static int32 ProcessThree(b2Vec2* x1, b2Vec2* x2, b2Vec2* p1s, b2Vec2* p2s, b2Vec2* points)
{
	b2Vec2 a = points[0];
	b2Vec2 b = points[1];
	b2Vec2 c = points[2];

	b2Vec2 ab = b - a;
	b2Vec2 ac = c - a;
	b2Vec2 bc = c - b;

	float32 sn = -b2Dot(a, ab), sd = b2Dot(b, ab);
	float32 tn = -b2Dot(a, ac), td = b2Dot(c, ac);
	float32 un = -b2Dot(b, bc), ud = b2Dot(c, bc);

	// In vertex c region?
	if (td <= 0.0f && ud <= 0.0f)
	{
		// Single point
		*x1 = p1s[2];
		*x2 = p2s[2];
		p1s[0] = p1s[2];
		p2s[0] = p2s[2];
		points[0] = points[2];
		return 1;
	}

	// Should not be in vertex a or b region.
	B2_NOT_USED(sd);
	B2_NOT_USED(sn);
	b2Assert(sn > 0.0f || tn > 0.0f);
	b2Assert(sd > 0.0f || un > 0.0f);

	float32 n = b2Cross(ab, ac);

#ifdef TARGET_FLOAT32_IS_FIXED
	n = (n < 0.0)? -1.0 : ((n > 0.0)? 1.0 : 0.0);
#endif

	// Should not be in edge ab region.
	float32 vc = n * b2Cross(a, b);
	b2Assert(vc > 0.0f || sn > 0.0f || sd > 0.0f);

	// In edge bc region?
	float32 va = n * b2Cross(b, c);
	if (va <= 0.0f && un >= 0.0f && ud >= 0.0f && (un+ud) > 0.0f)
	{
		b2Assert(un + ud > 0.0f);
		float32 lambda = un / (un + ud);
		*x1 = p1s[1] + lambda * (p1s[2] - p1s[1]);
		*x2 = p2s[1] + lambda * (p2s[2] - p2s[1]);
		p1s[0] = p1s[2];
		p2s[0] = p2s[2];
		points[0] = points[2];
		return 2;
	}

	// In edge ac region?
	float32 vb = n * b2Cross(c, a);
	if (vb <= 0.0f && tn >= 0.0f && td >= 0.0f && (tn+td) > 0.0f)
	{
		b2Assert(tn + td > 0.0f);
		float32 lambda = tn / (tn + td);
		*x1 = p1s[0] + lambda * (p1s[2] - p1s[0]);
		*x2 = p2s[0] + lambda * (p2s[2] - p2s[0]);
		p1s[1] = p1s[2];
		p2s[1] = p2s[2];
		points[1] = points[2];
		return 2;
	}

	// Inside the triangle, compute barycentric coordinates
	float32 denom = va + vb + vc;
	b2Assert(denom > 0.0f);
	denom = 1.0f / denom;

#ifdef TARGET_FLOAT32_IS_FIXED
	*x1 = denom * (va * p1s[0] + vb * p1s[1] + vc * p1s[2]);
	*x2 = denom * (va * p2s[0] + vb * p2s[1] + vc * p2s[2]);
#else
	float32 u = va * denom;
	float32 v = vb * denom;
	float32 w = 1.0f - u - v;
	*x1 = u * p1s[0] + v * p1s[1] + w * p1s[2];
	*x2 = u * p2s[0] + v * p2s[1] + w * p2s[2];
#endif
	return 3;
}

static bool InPoints(const b2Vec2& w, const b2Vec2* points, int32 pointCount)
{
	const float32 k_tolerance = 100.0f * B2_FLT_EPSILON;
	for (int32 i = 0; i < pointCount; ++i)
	{
		b2Vec2 d = b2Abs(w - points[i]);
		b2Vec2 m = b2Max(b2Abs(w), b2Abs(points[i]));
		
		if (d.x < k_tolerance * (m.x + 1.0f) &&
			d.y < k_tolerance * (m.y + 1.0f))
		{
			return true;
		}
	}

	return false;
}

template <typename T1, typename T2>
float32 DistanceGeneric(b2Vec2* x1, b2Vec2* x2,
				   const T1* shape1, const b2XForm& xf1,
				   const T2* shape2, const b2XForm& xf2)
{
	b2Vec2 p1s[3], p2s[3];
	b2Vec2 points[3];
	int32 pointCount = 0;

	*x1 = shape1->GetFirstVertex(xf1);
	*x2 = shape2->GetFirstVertex(xf2);

	float32 vSqr = 0.0f;
	const int32 maxIterations = 20;
	for (int32 iter = 0; iter < maxIterations; ++iter)
	{
		b2Vec2 v = *x2 - *x1;
		b2Vec2 w1 = shape1->Support(xf1, v);
		b2Vec2 w2 = shape2->Support(xf2, -v);

		vSqr = b2Dot(v, v);
		b2Vec2 w = w2 - w1;
		float32 vw = b2Dot(v, w);
		if (vSqr - vw <= 0.01f * vSqr || InPoints(w, points, pointCount)) // or w in points
		{
			if (pointCount == 0)
			{
				*x1 = w1;
				*x2 = w2;
			}
			g_GJK_Iterations = iter;
			return b2Sqrt(vSqr);
		}

		switch (pointCount)
		{
		case 0:
			p1s[0] = w1;
			p2s[0] = w2;
			points[0] = w;
			*x1 = p1s[0];
			*x2 = p2s[0];
			++pointCount;
			break;

		case 1:
			p1s[1] = w1;
			p2s[1] = w2;
			points[1] = w;
			pointCount = ProcessTwo(x1, x2, p1s, p2s, points);
			break;

		case 2:
			p1s[2] = w1;
			p2s[2] = w2;
			points[2] = w;
			pointCount = ProcessThree(x1, x2, p1s, p2s, points);
			break;
		}

		// If we have three points, then the origin is in the corresponding triangle.
		if (pointCount == 3)
		{
			g_GJK_Iterations = iter;
			return 0.0f;
		}

		float32 maxSqr = -B2_FLT_MAX;
		for (int32 i = 0; i < pointCount; ++i)
		{
			maxSqr = b2Max(maxSqr, b2Dot(points[i], points[i]));
		}

#ifdef TARGET_FLOAT32_IS_FIXED
		if (pointCount == 3 || vSqr <= 5.0*B2_FLT_EPSILON * maxSqr)
#else
		if (pointCount == 3 || vSqr <= 100.0f * B2_FLT_EPSILON * maxSqr)
#endif
		{
			g_GJK_Iterations = iter;
			v = *x2 - *x1;
			vSqr = b2Dot(v, v);
			return b2Sqrt(vSqr);
		}
	}

	g_GJK_Iterations = maxIterations;
	return b2Sqrt(vSqr);
}

static float32 DistanceCC(
	b2Vec2* x1, b2Vec2* x2,
	const b2CircleShape* circle1, const b2XForm& xf1,
	const b2CircleShape* circle2, const b2XForm& xf2)
{
	b2Vec2 p1 = b2Mul(xf1, circle1->GetLocalPosition());
	b2Vec2 p2 = b2Mul(xf2, circle2->GetLocalPosition());

	b2Vec2 d = p2 - p1;
	float32 dSqr = b2Dot(d, d);
	float32 r1 = circle1->GetRadius() - b2_toiSlop;
	float32 r2 = circle2->GetRadius() - b2_toiSlop;
	float32 r = r1 + r2;
	if (dSqr > r * r)
	{
		float32 dLen = d.Normalize();
		float32 distance = dLen - r;
		*x1 = p1 + r1 * d;
		*x2 = p2 - r2 * d;
		return distance;
	}
	else if (dSqr > B2_FLT_EPSILON * B2_FLT_EPSILON)
	{
		d.Normalize();
		*x1 = p1 + r1 * d;
		*x2 = *x1;
		return 0.0f;
	}

	*x1 = p1;
	*x2 = *x1;
	return 0.0f;
}

// This is used for polygon-vs-circle distance.
struct Point
{
	b2Vec2 Support(const b2XForm&, const b2Vec2&) const
	{
		return p;
	}

	b2Vec2 GetFirstVertex(const b2XForm&) const
	{
		return p;
	}
	
	b2Vec2 p;
};

// GJK is more robust with polygon-vs-point than polygon-vs-circle.
// So we convert polygon-vs-circle to polygon-vs-point.
static float32 DistancePC(
	b2Vec2* x1, b2Vec2* x2,
	const b2PolygonShape* polygon, const b2XForm& xf1,
	const b2CircleShape* circle, const b2XForm& xf2)
{
	Point point;
	point.p = b2Mul(xf2, circle->GetLocalPosition());

	float32 distance = DistanceGeneric(x1, x2, polygon, xf1, &point, b2XForm_identity);

	float32 r = circle->GetRadius() - b2_toiSlop;

	if (distance > r)
	{
		distance -= r;
		b2Vec2 d = *x2 - *x1;
		d.Normalize();
		*x2 -= r * d;
	}
	else
	{
		distance = 0.0f;
		*x2 = *x1;
	}

	return distance;
}

float32 b2Distance(b2Vec2* x1, b2Vec2* x2,
				   const b2Shape* shape1, const b2XForm& xf1,
				   const b2Shape* shape2, const b2XForm& xf2)
{
	b2ShapeType type1 = shape1->GetType();
	b2ShapeType type2 = shape2->GetType();

	if (type1 == e_circleShape && type2 == e_circleShape)
	{
		return DistanceCC(x1, x2, (b2CircleShape*)shape1, xf1, (b2CircleShape*)shape2, xf2);
	}
	
	if (type1 == e_polygonShape && type2 == e_circleShape)
	{
		return DistancePC(x1, x2, (b2PolygonShape*)shape1, xf1, (b2CircleShape*)shape2, xf2);
	}

	if (type1 == e_circleShape && type2 == e_polygonShape)
	{
		return DistancePC(x2, x1, (b2PolygonShape*)shape2, xf2, (b2CircleShape*)shape1, xf1);
	}

	if (type1 == e_polygonShape && type2 == e_polygonShape)
	{
		return DistanceGeneric(x1, x2, (b2PolygonShape*)shape1, xf1, (b2PolygonShape*)shape2, xf2);
	}

	return 0.0f;
}
