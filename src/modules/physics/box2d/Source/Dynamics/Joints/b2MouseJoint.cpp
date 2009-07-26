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

#include "b2MouseJoint.h"
#include "../b2Body.h"
#include "../b2World.h"

// p = attached point, m = mouse point
// C = p - m
// Cdot = v
//      = v + cross(w, r)
// J = [I r_skew]
// Identity used:
// w k % (rx i + ry j) = w * (-ry i + rx j)

b2MouseJoint::b2MouseJoint(const b2MouseJointDef* def)
: b2Joint(def)
{
	m_target = def->target;
	m_localAnchor = b2MulT(m_body2->GetXForm(), m_target);

	m_maxForce = B2FORCE_INV_SCALE(def->maxForce);
	m_impulse.SetZero();

	float32 mass = m_body2->m_mass;

	// Frequency
	float32 omega = 2.0f * b2_pi * def->frequencyHz;

	// Damping coefficient
	float32 d = 2.0f * mass * def->dampingRatio * omega;

	// Spring stiffness
	float32 k = (def->timeStep * mass) * (omega * omega);

	// magic formulas
	b2Assert(d + k > B2_FLT_EPSILON);
	m_gamma = 1.0f / (d + k);
	m_beta = k / (d + k);
}

void b2MouseJoint::SetTarget(const b2Vec2& target)
{
	if (m_body2->IsSleeping())
	{
		m_body2->WakeUp();
	}
	m_target = target;
}

void b2MouseJoint::InitVelocityConstraints(const b2TimeStep& step)
{
	b2Body* b = m_body2;

	// Compute the effective mass matrix.
	b2Vec2 r = b2Mul(b->GetXForm().R, m_localAnchor - b->GetLocalCenter());

	// K    = [(1/m1 + 1/m2) * eye(2) - skew(r1) * invI1 * skew(r1) - skew(r2) * invI2 * skew(r2)]
	//      = [1/m1+1/m2     0    ] + invI1 * [r1.y*r1.y -r1.x*r1.y] + invI2 * [r1.y*r1.y -r1.x*r1.y]
	//        [    0     1/m1+1/m2]           [-r1.x*r1.y r1.x*r1.x]           [-r1.x*r1.y r1.x*r1.x]
	float32 invMass = b->m_invMass;
	float32 invI = b->m_invI;

	b2Mat22 K1;
	K1.col1.x = invMass;	K1.col2.x = 0.0f;
	K1.col1.y = 0.0f;		K1.col2.y = invMass;

	b2Mat22 K2;
	K2.col1.x =  invI * r.y * r.y;	K2.col2.x = -invI * r.x * r.y;
	K2.col1.y = -invI * r.x * r.y;	K2.col2.y =  invI * r.x * r.x;

	b2Mat22 K = K1 + K2;
	K.col1.x += m_gamma;
	K.col2.y += m_gamma;

	m_mass = K.Invert();

	m_C = b->m_sweep.c + r - m_target;

	// Cheat with some damping
	b->m_angularVelocity *= 0.98f;

	// Warm starting.
	b2Vec2 P = B2FORCE_SCALE(step.dt) * m_impulse;
	b->m_linearVelocity += invMass * P;
	b->m_angularVelocity += invI * b2Cross(r, P);
}

void b2MouseJoint::SolveVelocityConstraints(const b2TimeStep& step)
{
	b2Body* b = m_body2;

	b2Vec2 r = b2Mul(b->GetXForm().R, m_localAnchor - b->GetLocalCenter());

	// Cdot = v + cross(w, r)
	b2Vec2 Cdot = b->m_linearVelocity + b2Cross(b->m_angularVelocity, r);
	b2Vec2 force = -B2FORCE_INV_SCALE(step.inv_dt) * b2Mul(m_mass, Cdot + (m_beta * step.inv_dt) * m_C + B2FORCE_SCALE(step.dt) * (m_gamma * m_impulse));

	b2Vec2 oldForce = m_impulse;
	m_impulse += force;
	float32 forceMagnitude = m_impulse.Length();
	if (forceMagnitude > m_maxForce)
	{
		m_impulse *= m_maxForce / forceMagnitude;
	}
	force = m_impulse - oldForce;

	b2Vec2 P = B2FORCE_SCALE(step.dt) * force;
	b->m_linearVelocity += b->m_invMass * P;
	b->m_angularVelocity += b->m_invI * b2Cross(r, P);
}

b2Vec2 b2MouseJoint::GetAnchor1() const
{
	return m_target;
}

b2Vec2 b2MouseJoint::GetAnchor2() const
{
	return m_body2->GetWorldPoint(m_localAnchor);
}

b2Vec2 b2MouseJoint::GetReactionForce() const
{
	return B2FORCE_SCALE(float32(1.0))*m_impulse;
}

float32 b2MouseJoint::GetReactionTorque() const
{
	return 0.0f;
}
