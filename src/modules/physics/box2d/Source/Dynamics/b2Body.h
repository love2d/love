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

#ifndef B2_BODY_H
#define B2_BODY_H

#include "../Common/b2Math.h"
#include "../Collision/Shapes/b2Shape.h"
#include "Joints/b2Joint.h"

#include <memory>

class b2Joint;
class b2Contact;
class b2World;
struct b2JointEdge;
struct b2ContactEdge;

/// A body definition holds all the data needed to construct a rigid body.
/// You can safely re-use body definitions.
struct b2BodyDef
{
	/// This constructor sets the body definition default values.
	b2BodyDef()
	{
		massData.center.SetZero();
		massData.mass = 0.0f;
		massData.I = 0.0f;
		userData = NULL;
		position.Set(0.0f, 0.0f);
		angle = 0.0f;
		linearDamping = 0.0f;
		angularDamping = 0.0f;
		allowSleep = true;
		isSleeping = false;
		fixedRotation = false;
		isBullet = false;
	}

	/// You can use this to initialized the mass properties of the body.
	/// If you prefer, you can set the mass properties after the shapes
	/// have been added using b2Body::SetMassFromShapes.
	b2MassData massData;

	/// Use this to store application specific body data.
	void* userData;

	/// The world position of the body. Avoid creating bodies at the origin
	/// since this can lead to many overlapping shapes.
	b2Vec2 position;

	/// The world angle of the body in radians.
	float32 angle;

	/// Linear damping is use to reduce the linear velocity. The damping parameter
	/// can be larger than 1.0f but the damping effect becomes sensitive to the
	/// time step when the damping parameter is large.
	float32 linearDamping;

	/// Angular damping is use to reduce the angular velocity. The damping parameter
	/// can be larger than 1.0f but the damping effect becomes sensitive to the
	/// time step when the damping parameter is large.
	float32 angularDamping;

	/// Set this flag to false if this body should never fall asleep. Note that
	/// this increases CPU usage.
	bool allowSleep;

	/// Is this body initially sleeping?
	bool isSleeping;

	/// Should this body be prevented from rotating? Useful for characters.
	bool fixedRotation;

	/// Is this a fast moving body that should be prevented from tunneling through
	/// other moving bodies? Note that all bodies are prevented from tunneling through
	/// static bodies.
	/// @warning You should use this flag sparingly since it increases processing time.
	bool isBullet;
};

/// A rigid body.
class b2Body
{
public:
	/// Creates a shape and attach it to this body.
	/// @param shapeDef the shape definition.
	/// @warning This function is locked during callbacks.
	b2Shape* CreateShape(b2ShapeDef* shapeDef);

	/// Destroy a shape. This removes the shape from the broad-phase and
	/// therefore destroys any contacts associated with this shape. All shapes
	/// attached to a body are implicitly destroyed when the body is destroyed.
	/// @param shape the shape to be removed.
	/// @warning This function is locked during callbacks.
	void DestroyShape(b2Shape* shape);

	/// Set the mass properties. Note that this changes the center of mass position.
	/// If you are not sure how to compute mass properties, use SetMassFromShapes.
	/// The inertia tensor is assumed to be relative to the center of mass.
	/// @param massData the mass properties.
	void SetMass(const b2MassData* massData);

	/// Compute the mass properties from the attached shapes. You typically call this
	/// after adding all the shapes. If you add or remove shapes later, you may want
	/// to call this again. Note that this changes the center of mass position.
	void SetMassFromShapes();

	/// Set the position of the body's origin and rotation (radians).
	/// This breaks any contacts and wakes the other bodies.
	/// @param position the new world position of the body's origin (not necessarily
	/// the center of mass).
	/// @param angle the new world rotation angle of the body in radians.
	/// @return false if the movement put a shape outside the world. In this case the
	/// body is automatically frozen.
	bool SetXForm(const b2Vec2& position, float32 angle);

	/// Get the body transform for the body's origin.
	/// @return the world transform of the body's origin.
	const b2XForm& GetXForm() const;

	/// Get the world body origin position.
	/// @return the world position of the body's origin.
	const b2Vec2& GetPosition() const;

	/// Get the angle in radians.
	/// @return the current world rotation angle in radians.
	float32 GetAngle() const;

	/// Get the world position of the center of mass.
	const b2Vec2& GetWorldCenter() const;

	/// Get the local position of the center of mass.
	const b2Vec2& GetLocalCenter() const;

	/// Set the linear velocity of the center of mass.
	/// @param v the new linear velocity of the center of mass.
	void SetLinearVelocity(const b2Vec2& v);

	/// Get the linear velocity of the center of mass.
	/// @return the linear velocity of the center of mass.
	b2Vec2 GetLinearVelocity() const;

	/// Set the angular velocity.
	/// @param omega the new angular velocity in radians/second.
	void SetAngularVelocity(float32 omega);

	/// Get the angular velocity.
	/// @return the angular velocity in radians/second.
	float32 GetAngularVelocity() const;

	/// Apply a force at a world point. If the force is not
	/// applied at the center of mass, it will generate a torque and
	/// affect the angular velocity. This wakes up the body.
	/// @param force the world force vector, usually in Newtons (N).
	/// @param point the world position of the point of application.
	void ApplyForce(const b2Vec2& force, const b2Vec2& point);

	/// Apply a torque. This affects the angular velocity
	/// without affecting the linear velocity of the center of mass.
	/// This wakes up the body.
	/// @param torque about the z-axis (out of the screen), usually in N-m.
	void ApplyTorque(float32 torque);

	/// Apply an impulse at a point. This immediately modifies the velocity.
	/// It also modifies the angular velocity if the point of application
	/// is not at the center of mass. This wakes up the body.
	/// @param impulse the world impulse vector, usually in N-seconds or kg-m/s.
	/// @param point the world position of the point of application.
	void ApplyImpulse(const b2Vec2& impulse, const b2Vec2& point);

	/// Get the total mass of the body.
	/// @return the mass, usually in kilograms (kg).
	float32 GetMass() const;

	/// Get the central rotational inertia of the body.
	/// @return the rotational inertia, usually in kg-m^2.
	float32 GetInertia() const;

	/// Get the world coordinates of a point given the local coordinates.
	/// @param localPoint a point on the body measured relative the the body's origin.
	/// @return the same point expressed in world coordinates.
	b2Vec2 GetWorldPoint(const b2Vec2& localPoint) const;

	/// Get the world coordinates of a vector given the local coordinates.
	/// @param localVector a vector fixed in the body.
	/// @return the same vector expressed in world coordinates.
	b2Vec2 GetWorldVector(const b2Vec2& localVector) const;

	/// Gets a local point relative to the body's origin given a world point.
	/// @param a point in world coordinates.
	/// @return the corresponding local point relative to the body's origin.
	b2Vec2 GetLocalPoint(const b2Vec2& worldPoint) const;

	/// Gets a local vector given a world vector.
	/// @param a vector in world coordinates.
	/// @return the corresponding local vector.
	b2Vec2 GetLocalVector(const b2Vec2& worldVector) const;

	/// Get the world linear velocity of a world point attached to this body.
	/// @param a point in world coordinates.
	/// @return the world velocity of a point.
	b2Vec2 GetLinearVelocityFromWorldPoint(const b2Vec2& worldPoint) const;

	/// Get the world velocity of a local point.
	/// @param a point in local coordinates.
	/// @return the world velocity of a point.
	b2Vec2 GetLinearVelocityFromLocalPoint(const b2Vec2& localPoint) const;

	/// Is this body treated like a bullet for continuous collision detection?
	bool IsBullet() const;

	/// Should this body be treated like a bullet for continuous collision detection?
	void SetBullet(bool flag);

	/// Is this body static (immovable)?
	bool IsStatic() const;

	/// Is this body dynamic (movable)?
	bool IsDynamic() const;

	/// Is this body frozen?
	bool IsFrozen() const;

	/// Is this body sleeping (not simulating).
	bool IsSleeping() const;

	/// You can disable sleeping on this body.
	void AllowSleeping(bool flag);

	/// Wake up this body so it will begin simulating.
	void WakeUp();

	/// Put this body to sleep so it will stop simulating.
	/// This also sets the velocity to zero.
	void PutToSleep();

	/// Get the list of all shapes attached to this body.
	b2Shape* GetShapeList();

	/// Get the list of all joints attached to this body.
	b2JointEdge* GetJointList();

	/// Get the next body in the world's body list.
	b2Body* GetNext();

	/// Get the user data pointer that was provided in the body definition.
	void* GetUserData();

	/// Set the user data. Use this to store your application specific data.
	void SetUserData(void* data);

	/// Get the parent world of this body.
	b2World* GetWorld();

public:

	friend class b2World;
	friend class b2Island;
	friend class b2ContactManager;
	friend class b2ContactSolver;
	
	friend class b2DistanceJoint;
	friend class b2GearJoint;
	friend class b2MouseJoint;
	friend class b2PrismaticJoint;
	friend class b2PulleyJoint;
	friend class b2RevoluteJoint;

	// m_flags
	enum
	{
		e_frozenFlag		= 0x0002,
		e_islandFlag		= 0x0004,
		e_sleepFlag			= 0x0008,
		e_allowSleepFlag	= 0x0010,
		e_bulletFlag		= 0x0020,
		e_fixedRotationFlag	= 0x0040,
	};

	// m_type
	enum
	{
		e_staticType,
		e_dynamicType,
		e_maxTypes,
	};

	b2Body(const b2BodyDef* bd, b2World* world);
	~b2Body();

	bool SynchronizeShapes();

	void SynchronizeTransform();

	// This is used to prevent connected bodies from colliding.
	// It may lie, depending on the collideConnected flag.
	bool IsConnected(const b2Body* other) const;

	void Advance(float32 t);

	uint16 m_flags;
	int16 m_type;

	b2XForm m_xf;		// the body origin transform

	b2Sweep m_sweep;	// the swept motion for CCD

	b2Vec2 m_linearVelocity;
	float32 m_angularVelocity;

	b2Vec2 m_force;
	float32 m_torque;

	b2World* m_world;
	b2Body* m_prev;
	b2Body* m_next;

	b2Shape* m_shapeList;
	int32 m_shapeCount;

	b2JointEdge* m_jointList;
	b2ContactEdge* m_contactList;

	float32 m_mass, m_invMass;
	float32 m_I, m_invI;

	float32 m_linearDamping;
	float32 m_angularDamping;

	float32 m_sleepTime;

	void* m_userData;
};

inline const b2XForm& b2Body::GetXForm() const
{
	return m_xf;
}

inline const b2Vec2& b2Body::GetPosition() const
{
	return m_xf.position;
}

inline float32 b2Body::GetAngle() const
{
	return m_sweep.a;
}

inline const b2Vec2& b2Body::GetWorldCenter() const
{
	return m_sweep.c;
}

inline const b2Vec2& b2Body::GetLocalCenter() const
{
	return m_sweep.localCenter;
}

inline void b2Body::SetLinearVelocity(const b2Vec2& v)
{
	m_linearVelocity = v;
}

inline b2Vec2 b2Body::GetLinearVelocity() const
{
	return m_linearVelocity;
}

inline void b2Body::SetAngularVelocity(float32 w)
{
	m_angularVelocity = w;
}

inline float32 b2Body::GetAngularVelocity() const
{
	return m_angularVelocity;
}

inline float32 b2Body::GetMass() const
{
	return m_mass;
}

inline float32 b2Body::GetInertia() const
{
	return m_I;
}

inline b2Vec2 b2Body::GetWorldPoint(const b2Vec2& localPoint) const
{
	return b2Mul(m_xf, localPoint);
}

inline b2Vec2 b2Body::GetWorldVector(const b2Vec2& localVector) const
{
	return b2Mul(m_xf.R, localVector);
}

inline b2Vec2 b2Body::GetLocalPoint(const b2Vec2& worldPoint) const
{
	return b2MulT(m_xf, worldPoint);
}

inline b2Vec2 b2Body::GetLocalVector(const b2Vec2& worldVector) const
{
	return b2MulT(m_xf.R, worldVector);
}

inline b2Vec2 b2Body::GetLinearVelocityFromWorldPoint(const b2Vec2& worldPoint) const
{
	return m_linearVelocity + b2Cross(m_angularVelocity, worldPoint - m_sweep.c);
}

inline b2Vec2 b2Body::GetLinearVelocityFromLocalPoint(const b2Vec2& localPoint) const
{
	return GetLinearVelocityFromWorldPoint(GetWorldPoint(localPoint));
}

inline bool b2Body::IsBullet() const
{
	return (m_flags & e_bulletFlag) == e_bulletFlag;
}

inline void b2Body::SetBullet(bool flag)
{
	if (flag)
	{
		m_flags |= e_bulletFlag;
	}
	else
	{
		m_flags &= ~e_bulletFlag;
	}
}

inline bool b2Body::IsStatic() const
{
	return m_type == e_staticType;
}

inline bool b2Body::IsDynamic() const
{
	return m_type == e_dynamicType;
}

inline bool b2Body::IsFrozen() const
{
	return (m_flags & e_frozenFlag) == e_frozenFlag;
}

inline bool b2Body::IsSleeping() const
{
	return (m_flags & e_sleepFlag) == e_sleepFlag;
}

inline void b2Body::AllowSleeping(bool flag)
{
	if (flag)
	{
		m_flags |= e_allowSleepFlag;
	}
	else
	{
		m_flags &= ~e_allowSleepFlag;
		WakeUp();
	}
}

inline void b2Body::WakeUp()
{
	m_flags &= ~e_sleepFlag;
	m_sleepTime = 0.0f;
}

inline void b2Body::PutToSleep()
{
	m_flags |= e_sleepFlag;
	m_sleepTime = 0.0f;
	m_linearVelocity.SetZero();
	m_angularVelocity = 0.0f;
	m_force.SetZero();
	m_torque = 0.0f;
}

inline b2Shape* b2Body::GetShapeList()
{
	return m_shapeList;
}

inline b2JointEdge* b2Body::GetJointList()
{
	return m_jointList;
}

inline b2Body* b2Body::GetNext()
{
	return m_next;
}

inline void* b2Body::GetUserData()
{
	return m_userData;
}

inline void b2Body::SetUserData(void* data)
{
	m_userData = data;
}

inline bool b2Body::IsConnected(const b2Body* other) const
{
	for (b2JointEdge* jn = m_jointList; jn; jn = jn->next)
	{
		if (jn->other == other)
			return jn->joint->m_collideConnected == false;
	}

	return false;
}

inline void b2Body::ApplyForce(const b2Vec2& force, const b2Vec2& point)
{
	if (IsSleeping())
	{
		WakeUp();
	}
	m_force += force;
	m_torque += b2Cross(point - m_sweep.c, force);
}

inline void b2Body::ApplyTorque(float32 torque)
{
	if (IsSleeping())
	{
		WakeUp();
	}
	m_torque += torque;
}

inline void b2Body::ApplyImpulse(const b2Vec2& impulse, const b2Vec2& point)
{
	if (IsSleeping())
	{
		WakeUp();
	}
	m_linearVelocity += m_invMass * impulse;
	m_angularVelocity += m_invI * b2Cross(point - m_sweep.c, impulse);
}

inline void b2Body::SynchronizeTransform()
{
	m_xf.R.Set(m_sweep.a);
	m_xf.position = m_sweep.c - b2Mul(m_xf.R, m_sweep.localCenter);
}

inline void b2Body::Advance(float32 t)
{
	// Advance to the new safe time.
	m_sweep.Advance(t);
	m_sweep.c = m_sweep.c0;
	m_sweep.a = m_sweep.a0;
	SynchronizeTransform();
}

inline b2World* b2Body::GetWorld()
{
	return m_world;
}

#endif
