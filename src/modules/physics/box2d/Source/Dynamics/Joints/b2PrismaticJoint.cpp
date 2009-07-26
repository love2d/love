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

#include "b2PrismaticJoint.h"
#include "../b2Body.h"
#include "../b2World.h"

// Linear constraint (point-to-line)
// d = p2 - p1 = x2 + r2 - x1 - r1
// C = dot(ay1, d)
// Cdot = dot(d, cross(w1, ay1)) + dot(ay1, v2 + cross(w2, r2) - v1 - cross(w1, r1))
//      = -dot(ay1, v1) - dot(cross(d + r1, ay1), w1) + dot(ay1, v2) + dot(cross(r2, ay1), v2)
// J = [-ay1 -cross(d+r1,ay1) ay1 cross(r2,ay1)]
//
// Angular constraint
// C = a2 - a1 + a_initial
// Cdot = w2 - w1
// J = [0 0 -1 0 0 1]

// Motor/Limit linear constraint
// C = dot(ax1, d)
// Cdot = = -dot(ax1, v1) - dot(cross(d + r1, ax1), w1) + dot(ax1, v2) + dot(cross(r2, ax1), v2)
// J = [-ax1 -cross(d+r1,ax1) ax1 cross(r2,ax1)]

void b2PrismaticJointDef::Initialize(b2Body* b1, b2Body* b2, const b2Vec2& anchor, const b2Vec2& axis)
{
	body1 = b1;
	body2 = b2;
	localAnchor1 = body1->GetLocalPoint(anchor);
	localAnchor2 = body2->GetLocalPoint(anchor);
	localAxis1 = body1->GetLocalVector(axis);
	referenceAngle = body2->GetAngle() - body1->GetAngle();
}

b2PrismaticJoint::b2PrismaticJoint(const b2PrismaticJointDef* def)
: b2Joint(def)
{
	m_localAnchor1 = def->localAnchor1;
	m_localAnchor2 = def->localAnchor2;
	m_localXAxis1 = def->localAxis1;
	m_localYAxis1 = b2Cross(1.0f, m_localXAxis1);
	m_refAngle = def->referenceAngle;

	m_linearJacobian.SetZero();
	m_linearMass = 0.0f;
	m_force = 0.0f;

	m_angularMass = 0.0f;
	m_torque = 0.0f;

	m_motorJacobian.SetZero();
	m_motorMass = 0.0;
	m_motorForce = 0.0f;
	m_limitForce = 0.0f;
	m_limitPositionImpulse = 0.0f;

	m_lowerTranslation = def->lowerTranslation;
	m_upperTranslation = def->upperTranslation;
	m_maxMotorForce = B2FORCE_INV_SCALE(def->maxMotorForce);
	m_motorSpeed = def->motorSpeed;
	m_enableLimit = def->enableLimit;
	m_enableMotor = def->enableMotor;
}

void b2PrismaticJoint::InitVelocityConstraints(const b2TimeStep& step)
{
	b2Body* b1 = m_body1;
	b2Body* b2 = m_body2;

	// Compute the effective masses.
	b2Vec2 r1 = b2Mul(b1->GetXForm().R, m_localAnchor1 - b1->GetLocalCenter());
	b2Vec2 r2 = b2Mul(b2->GetXForm().R, m_localAnchor2 - b2->GetLocalCenter());

	float32 invMass1 = b1->m_invMass, invMass2 = b2->m_invMass;
	float32 invI1 = b1->m_invI, invI2 = b2->m_invI;

	// Compute point to line constraint effective mass.
	// J = [-ay1 -cross(d+r1,ay1) ay1 cross(r2,ay1)]
	b2Vec2 ay1 = b2Mul(b1->GetXForm().R, m_localYAxis1);
	b2Vec2 e = b2->m_sweep.c + r2 - b1->m_sweep.c;	// e = d + r1

	m_linearJacobian.Set(-ay1, -b2Cross(e, ay1), ay1, b2Cross(r2, ay1));
	m_linearMass =	invMass1 + invI1 * m_linearJacobian.angular1 * m_linearJacobian.angular1 +
					invMass2 + invI2 * m_linearJacobian.angular2 * m_linearJacobian.angular2;
	b2Assert(m_linearMass > B2_FLT_EPSILON);
	m_linearMass = 1.0f / m_linearMass;

	// Compute angular constraint effective mass.
	m_angularMass = invI1 + invI2;
	if (m_angularMass > B2_FLT_EPSILON)
	{
		m_angularMass = 1.0f / m_angularMass;
	}

	// Compute motor and limit terms.
	if (m_enableLimit || m_enableMotor)
	{
		// The motor and limit share a Jacobian and effective mass.
		b2Vec2 ax1 = b2Mul(b1->GetXForm().R, m_localXAxis1);
		m_motorJacobian.Set(-ax1, -b2Cross(e, ax1), ax1, b2Cross(r2, ax1));
		m_motorMass =	invMass1 + invI1 * m_motorJacobian.angular1 * m_motorJacobian.angular1 +
						invMass2 + invI2 * m_motorJacobian.angular2 * m_motorJacobian.angular2;
		b2Assert(m_motorMass > B2_FLT_EPSILON);
		m_motorMass = 1.0f / m_motorMass;

		if (m_enableLimit)
		{
			b2Vec2 d = e - r1;	// p2 - p1
			float32 jointTranslation = b2Dot(ax1, d);
			if (b2Abs(m_upperTranslation - m_lowerTranslation) < 2.0f * b2_linearSlop)
			{
				m_limitState = e_equalLimits;
			}
			else if (jointTranslation <= m_lowerTranslation)
			{
				if (m_limitState != e_atLowerLimit)
				{
					m_limitForce = 0.0f;
				}
				m_limitState = e_atLowerLimit;
			}
			else if (jointTranslation >= m_upperTranslation)
			{
				if (m_limitState != e_atUpperLimit)
				{
					m_limitForce = 0.0f;
				}
				m_limitState = e_atUpperLimit;
			}
			else
			{
				m_limitState = e_inactiveLimit;
				m_limitForce = 0.0f;
			}
		}
	}

	if (m_enableMotor == false)
	{
		m_motorForce = 0.0f;
	}

	if (m_enableLimit == false)
	{
		m_limitForce = 0.0f;
	}

	if (step.warmStarting)
	{
		b2Vec2 P1 = B2FORCE_SCALE(step.dt) * (m_force * m_linearJacobian.linear1 + (m_motorForce + m_limitForce) * m_motorJacobian.linear1);
		b2Vec2 P2 = B2FORCE_SCALE(step.dt) * (m_force * m_linearJacobian.linear2 + (m_motorForce + m_limitForce) * m_motorJacobian.linear2);
		float32 L1 = B2FORCE_SCALE(step.dt) * (m_force * m_linearJacobian.angular1 - m_torque + (m_motorForce + m_limitForce) * m_motorJacobian.angular1);
		float32 L2 = B2FORCE_SCALE(step.dt) * (m_force * m_linearJacobian.angular2 + m_torque + (m_motorForce + m_limitForce) * m_motorJacobian.angular2);

		b1->m_linearVelocity += invMass1 * P1;
		b1->m_angularVelocity += invI1 * L1;

		b2->m_linearVelocity += invMass2 * P2;
		b2->m_angularVelocity += invI2 * L2;
	}
	else
	{
		m_force = 0.0f;
		m_torque = 0.0f;
		m_limitForce = 0.0f;
		m_motorForce = 0.0f;
	}

	m_limitPositionImpulse = 0.0f;
}

void b2PrismaticJoint::SolveVelocityConstraints(const b2TimeStep& step)
{
	b2Body* b1 = m_body1;
	b2Body* b2 = m_body2;

	float32 invMass1 = b1->m_invMass, invMass2 = b2->m_invMass;
	float32 invI1 = b1->m_invI, invI2 = b2->m_invI;

	// Solve linear constraint.
	float32 linearCdot = m_linearJacobian.Compute(b1->m_linearVelocity, b1->m_angularVelocity, b2->m_linearVelocity, b2->m_angularVelocity);
	float32 force = -B2FORCE_INV_SCALE(step.inv_dt) * m_linearMass * linearCdot;
	m_force += force;

	float32 P = B2FORCE_SCALE(step.dt) * force;
	b1->m_linearVelocity += (invMass1 * P) * m_linearJacobian.linear1;
	b1->m_angularVelocity += invI1 * P * m_linearJacobian.angular1;

	b2->m_linearVelocity += (invMass2 * P) * m_linearJacobian.linear2;
	b2->m_angularVelocity += invI2 * P * m_linearJacobian.angular2;

	// Solve angular constraint.
	float32 angularCdot = b2->m_angularVelocity - b1->m_angularVelocity;
	float32 torque = -B2FORCE_INV_SCALE(step.inv_dt) * m_angularMass * angularCdot;
	m_torque += torque;

	float32 L = B2FORCE_SCALE(step.dt) * torque;
	b1->m_angularVelocity -= invI1 * L;
	b2->m_angularVelocity += invI2 * L;

	// Solve linear motor constraint.
	if (m_enableMotor && m_limitState != e_equalLimits)
	{
		float32 motorCdot = m_motorJacobian.Compute(b1->m_linearVelocity, b1->m_angularVelocity, b2->m_linearVelocity, b2->m_angularVelocity) - m_motorSpeed;
		float32 motorForce = -B2FORCE_INV_SCALE(step.inv_dt) * m_motorMass * motorCdot;
		float32 oldMotorForce = m_motorForce;
		m_motorForce = b2Clamp(m_motorForce + motorForce, -m_maxMotorForce, m_maxMotorForce);
		motorForce = m_motorForce - oldMotorForce;

		float32 P = B2FORCE_SCALE(step.dt) * motorForce;
		b1->m_linearVelocity += (invMass1 * P) * m_motorJacobian.linear1;
		b1->m_angularVelocity += invI1 * P * m_motorJacobian.angular1;

		b2->m_linearVelocity += (invMass2 * P) * m_motorJacobian.linear2;
		b2->m_angularVelocity += invI2 * P * m_motorJacobian.angular2;
	}

	// Solve linear limit constraint.
	if (m_enableLimit && m_limitState != e_inactiveLimit)
	{
		float32 limitCdot = m_motorJacobian.Compute(b1->m_linearVelocity, b1->m_angularVelocity, b2->m_linearVelocity, b2->m_angularVelocity);
		float32 limitForce = -B2FORCE_INV_SCALE(step.inv_dt) * m_motorMass * limitCdot;

		if (m_limitState == e_equalLimits)
		{
			m_limitForce += limitForce;
		}
		else if (m_limitState == e_atLowerLimit)
		{
			float32 oldLimitForce = m_limitForce;
			m_limitForce = b2Max(m_limitForce + limitForce, 0.0f);
			limitForce = m_limitForce - oldLimitForce;
		}
		else if (m_limitState == e_atUpperLimit)
		{
			float32 oldLimitForce = m_limitForce;
			m_limitForce = b2Min(m_limitForce + limitForce, 0.0f);
			limitForce = m_limitForce - oldLimitForce;
		}

		float32 P = B2FORCE_SCALE(step.dt) * limitForce;

		b1->m_linearVelocity += (invMass1 * P) * m_motorJacobian.linear1;
		b1->m_angularVelocity += invI1 * P * m_motorJacobian.angular1;

		b2->m_linearVelocity += (invMass2 * P) * m_motorJacobian.linear2;
		b2->m_angularVelocity += invI2 * P * m_motorJacobian.angular2;
	}
}

bool b2PrismaticJoint::SolvePositionConstraints()
{
	b2Body* b1 = m_body1;
	b2Body* b2 = m_body2;

	float32 invMass1 = b1->m_invMass, invMass2 = b2->m_invMass;
	float32 invI1 = b1->m_invI, invI2 = b2->m_invI;

	b2Vec2 r1 = b2Mul(b1->GetXForm().R, m_localAnchor1 - b1->GetLocalCenter());
	b2Vec2 r2 = b2Mul(b2->GetXForm().R, m_localAnchor2 - b2->GetLocalCenter());
	b2Vec2 p1 = b1->m_sweep.c + r1;
	b2Vec2 p2 = b2->m_sweep.c + r2;
	b2Vec2 d = p2 - p1;
	b2Vec2 ay1 = b2Mul(b1->GetXForm().R, m_localYAxis1);

	// Solve linear (point-to-line) constraint.
	float32 linearC = b2Dot(ay1, d);
	// Prevent overly large corrections.
	linearC = b2Clamp(linearC, -b2_maxLinearCorrection, b2_maxLinearCorrection);
	float32 linearImpulse = -m_linearMass * linearC;

	b1->m_sweep.c += (invMass1 * linearImpulse) * m_linearJacobian.linear1;
	b1->m_sweep.a += invI1 * linearImpulse * m_linearJacobian.angular1;
	//b1->SynchronizeTransform(); // updated by angular constraint
	b2->m_sweep.c += (invMass2 * linearImpulse) * m_linearJacobian.linear2;
	b2->m_sweep.a += invI2 * linearImpulse * m_linearJacobian.angular2;
	//b2->SynchronizeTransform(); // updated by angular constraint

	float32 positionError = b2Abs(linearC);

	// Solve angular constraint.
	float32 angularC = b2->m_sweep.a - b1->m_sweep.a - m_refAngle;
	// Prevent overly large corrections.
	angularC = b2Clamp(angularC, -b2_maxAngularCorrection, b2_maxAngularCorrection);
	float32 angularImpulse = -m_angularMass * angularC;

	b1->m_sweep.a -= b1->m_invI * angularImpulse;
	b2->m_sweep.a += b2->m_invI * angularImpulse;

	b1->SynchronizeTransform();
	b2->SynchronizeTransform();

	float32 angularError = b2Abs(angularC);

	// Solve linear limit constraint.
	if (m_enableLimit && m_limitState != e_inactiveLimit)
	{
		b2Vec2 r1 = b2Mul(b1->GetXForm().R, m_localAnchor1 - b1->GetLocalCenter());
		b2Vec2 r2 = b2Mul(b2->GetXForm().R, m_localAnchor2 - b2->GetLocalCenter());
		b2Vec2 p1 = b1->m_sweep.c + r1;
		b2Vec2 p2 = b2->m_sweep.c + r2;
		b2Vec2 d = p2 - p1;
		b2Vec2 ax1 = b2Mul(b1->GetXForm().R, m_localXAxis1);

		float32 translation = b2Dot(ax1, d);
		float32 limitImpulse = 0.0f;

		if (m_limitState == e_equalLimits)
		{
			// Prevent large angular corrections
			float32 limitC = b2Clamp(translation, -b2_maxLinearCorrection, b2_maxLinearCorrection);
			limitImpulse = -m_motorMass * limitC;
			positionError = b2Max(positionError, b2Abs(angularC));
		}
		else if (m_limitState == e_atLowerLimit)
		{
			float32 limitC = translation - m_lowerTranslation;
			positionError = b2Max(positionError, -limitC);

			// Prevent large linear corrections and allow some slop.
			limitC = b2Clamp(limitC + b2_linearSlop, -b2_maxLinearCorrection, 0.0f);
			limitImpulse = -m_motorMass * limitC;
			float32 oldLimitImpulse = m_limitPositionImpulse;
			m_limitPositionImpulse = b2Max(m_limitPositionImpulse + limitImpulse, 0.0f);
			limitImpulse = m_limitPositionImpulse - oldLimitImpulse;
		}
		else if (m_limitState == e_atUpperLimit)
		{
			float32 limitC = translation - m_upperTranslation;
			positionError = b2Max(positionError, limitC);

			// Prevent large linear corrections and allow some slop.
			limitC = b2Clamp(limitC - b2_linearSlop, 0.0f, b2_maxLinearCorrection);
			limitImpulse = -m_motorMass * limitC;
			float32 oldLimitImpulse = m_limitPositionImpulse;
			m_limitPositionImpulse = b2Min(m_limitPositionImpulse + limitImpulse, 0.0f);
			limitImpulse = m_limitPositionImpulse - oldLimitImpulse;
		}

		b1->m_sweep.c += (invMass1 * limitImpulse) * m_motorJacobian.linear1;
		b1->m_sweep.a += invI1 * limitImpulse * m_motorJacobian.angular1;
		b2->m_sweep.c += (invMass2 * limitImpulse) * m_motorJacobian.linear2;
		b2->m_sweep.a += invI2 * limitImpulse * m_motorJacobian.angular2;

		b1->SynchronizeTransform();
		b2->SynchronizeTransform();
	}

	return positionError <= b2_linearSlop && angularError <= b2_angularSlop;
}

b2Vec2 b2PrismaticJoint::GetAnchor1() const
{
	return m_body1->GetWorldPoint(m_localAnchor1);
}

b2Vec2 b2PrismaticJoint::GetAnchor2() const
{
	return m_body2->GetWorldPoint(m_localAnchor2);
}

b2Vec2 b2PrismaticJoint::GetReactionForce() const
{
	b2Vec2 ax1 = b2Mul(m_body1->GetXForm().R, m_localXAxis1);
	b2Vec2 ay1 = b2Mul(m_body1->GetXForm().R, m_localYAxis1);

	return B2FORCE_SCALE(float32(1.0))*(m_limitForce * ax1 + m_force * ay1);
}

float32 b2PrismaticJoint::GetReactionTorque() const
{
	return B2FORCE_SCALE(m_torque);
}

float32 b2PrismaticJoint::GetJointTranslation() const
{
	b2Body* b1 = m_body1;
	b2Body* b2 = m_body2;

	b2Vec2 p1 = b1->GetWorldPoint(m_localAnchor1);
	b2Vec2 p2 = b2->GetWorldPoint(m_localAnchor2);
	b2Vec2 d = p2 - p1;
	b2Vec2 axis = b1->GetWorldVector(m_localXAxis1);

	float32 translation = b2Dot(d, axis);
	return translation;
}

float32 b2PrismaticJoint::GetJointSpeed() const
{
	b2Body* b1 = m_body1;
	b2Body* b2 = m_body2;

	b2Vec2 r1 = b2Mul(b1->GetXForm().R, m_localAnchor1 - b1->GetLocalCenter());
	b2Vec2 r2 = b2Mul(b2->GetXForm().R, m_localAnchor2 - b2->GetLocalCenter());
	b2Vec2 p1 = b1->m_sweep.c + r1;
	b2Vec2 p2 = b2->m_sweep.c + r2;
	b2Vec2 d = p2 - p1;
	b2Vec2 axis = b1->GetWorldVector(m_localXAxis1);

	b2Vec2 v1 = b1->m_linearVelocity;
	b2Vec2 v2 = b2->m_linearVelocity;
	float32 w1 = b1->m_angularVelocity;
	float32 w2 = b2->m_angularVelocity;

	float32 speed = b2Dot(d, b2Cross(w1, axis)) + b2Dot(axis, v2 + b2Cross(w2, r2) - v1 - b2Cross(w1, r1));
	return speed;
}

bool b2PrismaticJoint::IsLimitEnabled() const
{
	return m_enableLimit;
}

void b2PrismaticJoint::EnableLimit(bool flag)
{
	m_enableLimit = flag;
}

float32 b2PrismaticJoint::GetLowerLimit() const
{
	return m_lowerTranslation;
}

float32 b2PrismaticJoint::GetUpperLimit() const
{
	return m_upperTranslation;
}

void b2PrismaticJoint::SetLimits(float32 lower, float32 upper)
{
	b2Assert(lower <= upper);
	m_lowerTranslation = lower;
	m_upperTranslation = upper;
}

bool b2PrismaticJoint::IsMotorEnabled() const
{
	return m_enableMotor;
}

void b2PrismaticJoint::EnableMotor(bool flag)
{
	m_enableMotor = flag;
}

void b2PrismaticJoint::SetMotorSpeed(float32 speed)
{
	m_motorSpeed = speed;
}

void b2PrismaticJoint::SetMaxMotorForce(float32 force)
{
	m_maxMotorForce = B2FORCE_SCALE(float32(1.0))*force;
}

float32 b2PrismaticJoint::GetMotorForce() const
{
	return m_motorForce;
}



