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

#include "b2PairManager.h"
#include "b2BroadPhase.h"

#include <algorithm>

// Thomas Wang's hash, see: http://www.concentric.net/~Ttwang/tech/inthash.htm
// This assumes proxyId1 and proxyId2 are 16-bit.
inline uint32 Hash(uint32 proxyId1, uint32 proxyId2)
{
	uint32 key = (proxyId2 << 16) | proxyId1;
	key = ~key + (key << 15);
	key = key ^ (key >> 12);
	key = key + (key << 2);
	key = key ^ (key >> 4);
	key = key * 2057;
	key = key ^ (key >> 16);
	return key;
}

inline bool Equals(const b2Pair& pair, int32 proxyId1, int32 proxyId2)
{
	return pair.proxyId1 == proxyId1 && pair.proxyId2 == proxyId2;
}

inline bool Equals(const b2BufferedPair& pair1, const b2BufferedPair& pair2)
{
	return pair1.proxyId1 == pair2.proxyId1 && pair1.proxyId2 == pair2.proxyId2;
}

// For sorting.
inline bool operator < (const b2BufferedPair& pair1, const b2BufferedPair& pair2)
{
	if (pair1.proxyId1 < pair2.proxyId1)
	{
		return true;
	}

	if (pair1.proxyId1 == pair2.proxyId1)
	{
		return pair1.proxyId2 < pair2.proxyId2;
	}

	return false;
}


b2PairManager::b2PairManager()
{
	b2Assert(b2IsPowerOfTwo(b2_tableCapacity) == true);
	b2Assert(b2_tableCapacity >= b2_maxPairs);
	for (int32 i = 0; i < b2_tableCapacity; ++i)
	{
		m_hashTable[i] = b2_nullPair;
	}
	m_freePair = 0;
	for (int32 i = 0; i < b2_maxPairs; ++i)
	{
		m_pairs[i].proxyId1 = b2_nullProxy;
		m_pairs[i].proxyId2 = b2_nullProxy;
		m_pairs[i].userData = NULL;
		m_pairs[i].status = 0;
		m_pairs[i].next = uint16(i + 1);
	}
	m_pairs[b2_maxPairs-1].next = b2_nullPair;
	m_pairCount = 0;
	m_pairBufferCount = 0;
}

void b2PairManager::Initialize(b2BroadPhase* broadPhase, b2PairCallback* callback)
{
	m_broadPhase = broadPhase;
	m_callback = callback;
}

b2Pair* b2PairManager::Find(int32 proxyId1, int32 proxyId2, uint32 hash)
{
	int32 index = m_hashTable[hash];

	while (index != b2_nullPair && Equals(m_pairs[index], proxyId1, proxyId2) == false)
	{
		index = m_pairs[index].next;
	}

	if (index == b2_nullPair)
	{
		return NULL;
	}

	b2Assert(index < b2_maxPairs);

	return m_pairs + index;
}

b2Pair* b2PairManager::Find(int32 proxyId1, int32 proxyId2)
{
	if (proxyId1 > proxyId2) b2Swap(proxyId1, proxyId2);

	int32 hash = Hash(proxyId1, proxyId2) & b2_tableMask;

	return Find(proxyId1, proxyId2, hash);
}

// Returns existing pair or creates a new one.
b2Pair* b2PairManager::AddPair(int32 proxyId1, int32 proxyId2)
{
	if (proxyId1 > proxyId2) b2Swap(proxyId1, proxyId2);

	int32 hash = Hash(proxyId1, proxyId2) & b2_tableMask;

	b2Pair* pair = Find(proxyId1, proxyId2, hash);
	if (pair != NULL)
	{
		return pair;
	}

	b2Assert(m_pairCount < b2_maxPairs && m_freePair != b2_nullPair);

	uint16 pairIndex = m_freePair;
	pair = m_pairs + pairIndex;
	m_freePair = pair->next;

	pair->proxyId1 = (uint16)proxyId1;
	pair->proxyId2 = (uint16)proxyId2;
	pair->status = 0;
	pair->userData = NULL;
	pair->next = m_hashTable[hash];

	m_hashTable[hash] = pairIndex;

	++m_pairCount;

	return pair;
}

// Removes a pair. The pair must exist.
void* b2PairManager::RemovePair(int32 proxyId1, int32 proxyId2)
{
	b2Assert(m_pairCount > 0);

	if (proxyId1 > proxyId2) b2Swap(proxyId1, proxyId2);

	int32 hash = Hash(proxyId1, proxyId2) & b2_tableMask;

	uint16* node = &m_hashTable[hash];
	while (*node != b2_nullPair)
	{
		if (Equals(m_pairs[*node], proxyId1, proxyId2))
		{
			uint16 index = *node;
			*node = m_pairs[*node].next;
			
			b2Pair* pair = m_pairs + index;
			void* userData = pair->userData;

			// Scrub
			pair->next = m_freePair;
			pair->proxyId1 = b2_nullProxy;
			pair->proxyId2 = b2_nullProxy;
			pair->userData = NULL;
			pair->status = 0;

			m_freePair = index;
			--m_pairCount;
			return userData;
		}
		else
		{
			node = &m_pairs[*node].next;
		}
	}

	b2Assert(false);
	return NULL;
}

/*
As proxies are created and moved, many pairs are created and destroyed. Even worse, the same
pair may be added and removed multiple times in a single time step of the physics engine. To reduce
traffic in the pair manager, we try to avoid destroying pairs in the pair manager until the
end of the physics step. This is done by buffering all the RemovePair requests. AddPair
requests are processed immediately because we need the hash table entry for quick lookup.

All user user callbacks are delayed until the buffered pairs are confirmed in Commit.
This is very important because the user callbacks may be very expensive and client logic
may be harmed if pairs are added and removed within the same time step.

Buffer a pair for addition.
We may add a pair that is not in the pair manager or pair buffer.
We may add a pair that is already in the pair manager and pair buffer.
If the added pair is not a new pair, then it must be in the pair buffer (because RemovePair was called).
*/
void b2PairManager::AddBufferedPair(int32 id1, int32 id2)
{
	b2Assert(id1 != b2_nullProxy && id2 != b2_nullProxy);
	b2Assert(m_pairBufferCount < b2_maxPairs);

	b2Pair* pair = AddPair(id1, id2);

	// If this pair is not in the pair buffer ...
	if (pair->IsBuffered() == false)
	{
		// This must be a newly added pair.
		b2Assert(pair->IsFinal() == false);

		// Add it to the pair buffer.
		pair->SetBuffered();
		m_pairBuffer[m_pairBufferCount].proxyId1 = pair->proxyId1;
		m_pairBuffer[m_pairBufferCount].proxyId2 = pair->proxyId2;
		++m_pairBufferCount;

		b2Assert(m_pairBufferCount <= m_pairCount);
	}

	// Confirm this pair for the subsequent call to Commit.
	pair->ClearRemoved();

	if (b2BroadPhase::s_validate)
	{
		ValidateBuffer();
	}
}

// Buffer a pair for removal.
void b2PairManager::RemoveBufferedPair(int32 id1, int32 id2)
{
	b2Assert(id1 != b2_nullProxy && id2 != b2_nullProxy);
	b2Assert(m_pairBufferCount < b2_maxPairs);

	b2Pair* pair = Find(id1, id2);

	if (pair == NULL)
	{
		// The pair never existed. This is legal (due to collision filtering).
		return;
	}

	// If this pair is not in the pair buffer ...
	if (pair->IsBuffered() == false)
	{
		// This must be an old pair.
		b2Assert(pair->IsFinal() == true);

		pair->SetBuffered();
		m_pairBuffer[m_pairBufferCount].proxyId1 = pair->proxyId1;
		m_pairBuffer[m_pairBufferCount].proxyId2 = pair->proxyId2;
		++m_pairBufferCount;

		b2Assert(m_pairBufferCount <= m_pairCount);
	}

	pair->SetRemoved();

	if (b2BroadPhase::s_validate)
	{
		ValidateBuffer();
	}
}

void b2PairManager::Commit()
{
	int32 removeCount = 0;

	b2Proxy* proxies = m_broadPhase->m_proxyPool;

	for (int32 i = 0; i < m_pairBufferCount; ++i)
	{
		b2Pair* pair = Find(m_pairBuffer[i].proxyId1, m_pairBuffer[i].proxyId2);
		b2Assert(pair->IsBuffered());
		pair->ClearBuffered();

		b2Assert(pair->proxyId1 < b2_maxProxies && pair->proxyId2 < b2_maxProxies);

		b2Proxy* proxy1 = proxies + pair->proxyId1;
		b2Proxy* proxy2 = proxies + pair->proxyId2;

		b2Assert(proxy1->IsValid());
		b2Assert(proxy2->IsValid());

		if (pair->IsRemoved())
		{
			// It is possible a pair was added then removed before a commit. Therefore,
			// we should be careful not to tell the user the pair was removed when the
			// the user didn't receive a matching add.
			if (pair->IsFinal() == true)
			{
				m_callback->PairRemoved(proxy1->userData, proxy2->userData, pair->userData);
			}

			// Store the ids so we can actually remove the pair below.
			m_pairBuffer[removeCount].proxyId1 = pair->proxyId1;
			m_pairBuffer[removeCount].proxyId2 = pair->proxyId2;
			++removeCount;
		}
		else
		{
			b2Assert(m_broadPhase->TestOverlap(proxy1, proxy2) == true);

			if (pair->IsFinal() == false)
			{
				pair->userData = m_callback->PairAdded(proxy1->userData, proxy2->userData);
				pair->SetFinal();
			}
		}
	}

	for (int32 i = 0; i < removeCount; ++i)
	{
		RemovePair(m_pairBuffer[i].proxyId1, m_pairBuffer[i].proxyId2);
	}

	m_pairBufferCount = 0;

	if (b2BroadPhase::s_validate)
	{
		ValidateTable();
	}
}

void b2PairManager::ValidateBuffer()
{
#ifdef _DEBUG
	b2Assert(m_pairBufferCount <= m_pairCount);

	std::sort(m_pairBuffer, m_pairBuffer + m_pairBufferCount);

	for (int32 i = 0; i < m_pairBufferCount; ++i)
	{
		if (i > 0)
		{
			b2Assert(Equals(m_pairBuffer[i], m_pairBuffer[i-1]) == false);
		}

		b2Pair* pair = Find(m_pairBuffer[i].proxyId1, m_pairBuffer[i].proxyId2);
		b2Assert(pair->IsBuffered());

		b2Assert(pair->proxyId1 != pair->proxyId2);
		b2Assert(pair->proxyId1 < b2_maxProxies);
		b2Assert(pair->proxyId2 < b2_maxProxies);

		b2Proxy* proxy1 = m_broadPhase->m_proxyPool + pair->proxyId1;
		b2Proxy* proxy2 = m_broadPhase->m_proxyPool + pair->proxyId2;

		b2Assert(proxy1->IsValid() == true);
		b2Assert(proxy2->IsValid() == true);
	}
#endif
}

void b2PairManager::ValidateTable()
{
#ifdef _DEBUG
	for (int32 i = 0; i < b2_tableCapacity; ++i)
	{
		uint16 index = m_hashTable[i];
		while (index != b2_nullPair)
		{
			b2Pair* pair = m_pairs + index;
			b2Assert(pair->IsBuffered() == false);
			b2Assert(pair->IsFinal() == true);
			b2Assert(pair->IsRemoved() == false);

			b2Assert(pair->proxyId1 != pair->proxyId2);
			b2Assert(pair->proxyId1 < b2_maxProxies);
			b2Assert(pair->proxyId2 < b2_maxProxies);

			b2Proxy* proxy1 = m_broadPhase->m_proxyPool + pair->proxyId1;
			b2Proxy* proxy2 = m_broadPhase->m_proxyPool + pair->proxyId2;

			b2Assert(proxy1->IsValid() == true);
			b2Assert(proxy2->IsValid() == true);

			b2Assert(m_broadPhase->TestOverlap(proxy1, proxy2) == true);

			index = pair->next;
		}
	}
#endif
}
