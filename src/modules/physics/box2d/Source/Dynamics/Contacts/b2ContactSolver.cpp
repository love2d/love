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

#include "b2ContactSolver.h"
#include "b2Contact.h"
#include "../b2Body.h"
#include "../b2World.h"
#include "../../Common/b2StackAllocator.h"

b2ContactSolver::b2ContactSolver(const b2TimeStep& step, b2Contact** contacts, int32 contactCount, b2StackAllocator* allocator)
{
	m_step = step;
	m_allocator = allocator;

	m_constraintCount = 0;
	for (int32 i = 0; i < contactCount; ++i)
	{
		b2Assert(contacts[i]->IsSolid());
		m_constraintCount += contacts[i]->GetManifoldCount();
	}

	m_constraints = (b2ContactConstraint*)m_allocator->Allocate(m_constraintCount * sizeof(b2ContactConstraint));

	int32 count = 0;
	for (int32 i = 0; i < contactCount; ++i)
	{
		b2Contact* contact = contacts[i];

		b2Body* b1 = contact->m_shape1->GetBody();
		b2Body* b2 = contact->m_shape2->GetBody();
		int32 manifoldCount = contact->GetManifoldCount();
		b2Manifold* manifolds = contact->GetManifolds();
		float32 friction = contact->m_friction;
		float32 restitution = contact->m_restitution;

		b2Vec2 v1 = b1->m_linearVelocity;
		b2Vec2 v2 = b2->m_linearVelocity;
		float32 w1 = b1->m_angularVelocity;
		float32 w2 = b2->m_angularVelocity;

		for (int32 j = 0; j < manifoldCount; ++j)
		{
			b2Manifold* manifold = manifolds + j;

			b2Assert(manifold->pointCount > 0);

			const b2Vec2 normal = manifold->normal;

			b2Assert(count < m_constraintCount);
			b2ContactConstraint* c = m_constraints + count;
			c->body1 = b1;
			c->body2 = b2;
			c->manifold = manifold;
			c->normal = normal;
			c->pointCount = manifold->pointCount;
			c->friction = friction;
			c->restitution = restitution;

			for (int32 k = 0; k < c->pointCount; ++k)
			{
				b2ManifoldPoint* cp = manifold->points + k;
				b2ContactConstraintPoint* ccp = c->points + k;

				ccp->normalImpulse = cp->normalImpulse;
				ccp->tangentImpulse = cp->tangentImpulse;
				ccp->separation = cp->separation;
				ccp->positionImpulse = 0.0f;

				ccp->localAnchor1 = cp->localPoint1;
				ccp->localAnchor2 = cp->localPoint2;
				ccp->r1 = b2Mul(b1->GetXForm().R, cp->localPoint1 - b1->GetLocalCenter());
				ccp->r2 = b2Mul(b2->GetXForm().R, cp->localPoint2 - b2->GetLocalCenter());

				float32 r1Sqr = b2Dot(ccp->r1, ccp->r1);
				float32 r2Sqr = b2Dot(ccp->r2, ccp->r2);
				float32 rn1 = b2Dot(ccp->r1, normal);
				float32 rn2 = b2Dot(ccp->r2, normal);

				float32 kNormal = b1->m_invMass + b2->m_invMass;
				kNormal += b1->m_invI * (r1Sqr - rn1 * rn1) + b2->m_invI * (r2Sqr - rn2 * rn2);

				b2Assert(kNormal > B2_FLT_EPSILON);
				ccp->normalMass = 1.0f / kNormal;

				float32 kEqualized = b1->m_mass * b1->m_invMass + b2->m_mass * b2->m_invMass;
				kEqualized += b1->m_mass * b1->m_invI * (r1Sqr - rn1 * rn1) + b2->m_mass * b2->m_invI * (r2Sqr - rn2 * rn2);

				b2Assert(kEqualized > B2_FLT_EPSILON);
				ccp->equalizedMass = 1.0f / kEqualized;

				b2Vec2 tangent = b2Cross(normal, 1.0f);

				float32 rt1 = b2Dot(ccp->r1, tangent);
				float32 rt2 = b2Dot(ccp->r2, tangent);
				float32 kTangent = b1->m_invMass + b2->m_invMass;
				kTangent += b1->m_invI * (r1Sqr - rt1 * rt1) + b2->m_invI * (r2Sqr - rt2 * rt2);

				b2Assert(kTangent > B2_FLT_EPSILON);
				ccp->tangentMass = 1.0f /  kTangent;

				// Setup a velocity bias for restitution.
				ccp->velocityBias = 0.0f;
				if (ccp->separation > 0.0f)
				{
					ccp->velocityBias = -60.0f * ccp->separation; // TODO_ERIN b2TimeStep
				}

				float32 vRel = b2Dot(c->normal, v2 + b2Cross(w2, ccp->r2) - v1 - b2Cross(w1, ccp->r1));
				if (vRel < -b2_velocityThreshold)
				{
					ccp->velocityBias += -c->restitution * vRel;
				}
			}

			++count;
		}
	}

	b2Assert(count == m_constraintCount);
}

b2ContactSolver::~b2ContactSolver()
{
	m_allocator->Free(m_constraints);
}

void b2ContactSolver::InitVelocityConstraints(const b2TimeStep& step)
{
	// Warm start.
	for (int32 i = 0; i < m_constraintCount; ++i)
	{
		b2ContactConstraint* c = m_constraints + i;

		b2Body* b1 = c->body1;
		b2Body* b2 = c->body2;
		float32 invMass1 = b1->m_invMass;
		float32 invI1 = b1->m_invI;
		float32 invMass2 = b2->m_invMass;
		float32 invI2 = b2->m_invI;
		b2Vec2 normal = c->normal;
		b2Vec2 tangent = b2Cross(normal, 1.0f);

		if (step.warmStarting)
		{
			for (int32 j = 0; j < c->pointCount; ++j)
			{
				b2ContactConstraintPoint* ccp = c->points + j;
				ccp->normalImpulse *= step.dtRatio;
				ccp->tangentImpulse *= step.dtRatio;
				b2Vec2 P = ccp->normalImpulse * normal + ccp->tangentImpulse * tangent;
				b1->m_angularVelocity -= invI1 * b2Cross(ccp->r1, P);
				b1->m_linearVelocity -= invMass1 * P;
				b2->m_angularVelocity += invI2 * b2Cross(ccp->r2, P);
				b2->m_linearVelocity += invMass2 * P;
			}
		}
		else
		{
			for (int32 j = 0; j < c->pointCount; ++j)
			{
				b2ContactConstraintPoint* ccp = c->points + j;
				ccp->normalImpulse = 0.0f;
				ccp->tangentImpulse = 0.0f;
			}
		}
	}
}

void b2ContactSolver::SolveVelocityConstraints()
{
	for (int32 i = 0; i < m_constraintCount; ++i)
	{
		b2ContactConstraint* c = m_constraints + i;
		b2Body* b1 = c->body1;
		b2Body* b2 = c->body2;
		float32 w1 = b1->m_angularVelocity;
		float32 w2 = b2->m_angularVelocity;
		b2Vec2 v1 = b1->m_linearVelocity;
		b2Vec2 v2 = b2->m_linearVelocity;
		float32 invMass1 = b1->m_invMass;
		float32 invI1 = b1->m_invI;
		float32 invMass2 = b2->m_invMass;
		float32 invI2 = b2->m_invI;
		b2Vec2 normal = c->normal;
		b2Vec2 tangent = b2Cross(normal, 1.0f);
		float32 friction = c->friction;
//#define DEFERRED_UPDATE
#ifdef DEFERRED_UPDATE
		b2Vec2 b1_linearVelocity = b1->m_linearVelocity;
		float32 b1_angularVelocity = b1->m_angularVelocity;
		b2Vec2 b2_linearVelocity = b2->m_linearVelocity;
		float32 b2_angularVelocity = b2->m_angularVelocity;
#endif
		// Solve normal constraints
		for (int32 j = 0; j < c->pointCount; ++j)
		{
			b2ContactConstraintPoint* ccp = c->points + j;

			// Relative velocity at contact
			b2Vec2 dv = v2 + b2Cross(w2, ccp->r2) - v1 - b2Cross(w1, ccp->r1);

			// Compute normal impulse
			float32 vn = b2Dot(dv, normal);
			float32 lambda = -ccp->normalMass * (vn - ccp->velocityBias);

			// b2Clamp the accumulated impulse
			float32 newImpulse = b2Max(ccp->normalImpulse + lambda, 0.0f);
			lambda = newImpulse - ccp->normalImpulse;

			// Apply contact impulse
			b2Vec2 P = lambda * normal;
#ifdef DEFERRED_UPDATE
			b1_linearVelocity -= invMass1 * P;
			b1_angularVelocity -= invI1 * b2Cross(r1, P);

			b2_linearVelocity += invMass2 * P;
			b2_angularVelocity += invI2 * b2Cross(r2, P);
#else
			v1 -= invMass1 * P;
			w1 -= invI1 * b2Cross(ccp->r1, P);

			v2 += invMass2 * P;
			w2 += invI2 * b2Cross(ccp->r2, P);
#endif
			ccp->normalImpulse = newImpulse;
		}

#ifdef DEFERRED_UPDATE
		b1->m_linearVelocity = b1_linearVelocity;
		b1->m_angularVelocity = b1_angularVelocity;
		b2->m_linearVelocity = b2_linearVelocity;
		b2->m_angularVelocity = b2_angularVelocity;
#endif
		// Solve tangent constraints
		for (int32 j = 0; j < c->pointCount; ++j)
		{
			b2ContactConstraintPoint* ccp = c->points + j;

			// Relative velocity at contact
			b2Vec2 dv = v2 + b2Cross(w2, ccp->r2) - v1 - b2Cross(w1, ccp->r1);

			// Compute tangent force
			float32 vt = b2Dot(dv, tangent);
			float32 lambda = ccp->tangentMass * (-vt);

			// b2Clamp the accumulated force
			float32 maxFriction = friction * ccp->normalImpulse;
			float32 newImpulse = b2Clamp(ccp->tangentImpulse + lambda, -maxFriction, maxFriction);
			lambda = newImpulse - ccp->tangentImpulse;

			// Apply contact impulse
			b2Vec2 P = lambda * tangent;

			v1 -= invMass1 * P;
			w1 -= invI1 * b2Cross(ccp->r1, P);

			v2 += invMass2 * P;
			w2 += invI2 * b2Cross(ccp->r2, P);

			ccp->tangentImpulse = newImpulse;
		}

		b1->m_linearVelocity = v1;
		b1->m_angularVelocity = w1;
		b2->m_linearVelocity = v2;
		b2->m_angularVelocity = w2;
	}
}

void b2ContactSolver::FinalizeVelocityConstraints()
{
	for (int32 i = 0; i < m_constraintCount; ++i)
	{
		b2ContactConstraint* c = m_constraints + i;
		b2Manifold* m = c->manifold;

		for (int32 j = 0; j < c->pointCount; ++j)
		{
			m->points[j].normalImpulse = c->points[j].normalImpulse;
			m->points[j].tangentImpulse = c->points[j].tangentImpulse;
		}
	}
}

bool b2ContactSolver::SolvePositionConstraints(float32 baumgarte)
{
	float32 minSeparation = 0.0f;

	for (int32 i = 0; i < m_constraintCount; ++i)
	{
		b2ContactConstraint* c = m_constraints + i;
		b2Body* b1 = c->body1;
		b2Body* b2 = c->body2;
		float32 invMass1 = b1->m_mass * b1->m_invMass;
		float32 invI1 = b1->m_mass * b1->m_invI;
		float32 invMass2 = b2->m_mass * b2->m_invMass;
		float32 invI2 = b2->m_mass * b2->m_invI;
		
		b2Vec2 normal = c->normal;

		// Solver normal constraints
		for (int32 j = 0; j < c->pointCount; ++j)
		{
			b2ContactConstraintPoint* ccp = c->points + j;

			b2Vec2 r1 = b2Mul(b1->GetXForm().R, ccp->localAnchor1 - b1->GetLocalCenter());
			b2Vec2 r2 = b2Mul(b2->GetXForm().R, ccp->localAnchor2 - b2->GetLocalCenter());

			b2Vec2 p1 = b1->m_sweep.c + r1;
			b2Vec2 p2 = b2->m_sweep.c + r2;
			b2Vec2 dp = p2 - p1;

			// Approximate the current separation.
			float32 separation = b2Dot(dp, normal) + ccp->separation;

			// Track max constraint error.
			minSeparation = b2Min(minSeparation, separation);

			// Prevent large corrections and allow slop.
			float32 C = baumgarte * b2Clamp(separation + b2_linearSlop, -b2_maxLinearCorrection, 0.0f);

			// Compute normal impulse
			float32 dImpulse = -ccp->equalizedMass * C;

			// b2Clamp the accumulated impulse
			float32 impulse0 = ccp->positionImpulse;
			ccp->positionImpulse = b2Max(impulse0 + dImpulse, 0.0f);
			dImpulse = ccp->positionImpulse - impulse0;

			b2Vec2 impulse = dImpulse * normal;

			b1->m_sweep.c -= invMass1 * impulse;
			b1->m_sweep.a -= invI1 * b2Cross(r1, impulse);
			b1->SynchronizeTransform();

			b2->m_sweep.c += invMass2 * impulse;
			b2->m_sweep.a += invI2 * b2Cross(r2, impulse);
			b2->SynchronizeTransform();
		}
	}

	// We can't expect minSpeparation >= -b2_linearSlop because we don't
	// push the separation above -b2_linearSlop.
	return minSeparation >= -1.5f * b2_linearSlop;
}
