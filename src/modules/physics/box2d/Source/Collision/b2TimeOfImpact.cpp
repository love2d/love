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
#include "Shapes/b2Shape.h"

// This algorithm uses conservative advancement to compute the time of
// impact (TOI) of two shapes.
// Refs: Bullet, Young Kim
float32 b2TimeOfImpact(const b2Shape* shape1, const b2Sweep& sweep1,
					   const b2Shape* shape2, const b2Sweep& sweep2)
{
	float32 r1 = shape1->GetSweepRadius();
	float32 r2 = shape2->GetSweepRadius();

	b2Assert(sweep1.t0 == sweep2.t0);
	b2Assert(1.0f - sweep1.t0 > B2_FLT_EPSILON);

	float32 t0 = sweep1.t0;
	b2Vec2 v1 = sweep1.c - sweep1.c0;
	b2Vec2 v2 = sweep2.c - sweep2.c0;
	float32 omega1 = sweep1.a - sweep1.a0;
	float32 omega2 = sweep2.a - sweep2.a0;

	float32 alpha = 0.0f;

	b2Vec2 p1, p2;
	const int32 k_maxIterations = 20;	// TODO_ERIN b2Settings
	int32 iter = 0;
	b2Vec2 normal = b2Vec2_zero;
	float32 distance = 0.0f;
	float32 targetDistance = 0.0f;
	for(;;)
	{
		float32 t = (1.0f - alpha) * t0 + alpha;
		b2XForm xf1, xf2;
		sweep1.GetXForm(&xf1, t);
		sweep2.GetXForm(&xf2, t);

		// Get the distance between shapes.
		distance = b2Distance(&p1, &p2, shape1, xf1, shape2, xf2);

		if (iter == 0)
		{
			// Compute a reasonable target distance to give some breathing room
			// for conservative advancement.
			if (distance > 2.0f * b2_toiSlop)
			{
				targetDistance = 1.5f * b2_toiSlop;
			}
			else
			{
				targetDistance = b2Max(0.05f * b2_toiSlop, distance - 0.5f * b2_toiSlop);
			}
		}

		if (distance - targetDistance < 0.05f * b2_toiSlop || iter == k_maxIterations)
		{
			break;
		}

		normal = p2 - p1;
		normal.Normalize();

		// Compute upper bound on remaining movement.
		float32 approachVelocityBound = b2Dot(normal, v1 - v2) + b2Abs(omega1) * r1 + b2Abs(omega2) * r2;
		if (b2Abs(approachVelocityBound) < B2_FLT_EPSILON)
		{
			alpha = 1.0f;
			break;
		}

		// Get the conservative time increment. Don't advance all the way.
		float32 dAlpha = (distance - targetDistance) / approachVelocityBound;
		//float32 dt = (distance - 0.5f * b2_linearSlop) / approachVelocityBound;
		float32 newAlpha = alpha + dAlpha;

		// The shapes may be moving apart or a safe distance apart.
		if (newAlpha < 0.0f || 1.0f < newAlpha)
		{
			alpha = 1.0f;
			break;
		}

		// Ensure significant advancement.
		if (newAlpha < (1.0f + 100.0f * B2_FLT_EPSILON) * alpha)
		{
			break;
		}

		alpha = newAlpha;

		++iter;
	}

	return alpha;
}
