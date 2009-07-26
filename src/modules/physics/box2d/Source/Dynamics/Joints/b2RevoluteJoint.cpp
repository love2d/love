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

#include "b2RevoluteJoint.h"
#include "../b2Body.h"
#include "../b2World.h"

#include "../b2Island.h"

// Point-to-point constraint
// C = p2 - p1
// Cdot = v2 - v1
//      = v2 + cross(w2, r2) - v1 - cross(w1, r1)
// J = [-I -r1_skew I r2_skew ]
// Identity used:
// w k % (rx i + ry j) = w * (-ry i + rx j)

// Motor constraint
// Cdot = w2 - w1
// J = [0 0 -1 0 0 1]
// K = invI1 + invI2

void b2RevoluteJointDef::Initialize(b2Body* b1, b2Body* b2, const b2Vec2& anchor)
{
	body1 = b1;
	body2 = b2;
	localAnchor1 = body1->GetLocalPoint(anchor);
	localAnchor2 = body2->GetLocalPoint(anchor);
	referenceAngle = body2->GetAngle() - body1->GetAngle();
}

b2RevoluteJoint::b2RevoluteJoint(const b2RevoluteJointDef* def)
: b2Joint(def)
{
	m_localAnchor1 = def->localAnchor1;
	m_localAnchor2 = def->localAnchor2;
	m_referenceAngle = def->referenceAngle;

	m_pivotForce.Set(0.0f, 0.0f);
	m_motorForce = 0.0f;
	m_limitForce = 0.0f;
	m_limitPositionImpulse = 0.0f;

	m_lowerAngle = def->lowerAngle;
	m_upperAngle = def->upperAngle;
	m_maxMotorTorque = def->maxMotorTorque;
	m_motorSpeed = def->motorSpeed;
	m_enableLimit = def->enableLimit;
	m_enableMotor = def->enableMotor;
}

void b2RevoluteJoint::InitVelocityConstraints(const b2TimeStep& step)
{
	b2Body* b1 = m_body1;
	b2Body* b2 = m_body2;

	// Compute the effective mass matrix.
	b2Vec2 r1 = b2Mul(b1->GetXForm().R, m_localAnchor1 - b1->GetLocalCenter());
	b2Vec2 r2 = b2Mul(b2->GetXForm().R, m_localAnchor2 - b2->GetLocalCenter());

	// K    = [(1/m1 + 1/m2) * eye(2) - skew(r1) * invI1 * skew(r1) - skew(r2) * invI2 * skew(r2)]
	//      = [1/m1+1/m2     0    ] + invI1 * [r1.y*r1.y -r1.x*r1.y] + invI2 * [r1.y*r1.y -r1.x*r1.y]
	//        [    0     1/m1+1/m2]           [-r1.x*r1.y r1.x*r1.x]           [-r1.x*r1.y r1.x*r1.x]
	float32 invMass1 = b1->m_invMass, invMass2 = b2->m_invMass;
	float32 invI1 = b1->m_invI, invI2 = b2->m_invI;

	b2Mat22 K1;
	K1.col1.x = invMass1 + invMass2;	K1.col2.x = 0.0f;
	K1.col1.y = 0.0f;					K1.col2.y = invMass1 + invMass2;

	b2Mat22 K2;
	K2.col1.x =  invI1 * r1.y * r1.y;	K2.col2.x = -invI1 * r1.x * r1.y;
	K2.col1.y = -invI1 * r1.x * r1.y;	K2.col2.y =  invI1 * r1.x * r1.x;

	b2Mat22 K3;
	K3.col1.x =  invI2 * r2.y * r2.y;	K3.col2.x = -invI2 * r2.x * r2.y;
	K3.col1.y = -invI2 * r2.x * r2.y;	K3.col2.y =  invI2 * r2.x * r2.x;

	b2Mat22 K = K1 + K2 + K3;
	m_pivotMass = K.Invert();

	m_motorMass = 1.0f / (invI1 + invI2);

	if (m_enableMotor == false)
	{
		m_motorForce = 0.0f;
	}

	if (m_enableLimit)
	{
		float32 jointAngle = b2->m_sweep.a - b1->m_sweep.a - m_referenceAngle;
		if (b2Abs(m_upperAngle - m_lowerAngle) < 2.0f * b2_angularSlop)
		{
			m_limitState = e_equalLimits;
		}
		else if (jointAngle <= m_lowerAngle)
		{
			if (m_limitState != e_atLowerLimit)
			{
				m_limitForce = 0.0f;
			}
			m_limitState = e_atLowerLimit;
		}
		else if (jointAngle >= m_upperAngle)
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
	else
	{
		m_limitForce = 0.0f;
	}

	if (step.warmStarting)
	{
		b1->m_linearVelocity -= B2FORCE_SCALE(step.dt) * invMass1 * m_pivotForce;
		b1->m_angularVelocity -= B2FORCE_SCALE(step.dt) * invI1 * (b2Cross(r1, m_pivotForce) + B2FORCE_INV_SCALE(m_motorForce + m_limitForce));

		b2->m_linearVelocity += B2FORCE_SCALE(step.dt) * invMass2 * m_pivotForce;
		b2->m_angularVelocity += B2FORCE_SCALE(step.dt) * invI2 * (b2Cross(r2, m_pivotForce) + B2FORCE_INV_SCALE(m_motorForce + m_limitForce));
	}
	else
	{
		m_pivotForce.SetZero();
		m_motorForce = 0.0f;
		m_limitForce = 0.0f;
	}

	m_limitPositionImpulse = 0.0f;
}

void b2RevoluteJoint::SolveVelocityConstraints(const b2TimeStep& step)
{
	b2Body* b1 = m_body1;
	b2Body* b2 = m_body2;

	b2Vec2 r1 = b2Mul(b1->GetXForm().R, m_localAnchor1 - b1->GetLocalCenter());
	b2Vec2 r2 = b2Mul(b2->GetXForm().R, m_localAnchor2 - b2->GetLocalCenter());

	// Solve point-to-point constraint
	b2Vec2 pivotCdot = b2->m_linearVelocity + b2Cross(b2->m_angularVelocity, r2) - b1->m_linearVelocity - b2Cross(b1->m_angularVelocity, r1);
	b2Vec2 pivotForce = -B2FORCE_INV_SCALE(step.inv_dt) * b2Mul(m_pivotMass, pivotCdot);
	m_pivotForce += pivotForce;

	b2Vec2 P = B2FORCE_SCALE(step.dt) * pivotForce;
	b1->m_linearVelocity -= b1->m_invMass * P;
	b1->m_angularVelocity -= b1->m_invI * b2Cross(r1, P);

	b2->m_linearVelocity += b2->m_invMass * P;
	b2->m_angularVelocity += b2->m_invI * b2Cross(r2, P);

	if (m_enableMotor && m_limitState != e_equalLimits)
	{
		float32 motorCdot = b2->m_angularVelocity - b1->m_angularVelocity - m_motorSpeed;
		float32 motorForce = -step.inv_dt * m_motorMass * motorCdot;
		float32 oldMotorForce = m_motorForce;
		m_motorForce = b2Clamp(m_motorForce + motorForce, -m_maxMotorTorque, m_maxMotorTorque);
		motorForce = m_motorForce - oldMotorForce;

		float32 P = step.dt * motorForce;
		b1->m_angularVelocity -= b1->m_invI * P;
		b2->m_angularVelocity += b2->m_invI * P;
	}

	if (m_enableLimit && m_limitState != e_inactiveLimit)
	{
		float32 limitCdot = b2->m_angularVelocity - b1->m_angularVelocity;
		float32 limitForce = -step.inv_dt * m_motorMass * limitCdot;

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

		float32 P = step.dt * limitForce;
		b1->m_angularVelocity -= b1->m_invI * P;
		b2->m_angularVelocity += b2->m_invI * P;
	}
}

bool b2RevoluteJoint::SolvePositionConstraints()
{
	b2Body* b1 = m_body1;
	b2Body* b2 = m_body2;

	float32 positionError = 0.0f;

	// Solve point-to-point position error.
	b2Vec2 r1 = b2Mul(b1->GetXForm().R, m_localAnchor1 - b1->GetLocalCenter());
	b2Vec2 r2 = b2Mul(b2->GetXForm().R, m_localAnchor2 - b2->GetLocalCenter());

	b2Vec2 p1 = b1->m_sweep.c + r1;
	b2Vec2 p2 = b2->m_sweep.c + r2;
	b2Vec2 ptpC = p2 - p1;

	positionError = ptpC.Length();

	// Prevent overly large corrections.
	//b2Vec2 dpMax(b2_maxLinearCorrection, b2_maxLinearCorrection);
	//ptpC = b2Clamp(ptpC, -dpMax, dpMax);

	float32 invMass1 = b1->m_invMass, invMass2 = b2->m_invMass;
	float32 invI1 = b1->m_invI, invI2 = b2->m_invI;

	b2Mat22 K1;
	K1.col1.x = invMass1 + invMass2;	K1.col2.x = 0.0f;
	K1.col1.y = 0.0f;					K1.col2.y = invMass1 + invMass2;

	b2Mat22 K2;
	K2.col1.x =  invI1 * r1.y * r1.y;	K2.col2.x = -invI1 * r1.x * r1.y;
	K2.col1.y = -invI1 * r1.x * r1.y;	K2.col2.y =  invI1 * r1.x * r1.x;

	b2Mat22 K3;
	K3.col1.x =  invI2 * r2.y * r2.y;	K3.col2.x = -invI2 * r2.x * r2.y;
	K3.col1.y = -invI2 * r2.x * r2.y;	K3.col2.y =  invI2 * r2.x * r2.x;

	b2Mat22 K = K1 + K2 + K3;
	b2Vec2 impulse = K.Solve(-ptpC);

	b1->m_sweep.c -= b1->m_invMass * impulse;
	b1->m_sweep.a -= b1->m_invI * b2Cross(r1, impulse);

	b2->m_sweep.c += b2->m_invMass * impulse;
	b2->m_sweep.a += b2->m_invI * b2Cross(r2, impulse);

	b1->SynchronizeTransform();
	b2->SynchronizeTransform();

	// Handle limits.
	float32 angularError = 0.0f;

	if (m_enableLimit && m_limitState != e_inactiveLimit)
	{
		float32 angle = b2->m_sweep.a - b1->m_sweep.a - m_referenceAngle;
		float32 limitImpulse = 0.0f;

		if (m_limitState == e_equalLimits)
		{
			// Prevent large angular corrections
			float32 limitC = b2Clamp(angle, -b2_maxAngularCorrection, b2_maxAngularCorrection);
			limitImpulse = -m_motorMass * limitC;
			angularError = b2Abs(limitC);
		}
		else if (m_limitState == e_atLowerLimit)
		{
			float32 limitC = angle - m_lowerAngle;
			angularError = b2Max(0.0f, -limitC);

			// Prevent large angular corrections and allow some slop.
			limitC = b2Clamp(limitC + b2_angularSlop, -b2_maxAngularCorrection, 0.0f);
			limitImpulse = -m_motorMass * limitC;
			float32 oldLimitImpulse = m_limitPositionImpulse;
			m_limitPositionImpulse = b2Max(m_limitPositionImpulse + limitImpulse, 0.0f);
			limitImpulse = m_limitPositionImpulse - oldLimitImpulse;
		}
		else if (m_limitState == e_atUpperLimit)
		{
			float32 limitC = angle - m_upperAngle;
			angularError = b2Max(0.0f, limitC);

			// Prevent large angular corrections and allow some slop.
			limitC = b2Clamp(limitC - b2_angularSlop, 0.0f, b2_maxAngularCorrection);
			limitImpulse = -m_motorMass * limitC;
			float32 oldLimitImpulse = m_limitPositionImpulse;
			m_limitPositionImpulse = b2Min(m_limitPositionImpulse + limitImpulse, 0.0f);
			limitImpulse = m_limitPositionImpulse - oldLimitImpulse;
		}

		b1->m_sweep.a -= b1->m_invI * limitImpulse;
		b2->m_sweep.a += b2->m_invI * limitImpulse;

		b1->SynchronizeTransform();
		b2->SynchronizeTransform();
	}

	return positionError <= b2_linearSlop && angularError <= b2_angularSlop;
}

b2Vec2 b2RevoluteJoint::GetAnchor1() const
{
	return m_body1->GetWorldPoint(m_localAnchor1);
}

b2Vec2 b2RevoluteJoint::GetAnchor2() const
{
	return m_body2->GetWorldPoint(m_localAnchor2);
}

b2Vec2 b2RevoluteJoint::GetReactionForce() const
{
	return B2FORCE_SCALE(float32(1.0))*m_pivotForce;
}

float32 b2RevoluteJoint::GetReactionTorque() const
{
	return m_limitForce;
}

float32 b2RevoluteJoint::GetJointAngle() const
{
	b2Body* b1 = m_body1;
	b2Body* b2 = m_body2;
	return b2->m_sweep.a - b1->m_sweep.a - m_referenceAngle;
}

float32 b2RevoluteJoint::GetJointSpeed() const
{
	b2Body* b1 = m_body1;
	b2Body* b2 = m_body2;
	return b2->m_angularVelocity - b1->m_angularVelocity;
}

bool b2RevoluteJoint::IsMotorEnabled() const
{
	return m_enableMotor;
}

void b2RevoluteJoint::EnableMotor(bool flag)
{
	m_enableMotor = flag;
}

float32 b2RevoluteJoint::GetMotorTorque() const
{
	return m_motorForce;
}

void b2RevoluteJoint::SetMotorSpeed(float32 speed)
{
	m_motorSpeed = speed;
}

void b2RevoluteJoint::SetMaxMotorTorque(float32 torque)
{
	m_maxMotorTorque = torque;
}

bool b2RevoluteJoint::IsLimitEnabled() const
{
	return m_enableLimit;
}

void b2RevoluteJoint::EnableLimit(bool flag)
{
	m_enableLimit = flag;
}

float32 b2RevoluteJoint::GetLowerLimit() const
{
	return m_lowerAngle;
}

float32 b2RevoluteJoint::GetUpperLimit() const
{
	return m_upperAngle;
}

void b2RevoluteJoint::SetLimits(float32 lower, float32 upper)
{
	b2Assert(lower <= upper);
	m_lowerAngle = lower;
	m_upperAngle = upper;
}
