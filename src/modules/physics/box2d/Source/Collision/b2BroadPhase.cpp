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

#include "b2BroadPhase.h"
#include <algorithm>
#include <string.h>

// Notes:
// - we use bound arrays instead of linked lists for cache coherence.
// - we use quantized integral values for fast compares.
// - we use short indices rather than pointers to save memory.
// - we use a stabbing count for fast overlap queries (less than order N).
// - we also use a time stamp on each proxy to speed up the registration of
//   overlap query results.
// - where possible, we compare bound indices instead of values to reduce
//   cache misses (TODO_ERIN).
// - no broadphase is perfect and neither is this one: it is not great for huge
//   worlds (use a multi-SAP instead), it is not great for large objects.

bool b2BroadPhase::s_validate = false;

struct b2BoundValues
{
	uint16 lowerValues[2];
	uint16 upperValues[2];
};

static int32 BinarySearch(b2Bound* bounds, int32 count, uint16 value)
{
	int32 low = 0;
	int32 high = count - 1;
	while (low <= high)
	{
		int32 mid = (low + high) >> 1;
		if (bounds[mid].value > value)
		{
			high = mid - 1;
		}
		else if (bounds[mid].value < value)
		{
			low = mid + 1;
		}
		else
		{
			return (uint16)mid;
		}
	}
	
	return low;
}

b2BroadPhase::b2BroadPhase(const b2AABB& worldAABB, b2PairCallback* callback)
{
	m_pairManager.Initialize(this, callback);

	b2Assert(worldAABB.IsValid());
	m_worldAABB = worldAABB;
	m_proxyCount = 0;

	b2Vec2 d = worldAABB.upperBound - worldAABB.lowerBound;
	m_quantizationFactor.x = float32(B2BROADPHASE_MAX) / d.x;
	m_quantizationFactor.y = float32(B2BROADPHASE_MAX) / d.y;

	for (uint16 i = 0; i < b2_maxProxies - 1; ++i)
	{
		m_proxyPool[i].SetNext(i + 1);
		m_proxyPool[i].timeStamp = 0;
		m_proxyPool[i].overlapCount = b2_invalid;
		m_proxyPool[i].userData = NULL;
	}
	m_proxyPool[b2_maxProxies-1].SetNext(b2_nullProxy);
	m_proxyPool[b2_maxProxies-1].timeStamp = 0;
	m_proxyPool[b2_maxProxies-1].overlapCount = b2_invalid;
	m_proxyPool[b2_maxProxies-1].userData = NULL;
	m_freeProxy = 0;

	m_timeStamp = 1;
	m_queryResultCount = 0;
}

b2BroadPhase::~b2BroadPhase()
{
}

// This one is only used for validation.
bool b2BroadPhase::TestOverlap(b2Proxy* p1, b2Proxy* p2)
{
	for (int32 axis = 0; axis < 2; ++axis)
	{
		b2Bound* bounds = m_bounds[axis];

		b2Assert(p1->lowerBounds[axis] < 2 * m_proxyCount);
		b2Assert(p1->upperBounds[axis] < 2 * m_proxyCount);
		b2Assert(p2->lowerBounds[axis] < 2 * m_proxyCount);
		b2Assert(p2->upperBounds[axis] < 2 * m_proxyCount);

		if (bounds[p1->lowerBounds[axis]].value > bounds[p2->upperBounds[axis]].value)
			return false;

		if (bounds[p1->upperBounds[axis]].value < bounds[p2->lowerBounds[axis]].value)
			return false;
	}

	return true;
}

bool b2BroadPhase::TestOverlap(const b2BoundValues& b, b2Proxy* p)
{
	for (int32 axis = 0; axis < 2; ++axis)
	{
		b2Bound* bounds = m_bounds[axis];

		b2Assert(p->lowerBounds[axis] < 2 * m_proxyCount);
		b2Assert(p->upperBounds[axis] < 2 * m_proxyCount);

		if (b.lowerValues[axis] > bounds[p->upperBounds[axis]].value)
			return false;

		if (b.upperValues[axis] < bounds[p->lowerBounds[axis]].value)
			return false;
	}

	return true;
}

void b2BroadPhase::ComputeBounds(uint16* lowerValues, uint16* upperValues, const b2AABB& aabb)
{
	b2Assert(aabb.upperBound.x > aabb.lowerBound.x);
	b2Assert(aabb.upperBound.y > aabb.lowerBound.y);

	b2Vec2 minVertex = b2Clamp(aabb.lowerBound, m_worldAABB.lowerBound, m_worldAABB.upperBound);
	b2Vec2 maxVertex = b2Clamp(aabb.upperBound, m_worldAABB.lowerBound, m_worldAABB.upperBound);

	// Bump lower bounds downs and upper bounds up. This ensures correct sorting of
	// lower/upper bounds that would have equal values.
	// TODO_ERIN implement fast float to uint16 conversion.
	lowerValues[0] = (uint16)(m_quantizationFactor.x * (minVertex.x - m_worldAABB.lowerBound.x)) & (B2BROADPHASE_MAX - 1);
	upperValues[0] = (uint16)(m_quantizationFactor.x * (maxVertex.x - m_worldAABB.lowerBound.x)) | 1;

	lowerValues[1] = (uint16)(m_quantizationFactor.y * (minVertex.y - m_worldAABB.lowerBound.y)) & (B2BROADPHASE_MAX - 1);
	upperValues[1] = (uint16)(m_quantizationFactor.y * (maxVertex.y - m_worldAABB.lowerBound.y)) | 1;
}

void b2BroadPhase::IncrementTimeStamp()
{
	if (m_timeStamp == B2BROADPHASE_MAX)
	{
		for (uint16 i = 0; i < b2_maxProxies; ++i)
		{
			m_proxyPool[i].timeStamp = 0;
		}
		m_timeStamp = 1;
	}
	else
	{
		++m_timeStamp;
	}
}

void b2BroadPhase::IncrementOverlapCount(int32 proxyId)
{
	b2Proxy* proxy = m_proxyPool + proxyId;
	if (proxy->timeStamp < m_timeStamp)
	{
		proxy->timeStamp = m_timeStamp;
		proxy->overlapCount = 1;
	}
	else
	{
		proxy->overlapCount = 2;
		b2Assert(m_queryResultCount < b2_maxProxies);
		m_queryResults[m_queryResultCount] = (uint16)proxyId;
		++m_queryResultCount;
	}
}

void b2BroadPhase::Query(int32* lowerQueryOut, int32* upperQueryOut,
					   uint16 lowerValue, uint16 upperValue,
					   b2Bound* bounds, int32 boundCount, int32 axis)
{
	int32 lowerQuery = BinarySearch(bounds, boundCount, lowerValue);
	int32 upperQuery = BinarySearch(bounds, boundCount, upperValue);

	// Easy case: lowerQuery <= lowerIndex(i) < upperQuery
	// Solution: search query range for min bounds.
	for (int32 i = lowerQuery; i < upperQuery; ++i)
	{
		if (bounds[i].IsLower())
		{
			IncrementOverlapCount(bounds[i].proxyId);
		}
	}

	// Hard case: lowerIndex(i) < lowerQuery < upperIndex(i)
	// Solution: use the stabbing count to search down the bound array.
	if (lowerQuery > 0)
	{
		int32 i = lowerQuery - 1;
		int32 s = bounds[i].stabbingCount;

		// Find the s overlaps.
		while (s)
		{
			b2Assert(i >= 0);

			if (bounds[i].IsLower())
			{
				b2Proxy* proxy = m_proxyPool + bounds[i].proxyId;
				if (lowerQuery <= proxy->upperBounds[axis])
				{
					IncrementOverlapCount(bounds[i].proxyId);
					--s;
				}
			}
			--i;
		}
	}

	*lowerQueryOut = lowerQuery;
	*upperQueryOut = upperQuery;
}

uint16 b2BroadPhase::CreateProxy(const b2AABB& aabb, void* userData)
{
	b2Assert(m_proxyCount < b2_maxProxies);
	b2Assert(m_freeProxy != b2_nullProxy);

	uint16 proxyId = m_freeProxy;
	b2Proxy* proxy = m_proxyPool + proxyId;
	m_freeProxy = proxy->GetNext();

	proxy->overlapCount = 0;
	proxy->userData = userData;

	int32 boundCount = 2 * m_proxyCount;

	uint16 lowerValues[2], upperValues[2];
	ComputeBounds(lowerValues, upperValues, aabb);

	for (int32 axis = 0; axis < 2; ++axis)
	{
		b2Bound* bounds = m_bounds[axis];
		int32 lowerIndex, upperIndex;
		Query(&lowerIndex, &upperIndex, lowerValues[axis], upperValues[axis], bounds, boundCount, axis);

		memmove(bounds + upperIndex + 2, bounds + upperIndex, (boundCount - upperIndex) * sizeof(b2Bound));
		memmove(bounds + lowerIndex + 1, bounds + lowerIndex, (upperIndex - lowerIndex) * sizeof(b2Bound));

		// The upper index has increased because of the lower bound insertion.
		++upperIndex;

		// Copy in the new bounds.
		bounds[lowerIndex].value = lowerValues[axis];
		bounds[lowerIndex].proxyId = proxyId;
		bounds[upperIndex].value = upperValues[axis];
		bounds[upperIndex].proxyId = proxyId;

		bounds[lowerIndex].stabbingCount = lowerIndex == 0 ? 0 : bounds[lowerIndex-1].stabbingCount;
		bounds[upperIndex].stabbingCount = bounds[upperIndex-1].stabbingCount;

		// Adjust the stabbing count between the new bounds.
		for (int32 index = lowerIndex; index < upperIndex; ++index)
		{
			++bounds[index].stabbingCount;
		}

		// Adjust the all the affected bound indices.
		for (int32 index = lowerIndex; index < boundCount + 2; ++index)
		{
			b2Proxy* proxy = m_proxyPool + bounds[index].proxyId;
			if (bounds[index].IsLower())
			{
				proxy->lowerBounds[axis] = (uint16)index;
			}
			else
			{
				proxy->upperBounds[axis] = (uint16)index;
			}
		}
	}

	++m_proxyCount;

	b2Assert(m_queryResultCount < b2_maxProxies);

	// Create pairs if the AABB is in range.
	for (int32 i = 0; i < m_queryResultCount; ++i)
	{
		b2Assert(m_queryResults[i] < b2_maxProxies);
		b2Assert(m_proxyPool[m_queryResults[i]].IsValid());

		m_pairManager.AddBufferedPair(proxyId, m_queryResults[i]);
	}

	m_pairManager.Commit();

	if (s_validate)
	{
		Validate();
	}

	// Prepare for next query.
	m_queryResultCount = 0;
	IncrementTimeStamp();

	return proxyId;
}

void b2BroadPhase::DestroyProxy(int32 proxyId)
{
	b2Assert(0 < m_proxyCount && m_proxyCount <= b2_maxProxies);
	b2Proxy* proxy = m_proxyPool + proxyId;
	b2Assert(proxy->IsValid());

	int32 boundCount = 2 * m_proxyCount;

	for (int32 axis = 0; axis < 2; ++axis)
	{
		b2Bound* bounds = m_bounds[axis];

		int32 lowerIndex = proxy->lowerBounds[axis];
		int32 upperIndex = proxy->upperBounds[axis];
		uint16 lowerValue = bounds[lowerIndex].value;
		uint16 upperValue = bounds[upperIndex].value;

		memmove(bounds + lowerIndex, bounds + lowerIndex + 1, (upperIndex - lowerIndex - 1) * sizeof(b2Bound));
		memmove(bounds + upperIndex-1, bounds + upperIndex + 1, (boundCount - upperIndex - 1) * sizeof(b2Bound));

		// Fix bound indices.
		for (int32 index = lowerIndex; index < boundCount - 2; ++index)
		{
			b2Proxy* proxy = m_proxyPool + bounds[index].proxyId;
			if (bounds[index].IsLower())
			{
				proxy->lowerBounds[axis] = (uint16)index;
			}
			else
			{
				proxy->upperBounds[axis] = (uint16)index;
			}
		}

		// Fix stabbing count.
		for (int32 index = lowerIndex; index < upperIndex - 1; ++index)
		{
			--bounds[index].stabbingCount;
		}

		// Query for pairs to be removed. lowerIndex and upperIndex are not needed.
		Query(&lowerIndex, &upperIndex, lowerValue, upperValue, bounds, boundCount - 2, axis);
	}

	b2Assert(m_queryResultCount < b2_maxProxies);

	for (int32 i = 0; i < m_queryResultCount; ++i)
	{
		b2Assert(m_proxyPool[m_queryResults[i]].IsValid());
		m_pairManager.RemoveBufferedPair(proxyId, m_queryResults[i]);
	}

	m_pairManager.Commit();

	// Prepare for next query.
	m_queryResultCount = 0;
	IncrementTimeStamp();

	// Return the proxy to the pool.
	proxy->userData = NULL;
	proxy->overlapCount = b2_invalid;
	proxy->lowerBounds[0] = b2_invalid;
	proxy->lowerBounds[1] = b2_invalid;
	proxy->upperBounds[0] = b2_invalid;
	proxy->upperBounds[1] = b2_invalid;

	proxy->SetNext(m_freeProxy);
	m_freeProxy = (uint16)proxyId;
	--m_proxyCount;

	if (s_validate)
	{
		Validate();
	}
}

void b2BroadPhase::MoveProxy(int32 proxyId, const b2AABB& aabb)
{
	if (proxyId == b2_nullProxy || b2_maxProxies <= proxyId)
	{
		b2Assert(false);
		return;
	}

	if (aabb.IsValid() == false)
	{
		b2Assert(false);
		return;
	}

	int32 boundCount = 2 * m_proxyCount;

	b2Proxy* proxy = m_proxyPool + proxyId;

	// Get new bound values
	b2BoundValues newValues;
	ComputeBounds(newValues.lowerValues, newValues.upperValues, aabb);

	// Get old bound values
	b2BoundValues oldValues;
	for (int32 axis = 0; axis < 2; ++axis)
	{
		oldValues.lowerValues[axis] = m_bounds[axis][proxy->lowerBounds[axis]].value;
		oldValues.upperValues[axis] = m_bounds[axis][proxy->upperBounds[axis]].value;
	}

	for (int32 axis = 0; axis < 2; ++axis)
	{
		b2Bound* bounds = m_bounds[axis];

		int32 lowerIndex = proxy->lowerBounds[axis];
		int32 upperIndex = proxy->upperBounds[axis];

		uint16 lowerValue = newValues.lowerValues[axis];
		uint16 upperValue = newValues.upperValues[axis];

		int32 deltaLower = lowerValue - bounds[lowerIndex].value;
		int32 deltaUpper = upperValue - bounds[upperIndex].value;

		bounds[lowerIndex].value = lowerValue;
		bounds[upperIndex].value = upperValue;

		//
		// Expanding adds overlaps
		//

		// Should we move the lower bound down?
		if (deltaLower < 0)
		{
			int32 index = lowerIndex;
			while (index > 0 && lowerValue < bounds[index-1].value)
			{
				b2Bound* bound = bounds + index;
				b2Bound* prevBound = bound - 1;

				int32 prevProxyId = prevBound->proxyId;
				b2Proxy* prevProxy = m_proxyPool + prevBound->proxyId;

				++prevBound->stabbingCount;

				if (prevBound->IsUpper() == true)
				{
					if (TestOverlap(newValues, prevProxy))
					{
						m_pairManager.AddBufferedPair(proxyId, prevProxyId);
					}

					++prevProxy->upperBounds[axis];
					++bound->stabbingCount;
				}
				else
				{
					++prevProxy->lowerBounds[axis];
					--bound->stabbingCount;
				}

				--proxy->lowerBounds[axis];
				b2Swap(*bound, *prevBound);
				--index;
			}
		}

		// Should we move the upper bound up?
		if (deltaUpper > 0)
		{
			int32 index = upperIndex;
			while (index < boundCount-1 && bounds[index+1].value <= upperValue)
			{
				b2Bound* bound = bounds + index;
				b2Bound* nextBound = bound + 1;
				int32 nextProxyId = nextBound->proxyId;
				b2Proxy* nextProxy = m_proxyPool + nextProxyId;

				++nextBound->stabbingCount;

				if (nextBound->IsLower() == true)
				{
					if (TestOverlap(newValues, nextProxy))
					{
						m_pairManager.AddBufferedPair(proxyId, nextProxyId);
					}

					--nextProxy->lowerBounds[axis];
					++bound->stabbingCount;
				}
				else
				{
					--nextProxy->upperBounds[axis];
					--bound->stabbingCount;
				}

				++proxy->upperBounds[axis];
				b2Swap(*bound, *nextBound);
				++index;
			}
		}

		//
		// Shrinking removes overlaps
		//

		// Should we move the lower bound up?
		if (deltaLower > 0)
		{
			int32 index = lowerIndex;
			while (index < boundCount-1 && bounds[index+1].value <= lowerValue)
			{
				b2Bound* bound = bounds + index;
				b2Bound* nextBound = bound + 1;

				int32 nextProxyId = nextBound->proxyId;
				b2Proxy* nextProxy = m_proxyPool + nextProxyId;

				--nextBound->stabbingCount;

				if (nextBound->IsUpper())
				{
					if (TestOverlap(oldValues, nextProxy))
					{
						m_pairManager.RemoveBufferedPair(proxyId, nextProxyId);
					}

					--nextProxy->upperBounds[axis];
					--bound->stabbingCount;
				}
				else
				{
					--nextProxy->lowerBounds[axis];
					++bound->stabbingCount;
				}

				++proxy->lowerBounds[axis];
				b2Swap(*bound, *nextBound);
				++index;
			}
		}

		// Should we move the upper bound down?
		if (deltaUpper < 0)
		{
			int32 index = upperIndex;
			while (index > 0 && upperValue < bounds[index-1].value)
			{
				b2Bound* bound = bounds + index;
				b2Bound* prevBound = bound - 1;

				int32 prevProxyId = prevBound->proxyId;
				b2Proxy* prevProxy = m_proxyPool + prevProxyId;

				--prevBound->stabbingCount;

				if (prevBound->IsLower() == true)
				{
					if (TestOverlap(oldValues, prevProxy))
					{
						m_pairManager.RemoveBufferedPair(proxyId, prevProxyId);
					}

					++prevProxy->lowerBounds[axis];
					--bound->stabbingCount;
				}
				else
				{
					++prevProxy->upperBounds[axis];
					++bound->stabbingCount;
				}

				--proxy->upperBounds[axis];
				b2Swap(*bound, *prevBound);
				--index;
			}
		}
	}

	if (s_validate)
	{
		Validate();
	}
}

void b2BroadPhase::Commit()
{
	m_pairManager.Commit();
}

int32 b2BroadPhase::Query(const b2AABB& aabb, void** userData, int32 maxCount)
{
	uint16 lowerValues[2];
	uint16 upperValues[2];
	ComputeBounds(lowerValues, upperValues, aabb);

	int32 lowerIndex, upperIndex;

	Query(&lowerIndex, &upperIndex, lowerValues[0], upperValues[0], m_bounds[0], 2*m_proxyCount, 0);
	Query(&lowerIndex, &upperIndex, lowerValues[1], upperValues[1], m_bounds[1], 2*m_proxyCount, 1);

	b2Assert(m_queryResultCount < b2_maxProxies);

	int32 count = 0;
	for (int32 i = 0; i < m_queryResultCount && count < maxCount; ++i, ++count)
	{
		b2Assert(m_queryResults[i] < b2_maxProxies);
		b2Proxy* proxy = m_proxyPool + m_queryResults[i];
		b2Assert(proxy->IsValid());
		userData[i] = proxy->userData;
	}

	// Prepare for next query.
	m_queryResultCount = 0;
	IncrementTimeStamp();

	return count;
}

void b2BroadPhase::Validate()
{
	for (int32 axis = 0; axis < 2; ++axis)
	{
		b2Bound* bounds = m_bounds[axis];

		int32 boundCount = 2 * m_proxyCount;
		uint16 stabbingCount = 0;

		for (int32 i = 0; i < boundCount; ++i)
		{
			b2Bound* bound = bounds + i;
			b2Assert(i == 0 || bounds[i-1].value <= bound->value);
			b2Assert(bound->proxyId != b2_nullProxy);
			b2Assert(m_proxyPool[bound->proxyId].IsValid());

			if (bound->IsLower() == true)
			{
				b2Assert(m_proxyPool[bound->proxyId].lowerBounds[axis] == i);
				++stabbingCount;
			}
			else
			{
				b2Assert(m_proxyPool[bound->proxyId].upperBounds[axis] == i);
				--stabbingCount;
			}

			b2Assert(bound->stabbingCount == stabbingCount);
		}
	}
}
