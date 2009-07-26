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

#include "b2PolyContact.h"
#include "../b2Body.h"
#include "../b2WorldCallbacks.h"
#include "../../Common/b2BlockAllocator.h"

#include <memory>
#include <new>
#include <string.h>

b2Contact* b2PolygonContact::Create(b2Shape* shape1, b2Shape* shape2, b2BlockAllocator* allocator)
{
	void* mem = allocator->Allocate(sizeof(b2PolygonContact));
	return new (mem) b2PolygonContact(shape1, shape2);
}

void b2PolygonContact::Destroy(b2Contact* contact, b2BlockAllocator* allocator)
{
	((b2PolygonContact*)contact)->~b2PolygonContact();
	allocator->Free(contact, sizeof(b2PolygonContact));
}

b2PolygonContact::b2PolygonContact(b2Shape* s1, b2Shape* s2)
	: b2Contact(s1, s2)
{
	b2Assert(m_shape1->GetType() == e_polygonShape);
	b2Assert(m_shape2->GetType() == e_polygonShape);
	m_manifold.pointCount = 0;
}

void b2PolygonContact::Evaluate(b2ContactListener* listener)
{
	b2Body* b1 = m_shape1->GetBody();
	b2Body* b2 = m_shape2->GetBody();

	b2Manifold m0;
	memcpy(&m0, &m_manifold, sizeof(b2Manifold));

	b2CollidePolygons(&m_manifold, (b2PolygonShape*)m_shape1, b1->GetXForm(), (b2PolygonShape*)m_shape2, b2->GetXForm());

	bool persisted[b2_maxManifoldPoints] = {false, false};

	b2ContactPoint cp;
	cp.shape1 = m_shape1;
	cp.shape2 = m_shape2;
	cp.friction = m_friction;
	cp.restitution = m_restitution;

	// Match contact ids to facilitate warm starting.
	if (m_manifold.pointCount > 0)
	{
		// Match old contact ids to new contact ids and copy the
		// stored impulses to warm start the solver.
		for (int32 i = 0; i < m_manifold.pointCount; ++i)
		{
			b2ManifoldPoint* mp = m_manifold.points + i;
			mp->normalImpulse = 0.0f;
			mp->tangentImpulse = 0.0f;
			bool found = false;
			b2ContactID id = mp->id;

			for (int32 j = 0; j < m0.pointCount; ++j)
			{
				if (persisted[j] == true)
				{
					continue;
				}

				b2ManifoldPoint* mp0 = m0.points + j;

				if (mp0->id.key == id.key)
				{
					persisted[j] = true;
					mp->normalImpulse = mp0->normalImpulse;
					mp->tangentImpulse = mp0->tangentImpulse;

					// A persistent point.
					found = true;

					// Report persistent point.
					if (listener != NULL)
					{
						cp.position = b1->GetWorldPoint(mp->localPoint1);
						b2Vec2 v1 = b1->GetLinearVelocityFromLocalPoint(mp->localPoint1);
						b2Vec2 v2 = b2->GetLinearVelocityFromLocalPoint(mp->localPoint2);
						cp.velocity = v2 - v1;
						cp.normal = m_manifold.normal;
						cp.separation = mp->separation;
						cp.id = id;
						listener->Persist(&cp);
					}
					break;
				}
			}

			// Report added point.
			if (found == false && listener != NULL)
			{
				cp.position = b1->GetWorldPoint(mp->localPoint1);
				b2Vec2 v1 = b1->GetLinearVelocityFromLocalPoint(mp->localPoint1);
				b2Vec2 v2 = b2->GetLinearVelocityFromLocalPoint(mp->localPoint2);
				cp.velocity = v2 - v1;
				cp.normal = m_manifold.normal;
				cp.separation = mp->separation;
				cp.id = id;
				listener->Add(&cp);
			}
		}

		m_manifoldCount = 1;
	}
	else
	{
		m_manifoldCount = 0;
	}

	if (listener == NULL)
	{
		return;
	}

	// Report removed points.
	for (int32 i = 0; i < m0.pointCount; ++i)
	{
		if (persisted[i])
		{
			continue;
		}

		b2ManifoldPoint* mp0 = m0.points + i;
		cp.position = b1->GetWorldPoint(mp0->localPoint1);
		b2Vec2 v1 = b1->GetLinearVelocityFromLocalPoint(mp0->localPoint1);
		b2Vec2 v2 = b2->GetLinearVelocityFromLocalPoint(mp0->localPoint2);
		cp.velocity = v2 - v1;
		cp.normal = m0.normal;
		cp.separation = mp0->separation;
		cp.id = mp0->id;
		listener->Remove(&cp);
	}
}
