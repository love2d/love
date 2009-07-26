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

#ifndef CONTACT_H
#define CONTACT_H

#include "../../Common/b2Math.h"
#include "../../Collision/b2Collision.h"
#include "../../Collision/Shapes/b2Shape.h"

class b2Body;
class b2Contact;
class b2World;
class b2BlockAllocator;
class b2StackAllocator;
class b2ContactListener;

typedef b2Contact* b2ContactCreateFcn(b2Shape* shape1, b2Shape* shape2, b2BlockAllocator* allocator);
typedef void b2ContactDestroyFcn(b2Contact* contact, b2BlockAllocator* allocator);

struct b2ContactRegister
{
	b2ContactCreateFcn* createFcn;
	b2ContactDestroyFcn* destroyFcn;
	bool primary;
};

/// A contact edge is used to connect bodies and contacts together
/// in a contact graph where each body is a node and each contact
/// is an edge. A contact edge belongs to a doubly linked list
/// maintained in each attached body. Each contact has two contact
/// nodes, one for each attached body.
struct b2ContactEdge
{
	b2Body* other;			///< provides quick access to the other body attached.
	b2Contact* contact;		///< the contact
	b2ContactEdge* prev;	///< the previous contact edge in the body's contact list
	b2ContactEdge* next;	///< the next contact edge in the body's contact list
};

/// This structure is used to report contact points.
struct b2ContactPoint
{
	b2Shape* shape1;		///< the first shape
	b2Shape* shape2;		///< the second shape
	b2Vec2 position;		///< position in world coordinates
	b2Vec2 velocity;		///< velocity of point on body2 relative to point on body1 (pre-solver)
	b2Vec2 normal;			///< points from shape1 to shape2
	float32 separation;		///< the separation is negative when shapes are touching
	float32 friction;		///< the combined friction coefficient
	float32 restitution;	///< the combined restitution coefficient
	b2ContactID id;			///< the contact id identifies the features in contact
};

/// This structure is used to report contact point results.
struct b2ContactResult
{
	b2Shape* shape1;		///< the first shape
	b2Shape* shape2;		///< the second shape
	b2Vec2 position;		///< position in world coordinates
	b2Vec2 normal;			///< points from shape1 to shape2
	float32 normalImpulse;	///< the normal impulse applied to body2
	float32 tangentImpulse;	///< the tangent impulse applied to body2
	b2ContactID id;			///< the contact id identifies the features in contact
};

/// The class manages contact between two shapes. A contact exists for each overlapping
/// AABB in the broad-phase (except if filtered). Therefore a contact object may exist
/// that has no contact points.
class b2Contact
{
public:

	/// Get the manifold array.
	virtual b2Manifold* GetManifolds() = 0;

	/// Get the number of manifolds. This is 0 or 1 between convex shapes.
	/// This may be greater than 1 for convex-vs-concave shapes. Each
	/// manifold holds up to two contact points with a shared contact normal.
	int32 GetManifoldCount() const;

	/// Is this contact solid?
	/// @return true if this contact should generate a response.
	bool IsSolid() const;

	/// Get the next contact in the world's contact list.
	b2Contact* GetNext();

	/// Get the first shape in this contact.
	b2Shape* GetShape1();

	/// Get the second shape in this contact.
	b2Shape* GetShape2();

	//--------------- Internals Below -------------------
public:

	// m_flags
	enum
	{
		e_nonSolidFlag	= 0x0001,
		e_slowFlag		= 0x0002,
		e_islandFlag	= 0x0004,
		e_toiFlag		= 0x0008,
	};

	static void AddType(b2ContactCreateFcn* createFcn, b2ContactDestroyFcn* destroyFcn,
						b2ShapeType type1, b2ShapeType type2);
	static void InitializeRegisters();
	static b2Contact* Create(b2Shape* shape1, b2Shape* shape2, b2BlockAllocator* allocator);
	static void Destroy(b2Contact* contact, b2BlockAllocator* allocator);

	b2Contact() : m_shape1(NULL), m_shape2(NULL) {}
	b2Contact(b2Shape* shape1, b2Shape* shape2);
	virtual ~b2Contact() {}

	void Update(b2ContactListener* listener);
	virtual void Evaluate(b2ContactListener* listener) = 0;
	static b2ContactRegister s_registers[e_shapeTypeCount][e_shapeTypeCount];
	static bool s_initialized;

	uint32 m_flags;
	int32 m_manifoldCount;

	// World pool and list pointers.
	b2Contact* m_prev;
	b2Contact* m_next;

	// Nodes for connecting bodies.
	b2ContactEdge m_node1;
	b2ContactEdge m_node2;

	b2Shape* m_shape1;
	b2Shape* m_shape2;

	// Combined friction
	float32 m_friction;
	float32 m_restitution;

	float32 m_toi;
};

inline int32 b2Contact::GetManifoldCount() const
{
	return m_manifoldCount;
}

inline bool b2Contact::IsSolid() const
{
	return (m_flags & e_nonSolidFlag) == 0;
}

inline b2Contact* b2Contact::GetNext()
{
	return m_next;
}

inline b2Shape* b2Contact::GetShape1()
{
	return m_shape1;
}

inline b2Shape* b2Contact::GetShape2()
{
	return m_shape2;
}

#endif
