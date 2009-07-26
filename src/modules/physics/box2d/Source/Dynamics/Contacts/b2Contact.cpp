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

#include "b2Contact.h"
#include "b2CircleContact.h"
#include "b2PolyAndCircleContact.h"
#include "b2PolyContact.h"
#include "b2ContactSolver.h"
#include "../../Collision/b2Collision.h"
#include "../../Collision/Shapes/b2Shape.h"
#include "../../Common/b2BlockAllocator.h"
#include "../../Dynamics/b2World.h"
#include "../../Dynamics/b2Body.h"

b2ContactRegister b2Contact::s_registers[e_shapeTypeCount][e_shapeTypeCount];
bool b2Contact::s_initialized = false;

void b2Contact::InitializeRegisters()
{
	AddType(b2CircleContact::Create, b2CircleContact::Destroy, e_circleShape, e_circleShape);
	AddType(b2PolyAndCircleContact::Create, b2PolyAndCircleContact::Destroy, e_polygonShape, e_circleShape);
	AddType(b2PolygonContact::Create, b2PolygonContact::Destroy, e_polygonShape, e_polygonShape);
}

void b2Contact::AddType(b2ContactCreateFcn* createFcn, b2ContactDestroyFcn* destoryFcn,
					  b2ShapeType type1, b2ShapeType type2)
{
	b2Assert(e_unknownShape < type1 && type1 < e_shapeTypeCount);
	b2Assert(e_unknownShape < type2 && type2 < e_shapeTypeCount);
	
	s_registers[type1][type2].createFcn = createFcn;
	s_registers[type1][type2].destroyFcn = destoryFcn;
	s_registers[type1][type2].primary = true;

	if (type1 != type2)
	{
		s_registers[type2][type1].createFcn = createFcn;
		s_registers[type2][type1].destroyFcn = destoryFcn;
		s_registers[type2][type1].primary = false;
	}
}

b2Contact* b2Contact::Create(b2Shape* shape1, b2Shape* shape2, b2BlockAllocator* allocator)
{
	if (s_initialized == false)
	{
		InitializeRegisters();
		s_initialized = true;
	}

	b2ShapeType type1 = shape1->GetType();
	b2ShapeType type2 = shape2->GetType();

	b2Assert(e_unknownShape < type1 && type1 < e_shapeTypeCount);
	b2Assert(e_unknownShape < type2 && type2 < e_shapeTypeCount);
	
	b2ContactCreateFcn* createFcn = s_registers[type1][type2].createFcn;
	if (createFcn)
	{
		if (s_registers[type1][type2].primary)
		{
			return createFcn(shape1, shape2, allocator);
		}
		else
		{
			b2Contact* c = createFcn(shape2, shape1, allocator);
			for (int32 i = 0; i < c->GetManifoldCount(); ++i)
			{
				b2Manifold* m = c->GetManifolds() + i;
				m->normal = -m->normal;
			}
			return c;
		}
	}
	else
	{
		return NULL;
	}
}

void b2Contact::Destroy(b2Contact* contact, b2BlockAllocator* allocator)
{
	b2Assert(s_initialized == true);

	if (contact->GetManifoldCount() > 0)
	{
		contact->GetShape1()->GetBody()->WakeUp();
		contact->GetShape2()->GetBody()->WakeUp();
	}

	b2ShapeType type1 = contact->GetShape1()->GetType();
	b2ShapeType type2 = contact->GetShape2()->GetType();

	b2Assert(e_unknownShape < type1 && type1 < e_shapeTypeCount);
	b2Assert(e_unknownShape < type2 && type2 < e_shapeTypeCount);

	b2ContactDestroyFcn* destroyFcn = s_registers[type1][type2].destroyFcn;
	destroyFcn(contact, allocator);
}

b2Contact::b2Contact(b2Shape* s1, b2Shape* s2)
{
	m_flags = 0;

	if (s1->IsSensor() || s2->IsSensor())
	{
		m_flags |= e_nonSolidFlag;
	}

	m_shape1 = s1;
	m_shape2 = s2;

	m_manifoldCount = 0;

	m_friction = b2MixFriction(m_shape1->GetFriction(), m_shape2->GetFriction());
	m_restitution = b2MixRestitution(m_shape1->GetRestitution(), m_shape2->GetRestitution());
	m_prev = NULL;
	m_next = NULL;

	m_node1.contact = NULL;
	m_node1.prev = NULL;
	m_node1.next = NULL;
	m_node1.other = NULL;

	m_node2.contact = NULL;
	m_node2.prev = NULL;
	m_node2.next = NULL;
	m_node2.other = NULL;
}

void b2Contact::Update(b2ContactListener* listener)
{
	int32 oldCount = GetManifoldCount();

	Evaluate(listener);

	int32 newCount = GetManifoldCount();

	b2Body* body1 = m_shape1->GetBody();
	b2Body* body2 = m_shape2->GetBody();

	if (newCount == 0 && oldCount > 0)
	{
		body1->WakeUp();
		body2->WakeUp();
	}

	// Slow contacts don't generate TOI events.
	if (body1->IsStatic() || body1->IsBullet() || body2->IsStatic() || body2->IsBullet())
	{
		m_flags &= ~e_slowFlag;
	}
	else
	{
		m_flags |= e_slowFlag;
	}
}
