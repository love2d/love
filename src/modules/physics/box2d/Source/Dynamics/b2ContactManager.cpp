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

#include "b2ContactManager.h"
#include "b2World.h"
#include "b2Body.h"

// This is a callback from the broadphase when two AABB proxies begin
// to overlap. We create a b2Contact to manage the narrow phase.
void* b2ContactManager::PairAdded(void* proxyUserData1, void* proxyUserData2)
{
	b2Shape* shape1 = (b2Shape*)proxyUserData1;
	b2Shape* shape2 = (b2Shape*)proxyUserData2;

	b2Body* body1 = shape1->GetBody();
	b2Body* body2 = shape2->GetBody();

	if (body1->IsStatic() && body2->IsStatic())
	{
		return &m_nullContact;
	}

	if (shape1->GetBody() == shape2->GetBody())
	{
		return &m_nullContact;
	}

	if (body2->IsConnected(body1))
	{
		return &m_nullContact;
	}

	if (m_world->m_contactFilter != NULL && m_world->m_contactFilter->ShouldCollide(shape1, shape2) == false)
	{
		return &m_nullContact;
	}

	// Call the factory.
	b2Contact* c = b2Contact::Create(shape1, shape2, &m_world->m_blockAllocator);

	if (c == NULL)
	{
		return &m_nullContact;
	}

	// Contact creation may swap shapes.
	shape1 = c->GetShape1();
	shape2 = c->GetShape2();
	body1 = shape1->GetBody();
	body2 = shape2->GetBody();

	// Insert into the world.
	c->m_prev = NULL;
	c->m_next = m_world->m_contactList;
	if (m_world->m_contactList != NULL)
	{
		m_world->m_contactList->m_prev = c;
	}
	m_world->m_contactList = c;

	// Connect to island graph.

	// Connect to body 1
	c->m_node1.contact = c;
	c->m_node1.other = body2;

	c->m_node1.prev = NULL;
	c->m_node1.next = body1->m_contactList;
	if (body1->m_contactList != NULL)
	{
		body1->m_contactList->prev = &c->m_node1;
	}
	body1->m_contactList = &c->m_node1;

	// Connect to body 2
	c->m_node2.contact = c;
	c->m_node2.other = body1;

	c->m_node2.prev = NULL;
	c->m_node2.next = body2->m_contactList;
	if (body2->m_contactList != NULL)
	{
		body2->m_contactList->prev = &c->m_node2;
	}
	body2->m_contactList = &c->m_node2;

	++m_world->m_contactCount;
	return c;
}

// This is a callback from the broadphase when two AABB proxies cease
// to overlap. We retire the b2Contact.
void b2ContactManager::PairRemoved(void* proxyUserData1, void* proxyUserData2, void* pairUserData)
{
	B2_NOT_USED(proxyUserData1);
	B2_NOT_USED(proxyUserData2);

	if (pairUserData == NULL)
	{
		return;
	}

	b2Contact* c = (b2Contact*)pairUserData;
	if (c == &m_nullContact)
	{
		return;
	}

	// An attached body is being destroyed, we must destroy this contact
	// immediately to avoid orphaned shape pointers.
	Destroy(c);
}

void b2ContactManager::Destroy(b2Contact* c)
{
	b2Shape* shape1 = c->GetShape1();
	b2Shape* shape2 = c->GetShape2();

	// Inform the user that this contact is ending.
	int32 manifoldCount = c->GetManifoldCount();
	if (manifoldCount > 0 && m_world->m_contactListener)
	{
		b2Body* b1 = shape1->GetBody();
		b2Body* b2 = shape2->GetBody();

		b2Manifold* manifolds = c->GetManifolds();
		b2ContactPoint cp;
		cp.shape1 = c->GetShape1();
		cp.shape2 = c->GetShape2();
		cp.friction = c->m_friction;
		cp.restitution = c->m_restitution;

		for (int32 i = 0; i < manifoldCount; ++i)
		{
			b2Manifold* manifold = manifolds + i;
			cp.normal = manifold->normal;

			for (int32 j = 0; j < manifold->pointCount; ++j)
			{
				b2ManifoldPoint* mp = manifold->points + j;
				cp.position = b1->GetWorldPoint(mp->localPoint1);
				b2Vec2 v1 = b1->GetLinearVelocityFromLocalPoint(mp->localPoint1);
				b2Vec2 v2 = b2->GetLinearVelocityFromLocalPoint(mp->localPoint2);
				cp.velocity = v2 - v1;
				cp.separation = mp->separation;
				cp.id = mp->id;
				m_world->m_contactListener->Remove(&cp);
			}
		}
	}

	// Remove from the world.
	if (c->m_prev)
	{
		c->m_prev->m_next = c->m_next;
	}

	if (c->m_next)
	{
		c->m_next->m_prev = c->m_prev;
	}

	if (c == m_world->m_contactList)
	{
		m_world->m_contactList = c->m_next;
	}

	b2Body* body1 = shape1->GetBody();
	b2Body* body2 = shape2->GetBody();

	// Remove from body 1
	if (c->m_node1.prev)
	{
		c->m_node1.prev->next = c->m_node1.next;
	}

	if (c->m_node1.next)
	{
		c->m_node1.next->prev = c->m_node1.prev;
	}

	if (&c->m_node1 == body1->m_contactList)
	{
		body1->m_contactList = c->m_node1.next;
	}

	// Remove from body 2
	if (c->m_node2.prev)
	{
		c->m_node2.prev->next = c->m_node2.next;
	}

	if (c->m_node2.next)
	{
		c->m_node2.next->prev = c->m_node2.prev;
	}

	if (&c->m_node2 == body2->m_contactList)
	{
		body2->m_contactList = c->m_node2.next;
	}

	// Call the factory.
	b2Contact::Destroy(c, &m_world->m_blockAllocator);
	--m_world->m_contactCount;
}

// This is the top level collision call for the time step. Here
// all the narrow phase collision is processed for the world
// contact list.
void b2ContactManager::Collide()
{
	// Update awake contacts.
	for (b2Contact* c = m_world->m_contactList; c; c = c->GetNext())
	{
		b2Body* body1 = c->GetShape1()->GetBody();
		b2Body* body2 = c->GetShape2()->GetBody();
		if (body1->IsSleeping() && body2->IsSleeping())
		{
			continue;
		}

		c->Update(m_world->m_contactListener);
	}
}
